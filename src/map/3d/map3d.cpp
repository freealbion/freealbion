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

#include "map3d.h"

static const float tile_size = 16.0f;

/*! map3d constructor
 */
map3d::map3d(labdata* lab_data, xld* maps1, xld* maps2, xld* maps3) : lab_data(lab_data), maps1(maps1), maps2(maps2), maps3(maps3) {
	map_loaded = false;
	cur_map_num = (~0);
	mnpcs = NULL;

	ow_tiles = NULL;
	floor_tiles = NULL;
	ceiling_tiles = NULL;
	
	vertices = NULL;
	tex_coords = NULL;
	indices = NULL;
	map_model = NULL;
	
	obj_vertices = NULL;
	obj_ws_positions = NULL;
	obj_tex_coords = NULL;
	obj_indices = NULL;
	objects_model = NULL;

	//
	player_light = new light(e, 0.0f, 20.0f, 0.0f);
	
#if 1 // sun
	player_light->set_lambient(1.0f, 1.0f, 1.0f);
	player_light->set_ldiffuse(0.3f, 0.3f, 0.3f);
	player_light->set_lspecular(0.0f, 0.0f, 0.0f);
	player_light->set_constant_attenuation(1.0f);
	player_light->set_linear_attenuation(0.0f);
	player_light->set_quadratic_attenuation(0.0f);
#else // dungeon
	player_light->set_lambient(0.2f, 0.2f, 0.2f);
	player_light->set_ldiffuse(1.0f, 1.0f, 1.0f);
	player_light->set_lspecular(1.0f, 1.0f, 1.0f);
	player_light->set_constant_attenuation(0.2f);
	player_light->set_linear_attenuation(0.002f);
	player_light->set_quadratic_attenuation(0.0f);
#endif
	
	player_light->set_spot_direction(0.0f, 0.0f, 0.0f);
	player_light->set_spot_cutoff(0.0f);
	player_light->set_enabled(true);
	player_light->set_spot_light(false);
	sce->add_light(player_light);
}

/*! map3d destructor
 */
map3d::~map3d() {
	delete player_light;
	unload();
}

void map3d::load(const size_t& map_num) {
	if(cur_map_num == map_num) return;
	a2e_debug("loading map %u ...", map_num);
	unload();

	//
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
	const unsigned char* cur_map_data = object->data;
	
	// get map infos
	// header
	const size_t header_len = 10;
	size_t npc_data_len = (size_t)cur_map_data[1];
	map_size.set(cur_map_data[4], cur_map_data[5]);
	cur_labdata_num = cur_map_data[6];
	map_palette = (size_t)cur_map_data[8];
	
	// npc/monster info
	if(npc_data_len == 0x00) npc_data_len = 32 * 10;
	else if(npc_data_len == 0x40) npc_data_len = 96 * 10;
	else npc_data_len *= 10;
	
	// actual map data
	ow_tiles = new unsigned int[map_size.x*map_size.y];
	floor_tiles = new unsigned int[map_size.x*map_size.y];
	ceiling_tiles = new unsigned int[map_size.x*map_size.y];

	lab_data->load((cur_labdata_num < 100 ? cur_labdata_num-1 : cur_labdata_num), map_palette-1);

	const size_t map_data_offset = npc_data_len + header_len;
	const size_t map_data_len = (map_size.x * map_size.y) * 3;
	for(size_t i = map_data_offset, tile_num = 0; i < (map_data_offset + map_data_len); i+=3, tile_num++) {
		ow_tiles[tile_num] = (unsigned int)cur_map_data[i];
		floor_tiles[tile_num] = (unsigned int)cur_map_data[i+1];
		ceiling_tiles[tile_num] = (unsigned int)cur_map_data[i+2];
	}
	
	// events
	const size_t events_offset = map_data_offset + map_data_len;
	mevents.load(cur_map_data, events_offset, map_size);

	// load npc/monster data (has to be done here, b/c we need info that is available only after all events have been loaded)
	mnpcs = new map_npcs();
	mnpcs->load(cur_map_data, mevents.get_end_offset());
	const vector<map_npcs::map_npc*>& npc_data = mnpcs->get_npcs();
	for(vector<map_npcs::map_npc*>::const_iterator npc_iter = npc_data.begin(); npc_iter != npc_data.end(); npc_iter++) {
		// TODO: !!!
		/*npcs.push_back(new npc2d(this, npc_graphics));
		npcs.back()->set_npc_data(*npc_iter);*/
	}
	
	// create map models
	size_t floor_count = 0, ceiling_count = 0, wall_count = 0, object_count = 0, sub_object_count = 0;
	for(size_t y = 0; y < map_size.y; y++) {
		for(size_t x = 0; x < map_size.x; x++) {
			if(floor_tiles[y*map_size.x + x] > 0) floor_count++;
			if(ceiling_tiles[y*map_size.x + x] > 0) ceiling_count++;
			if(ow_tiles[y*map_size.x + x] >= 101) wall_count++;
			if(ow_tiles[y*map_size.x + x] > 0 && ow_tiles[y*map_size.x + x] < 101) {
				const labdata::lab_object* obj = lab_data->get_object(ow_tiles[y*map_size.x + x]);
				if(obj != NULL) {
					sub_object_count += obj->sub_object_count;
					object_count++;
				}
			}
		}
	}
	wall_count *= 4; // 4 walls per tile

	tile_count = floor_count + ceiling_count + wall_count;
	size_t fc_count = floor_count + ceiling_count;

	// map model
	vertices = new float3[tile_count*4];
	tex_coords = new float2[tile_count*4];
	indices = new index3*[2];
	indices[0] = new index3[fc_count*2];
	indices[1] = new index3[wall_count*2];
	unsigned int* index_count = new unsigned int[2];
	index_count[0] = fc_count*2;
	index_count[1] = wall_count*2;
	
	// map objects
	obj_vertices = new float3[sub_object_count*4];
	obj_ws_positions = new float3[sub_object_count*4];
	obj_tex_coords = new float2[sub_object_count*4];
	obj_indices = new index3*[1];
	obj_indices[0] = new index3[sub_object_count*2];
	unsigned int* obj_index_count = new unsigned int[1];
	obj_index_count[0] = sub_object_count*2;
	
	size_t num = 0, wall_num = 0, wall_offset = fc_count*4, object_num = 0, object_offset = 0;
	const float2 tile_tc_size = float2(256.0f/4096.0f);
	const float floor_height = 0.0f;
	const float ceiling_height = tile_size;
	for(size_t y = 0; y < map_size.y; y++) {
		for(size_t x = 0; x < map_size.x; x++) {
			// floors/ceilings
			for(size_t i = 0; i < 2; i++) {
				const unsigned int* tiles = (i == 0 ? floor_tiles : ceiling_tiles);
				if(tiles[y*map_size.x + x] > 0) {
					const float& theight = (i == 0 ? floor_height : ceiling_height);
					vertices[num*4 + 0].set(float(x)*tile_size, theight, float(y)*tile_size);
					vertices[num*4 + 1].set(float(x)*tile_size + tile_size, theight, float(y)*tile_size);
					vertices[num*4 + 2].set(float(x)*tile_size + tile_size, theight, float(y)*tile_size + tile_size);
					vertices[num*4 + 3].set(float(x)*tile_size, theight, float(y)*tile_size + tile_size);
				
					const float2& tc = lab_data->get_floor(tiles[y*map_size.x + x])->tex_coord;
					tex_coords[num*4 + 0].set(tc.x, tc.y);
					tex_coords[num*4 + 1].set(tc.x + tile_tc_size.x, tc.y);
					tex_coords[num*4 + 2].set(tc.x + tile_tc_size.x, tc.y + tile_tc_size.y);
					tex_coords[num*4 + 3].set(tc.x, tc.y + tile_tc_size.y);

					const size2 idx_order = (i == 0 ? size2(2, 0) : size2(0, 2));
					indices[0][num*2].set(num*4 + idx_order.x, num*4 + 1, num*4 + idx_order.y);
					indices[0][num*2 + 1].set(num*4 + idx_order.y, num*4 + 3, num*4 + idx_order.x);
					num++;
				}
			}
			// walls
			if(ow_tiles[y*map_size.x + x] >= 101) {
				size_t wall_index = wall_offset + wall_num*4;

				const float2& tc_b = lab_data->get_wall(ow_tiles[y*map_size.x + x])->tex_coord_begin;
				const float2& tc_e = lab_data->get_wall(ow_tiles[y*map_size.x + x])->tex_coord_end;
				// TODO: only add necessary walls
				for(size_t i = 0; i < 4; i++) {
					switch(i) {
						case 0:
							vertices[wall_index + 0].set(float(x)*tile_size, ceiling_height, float(y)*tile_size);
							vertices[wall_index + 1].set(float(x)*tile_size, floor_height, float(y)*tile_size);
							vertices[wall_index + 2].set(float(x)*tile_size + tile_size, ceiling_height, float(y)*tile_size);
							vertices[wall_index + 3].set(float(x)*tile_size + tile_size, floor_height, float(y)*tile_size);
							break;
						case 1:
							vertices[wall_index + 0].set(float(x)*tile_size + tile_size, ceiling_height, float(y)*tile_size + tile_size);
							vertices[wall_index + 1].set(float(x)*tile_size + tile_size, floor_height, float(y)*tile_size + tile_size);
							vertices[wall_index + 2].set(float(x)*tile_size, ceiling_height, float(y)*tile_size + tile_size);
							vertices[wall_index + 3].set(float(x)*tile_size, floor_height, float(y)*tile_size + tile_size);
							break;
						case 2:
							vertices[wall_index + 0].set(float(x)*tile_size, ceiling_height, float(y)*tile_size + tile_size);
							vertices[wall_index + 1].set(float(x)*tile_size, floor_height, float(y)*tile_size + tile_size);
							vertices[wall_index + 2].set(float(x)*tile_size, ceiling_height, float(y)*tile_size);
							vertices[wall_index + 3].set(float(x)*tile_size, floor_height, float(y)*tile_size);
							break;
						case 3:
							vertices[wall_index + 0].set(float(x)*tile_size + tile_size, ceiling_height, float(y)*tile_size);
							vertices[wall_index + 1].set(float(x)*tile_size + tile_size, floor_height, float(y)*tile_size);
							vertices[wall_index + 2].set(float(x)*tile_size + tile_size, ceiling_height, float(y)*tile_size + tile_size);
							vertices[wall_index + 3].set(float(x)*tile_size + tile_size, floor_height, float(y)*tile_size + tile_size);
							break;
						default: break;
					}
				
					tex_coords[wall_index + 0].set(tc_b.x, tc_b.y);
					tex_coords[wall_index + 1].set(tc_e.x, tc_b.y);
					tex_coords[wall_index + 2].set(tc_b.x, tc_e.y);
					tex_coords[wall_index + 3].set(tc_e.x, tc_e.y);
				
					indices[1][wall_num*2].set(wall_index + 1, wall_index + 0, wall_index + 2);
					indices[1][wall_num*2 + 1].set(wall_index + 2, wall_index + 3, wall_index + 1);

					wall_num++;
					wall_index+=4;
				}
			}
			// objects
			if(ow_tiles[y*map_size.x + x] > 0 && ow_tiles[y*map_size.x + x] < 101) {
				size_t object_index = object_offset + object_num*4;

				const labdata::lab_object* obj = lab_data->get_object(ow_tiles[y*map_size.x + x]);
				if(obj == NULL) continue;

				for(size_t i = 0; i < obj->sub_object_count; i++) {
					//
					const labdata::lab_object_info* sub_object = obj->sub_objects[i];
					const float2 obj_size = float2(sub_object->x_size, sub_object->y_size);
					const float2 obj_scale = float2(sub_object->x_scale, sub_object->y_scale);
					const float2& tc_b = sub_object->tex_coord_begin;
					const float2& tc_e = sub_object->tex_coord_end;
					const float x_size = float(sub_object->x_scale)/32.0f;
					const float y_size = float(sub_object->y_scale)/32.0f;
					float3 offset = float3(obj->offset[i].x, -obj->offset[i].y, obj->offset[i].z)/32.0f;

					if(sub_object->type & 0x4) {
						if(obj->offset[i].z == 0) {
							/*obj_vertices[object_index + 0].set((x_size/2.0f), 0.0f, (-y_size/2.0f));
							obj_vertices[object_index + 1].set((x_size/2.0f), 0.0f, (y_size/2.0f));
							obj_vertices[object_index + 2].set((-x_size/2.0f), 0.0f, (-y_size/2.0f));
							obj_vertices[object_index + 3].set((-x_size/2.0f), 0.0f, (y_size/2.0f));*/
							
							obj_vertices[object_index + 0].set((-x_size/2.0f), 0.0f, (-y_size/2.0f));
							obj_vertices[object_index + 1].set((-x_size/2.0f), 0.0f, (y_size/2.0f));
							obj_vertices[object_index + 2].set((x_size/2.0f), 0.0f, (-y_size/2.0f));
							obj_vertices[object_index + 3].set((x_size/2.0f), 0.0f, (y_size/2.0f));
						}
						else {
							/*obj_vertices[object_index + 0].set((-x_size/2.0f), 0.0f, (-y_size/2.0f));
							obj_vertices[object_index + 1].set((-x_size/2.0f), 0.0f, (y_size/2.0f));
							obj_vertices[object_index + 2].set((x_size/2.0f), 0.0f, (-y_size/2.0f));
							obj_vertices[object_index + 3].set((x_size/2.0f), 0.0f, (y_size/2.0f));*/
							
							obj_vertices[object_index + 0].set((x_size/2.0f), 0.0f, (-y_size/2.0f));
							obj_vertices[object_index + 1].set((x_size/2.0f), 0.0f, (y_size/2.0f));
							obj_vertices[object_index + 2].set((-x_size/2.0f), 0.0f, (-y_size/2.0f));
							obj_vertices[object_index + 3].set((-x_size/2.0f), 0.0f, (y_size/2.0f));
						}
					}
					else {
						obj_vertices[object_index + 0].set((-x_size/2.0f), (y_size/2.0f), 0.0f);
						obj_vertices[object_index + 1].set((-x_size/2.0f), (-y_size/2.0f), 0.0f);
						obj_vertices[object_index + 2].set((x_size/2.0f), (y_size/2.0f), 0.0f);
						obj_vertices[object_index + 3].set((x_size/2.0f), (-y_size/2.0f), 0.0f);
						offset.z += (y_size/2.0f);
					}

					obj_ws_positions[object_index + 0] = float3(float(x)*tile_size + offset.x, offset.z, offset.y + float(y+1)*tile_size);
					obj_ws_positions[object_index + 1] = float3(float(x)*tile_size + offset.x, offset.z, offset.y + float(y+1)*tile_size);
					obj_ws_positions[object_index + 2] = float3(float(x)*tile_size + offset.x, offset.z, offset.y + float(y+1)*tile_size);
					obj_ws_positions[object_index + 3] = float3(float(x)*tile_size + offset.x, offset.z, offset.y + float(y+1)*tile_size);
					
					obj_tex_coords[object_index + 0].set(tc_e.x, tc_b.y);
					obj_tex_coords[object_index + 1].set(tc_e.x, tc_e.y);
					obj_tex_coords[object_index + 2].set(tc_b.x, tc_b.y);
					obj_tex_coords[object_index + 3].set(tc_b.x, tc_e.y);
				
					obj_indices[0][object_num*2].set(object_index + 1, object_index + 0, object_index + 2);
					obj_indices[0][object_num*2 + 1].set(object_index + 2, object_index + 3, object_index + 1);

					object_num++;
					object_index+=4;
				}
			}
		}
	}
	
	cout << ":: #triangles: " << (tile_count*2) << endl;
	cout << ":: #obj-triangles: " << (sub_object_count*2) << endl;
	
	map_model = (a2estatic*)sce->create_a2emodel<a2estatic>();
	map_model->load_from_memory(2, tile_count*4, vertices, tex_coords, index_count, indices);
	map_model->set_material(lab_data->get_material());
	sce->add_model(map_model);
	
	objects_model = new map_objects();
	objects_model->load_from_memory(1, sub_object_count*4, obj_vertices, obj_tex_coords, obj_index_count, obj_indices);
	objects_model->set_ws_positions(obj_ws_positions);
	objects_model->set_material(lab_data->get_object_material());
	sce->add_model(objects_model);

	// don't delete model data, these are taken care of by a2estatic/a2emodel now!

	// and finally:
	map_loaded = true;
	cur_map_num = map_num;
	a2e_debug("map #%u loaded!", map_num);
}

void map3d::unload() {
	if(map_model != NULL) {
		sce->delete_model(map_model);
		delete map_model;
		map_model = NULL;
		
		// can be NULLed now
		vertices = NULL;
		tex_coords = NULL;
		indices = NULL;
	}
	if(objects_model != NULL) {
		sce->delete_model(objects_model);
		delete objects_model;
		objects_model = NULL;
		
		// can be NULLed now
		obj_vertices = NULL;
		obj_ws_positions = NULL;
		obj_tex_coords = NULL;
		obj_indices = NULL;
	}
	if(mnpcs != NULL) {
		delete mnpcs;
		mnpcs = NULL;
	}
	if(ow_tiles != NULL) {
		delete ow_tiles;
		ow_tiles = NULL;
	}
	if(floor_tiles != NULL) {
		delete floor_tiles;
		floor_tiles = NULL;
	}
	if(ceiling_tiles != NULL) {
		delete ceiling_tiles;
		ceiling_tiles = NULL;
	}
	//npcs.clear();
	mevents.unload();
	map_loaded = false;
}

bool map3d::is_3d_map(const size_t& map_num) const {
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
	
	return (object->data[2] == 0x01);
}

void map3d::draw() const {
	if(!map_loaded) return;
}

void map3d::handle() {
	if(!map_loaded) return;

	// attach light to camera for now (TODO: better method?)
	float3 player_pos = -float3(*e->get_position());
	player_light->set_position(player_pos);
}

const ssize3 map3d::get_tile() const {
	ssize3 ret(-1);
	const float3 cam_pos = -float3(e->get_position());
	if(cam_pos.x >= 0.0f && cam_pos.x < tile_size*map_size.x &&
		cam_pos.z >= 0.0f && cam_pos.z < tile_size*map_size.y) {
		size2 cur_pos = float2(cam_pos.x, cam_pos.z)/tile_size;
		ret = ssize3(floor_tiles[cur_pos.y*map_size.x + cur_pos.x], ceiling_tiles[cur_pos.y*map_size.x + cur_pos.x], ow_tiles[cur_pos.y*map_size.x + cur_pos.x]);
	}
	return ret;
}
