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

#include "map2d.h"
#include "npc2d.h"

static const float tile_size = 16.0f; // ^= 16 px, original size
static const float snap_epsilon = 0.05f;

/*! map2d constructor
 */
map2d::map2d(tileset* tilesets, npcgfx* npc_graphics) : tilesets(tilesets), npc_graphics(npc_graphics) {
	cur_map_data = NULL;
	underlay_tiles = NULL;
	overlay_tiles = NULL;
	map_loaded = false;
	map_palette = 0;
	cur_map_num = (~0);
	
	row_offsets = NULL;

	mnpcs = NULL;

	// load maps
	maps1 = new xld("MAPDATA1.XLD");
	maps2 = new xld("MAPDATA2.XLD");
	maps3 = new xld("MAPDATA3.XLD");
}

/*! map2d destructor
 */
map2d::~map2d() {
	delete maps1;
	delete maps2;
	delete maps3;

	unload();
}

void map2d::load(const size_t& map_num) {
	if(cur_map_num == map_num) return;
	a2e_debug("loading map %u ...", map_num);
	unload();

	//
	size_t len;
	const xld::xld_object* object;
	if(map_num < 100) {
		object = maps1->get_object(map_num);
	}
	else if(map_num >= 100 && map_num < 200) {
		object = maps2->get_object(map_num-100);
	}
	else if(map_num >= 200 && map_num < 300) {
		object = maps3->get_object(map_num-200);
	}
	else {
		a2e_error("invalid map number: %u!", map_num);
		return;
	}

	// copy map data
	len = object->length;
	if(len != 0) {
		cur_map_data = new unsigned char[len];
		memcpy(cur_map_data, object->data, len);
	}
	else {
		a2e_error("invalid map (len = 0)!");
		return;
	}

	if(cur_map_data[2] != 0x02) {
		a2e_error("map #%u is no 2D map!", map_num);
		return;
	}

	// get map infos
	// header
	const size_t header_len = 10;
	size_t npc_data_len = ((size_t)cur_map_data[1]) * 10;
	map_size.set(cur_map_data[4], cur_map_data[5]);
	cur_tileset_num = cur_map_data[6];
	map_palette = (size_t)cur_map_data[8];
	
	// npc/monster info
	if(npc_data_len == 0x00) npc_data_len = 32 * 10;
	if(npc_data_len == 0x40) npc_data_len = 96 * 10;
	
	// actual map data
	underlay_tiles = new unsigned int[map_size.x*map_size.y];
	overlay_tiles = new unsigned int[map_size.x*map_size.y];

	tilesets->load(cur_tileset_num-1);

	const size_t map_data_offset = npc_data_len + header_len;
	const size_t map_data_len = (map_size.x * map_size.y) * 3;
	for(size_t i = map_data_offset, tile_num = 0; i < (map_data_offset + map_data_len); i+=3, tile_num++) {
		overlay_tiles[tile_num] = (unsigned int)((cur_map_data[i] << 4) + (cur_map_data[i+1] >> 4));
		underlay_tiles[tile_num] = (unsigned int)(((cur_map_data[i+1] & 0x0F) << 8) + cur_map_data[i+2]);
	}
	
	// events
	const size_t events_offset = map_data_offset + map_data_len;
	mevents.load(cur_map_data, events_offset, map_size);

	// load npc/monster data (has to be done here, b/c we need info that is available only after all events have been loaded)
	mnpcs = new map_npcs();
	mnpcs->load(cur_map_data, mevents.get_end_offset());
	const vector<map_npcs::map_npc*>& npc_data = mnpcs->get_npcs();
	for(vector<map_npcs::map_npc*>::const_iterator npc_iter = npc_data.begin(); npc_iter != npc_data.end(); npc_iter++) {
		npcs.push_back(new npc2d(this, npc_graphics));
		npcs.back()->set_npc_data(*npc_iter);
	}
	
	// create opengl buffers
	row_offsets = new size_t[map_size.y];
	
	const float tile_tc_size = tilesets->get_tile_tex_coord_size();
	const tileset::tile_object* tile_obj;
	const tileset::tileset_object& tileset_obj = tilesets->get_cur_tileset();
	for(size_t i = 0; i < 2; i++) {
		// create buffer data
		unsigned int* tile_nums = (i == 0 ? underlay_tiles : overlay_tiles);
		
		// figure out tile count, so we can allocate the right amount of memory
		size_t tile_count = 0;
		for(size_t y = 0; y < map_size.y; y++) {
			for(size_t x = 0; x < map_size.x; x++) {
				tile_obj = &(tileset_obj.tiles[tile_nums[y*map_size.x + x]]);
				if(tile_obj->num == 0xFFF) continue;
				tile_count++;
			}
			
			// while we're at it, also save the tile num/id offset of each row (only applies to overlay layer)
			if(i == 1) {
				row_offsets[y] = tile_count;
			}
		}
		
		// allocate memory and fill data
		layers[i].vertices = new float2[tile_count*4];
		layers[i].tex_coords = new float2[tile_count*4];
		layers[i].indices = new index4[tile_count];
		unsigned int tile_num = 0;
		
		for(size_t y = 0; y < map_size.y; y++) {
			for(size_t x = 0; x < map_size.x; x++) {
				if(tile_nums[y*map_size.x + x] != 0) {
					tile_obj = &(tileset_obj.tiles[tile_nums[y*map_size.x + x]]);
					if(tile_obj->num == 0xFFF) continue;

					layers[i].vertices[tile_num*4 + 0].set(float(x)*tile_size, float(y)*tile_size);
					layers[i].vertices[tile_num*4 + 1].set(float(x)*tile_size + tile_size, float(y)*tile_size);
					layers[i].vertices[tile_num*4 + 2].set(float(x)*tile_size + tile_size, float(y)*tile_size + tile_size);
					layers[i].vertices[tile_num*4 + 3].set(float(x)*tile_size, float(y)*tile_size + tile_size);
					
					layers[i].tex_coords[tile_num*4 + 0].set(tile_obj->tex_coord.x, tile_obj->tex_coord.y);
					layers[i].tex_coords[tile_num*4 + 1].set(tile_obj->tex_coord.x + tile_tc_size, tile_obj->tex_coord.y);
					layers[i].tex_coords[tile_num*4 + 2].set(tile_obj->tex_coord.x + tile_tc_size, tile_obj->tex_coord.y + tile_tc_size);
					layers[i].tex_coords[tile_num*4 + 3].set(tile_obj->tex_coord.x, tile_obj->tex_coord.y + tile_tc_size);
					
					layers[i].indices[tile_num].set(tile_num*4 + 0, tile_num*4 + 1, tile_num*4 + 2, tile_num*4 + 3);
					tile_num++;
				}
			}
		}
		
		// create vbos
		glGenBuffers(1, &layers[i].vertices_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, layers[i].vertices_vbo);
		glBufferData(GL_ARRAY_BUFFER, tile_count * 4 * 2 * sizeof(float), layers[i].vertices, GL_STATIC_DRAW);
		
		glGenBuffers(1, &layers[i].tex_coords_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, layers[i].tex_coords_vbo);
		glBufferData(GL_ARRAY_BUFFER, tile_count * 4 * 2 * sizeof(float), layers[i].tex_coords, GL_STATIC_DRAW);
		
		glGenBuffers(1, &layers[i].indices_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, layers[i].indices_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, tile_count * 4 * sizeof(unsigned int), layers[i].indices, GL_STATIC_DRAW);
		
		cout << ":: #tiles for layer " << i << ": " << tile_count << endl;
		
		layers[i].index_count = tile_count * 4;
	}
	
	//
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// and finally:
	map_loaded = true;
	cur_map_num = map_num;
	a2e_debug("map #%u loaded!", map_num);
}

void map2d::unload() {
	if(cur_map_data != NULL) {
		delete [] cur_map_data;
		cur_map_data = NULL;
	}
	if(underlay_tiles != NULL) {
		delete [] underlay_tiles;
		underlay_tiles = NULL;
	}
	if(overlay_tiles != NULL) {
		delete [] overlay_tiles;
		overlay_tiles = NULL;
	}
	if(row_offsets != NULL) {
		delete [] row_offsets;
		row_offsets = NULL;
	}
	if(mnpcs != NULL) {
		delete mnpcs;
		mnpcs = NULL;
	}
	layers[0].clear();
	layers[1].clear();
	npcs.clear();
	mevents.unload();
	map_loaded = false;
}

bool map2d::is_2d_map(const size_t& map_num) const {
	const xld::xld_object* object;
	if(map_num < 100) {
		object = maps1->get_object(map_num);
	}
	else if(map_num >= 100 && map_num < 200) {
		object = maps2->get_object(map_num-100);
	}
	else if(map_num >= 200 && map_num < 300) {
		object = maps3->get_object(map_num-200);
	}
	else {
		a2e_error("invalid map number: %u!", map_num);
		return false;
	}
	
	return (object->data[2] == 0x02);
}

// TODO: write correct/nice windows roundf
#ifdef __WINDOWS__
#ifndef roundf
#define roundf(x) (x > 0.0 ? floorf(x + 0.5f) : ceilf(x - 0.5f))
#endif
#endif

void map2d::handle() {
	// handle npcs
	for(vector<npc2d*>::iterator npc_iter = npcs.begin(); npc_iter != npcs.end(); npc_iter++) {
		(*npc_iter)->handle();
	}

	//
	static size_t last_frame_time = SDL_GetTicks();
	static const float scroll_factor_per_second = 10.0f;
	float scroll_factor = float(SDL_GetTicks() - last_frame_time)/1000.0f * scroll_factor_per_second;
	last_frame_time = SDL_GetTicks();
	
	// compute target position (aka map draw offset/position, left top corner)
	float scaled_tile_size = tile_size * conf::get<float>("global.scale");
	normal_player_offset.set(e->get_width() / size_t(scaled_tile_size) / 2, e->get_height() / size_t(scaled_tile_size) / 2);
	float2 target_position = ssize2(next_position) - ssize2(normal_player_offset);
	target_position.clamp(float2(0.0f),
						  float2(map_size) - float2(e->get_width() / size_t(scaled_tile_size), e->get_height() / size_t(scaled_tile_size)));
	
	// compute scroll speed depending on distance (speed = 1 - 1/exp(|distance|))
	float2 pos_diff = target_position - screen_position;
	float2 scroll_speed = float2(expf(fabs(pos_diff.x)), expf(fabs(pos_diff.y)));
	scroll_speed = float2(1.0f) - float2(1.0f) / scroll_speed;
	scroll_speed.clamp(float2(fabs(pos_diff.x) > snap_epsilon ? snap_epsilon : 0.0f,
							  fabs(pos_diff.y) > snap_epsilon ? snap_epsilon : 0.0f),
					   1.0f);
	scroll_speed = scroll_speed * pos_diff.sign(); // correct direction
	
	// snap screen position to some even coordinate, so we don't get fugly aliasing artifacts or even wrong texels!
	float2 snap_screen_position = screen_position;
	if(fabs(roundf(snap_screen_position.x) - snap_screen_position.x) <= snap_epsilon) {
		screen_position.x = roundf(snap_screen_position.x);
	}
	if(fabs(roundf(snap_screen_position.y) - snap_screen_position.y) <= snap_epsilon) {
		screen_position.y = roundf(snap_screen_position.y);
	}
	
	// set new, scrolled/interpolated screen position
	screen_position = screen_position + scroll_speed * scroll_factor;
}

void map2d::draw(const MAP_DRAW_STAGE& draw_stage) const {
	if(map_loaded) {
		glPushMatrix();
		glFrontFace(GL_CW);

		const float scale = conf::get<float>("global.scale");
		const float scaled_tile_size = scale * tile_size;

		static const float snap_factor = 100.0f;
		float2 snapped_position = screen_position * snap_factor;
		snapped_position.x = roundf(snapped_position.x);
		snapped_position.y = roundf(snapped_position.y);
		snapped_position = snapped_position / snap_factor;
		glTranslatef(-snapped_position.x * scaled_tile_size, -snapped_position.y * scaled_tile_size, 0.0f);

		if(draw_stage == MDS_UNDERLAY || draw_stage == MDS_OVERLAY) {
			glScalef(scale, scale, scale);
			glColor3f(1.0f, 1.0f, 1.0f);

			const tileset::tileset_object& tileset_obj = tilesets->get_cur_tileset();
			const map_layer& cur_layer = (draw_stage == MDS_UNDERLAY ? layers[0] : layers[1]);

			if(draw_stage == MDS_OVERLAY) glEnable(GL_BLEND);
			else glDisable(GL_BLEND);
		
			glActiveTexture(GL_TEXTURE0);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, tileset_obj.tileset->tex());
		
			glBindBuffer(GL_ARRAY_BUFFER, cur_layer.vertices_vbo);
			glVertexPointer(2, GL_FLOAT, 0, NULL);
			glEnableClientState(GL_VERTEX_ARRAY);
		
			glClientActiveTexture(GL_TEXTURE0);
			glBindBuffer(GL_ARRAY_BUFFER, cur_layer.tex_coords_vbo);
			glTexCoordPointer(2, GL_FLOAT, 0, NULL);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cur_layer.indices_vbo);
			glDrawElements(GL_QUADS, (GLsizei)cur_layer.index_count, GL_UNSIGNED_INT, NULL);
		
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
			glActiveTexture(GL_TEXTURE0);
			glDisable(GL_TEXTURE_2D);
		}
		else if(draw_stage == MDS_NPCS) {
			// draw npcs
			for(vector<npc2d*>::const_iterator npc_iter = npcs.begin(); npc_iter != npcs.end(); npc_iter++) {
				(*npc_iter)->draw();
			}
		}
		else if(draw_stage == MDS_DEBUG) {
			// draw events onto the map
			if(conf::get<bool>("debug.draw_events")) {
				glScalef(scale, scale, scale);
				glColor3f(1.0f, 1.0f, 1.0f);

				glDisable(GL_TEXTURE_2D);
				glFrontFace(GL_CCW);

				gfx::rect evt_rect;
				for(size_t i = 0; i < mevents.get_event_info_count(); i++) {
					const map_events::map_event_info* cur_evt = mevents.get_event_info(i);
					
					evt_rect.x1 = (cur_evt->xpos - 1) * tile_size + tile_size/2 - 3;
					evt_rect.y1 = cur_evt->ypos * tile_size + tile_size/2 - 3;
					evt_rect.x2 = evt_rect.x1 + 6;
					evt_rect.y2 = evt_rect.y1 + 6;
					egfx->draw_fade_rectangle(&evt_rect, 0xCFFFFFFF, 0x2FFFFFFF, gfx::FT_DIAGONAL);
				}
			}
		}

		glFrontFace(GL_CCW);
		glPopMatrix();
	}
}

tileset::tile_object* map2d::get_tile(unsigned int type) {
	return map_loaded ? &(tilesets->get_cur_tileset().tiles[(type==1?overlay_tiles:underlay_tiles)[player_position.y*map_size.x+player_position.x]]) : NULL;
}

const size2& map2d::get_size() const {
	return map_size;
}

void map2d::set_pos(const size_t& x, const size_t& y) {
	next_position.set(x, y);
}

bool map2d::collide(const MOVE_DIRECTION& direction, const size2& cur_position) const {
	if(map_loaded && conf::get<bool>("map.collision")) {
		size2 next_position_1;
		size2 next_position_2;
		switch(direction) {
			case MD_DOWN:
				if(cur_position.y == map_size.y-1) return true;
				next_position_1 = size2(cur_position.x, cur_position.y+1);
				next_position_2 = size2(cur_position.x+1, cur_position.y+1);
				break;
			case MD_UP:
				if(cur_position.y == 0) return true;
				next_position_1 = size2(cur_position.x, cur_position.y-1);
				next_position_2 = size2(cur_position.x+1, cur_position.y-1);
				break;
			case MD_LEFT:
				if(cur_position.x == 0) return true;
				next_position_1 = size2(cur_position.x-1, cur_position.y);
				next_position_2 = next_position_1;
				//next_position_2 = size2(cur_position.x-1, cur_position.y+1);
				break;
			case MD_RIGHT:
				if(cur_position.x+1 == map_size.x-1) return true;
				next_position_1 = size2(cur_position.x+2, cur_position.y);
				next_position_2 = next_position_1;
				//next_position_2 = size2(cur_position.x+2, cur_position.y+1);
				break;
			default:
				break;
		}
		
		//
		const tileset::tileset_object& cur_tileset = tilesets->get_cur_tileset();
		const tileset::tile_object& tile_1_u = cur_tileset.tiles[underlay_tiles[next_position_1.y*map_size.x + next_position_1.x]];
		const tileset::tile_object& tile_1_o = cur_tileset.tiles[overlay_tiles[next_position_1.y*map_size.x + next_position_1.x]];
		const tileset::tile_object& tile_2_u = cur_tileset.tiles[underlay_tiles[next_position_2.y*map_size.x + next_position_2.x]];
		const tileset::tile_object& tile_2_o = cur_tileset.tiles[overlay_tiles[next_position_2.y*map_size.x + next_position_2.x]];
		
		// first, check special types
		for(size_t i = 0; i < 4; i++) {
			const tileset::tile_object& cur_tile = (i == 0 ? tile_1_u : (i == 1 ? tile_2_u : (i == 2 ? tile_1_o : tile_2_o)));
			switch(cur_tile.special_2) {
				case 0x0020:
					return (cur_tile.collision != 0);
				case 0x0204:
				case 0x0284:
				case 0x0080:
				case 0x0100:
				case 0x0304:
					return true;
					break;
				default:
					break;
			}
		}
		
		// check collision byte
		if(tile_1_u.collision != 0 ||
		   tile_1_o.collision != 0 ||
		   tile_2_u.collision != 0 ||
		   tile_2_o.collision != 0) {
			return true;
		}
	}
	return false;
}

const size2& map2d::get_player_offset() const {
	return player_offset;
}

const float map2d::get_tile_size() const {
	return tile_size * conf::get<float>("global.scale");
}

const size_t& map2d::get_palette() const {
	return map_palette;
}

const float2& map2d::get_screen_position() const {
	return screen_position;
}
