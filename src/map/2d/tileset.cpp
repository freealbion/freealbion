/*
 *  Albion Remake
 *  Copyright (C) 2007 - 2010 Florian Ziesche
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

#include "tileset.h"

static const size_t tiles_per_row = 64;
static const float tile_tc_size = 1.0f / float(tiles_per_row);

/*! tileset constructor
 */
tileset::tileset(const pal* palettes) : palettes(palettes), cur_tileset_num(0) {
	// load tilesets
	xld* icongfx = new xld("ICONGFX0.XLD");

	const size_t tileset_count = icongfx->get_object_count();
	tilesets.reserve(tileset_count);
	for(size_t i = 0; i < tileset_count; i++) {
		tilesets.push_back(new tileset_object());
		const xld::xld_object* object = icongfx->get_object(i);
		tilesets.back()->tile_count = object->length/256;
		tilesets.back()->palette_num = get_tileset_palette(i);
		tilesets.back()->tile_data = new unsigned char[object->length];
		memcpy(tilesets.back()->tile_data, object->data, object->length);
	}

	delete icongfx;

	// index tilesets
	xld* icondata = new xld("ICONDAT0.XLD");
	
	// TODO: collision/special possibly swapped?

	/* icon data spec:
	 * 
	 * [8 byte chunk]
	 * [byte 1: overlay/underlay type]
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
	 * 				0x0400 = none (wall sit?), to the left, chair right side
	 * 				0x2000 = black on white, override collision (-> no collision)
	 * [byte 4: ?]
	 * [byte 5 + 6: tile number]
	 * [byte 7: animation tile count]
	 * [byte 8: ?]
	 * 
	 */

	size_t tile_count = 0;
	const float2 empty_tc = float2(tile_tc_size * float(tiles_per_row-1));
	for(size_t i = 0; i < icondata->get_object_count(); i++) {
		const xld::xld_object* object = icondata->get_object(i);
		tile_count = object->length/8;
		tilesets[i]->tiles = new tile_object[tile_count+2];

		// first two tiles are "empty"
		for(size_t j = 0; j < 2; j++) {
			tilesets[i]->tiles[j].num = 0xFFF;
			tilesets[i]->tiles[j].ani_tiles = 0;
			tilesets[i]->tiles[j].tex_coord = empty_tc;
			tilesets[i]->tiles[j].upper_bytes = 0;
			tilesets[i]->tiles[j].lower_bytes = 0;
			tilesets[i]->tiles[j].layer_type = TL_UNDERLAY;
			tilesets[i]->tiles[j].collision = 0;
		}

		size_t index = 0;
		for(size_t j = 0; j < tile_count; j++) {
			index = j+2;
			if(object->data[j*8+2] == 0x20 || object->data[j*8+6] == 0x00) {
				tilesets[i]->tiles[index].num = 0xFFF;
				tilesets[i]->tiles[index].ani_tiles = 0;
				tilesets[i]->tiles[index].tex_coord = empty_tc;
				tilesets[i]->tiles[index].layer_type = TL_UNDERLAY;
			}
			else {
				tilesets[i]->tiles[index].num = object->data[j*8+4] + (object->data[j*8+5] << 8);
				tilesets[i]->tiles[index].ani_tiles = object->data[j*8+6];
				tilesets[i]->tiles[index].tex_coord.x = float(tilesets[i]->tiles[index].num % tiles_per_row) * tile_tc_size;
				tilesets[i]->tiles[index].tex_coord.y = float(tilesets[i]->tiles[index].num / tiles_per_row) * tile_tc_size;
				tilesets[i]->tiles[index].layer_type = get_layer_type((object->data[j*8] >> 4) & 0x0F);
			}
			
			tilesets[i]->tiles[index].special_1 = object->data[j*8];
			tilesets[i]->tiles[index].special_2 = object->data[j*8+2] + (object->data[j*8+3] << 8);
			
			tilesets[i]->tiles[index].collision = (size_t)object->data[j*8+1];
			
			tilesets[i]->tiles[index].upper_bytes = (((size_t)object->data[j*8]) << 24) + (((size_t)object->data[j*8+1]) << 16) + (((size_t)object->data[j*8+2]) << 8) + (size_t)object->data[j*8+3];
			tilesets[i]->tiles[index].lower_bytes = (object->data[j*8+4] << 24) + (object->data[j*8+5] << 16) + (object->data[j*8+6] << 8) + object->data[j*8+7];
		}
		
		// highest tile count: 4041
		//cout << "tile count: " << tilesets[i]->tile_count << endl;
	}

	delete icondata;
}

/*! tileset destructor
 */
tileset::~tileset() {
}

void tileset::load(const size_t& num) {
	if(cur_tileset_num == num && tilesets[cur_tileset_num]->loaded) return;
	a2e_debug("loading tileset %u ...", num);
	if(cur_tileset_num != num && tilesets[cur_tileset_num]->loaded) {
		// unload old tileset
		t->delete_texture(tilesets[cur_tileset_num]->tileset);
		cur_tileset_tex = t->get_dummy_texture();
		tilesets[cur_tileset_num]->loaded = false;
	}
	
	cur_tileset = &get_tileset(num);
	cur_tileset_num = num;

	const size2 tileset_size = size2(4096, 4096);
	unsigned int* tiles_surface = new unsigned int[tileset_size.x*tileset_size.y];
	memset(tiles_surface, 0, tileset_size.x*tileset_size.y*sizeof(unsigned int));
	
	const size2 tile_size = size2(16, 16);
	const size2 scaled_tile_size = size2(64, 64);
	unsigned int* data_32bpp = new unsigned int[tile_size.x*tile_size.y];
	unsigned int* scaled_data = new unsigned int[scaled_tile_size.x*scaled_tile_size.y];
	for(size_t i = 0; i < tilesets[num]->tile_count; i++) {
		gfxconv::convert_8to32(&(tilesets[num]->tile_data[i*256]), data_32bpp, tile_size.x, tile_size.y, tilesets[num]->palette_num);
		//scaling::scale_4x(scaling::ST_NEAREST, data_32bpp, tile_size, scaled_data);
		scaling::scale_4x(scaling::ST_HQ4X, data_32bpp, tile_size, scaled_data);

		// copy data into tiles surface
		const size_t offset_x = (scaled_tile_size.x * i) % tileset_size.x;
		const size_t offset_y = scaled_tile_size.y * (scaled_tile_size.x * i / tileset_size.x);
		for(size_t y = 0; y < scaled_tile_size.y; y++) {
			memcpy(&tiles_surface[(offset_y + y) * tileset_size.x + offset_x], &scaled_data[y*64], 64*sizeof(unsigned int));
		}
	}
	delete [] scaled_data;
	delete [] data_32bpp;

	// don't use any filtering here! this will only produce artifacts!
	cur_tileset_tex = t->add_texture(tiles_surface, tileset_size.x, tileset_size.y, GL_RGBA8, GL_RGBA, texture_object::TF_POINT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_UNSIGNED_BYTE);
	tilesets[num]->tileset = cur_tileset_tex;
	tilesets[num]->loaded = true;

	delete [] tiles_surface;
	a2e_debug("tileset %u loaded!", num);
}

const float tileset::get_tile_tex_coord_size() const {
	return tile_tc_size;
}

const size_t tileset::get_tileset_palette(const size_t& num) const {
	switch(num) {
		case 0: return 0;
		case 1: return 0;
		case 2: return 5;
		case 3: return 3;
		case 4: return 4;
		case 5: return 10;
		case 6: return 8;
		case 7: return 6;
		case 8: return 27;
		case 9: return 44;
		case 10: return 55;
		default:
			a2e_error("tileset #%u doesn't exist!", num);
			return 0;
	}
	return 0;
}

const tileset::tileset_object& tileset::get_tileset(const size_t& num) const {
	return *tilesets[num];
}

const tileset::tileset_object& tileset::get_cur_tileset() const {
	return *cur_tileset;
}

const tileset::TILE_LAYER tileset::get_layer_type(const unsigned char& ch) const {
	/*
	 * 0 = always underlay
	 * 2 = test, overlay if player y <= tile y, else underlay
	 * 4 = test, overlay if player y <= tile y, else underlay
	 * ////4 = always overlay
	 * 6 = always overlay?
	 * 8 = ???
	 */
	
	if(ch == 0) return tileset::TL_UNDERLAY;
	if(ch & 2 && ch & 4) return tileset::TL_OVERLAY;
	if(ch & 2) return tileset::TL_DYNAMIC_1;
	if(ch & 4) return tileset::TL_DYNAMIC_2;
	return tileset::TL_UNKNOWN;
}
