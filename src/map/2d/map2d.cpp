/*
 *  Albion Remake
 *  Copyright (C) 2007 - 2015 Florian Ziesche
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

#include "map/2d/map2d.hpp"
#include "map/2d/npc2d.hpp"
#include <engine.hpp>
#include <rendering/shader.hpp>

static const float tile_size = 16.0f; // ^= 16 px, original size
static const float snap_epsilon = 0.05f;

/*! map2d constructor
 */
map2d::map2d(tileset* tilesets_, npcgfx* npc_graphics_, array<lazy_xld, 3>& maps_) :
p2d(nullptr), tilesets(tilesets_), npc_graphics(npc_graphics_), maps(maps_) {
	underlay_tiles = nullptr;
	overlay_tiles = nullptr;
	map_loaded = false;
	map_palette = 0;
	cur_map_num = (~0u);
	last_tile_animation = SDL_GetTicks();
	
	set_initial_position(size2(0, 0));

	mnpcs = nullptr;
}

/*! map2d destructor
 */
map2d::~map2d() {
	unload();
}

void map2d::load(const size_t& map_num) {
	if(cur_map_num == map_num) return;
	log_debug("loading map %u ...", map_num);
	unload();
	
	//
	if(map_num >= MAX_MAP_NUMBER) {
		log_error("invalid map number: %u!", map_num);
		return;
	}
	
	const auto len = maps[map_num/100].get_object_size(map_num % 100);
	if(len == 0) {
		log_error("map %u is empty!", map_num);
		return;
	}
	
	auto object_data = maps[map_num/100].get_object_data(map_num % 100);
	const auto cur_map_data = object_data->data();

	if(cur_map_data[2] != 0x02) {
		log_error("map #%u is no 2D map!", map_num);
		return;
	}

	// get map infos
	// header
	const size_t header_len = 10;
	size_t npc_data_len = (size_t)cur_map_data[1];
	map_size.set(cur_map_data[4], cur_map_data[5]);
	cur_tileset_num = (size_t)cur_map_data[6];
	map_palette = (size_t)cur_map_data[8];
	
	continent_map = (cur_tileset_num == 1 || cur_tileset_num == 2 || cur_tileset_num == 4); //0,1,3
	cout << "continent_map? " << continent_map << endl;
	if(p2d != nullptr) p2d->set_continent(continent_map);

	for(size_t i = 0; i < 32; i++) {
		cout << ((size_t)cur_map_data[i] < 16 ? "0":"") << hex << (size_t)cur_map_data[i] << dec;
	}
	cout << endl;
	cout << "map_palette: " << map_palette << endl;
	
	// npc/monster info
	if(npc_data_len == 0x00) npc_data_len = 32 * 10;
	else if(npc_data_len == 0x40) npc_data_len = 96 * 10;
	else npc_data_len *= 10;
	
	// actual map data
	underlay_tiles = new unsigned int[map_size.x*map_size.y];
	overlay_tiles = new unsigned int[map_size.x*map_size.y];

	tilesets->load(cur_tileset_num-1, map_palette-1);

	const size_t map_data_offset = npc_data_len + header_len;
	const size_t map_data_len = (map_size.x * map_size.y) * 3;
	for(size_t i = map_data_offset, tile_num = 0; i < (map_data_offset + map_data_len); i+=3, tile_num++) {
		overlay_tiles[tile_num] = (unsigned int)((cur_map_data[i] << 4) + (cur_map_data[i+1] >> 4));
		underlay_tiles[tile_num] = (unsigned int)(((cur_map_data[i+1] & 0x0F) << 8) + cur_map_data[i+2]);
	}
	
	// events
	const size_t events_offset = map_data_offset + map_data_len;
	mevents.load(cur_map_data, len, events_offset, map_size);

	// load npc/monster data (has to be done here, b/c we need info that is available only after all events have been loaded)
	mnpcs = new map_npcs();
	mnpcs->load(cur_map_data, mevents.get_end_offset());
	const vector<map_npcs::map_npc*>& npc_data = mnpcs->get_npcs();
	for(vector<map_npcs::map_npc*>::const_iterator npc_iter = npc_data.begin(); npc_iter != npc_data.end(); npc_iter++) {
		npcs.push_back(new npc2d(this, npc_graphics));
		npcs.back()->set_npc_data(*npc_iter);
		npcs.back()->set_continent(continent_map);
	}
	
	// create opengl buffers

	const float2 tile_tc_size = tilesets->get_tile_tex_coord_size();
	const tileset::tile_object* tile_obj;
	const tileset::tileset_object& tileset_obj = tilesets->get_cur_tileset();
	for(size_t i = 0; i < 2; i++) {
		// create buffer data
		unsigned int* tile_nums = (i == 0 ? underlay_tiles : overlay_tiles);
		
		// figure out tile count, so we can allocate the right amount of memory
		size_t tile_count = 0, ani_tile_count = 0;
		for(size_t y = 0; y < map_size.y; y++) {
			for(size_t x = 0; x < map_size.x; x++) {
				tile_obj = &(tileset_obj.tiles[tile_nums[y*map_size.x + x]]);
				if(tile_obj->num == 0xFFF) continue;
				tile_count++;
				if(tile_obj->ani_tiles > 1) ani_tile_count++;
			}
		}
		
		// allocate memory and fill data
		layers[i].vertices = new float3[tile_count*4];
		layers[i].tex_coords = new float2[tile_count*4];
		layers[i].indices = new uint3[tile_count*2];
		layers[i].tile_nums = new unsigned int[tile_count];
		layers[i].ani_offset = tile_count-ani_tile_count;

		// note: animated tiles will be stored at the end (in one block), so they
		// can be updated in a faster way (w/o reuploading unanimated tiles)
		size_t tile_num = 0, ani_tile_num = layers[i].ani_offset;
		for(size_t y = 0; y < map_size.y; y++) {
			for(size_t x = 0; x < map_size.x; x++) {
				if(tile_nums[y*map_size.x + x] != 0) {
					tile_obj = &(tileset_obj.tiles[tile_nums[y*map_size.x + x]]);
					if(tile_obj->num == 0xFFF) continue;

					float tile_depth = 0.0f;
					switch(tile_obj->layer_type) {
						case TILE_LAYER::UNDERLAY: tile_depth = 0.0f; break;
						case TILE_LAYER::OVERLAY: tile_depth = 1.0f; break;
						case TILE_LAYER::DYNAMIC_1:
							tile_depth = float(y)/255.0f;
							break;
						case TILE_LAYER::DYNAMIC_2:
							tile_depth = float(y+1)/255.0f;
							break;
						case TILE_LAYER::DYNAMIC_3:
							tile_depth = float(y+2)/255.0f;
							break;
						default:
							tile_depth = -1.0f;
							break;
					}

					const size_t& num = (tile_obj->ani_tiles > 1 ? ani_tile_num : tile_num);
					layers[i].vertices[num*4 + 0].set(float(x)*tile_size, float(y)*tile_size, tile_depth);
					layers[i].vertices[num*4 + 1].set(float(x)*tile_size + tile_size, float(y)*tile_size, tile_depth);
					layers[i].vertices[num*4 + 2].set(float(x)*tile_size + tile_size, float(y)*tile_size + tile_size, tile_depth);
					layers[i].vertices[num*4 + 3].set(float(x)*tile_size, float(y)*tile_size + tile_size, tile_depth);
					
					layers[i].tex_coords[num*4 + 0].set(tile_obj->tex_coord.x, tile_obj->tex_coord.y);
					layers[i].tex_coords[num*4 + 1].set(tile_obj->tex_coord.x + tile_tc_size.x, tile_obj->tex_coord.y);
					layers[i].tex_coords[num*4 + 2].set(tile_obj->tex_coord.x + tile_tc_size.x, tile_obj->tex_coord.y + tile_tc_size.y);
					layers[i].tex_coords[num*4 + 3].set(tile_obj->tex_coord.x, tile_obj->tex_coord.y + tile_tc_size.y);
					
					layers[i].indices[num*2].set((unsigned int)num*4 + 0, (unsigned int)num*4 + 1, (unsigned int)num*4 + 2);
					layers[i].indices[num*2 + 1].set((unsigned int)num*4 + 0, (unsigned int)num*4 + 2, (unsigned int)num*4 + 3);
					layers[i].tile_nums[num] = tile_nums[y*map_size.x + x];

					if(tile_obj->ani_tiles > 1) ani_tile_num++;
					else tile_num++;
				}
			}
		}
		
		// create vbos
		glGenBuffers(1, &layers[i].vertices_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, layers[i].vertices_vbo);
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(tile_count * 4 * 3 * sizeof(float)), layers[i].vertices, GL_STATIC_DRAW);
		
		glGenBuffers(1, &layers[i].tex_coords_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, layers[i].tex_coords_vbo);
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(tile_count * 4 * 2 * sizeof(float)), layers[i].tex_coords, GL_DYNAMIC_DRAW);
		
		glGenBuffers(1, &layers[i].indices_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, layers[i].indices_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(tile_count * 2 * sizeof(uint3)), layers[i].indices, GL_STATIC_DRAW);
		
		cout << ":: #tiles for layer " << i << ": " << tile_count << endl;
		
		layers[i].index_count = tile_count;
	}
	
	//
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// and finally:
	map_loaded = true;
	cur_map_num = map_num;
	log_debug("map #%u loaded!", map_num);
}

void map2d::unload() {
	if(underlay_tiles != nullptr) {
		delete [] underlay_tiles;
		underlay_tiles = nullptr;
	}
	if(overlay_tiles != nullptr) {
		delete [] overlay_tiles;
		overlay_tiles = nullptr;
	}
	
	//
	for(vector<npc2d*>::iterator npc_iter = npcs.begin(); npc_iter != npcs.end(); npc_iter++) {
		delete *npc_iter;
	}
	npcs.clear();
	if(mnpcs != nullptr) {
		delete mnpcs;
		mnpcs = nullptr;
	}
	
	layers[0].clear();
	layers[1].clear();
	mevents.unload();
	map_loaded = false;
	cur_map_num = (~0u);
}

void map2d::handle() {
	if(!map_loaded) return;

	// handle npcs
	for(vector<npc2d*>::iterator npc_iter = npcs.begin(); npc_iter != npcs.end(); npc_iter++) {
		(*npc_iter)->handle();
	}

	//
	static size_t last_frame_time = SDL_GetTicks();
	static const float scroll_factor_per_second = 10.0f;
	float scroll_factor = std::min(float(SDL_GetTicks() - last_frame_time), 1000.0f)/1000.0f * scroll_factor_per_second;
	last_frame_time = SDL_GetTicks();
	
	// compute target position (aka map draw offset/position, left top corner)
	const float2 target_position = compute_target_position();
	
	// compute scroll speed depending on distance (speed = 1 - 1/exp(|distance|))
	const float2 pos_diff = target_position - screen_position;
	float2 scroll_speed = 1.0f - 1.0f / pos_diff.absed().exped();
	scroll_speed.clamp(float2(fabs(pos_diff.x) > snap_epsilon ? snap_epsilon : 0.0f,
							  fabs(pos_diff.y) > snap_epsilon ? snap_epsilon : 0.0f),
					   1.0f);
	scroll_speed *= pos_diff.sign(); // correct direction
	
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

	// handle tileset and animations
	if((SDL_GetTicks() - last_tile_animation) > TIME_PER_TILE_ANIMATION_FRAME) {
		last_tile_animation = SDL_GetTicks();

		set<unsigned int> modified_tiles;
		tilesets->handle_animations(modified_tiles);
	
		//cout << "modified_tiles: " << modified_tiles.size() << endl;
		if(modified_tiles.size() > 0) {
			const float2 tile_tc_size = tilesets->get_tile_tex_coord_size();
			const tileset::tile_object* tile_obj;
			const tileset::tileset_object& tileset_obj = tilesets->get_cur_tileset();
			for(size_t i = 0; i < 2; i++) {
				map_layer& cur_layer = layers[i];
				for(size_t tl = cur_layer.ani_offset; tl < cur_layer.index_count; tl++) {
					if(modified_tiles.count(cur_layer.tile_nums[tl]) > 0) {
						tile_obj = &(tileset_obj.tiles[cur_layer.tile_nums[tl]]);
						if(tile_obj->num == 0xFFF) continue;
					
						cur_layer.tex_coords[tl*4 + 0].set(tile_obj->tex_coord.x, tile_obj->tex_coord.y);
						cur_layer.tex_coords[tl*4 + 1].set(tile_obj->tex_coord.x + tile_tc_size.x, tile_obj->tex_coord.y);
						cur_layer.tex_coords[tl*4 + 2].set(tile_obj->tex_coord.x + tile_tc_size.x, tile_obj->tex_coord.y + tile_tc_size.y);
						cur_layer.tex_coords[tl*4 + 3].set(tile_obj->tex_coord.x, tile_obj->tex_coord.y + tile_tc_size.y);
					}
				}

				size_t update_size = (cur_layer.index_count - cur_layer.ani_offset)*4;
				if(update_size == 0) continue;

				glBindBuffer(GL_ARRAY_BUFFER, cur_layer.tex_coords_vbo);
				glBufferSubData(GL_ARRAY_BUFFER, (GLsizeiptr)((cur_layer.ani_offset*4) * 2 * sizeof(float)),
								(GLsizeiptr)(update_size * 2 * sizeof(float)), &cur_layer.tex_coords[cur_layer.ani_offset*4]);
			}

			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	}
}

float2 map2d::compute_target_position() {
	float scaled_tile_size = tile_size * conf::get<float>("map.scale");
	normal_player_offset.set(floor::get_physical_width() / size_t(scaled_tile_size) / 2,
							 floor::get_physical_height() / size_t(scaled_tile_size) / 2);
	float2 target_position = ssize2(next_position) - ssize2(normal_player_offset);
	target_position.clamp(float2(0.0f),
						  float2(map_size) - float2(floor::get_physical_width() / size_t(scaled_tile_size),
													floor::get_physical_height() / size_t(scaled_tile_size)));
	return target_position;
}

void map2d::draw(const MAP_DRAW_STAGE& draw_stage, const NPC_DRAW_STAGE& npc_draw_stage) const {
	if(!map_loaded) return;

	engine::push_modelview_matrix();
	matrix4f* mvm = engine::get_modelview_matrix();
	matrix4f* pm = engine::get_projection_matrix();

	const float scale = conf::get<float>("map.scale");
	const float scaled_tile_size = scale * tile_size;

	static const float snap_factor = 100.0f;
	const float2 snapped_position = (screen_position * snap_factor).round() / snap_factor;
	*mvm *= matrix4f().translate(-snapped_position.x * scaled_tile_size,
								 -snapped_position.y * scaled_tile_size,
								 0.0f);

	if(draw_stage == MAP_DRAW_STAGE::UNDERLAY || draw_stage == MAP_DRAW_STAGE::OVERLAY) {
		*mvm = matrix4f().scale(scale, scale, 1.0f) * *mvm; // *reversed order

		const tileset::tileset_object& tileset_obj = tilesets->get_cur_tileset();
		const map_layer& cur_layer = (draw_stage == MAP_DRAW_STAGE::UNDERLAY ? layers[0] : layers[1]);
		
		gl_shader shd = s->get_gl_shader("SIMPLE");
		shd->use("textured");
		shd->uniform("mvpm", *mvm * *pm);
		shd->texture("tex", tileset_obj.tileset->tex());
		shd->attribute_array("in_vertex", cur_layer.vertices_vbo, 3, GL_FLOAT);
		shd->attribute_array("in_tex_coord", cur_layer.tex_coords_vbo, 2, GL_FLOAT);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cur_layer.indices_vbo);
		glDrawElements(GL_TRIANGLES, (GLsizei)cur_layer.index_count*6, GL_UNSIGNED_INT, nullptr);
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	else if(draw_stage == MAP_DRAW_STAGE::NPCS) {
		// draw npcs
		*engine::get_mvp_matrix() = *mvm * *pm;
		for(vector<npc2d*>::const_iterator npc_iter = npcs.begin(); npc_iter != npcs.end(); npc_iter++) {
			(*npc_iter)->draw(npc_draw_stage);
		}
	}
	else if(draw_stage == MAP_DRAW_STAGE::DEBUGGING) {
		// draw events onto the map
		if(conf::get<bool>("debug.draw_events")) {
			*mvm *= matrix4f().scale(scale, scale, 1.0f);
			*mvm *= matrix4f().translate(-snapped_position.x * scaled_tile_size,
										 -snapped_position.y * scaled_tile_size,
										 0.0f);
			*engine::get_mvp_matrix() = *mvm * *pm;

			rect evt_rect;
			for(size_t i = 0; i < mevents.get_event_info_count(); i++) {
				const map_events::map_event_info* cur_evt = mevents.get_event_info(i);
					
				evt_rect.x1 = uint32_t(float(cur_evt->xpos) * tile_size + tile_size/2.0f - 3.0f);
				evt_rect.y1 = uint32_t(float(cur_evt->ypos) * tile_size + tile_size/2.0f - 3.0f);
				evt_rect.x2 = evt_rect.x1 + 6;
				evt_rect.y2 = evt_rect.y1 + 6;
				//gfx2d::draw_fade_rectangle(&evt_rect, 0xCFFFFFFF, 0x2FFFFFFF, gfx2d::FT_DIAGONAL);
			}
		}
	}

	engine::pop_modelview_matrix();
}

tileset::tile_object* map2d::get_tile(unsigned int type) {
	return map_loaded ? &(tilesets->get_cur_tileset().tiles[(type==1?overlay_tiles:underlay_tiles)[size_t(next_position.y)*map_size.x+size_t(next_position.x)]]) : nullptr;
}

unsigned int map2d::get_tile_num(unsigned int type) {
	return map_loaded ? (type==1?overlay_tiles:underlay_tiles)[size_t(next_position.y)*map_size.x+size_t(next_position.x)] : 0;
}

const size2& map2d::get_size() const {
	return map_size;
}

void map2d::set_pos(const size_t& x, const size_t& y) {
	next_position.set((ssize_t)x, (ssize_t)y);
}

bool map2d::collide(const MOVE_DIRECTION& direction, const size2& cur_position, const CHARACTER_TYPE& char_type) const {
	if(!map_loaded) return false;
	
	if(!conf::get<bool>("map.collision") && char_type == CHARACTER_TYPE::PLAYER) return false;
	
	if(cur_position.x >= map_size.x) return true;
	if(cur_position.y >= map_size.y) return true;
	
	size2 next_pos[6]; // just to be on the safe side ...
	size_t position_count = 0;
	
	if((direction & MOVE_DIRECTION::LEFT) != MOVE_DIRECTION::NONE) {
		if(cur_position.x == 0) return true;
		next_pos[position_count++] = size2(cur_position.x-1, cur_position.y);
	}
	if((direction & MOVE_DIRECTION::RIGHT) != MOVE_DIRECTION::NONE) {
		if(cur_position.x+2 >= map_size.x) return true;
		next_pos[position_count++] = size2(cur_position.x+2, cur_position.y);
	}
	if((direction & MOVE_DIRECTION::UP) != MOVE_DIRECTION::NONE) {
		if(cur_position.y == 0) return true;
		if(position_count > 0) next_pos[0].y -= 1; // sideways movement
		next_pos[position_count++] = size2(cur_position.x, cur_position.y-1);
		next_pos[position_count++] = size2(cur_position.x+1, cur_position.y-1);
	}
	if((direction & MOVE_DIRECTION::DOWN) != MOVE_DIRECTION::NONE) {
		if(cur_position.y+1 >= map_size.y) return true;
		if(position_count > 0) next_pos[0].y += 1;
		next_pos[position_count++] = size2(cur_position.x, cur_position.y+1);
		next_pos[position_count++] = size2(cur_position.x+1, cur_position.y+1);
	}
	
	if(position_count == 0 || position_count > 3) {
		if(position_count > 3) log_error("impossible movement (#pos: %u, dir: %X)!", position_count, direction);
		return true;
	}
	
	//
	const tileset::tileset_object& cur_tileset = tilesets->get_cur_tileset();
	for(size_t p = 0; p < position_count; p++) {
		const tileset::tile_object& tile_u = cur_tileset.tiles[underlay_tiles[next_pos[p].y*map_size.x + next_pos[p].x]];
		const tileset::tile_object& tile_o = cur_tileset.tiles[overlay_tiles[next_pos[p].y*map_size.x + next_pos[p].x]];
		
		// first, check special types
		bool collision_override = false;
		for(size_t i = 0; i < 2; i++) {
			const tileset::tile_object& cur_tile = (i == 0 ? tile_u : tile_o);
			switch(cur_tile.special_2) {
				case 0x0020:
					collision_override = (cur_tile.collision == 0);
					break;
				case 0x0204:
				case 0x0284:
				case 0x0080:
				case 0x0100:
				case 0x0304:
					return true;
				default:
					break;
			}
		}
		
		// if we have a collision override, consider the other tile (if count == 2) or default to false (no collision)
		if(collision_override) continue;
		
		// check collision byte
		if(tile_u.collision != 0 ||
		   tile_o.collision != 0) {
			return true;
		}
	}
	
	return false;
}

float map2d::get_tile_size() const {
	return tile_size * conf::get<float>("map.scale");
}

const size_t& map2d::get_palette() const {
	return map_palette;
}

const float2& map2d::get_screen_position() const {
	return screen_position;
}

map_events& map2d::get_map_events() {
	return mevents;
}

void map2d::set_initial_position(const size2& init_pos) {
	set_pos(init_pos.x, init_pos.y);
	screen_position = compute_target_position();
}

void map2d::set_player(player2d* player) {
	p2d = player;
}
