/*
 *  Albion Remake
 *  Copyright (C) 2007 - 2014 Florian Ziesche
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "tileset.hpp"
#include <rendering/texman.hpp>

/*! tileset constructor
 */
tileset::tileset(const pal* palettes_) : palettes(palettes_), cur_tileset_num(0), cur_tileset(nullptr) {
	// load tilesets
	icongfx = new xld("ICONGFX0.XLD");

	const size_t tileset_count = icongfx->get_object_count();
	tilesets.reserve(tileset_count);
	for(size_t i = 0; i < tileset_count; i++) {
		tilesets.push_back(new tileset_object());
		const xld::xld_object* object = icongfx->get_object(i);
		tilesets.back()->tile_count = object->length/256;
		tilesets.back()->tile_data = object->data;
	}

	// index tilesets
	xld* icondata = new xld("ICONDAT0.XLD");
	
	// TODO: collision/special possibly swapped?

	/* icon data spec:
	 * 
	 * [8 byte chunk]
	 * [byte 1: higher 4 bit: overlay/underlay type, lower 4 bit: unknown]
	 * [byte 2: collision byte (0 = passable; 8 = not passable)
	 * [byte 3 + 4: special data
	 * 				0x0000 = normal tile
	 * 				0x20?? = debug tile/discard tile
	 * 				0x0402 = SL (southern left), chair left side
	 * 				0x8402 = SR (southern right), chair right side
	 * 				0x8000 = NL (northern left), chair left side
	 * 				0x0001 = NR (northern right), chair right side
	 * 				0x0403 = W (western), to the left, chair left side
	 * 				0x---- = E (eastern), to the right
	 * 				0x---- = Z ?
	 * 				0x0400 = none (wall sit?)
	 * 				0x2000 = black on white, override collision (-> no collision)
	 * [byte 5 + 6: tile number]
	 * [byte 7: animation tile count]
	 * [byte 8: ?]
	 *
	 * 0x8000 = NL
	 * 0x???1 = NR
	 * 0x*??2 = S (*=0: L, *=8: R)
	 * 0x???3 = W
	 * 0xC??1 = E
	 * 
	 */

	size_t tile_count = 0;
	const float2 empty_tc = float2(1.0f, 1.0f);
	for(size_t i = 0; i < icondata->get_object_count(); i++) {
		const xld::xld_object* object = icondata->get_object(i);
		tile_count = object->length/8;
		tilesets[i]->tiles = new tile_object[tile_count+2];
		tilesets[i]->tile_obj_count = tile_count+2;

		// first two tiles are "empty"
		for(size_t j = 0; j < 2; j++) {
			tilesets[i]->tiles[j].num = 0xFFF;
			tilesets[i]->tiles[j].ani_num = 0xFFF;
			tilesets[i]->tiles[j].ani_tiles = 1;
			tilesets[i]->tiles[j].tex_coord = empty_tc;
			tilesets[i]->tiles[j].upper_bytes = 0;
			tilesets[i]->tiles[j].lower_bytes = 0;
			tilesets[i]->tiles[j].layer_type = TILE_LAYER::UNDERLAY;
			tilesets[i]->tiles[j].collision = 0;
		}

		size_t index = 0;
		for(size_t j = 0; j < tile_count; j++) {
			index = j+2;
			if(object->data[j*8+2] == 0x20 || object->data[j*8+6] == 0x00) {
				tilesets[i]->tiles[index].num = 0xFFF;
				tilesets[i]->tiles[index].ani_num = 0xFFF;
				tilesets[i]->tiles[index].ani_tiles = 1;
				tilesets[i]->tiles[index].tex_coord = empty_tc;
				tilesets[i]->tiles[index].layer_type = TILE_LAYER::UNDERLAY;
			}
			else {
				tilesets[i]->tiles[index].num = object->data[j*8+4] + (object->data[j*8+5] << 8);
				tilesets[i]->tiles[index].ani_num = tilesets[i]->tiles[index].num;
				tilesets[i]->tiles[index].ani_tiles = object->data[j*8+6];
				tilesets[i]->tiles[index].layer_type = get_layer_type((object->data[j*8] >> 4) & 0x0F);
			}
			
			tilesets[i]->tiles[index].special_1 = object->data[j*8];
			tilesets[i]->tiles[index].special_2 = object->data[j*8+2] + (object->data[j*8+3] << 8);
			
			tilesets[i]->tiles[index].collision = (size_t)object->data[j*8+1];
			
			tilesets[i]->tiles[index].upper_bytes = (unsigned int)((((size_t)object->data[j*8]) << 24) + (((size_t)object->data[j*8+1]) << 16) + (((size_t)object->data[j*8+2]) << 8) + (size_t)object->data[j*8+3]);
			tilesets[i]->tiles[index].lower_bytes = (object->data[j*8+4] << 24) + (object->data[j*8+5] << 16) + (object->data[j*8+6] << 8) + object->data[j*8+7];
		}
	}

	delete icondata;
}

/*! tileset destructor
 */
tileset::~tileset() {
	delete icongfx;
}

void tileset::load(const size_t& num, const size_t& palette) {
	if(cur_tileset_num == num && tilesets[cur_tileset_num]->loaded) return;
	log_debug("loading tileset %u ...", num);
	if(cur_tileset_num != num && tilesets[cur_tileset_num]->loaded) {
		// unload old tileset
		t->delete_texture(tilesets[cur_tileset_num]->tileset);
		tilesets[cur_tileset_num]->loaded = false;
	}
	
	cur_tileset = &get_tileset(num);
	cur_tileset_num = num;

	////////////////////
	// blklist test
	// maybe these are connected to the event block number? possibly for easier tile access/changing
	/*cout << "tile count: " << tilesets[cur_tileset_num]->tile_count << endl;
	cout << "tile obj count: " << tilesets[cur_tileset_num]->tile_obj_count << endl;
	xld* blklist = new xld("BLKLIST0.XLD");
	const xld::xld_object* blk_obj = blklist->get_object(cur_tileset_num);
	size_t offset = 0;
	size_t mappings = 0;
	while(offset < blk_obj->length) {
		size_t bx = blk_obj->data[offset];
		size_t by = blk_obj->data[offset+1];
		offset += 2;
		cout << "##########" << endl;
		cout << ": " << bx << " * " << by << endl;
		if(bx == 0 && by == 0) break;
		if(offset >= blk_obj->length) {
			cout << "!! OFFSET FAIL1" << endl;
		}
		for(size_t y = 0; y < by; y++) {
			cout << ": " << y << ": ";
			for(size_t x = 0; x < bx; x++) {
				cout << hex << (size_t)blk_obj->data[offset] << "_" << (size_t)blk_obj->data[offset+1] << "_" << (size_t)blk_obj->data[offset+2] << dec;
				cout << (x < bx-1 ? ", " : "");
				offset+=3;
				if(offset >= blk_obj->length) {
					cout << "!! OFFSET FAIL2" << endl;
					x = bx;
					y = by;
					break;
				}
			}
			cout << endl;
		}
		if(offset >= blk_obj->length) {
			cout << "!! OFFSET FAIL*" << endl;
		}
		else mappings++;
	}
	cout << "##########" << endl;
	cout << ": mappings: " << mappings << endl;
	delete blklist;*/
	////////////////////
	
	tilesets[num]->tex_info_obj.object_count = (unsigned int)tilesets[num]->tile_count;
	tilesets[num]->tex_info_obj.object = icongfx->get_object(num);
	vector<albion_texture::albion_texture_info*> tex_info;
	tex_info.push_back(&tilesets[num]->tex_info_obj);
	
	// compute required texture size
	const size_t max_tex_size = exts->get_max_texture_size();
	const size_t tiles_per_row = max_tex_size / 64;
	size2 tileset_tex_size;
	const size2 tile_size = size2(16, 16);
	const vector<size2>& animated_ranges = palettes->get_animated_ranges(palette);
	size_t tile_count = 0;
	const xld::xld_object* gfx_obj = icongfx->get_object(num);
	for(size_t i = 0; i < tilesets[num]->tile_count; i++) {
		// check if tile contains animated colors
		size_t animations = 1;
		for(vector<size2>::const_iterator ani_range = animated_ranges.begin(); ani_range != animated_ranges.end(); ani_range++) {
			const size_t obj_size = tile_size.x*tile_size.y;
			const size_t p_offset = i*obj_size;
			for(size_t p = 0; p < obj_size; p++) {
				if(gfx_obj->data[p_offset + p] >= ani_range->x && gfx_obj->data[p_offset + p] <= ani_range->y) {
					animations = std::max(ani_range->y - ani_range->x + 1, animations);
					break;
				}
			}
		}
		tile_count += animations;
	}
	tileset_tex_size.set(tile_count >= tiles_per_row ? max_tex_size : (tile_count * 64),
						 ((tile_count/tiles_per_row)+1) * 64);
	tilesets[num]->tile_tc_size = float2(64.0f)/float2(tileset_tex_size);

	// create texture
	cur_tileset_tex = albion_texture::create(MAP_TYPE::MAP_2D, tileset_tex_size, size2(16, 16), palette, tex_info, nullptr, albion_texture::TST_NONE, 0, false, TEXTURE_FILTERING::POINT);
	//conf::set<a2e_texture>("debug.texture", cur_tileset_tex);
	tilesets[num]->tileset = cur_tileset_tex;
	tilesets[num]->tex_coords = &tilesets[num]->tex_info_obj.tex_coords;
	tilesets[num]->loaded = true;

	// set tile tex coords
	for(size_t i = 2; i < tilesets[num]->tile_obj_count; i++) {
		if(tilesets[num]->tiles[i].num != 0xFFF) {
			tilesets[num]->tiles[i].tex_coord = (*tilesets[num]->tex_coords)[tilesets[num]->tiles[i].num][0];
			if(tilesets[num]->tiles[i].ani_tiles == 1 && (*tilesets[num]->tex_coords)[tilesets[num]->tiles[i].num].size() > 1) {
				tilesets[num]->tiles[i].ani_tiles = (*tilesets[num]->tex_coords)[tilesets[num]->tiles[i].num].size();
				tilesets[num]->tiles[i].palette_shift = true;
			}
		}
	}

	log_debug("tileset %u loaded!", num);
}

void tileset::handle_animations(set<unsigned int>& modified_tiles) {
	if(cur_tileset == nullptr) return;

	const vector<vector<float2>>* tex_coords = cur_tileset->tex_coords;
	for(unsigned int i = 0; i < (unsigned int)cur_tileset->tile_obj_count; i++) {
		if(cur_tileset->tiles[i].ani_tiles > 1) {
			// update num
			cur_tileset->tiles[i].ani_num++;
			if(cur_tileset->tiles[i].ani_num > (cur_tileset->tiles[i].num+cur_tileset->tiles[i].ani_tiles-1)) {
				cur_tileset->tiles[i].ani_num = cur_tileset->tiles[i].num;
			}

			// update tex coord
			if(cur_tileset->tiles[i].palette_shift) {
				cur_tileset->tiles[i].tex_coord = (*tex_coords)[cur_tileset->tiles[i].num][cur_tileset->tiles[i].ani_num-cur_tileset->tiles[i].num];
			}
			else {
				cur_tileset->tiles[i].tex_coord = (*tex_coords)[cur_tileset->tiles[i].ani_num][0];
			}

			//
			modified_tiles.insert(i);
		}
	}
}

const float2 tileset::get_tile_tex_coord_size() const {
	if(cur_tileset == nullptr) return float2(0.0f);
	return cur_tileset->tile_tc_size;
}

const tileset::tileset_object& tileset::get_tileset(const size_t& num) const {
	return *tilesets[num];
}

const tileset::tileset_object& tileset::get_cur_tileset() const {
	return *cur_tileset;
}

const vector<vector<float2>>* tileset::get_tex_coords() const {
	return cur_tileset->tex_coords;
}

TILE_LAYER tileset::get_layer_type(const unsigned char& ch) const {
	// TODO: find out what 0x8 is used for

	if(ch == 0) return TILE_LAYER::UNDERLAY;
	//if(ch & 2 && ch & 4) return TILE_LAYER::OVERLAY;
	if(ch & 2 && ch & 4) return TILE_LAYER::DYNAMIC_3;
	if(ch & 2) return TILE_LAYER::DYNAMIC_1;
	if(ch & 4) return TILE_LAYER::DYNAMIC_2;
	return TILE_LAYER::UNKNOWN;
}
