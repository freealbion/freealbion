/*
 *  Albion Remake
 *  Copyright (C) 2007 - 2011 Florian Ziesche
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
#include "npc3d.h"

/*! map3d constructor
 */
map3d::map3d(labdata* lab_data_, xld* maps1, xld* maps2, xld* maps3) : lab_data(lab_data_) {
	map_xlds[0] = maps1;
	map_xlds[1] = maps2;
	map_xlds[2] = maps3;

	tile_size = std_tile_size;
	map_loaded = false;
	cur_map_num = (~0);
	mnpcs = NULL;
	npcs_model = NULL;
	npc_object_count = 0;

	ow_tiles = NULL;
	floor_tiles = NULL;
	ceiling_tiles = NULL;
	
	wall_vertices = NULL;
	wall_tex_coords = NULL;
	wall_indices = NULL;
	wall_model = NULL;
	wall_sides = NULL;

	fc_vertices = NULL;
	fc_tex_coords = NULL;
	fc_indices = NULL;
	fc_tiles_model = NULL;
	
	obj_vertices = NULL;
	obj_ws_positions = NULL;
	obj_tex_coords = NULL;
	obj_indices = NULL;
	objects_model = NULL;

	last_tile_animation = SDL_GetTicks();

	bg3d = new background3d();
	bg_loaded = false;

	//
	player_light = new light(e, 0.0f, 20.0f, 0.0f);
	
#if 1 // sun
	player_light->set_lambient(1.0f, 1.0f, 1.0f);
	player_light->set_ldiffuse(0.0f, 0.0f, 0.0f);
	player_light->set_lspecular(0.0f, 0.0f, 0.0f);
	player_light->set_constant_attenuation(1.0f);
	player_light->set_linear_attenuation(0.0f);
	player_light->set_quadratic_attenuation(0.0f);
#elif 1
	player_light->set_lambient(0.1f, 0.05f, 0.05f);
	player_light->set_ldiffuse(0.3f, 0.3f, 0.2f);
	player_light->set_lspecular(0.2f, 0.15f, 0.1f);
	player_light->set_constant_attenuation(0.2f);
	player_light->set_linear_attenuation(0.03f);
	player_light->set_quadratic_attenuation(0.0f);
#else // dungeon
	player_light->set_lambient(0.2f, 0.2f, 0.2f);
	player_light->set_ldiffuse(1.0f, 1.0f, 1.0f);
	//player_light->set_lspecular(1.0f, 1.0f, 1.0f);
	player_light->set_lspecular(0.1f, 0.1f, 0.1f);
	player_light->set_constant_attenuation(0.2f);
	player_light->set_linear_attenuation(0.04f);
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
	delete bg3d;
	unload();
}

void map3d::load(const size_t& map_num) {
	if(cur_map_num == map_num) return;
	a2e_debug("loading map %u ...", map_num);
	unload();

	//
	const xld::xld_object* object = xld::get_object(map_num, map_xlds, 300);
	if(object == NULL) {
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
	
	cout << 0 << ": " << (size_t)cur_map_data[0] << endl;
	cout << 3 << ": " << (size_t)cur_map_data[3] << endl;
	cout << 7 << ": " << (size_t)cur_map_data[7] << endl;
	cout << 9 << ": " << (size_t)cur_map_data[9] << endl;
	cout << "map_palette: " << map_palette << endl;

	for(size_t i = 0; i < 32; i++) {
		cout << ((size_t)cur_map_data[i] < 16 ? "0":"") << hex << (size_t)cur_map_data[i] << dec;
	}
	cout << endl;
	
	// npc/monster info
	if(npc_data_len == 0x00) npc_data_len = 32 * 10;
	else if(npc_data_len == 0x40) npc_data_len = 96 * 10;
	else npc_data_len *= 10;
	
	// actual map data
	ow_tiles = new unsigned int[map_size.x*map_size.y];
	floor_tiles = new unsigned int[map_size.x*map_size.y];
	ceiling_tiles = new unsigned int[map_size.x*map_size.y];

	lab_data->load((cur_labdata_num < 100 ? cur_labdata_num-1 : cur_labdata_num), map_palette-1);

	if(lab_data->get_lab_info()->background != 0) {
		bg3d->load(lab_data->get_lab_info()->background-1, map_palette-1);
	}
	else bg3d->unload();
	sce->add_model(bg3d);
	bg_loaded = true;

	//
	/*
		7 = y streckung (*2.25), kein einfluss auf objekte?
		8 = normale tile/textur größe ### y * 1.87, kein einfluss auf objekte?
		9 = normale wall textur größe, senkung der decke, einfluss auf objekte?
		A = x streckung (*2.25), kein einfluss auf objekte?
	*/
	const float floor_height = 0.0f;
	float ceiling_height = tile_size;
	/*switch(lab_data->get_lab_info()->scale_2) {
		case 0x7:
			ceiling_height *= 2.0f;
			break;
		case 0x8:
			break;
		case 0x9:
			ceiling_height *= 0.5f;
			break;
		case 0xA:
			ceiling_height *= 0.5f;
			break;
		default:
			break;
	}*/

	// map scaling ...
	float2 map_scale(1.0f, 1.0f);
	
	// scale byte 1
	/*if(lab_data->get_lab_info()->scale_1 == 0x1) {
		// everything is fine, nothing to see here, move on ...
	}
	else if(lab_data->get_lab_info()->scale_1 == 0x2) {
		map_scale.y = float(lab_data->get_wall(101)->x_size) / float(lab_data->get_wall(101)->y_size);
		
		map_scale.x *= 1.2f;
		map_scale.y *= 1.2f;
	}
	else if(lab_data->get_lab_info()->scale_1 == 0x3) {
		map_scale.y = float(lab_data->get_wall(101)->x_size) / float(lab_data->get_wall(101)->y_size);
		
		map_scale.y *= 1.5f;
	}*/

	// scale byte 2
	/*if(lab_data->get_lab_info()->scale_2 == 0x7) map_scale.y = 2.25f;
	//if(lab_data->get_lab_info()->scale_2 == 0x8) map_scale.y = 1.87f;
	if(lab_data->get_lab_info()->scale_2 == 0x8) map_scale.y = 1.2f;
	if(lab_data->get_lab_info()->scale_2 == 0xA) map_scale.x = 2.25f;*/
	
	// TODO: apply map scale and new size to all other classes (player3d, collision detection!)

	tile_size = std_tile_size * map_scale.x;
	ceiling_height = map_scale.y * float(lab_data->get_wall(101)->y_size)/8.0f;

	// get tiles
	const size_t map_data_offset = npc_data_len + header_len;
	const size_t map_data_len = (map_size.x * map_size.y) * 3;
	for(size_t i = map_data_offset, tile_num = 0; i < (map_data_offset + map_data_len); i+=3, tile_num++) {
		ow_tiles[tile_num] = (unsigned int)cur_map_data[i];
		floor_tiles[tile_num] = (unsigned int)cur_map_data[i+1];
		ceiling_tiles[tile_num] = (unsigned int)cur_map_data[i+2];
	}
	
	// events
	const size_t events_offset = map_data_offset + map_data_len;
	mevents.load(object, events_offset, map_size);

	// load npc/monster data (has to be done here, b/c we need info that is available only after all events have been loaded)
	mnpcs = new map_npcs();
	mnpcs->load(cur_map_data, mevents.get_end_offset());
	const vector<map_npcs::map_npc*>& npc_data = mnpcs->get_npcs();
	for(vector<map_npcs::map_npc*>::const_iterator npc_iter = npc_data.begin(); npc_iter != npc_data.end(); npc_iter++) {
		npcs.push_back(new npc3d(this));
		npcs.back()->set_npc_data(*npc_iter);
	}
	
	// create npc object models
	unsigned int npc_count = (unsigned int)npcs.size();
	npc_object_count = 0;
	for(unsigned int i = 0; i < npc_count; i++) {
		const labdata::lab_object* obj = lab_data->get_object(npc_data[i]->object_num);
		if(obj == NULL) continue;
		
		npc_object_count += obj->sub_object_count;
	}
	
	npcs_vertices = new float3[npc_object_count*4];
	npcs_ws_positions = new float3[npc_object_count*4];
	npcs_tex_coords = new float2[npc_object_count*4];
	npcs_indices = new index3*[1];
	npcs_indices[0] = new index3[npc_object_count*2];
	unsigned int* npcs_index_count = new unsigned int[1];
	npcs_index_count[0] = npc_object_count*2;
	
	unsigned int npc_index = 0, npc_index_num = 0;
	for(unsigned int i = 0; i < npc_count; i++) {
		const labdata::lab_object* obj = lab_data->get_object(npc_data[i]->object_num);
		if(obj == NULL) continue;
		
		//
		for(size_t so = 0; so < obj->sub_object_count; so++) {
			const labdata::lab_object_info* sub_object = obj->sub_objects[so];
			const float2 obj_size = float2(sub_object->x_size, sub_object->y_size);
			const float2 obj_scale = float2(sub_object->x_scale, sub_object->y_scale);
			const float2& tc_b = sub_object->tex_coord_begin[0];
			const float2& tc_e = sub_object->tex_coord_end[0];
			const float x_size = float(sub_object->x_scale)/32.0f;
			const float y_size = float(sub_object->y_scale)/32.0f;
			float3 offset = float3(obj->offset[so].x, -obj->offset[so].y, obj->offset[so].z)/32.0f;
			
			if(sub_object->type & 0x4) {
				if(obj->offset[so].z == 0) {
					npcs_vertices[npc_index + 0].set((-x_size/2.0f), 0.001f, (-y_size/2.0f));
					npcs_vertices[npc_index + 1].set((-x_size/2.0f), 0.001f, (y_size/2.0f));
					npcs_vertices[npc_index + 2].set((x_size/2.0f), 0.001f, (-y_size/2.0f));
					npcs_vertices[npc_index + 3].set((x_size/2.0f), 0.001f, (y_size/2.0f));
				}
				else {
					npcs_vertices[npc_index + 0].set((x_size/2.0f), 0.0f, (-y_size/2.0f));
					npcs_vertices[npc_index + 1].set((x_size/2.0f), 0.0f, (y_size/2.0f));
					npcs_vertices[npc_index + 2].set((-x_size/2.0f), 0.0f, (-y_size/2.0f));
					npcs_vertices[npc_index + 3].set((-x_size/2.0f), 0.0f, (y_size/2.0f));
				}
			}
			else {
				npcs_vertices[npc_index + 0].set((-x_size/2.0f), (y_size/2.0f), 0.0f);
				npcs_vertices[npc_index + 1].set((-x_size/2.0f), (-y_size/2.0f), 0.0f);
				npcs_vertices[npc_index + 2].set((x_size/2.0f), (y_size/2.0f), 0.0f);
				npcs_vertices[npc_index + 3].set((x_size/2.0f), (-y_size/2.0f), 0.0f);
				offset.z += (y_size/2.0f);
			}
			
			const float2 npc_pos = float2(*npc_data[i]->position);
			npcs_ws_positions[npc_index + 0] = float3(npc_pos.x*tile_size + offset.x, offset.z, offset.y + (npc_pos.y+1.0f)*tile_size);
			npcs_ws_positions[npc_index + 1] = float3(npc_pos.x*tile_size + offset.x, offset.z, offset.y + (npc_pos.y+1.0f)*tile_size);
			npcs_ws_positions[npc_index + 2] = float3(npc_pos.x*tile_size + offset.x, offset.z, offset.y + (npc_pos.y+1.0f)*tile_size);
			npcs_ws_positions[npc_index + 3] = float3(npc_pos.x*tile_size + offset.x, offset.z, offset.y + (npc_pos.y+1.0f)*tile_size);
			
			npcs_tex_coords[npc_index + 0].set(tc_e.x, tc_b.y);
			npcs_tex_coords[npc_index + 1].set(tc_e.x, tc_e.y);
			npcs_tex_coords[npc_index + 2].set(tc_b.x, tc_b.y);
			npcs_tex_coords[npc_index + 3].set(tc_b.x, tc_e.y);
			
			npcs_indices[0][npc_index_num*2].set(npc_index + 1, npc_index + 0, npc_index + 2);
			npcs_indices[0][npc_index_num*2 + 1].set(npc_index + 2, npc_index + 3, npc_index + 1);
			
			npc_index_num++;
			npc_index+=4;
		}
	}
	
	npcs_model = new map_objects();
	npcs_model->load_from_memory(1, npc_object_count*4, npcs_vertices, npcs_tex_coords, npcs_index_count, npcs_indices);
	npcs_model->set_ws_positions(npcs_ws_positions);
	npcs_model->set_material(lab_data->get_object_material());
	sce->add_model(npcs_model);
	
	// compute which wall sides we need
	wall_sides = new unsigned char[map_size.x*map_size.y];
	for(size_t y = 0; y < map_size.y; y++) {
		for(size_t x = 0; x < map_size.x; x++) {
			if(ow_tiles[y*map_size.x + x] >= 101) {
				unsigned char& side = wall_sides[y*map_size.x + x];
				const labdata::lab_wall* wall = lab_data->get_wall(ow_tiles[y*map_size.x + x]);
				if(wall == NULL) continue;
				
				const labdata::lab_wall* walls[] = {
					(y > 0 ? lab_data->get_wall(ow_tiles[(y-1)*map_size.x + x]) : NULL),				// north
					(x != map_size.x-1 ? lab_data->get_wall(ow_tiles[y*map_size.x + x+1]) : NULL),		// east
					(y != map_size.y-1 ? lab_data->get_wall(ow_tiles[(y+1)*map_size.x + x]) : NULL),	// south
					(x > 0 ? lab_data->get_wall(ow_tiles[y*map_size.x + x-1]) : NULL),					// west
				};
				
				side = 0;
				for(size_t i = 0; i < 4; i++) {
					// TODO: does >=0x20 apply to all kinds of transparency?
					if(walls[i] == NULL ||
					   (wall->type < 0x20 && walls[i]->type >= 0x20)) {
						side |= (unsigned char)(1 << i);
					}
				}
			}
		}
	}
	
	// create map models
	unsigned int floor_count = 0, ceiling_count = 0, wall_count = 0, wall_cutalpha_count = 0, wall_transp_obj_count = 0, wall_transp_count = 0, object_count = 0, sub_object_count = 0;
	wall_ani_count = 0;
	fc_ani_count = 0;
	obj_ani_count = 0;
	for(size_t y = 0; y < map_size.y; y++) {
		for(size_t x = 0; x < map_size.x; x++) {
			if(floor_tiles[y*map_size.x + x] > 0) {
				const labdata::lab_floor* floor = lab_data->get_floor(floor_tiles[y*map_size.x + x]);
				if(floor->animation > 1) fc_ani_count++;

				floor_count++;
			}
			if(ceiling_tiles[y*map_size.x + x] > 0) {
				const labdata::lab_floor* ceiling = lab_data->get_floor(ceiling_tiles[y*map_size.x + x]);
				if(ceiling->animation > 1) fc_ani_count++;

				ceiling_count++;
			}
			if(ow_tiles[y*map_size.x + x] >= 101) {
				const labdata::lab_wall* wall = lab_data->get_wall(ow_tiles[y*map_size.x + x]);
				if(wall == NULL) continue;
				if(wall->type & labdata::WT_TRANSPARENT) wall_transp_obj_count++;
								
				unsigned char& side = wall_sides[y*map_size.x + x];
				for(size_t i = 0; i < 4; i++) {
					if((side & (unsigned char)(1 << i)) > 0) {
						if(wall->animation > 1) wall_ani_count++;
						if(wall->type & labdata::WT_TRANSPARENT) wall_transp_count++;
						else if(wall->type & labdata::WT_CUT_ALPHA) wall_cutalpha_count++;
						wall_count++;
					}
				}
			}
			if(ow_tiles[y*map_size.x + x] > 0 && ow_tiles[y*map_size.x + x] < 101) {
				const labdata::lab_object* obj = lab_data->get_object(ow_tiles[y*map_size.x + x]);
				if(obj != NULL) {
					if(obj->animated) obj_ani_count += obj->sub_object_count;

					sub_object_count += obj->sub_object_count;
					object_count++;
				}
			}
		}
	}
	wall_transp_obj_count *= 2; // transparent walls are double-sided
	
	unsigned int fc_count = floor_count + ceiling_count;

	// map model
	wall_vertices = new float3[wall_count*4];
	wall_tex_coords = new float2[wall_count*4];
	wall_indices = new index3*[1+wall_transp_obj_count];
	unsigned int* wall_index_count = new unsigned int[1+wall_transp_obj_count];
	wall_indices[0] = new index3[(wall_count+wall_cutalpha_count-wall_transp_count)*2];
	wall_index_count[0] = (unsigned int)(wall_count+wall_cutalpha_count-wall_transp_count)*2;

	// floors/ceilings model
	fc_vertices = new float3[fc_count*4];
	fc_tex_coords = new float2[fc_count*4];
	fc_indices = new index3*[1];
	fc_indices[0] = new index3[fc_count*2];
	unsigned int* fc_index_count = new unsigned int[1];
	fc_index_count[0] = (unsigned int)fc_count*2;
	
	// map objects
	obj_vertices = new float3[sub_object_count*4];
	obj_ws_positions = new float3[sub_object_count*4];
	obj_tex_coords = new float2[sub_object_count*4];
	obj_indices = new index3*[1];
	obj_indices[0] = new index3[sub_object_count*2];
	unsigned int* obj_index_count = new unsigned int[1];
	obj_index_count[0] = (unsigned int)sub_object_count*2;
	
	unsigned int fc_num = 0, wall_num = 0, object_num = 0;

	// set animation offset (this will be used later for updating the texture coordinates)
	fc_ani_offset = (fc_count - fc_ani_count)*4;
	wall_ani_offset = (wall_count - wall_ani_count)*4;
	obj_ani_offset = (sub_object_count - obj_ani_count)*4;
	unsigned int fc_ani_num = 0, wall_ani_num = 0, obj_ani_num = 0;
	unsigned int fc_static_num = 0, wall_static_num = 0, obj_static_num = 0;
	unsigned int wall_subobj = 0;
	
	for(unsigned int y = 0; y < (unsigned int)map_size.y; y++) {
		for(unsigned int x = 0; x < (unsigned int)map_size.x; x++) {
			// floors/ceilings
			for(size_t i = 0; i < 2; i++) {
				const unsigned int* tiles = (i == 0 ? floor_tiles : ceiling_tiles);
				if(tiles[y*map_size.x + x] > 0) {
					const labdata::lab_floor* tile_data = lab_data->get_floor(tiles[y*map_size.x + x]);
					unsigned int fc_index = fc_static_num*4;
					if(tile_data->animation > 1) {
						fc_index = fc_ani_offset + fc_ani_num*4;
						fc_ani_num++;
						animated_tiles.push_back(make_tuple(0, tiles[y*map_size.x + x], uint2(x, y)));
					}
					else fc_static_num++;

					const float& theight = (i == 0 ? floor_height : ceiling_height);
					fc_vertices[fc_index + 0].set(float(x)*tile_size, theight, float(y)*tile_size);
					fc_vertices[fc_index + 1].set(float(x)*tile_size + tile_size, theight, float(y)*tile_size);
					fc_vertices[fc_index + 2].set(float(x)*tile_size + tile_size, theight, float(y)*tile_size + tile_size);
					fc_vertices[fc_index + 3].set(float(x)*tile_size, theight, float(y)*tile_size + tile_size);
					
					const float2& tc_b = tile_data->tex_info[0].tex_coord_begin;
					const float2& tc_e = tile_data->tex_info[0].tex_coord_end;
					fc_tex_coords[fc_index + 0].set(tc_b.x, tc_b.y);
					fc_tex_coords[fc_index + 1].set(tc_e.x, tc_b.y);
					fc_tex_coords[fc_index + 2].set(tc_e.x, tc_e.y);
					fc_tex_coords[fc_index + 3].set(tc_b.x, tc_e.y);

					const uint2 idx_order = (i == 0 ? uint2(2, 0) : uint2(0, 2));
					fc_indices[0][fc_num*2].set(fc_index + idx_order.x, fc_index + 1, fc_index + idx_order.y);
					fc_indices[0][fc_num*2 + 1].set(fc_index + idx_order.y, fc_index + 3, fc_index + idx_order.x);
					fc_num++;
				}
			}
			// walls
			if(ow_tiles[y*map_size.x + x] >= 101) {
				const labdata::lab_wall* tile_data = lab_data->get_wall(ow_tiles[y*map_size.x + x]);
				if(tile_data == NULL) continue;
				
				const unsigned char& side = wall_sides[y*map_size.x + x];
				unsigned int side_count = 0;
				for(size_t i = 0; i < 4; i++) {
					if((side & (unsigned char)(1 << i)) != 0) side_count++;
				}
				
				bool transp_wall = false;
				if(tile_data->type & labdata::WT_TRANSPARENT) {
					wall_subobj+=2;
					transp_wall = true;
					wall_indices[wall_subobj-1] = new index3[side_count*2];
					wall_index_count[wall_subobj-1] = side_count*2;
					wall_indices[wall_subobj] = new index3[side_count*2];
					wall_index_count[wall_subobj] = side_count*2;
				}
				
				if(side_count == 0) continue;

				unsigned int wall_index = wall_static_num*4;
				if(tile_data->animation > 1) {
					wall_index = wall_ani_offset + wall_ani_num*4;
					wall_ani_num+=side_count;
					animated_tiles.push_back(make_tuple(1, ow_tiles[y*map_size.x + x], uint2(x, y)));
				}
				else {
					wall_static_num+=side_count;
				}

				const float y_size = map_scale.y * float(tile_data->y_size)/8.0f;

				const float2& tc_b = tile_data->tex_coord_begin[0];
				const float2& tc_e = tile_data->tex_coord_end[0];
				unsigned int wall_transp_num = 0;
				for(size_t i = 0; i < 4; i++) {
					// only add necessary walls
					if((side & (unsigned char)(1 << i)) == 0) continue;
					
					switch(i) {
						case 0: // north
							wall_vertices[wall_index + 0].set(float(x)*tile_size, y_size, float(y)*tile_size);
							wall_vertices[wall_index + 1].set(float(x)*tile_size, floor_height, float(y)*tile_size);
							wall_vertices[wall_index + 2].set(float(x)*tile_size + tile_size, y_size, float(y)*tile_size);
							wall_vertices[wall_index + 3].set(float(x)*tile_size + tile_size, floor_height, float(y)*tile_size);
							break;
						case 1: // east
							wall_vertices[wall_index + 0].set(float(x)*tile_size + tile_size, y_size, float(y)*tile_size);
							wall_vertices[wall_index + 1].set(float(x)*tile_size + tile_size, floor_height, float(y)*tile_size);
							wall_vertices[wall_index + 2].set(float(x)*tile_size + tile_size, y_size, float(y)*tile_size + tile_size);
							wall_vertices[wall_index + 3].set(float(x)*tile_size + tile_size, floor_height, float(y)*tile_size + tile_size);
							break;
						case 2: // south
							wall_vertices[wall_index + 0].set(float(x)*tile_size + tile_size, y_size, float(y)*tile_size + tile_size);
							wall_vertices[wall_index + 1].set(float(x)*tile_size + tile_size, floor_height, float(y)*tile_size + tile_size);
							wall_vertices[wall_index + 2].set(float(x)*tile_size, y_size, float(y)*tile_size + tile_size);
							wall_vertices[wall_index + 3].set(float(x)*tile_size, floor_height, float(y)*tile_size + tile_size);
							break;
						case 3: // west
							wall_vertices[wall_index + 0].set(float(x)*tile_size, y_size, float(y)*tile_size + tile_size);
							wall_vertices[wall_index + 1].set(float(x)*tile_size, floor_height, float(y)*tile_size + tile_size);
							wall_vertices[wall_index + 2].set(float(x)*tile_size, y_size, float(y)*tile_size);
							wall_vertices[wall_index + 3].set(float(x)*tile_size, floor_height, float(y)*tile_size);
							break;
						default: break;
					}

					wall_tex_coords[wall_index + 0].set(tc_b.x, tc_e.y);
					wall_tex_coords[wall_index + 1].set(tc_e.x, tc_e.y);
					wall_tex_coords[wall_index + 2].set(tc_b.x, tc_b.y);
					wall_tex_coords[wall_index + 3].set(tc_e.x, tc_b.y);
				
					if(transp_wall) {
						// side #1
						wall_indices[wall_subobj-1][wall_transp_num*2].set(wall_index + 1, wall_index + 0, wall_index + 2);
						wall_indices[wall_subobj-1][wall_transp_num*2 + 1].set(wall_index + 2, wall_index + 3, wall_index + 1);
						
						// side #2
						wall_indices[wall_subobj][wall_transp_num*2].set(wall_index + 2, wall_index + 0, wall_index + 1);
						wall_indices[wall_subobj][wall_transp_num*2 + 1].set(wall_index + 1, wall_index + 3, wall_index + 2);
						
						wall_transp_num++;
					}
					else {
						wall_indices[0][wall_num*2].set(wall_index + 1, wall_index + 0, wall_index + 2);
						wall_indices[0][wall_num*2 + 1].set(wall_index + 2, wall_index + 3, wall_index + 1);
						wall_num++;
						
						// draw both sides of an alpha tested wall
						if(tile_data->type & labdata::WT_CUT_ALPHA) {
							wall_indices[0][wall_num*2].set(wall_index + 2, wall_index + 0, wall_index + 1);
							wall_indices[0][wall_num*2 + 1].set(wall_index + 1, wall_index + 3, wall_index + 2);
							wall_num++;
						}
					}
					
					wall_index+=4;
				}
			}
			// objects
			if(ow_tiles[y*map_size.x + x] > 0 && ow_tiles[y*map_size.x + x] < 101) {
				const labdata::lab_object* obj = lab_data->get_object(ow_tiles[y*map_size.x + x]);
				if(obj == NULL) continue;

				unsigned int object_index = object_num*4;
				if(obj->animated) {
					object_index = obj_ani_offset + obj_ani_num*4;
					obj_ani_num += obj->sub_object_count;
					animated_tiles.push_back(make_tuple(2, ow_tiles[y*map_size.x + x], uint2(x, y)));
				}
				else object_num += obj->sub_object_count;

				for(size_t i = 0; i < obj->sub_object_count; i++) {
					//
					const labdata::lab_object_info* sub_object = obj->sub_objects[i];
					const float2 obj_size = float2(sub_object->x_size, sub_object->y_size);
					const float2 obj_scale = float2(sub_object->x_scale, sub_object->y_scale);
					const float2& tc_b = sub_object->tex_coord_begin[0];
					const float2& tc_e = sub_object->tex_coord_end[0];
					const float x_size = float(sub_object->x_scale)/32.0f;
					const float y_size = float(sub_object->y_scale)/32.0f;
					float3 offset = float3(obj->offset[i].x, -obj->offset[i].y, obj->offset[i].z)/32.0f;

					if(sub_object->type & 0x4) {
						if(obj->offset[i].z == 0) {
							obj_vertices[object_index + 0].set((-x_size/2.0f), 0.001f, (-y_size/2.0f));
							obj_vertices[object_index + 1].set((-x_size/2.0f), 0.001f, (y_size/2.0f));
							obj_vertices[object_index + 2].set((x_size/2.0f), 0.001f, (-y_size/2.0f));
							obj_vertices[object_index + 3].set((x_size/2.0f), 0.001f, (y_size/2.0f));
						}
						else {
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
				
					obj_indices[0][obj_static_num*2].set(object_index + 1, object_index + 0, object_index + 2);
					obj_indices[0][obj_static_num*2 + 1].set(object_index + 2, object_index + 3, object_index + 1);

					obj_static_num++;
					object_index+=4;
				}
			}
		}
	}
	
	//cout << ":: #triangles: " << (tile_count*2) << endl;
	//cout << ":: #obj-triangles: " << (sub_object_count*2) << endl;

	fc_tiles_model = new map_tiles();
	fc_tiles_model->load_from_memory(1, fc_count*4, fc_vertices, fc_tex_coords, fc_index_count, fc_indices);
	fc_tiles_model->set_material(lab_data->get_fc_material());
	sce->add_model(fc_tiles_model);

	wall_model = new map_tiles();
	wall_model->load_from_memory(1+wall_transp_obj_count, wall_count*4, wall_vertices, wall_tex_coords, wall_index_count, wall_indices);
	wall_model->set_material(lab_data->get_wall_material());
	sce->add_model(wall_model);
	
	// make wall sub-objects transparent
	vector<size_t> transp_subobjs;
	for(size_t i = 0; i < wall_transp_obj_count; i++) {
		wall_model->set_transparent(1+i, true);
		transp_subobjs.push_back(1+i);
	}
	lab_data->get_wall_material()->copy_object_mapping(0, transp_subobjs);
	
	objects_model = new map_objects();
	objects_model->load_from_memory(1, sub_object_count*4, obj_vertices, obj_tex_coords, obj_index_count, obj_indices);
	objects_model->set_ws_positions(obj_ws_positions);
	objects_model->set_material(lab_data->get_object_material());
	sce->add_model(objects_model);

	// don't delete model data, these are taken care of by a2estatic/a2emodel now!
	
#if 0
	// lights (this is only for testing purposes atm)
	const float half_tile_size = tile_size * 0.5f;
	for(size_t y = 0; y < map_size.y; y++) {
		for(size_t x = 0; x < map_size.x; x++) {
			const unsigned int& tile_obj = ow_tiles[y*map_size.x + x];
			if(tile_obj > 0 && tile_obj < 101) {
				if((map_num == 183 || map_num == 135) &&
				   (tile_obj == 8 || tile_obj == 9 || tile_obj == 10 || tile_obj == 39)) {
					light* obj_light = new light(e, float(x)*tile_size + half_tile_size, 18.0f, float(y)*tile_size + half_tile_size);
					obj_light->set_lambient(0.2f, 0.2f, 0.02f);
					obj_light->set_ldiffuse(0.8f, 0.8f, 0.1f);
					obj_light->set_lspecular(0.2f, 0.2f, 0.05f);
					obj_light->set_constant_attenuation(0.0f);
					//obj_light->set_linear_attenuation(0.008f);
					//obj_light->set_quadratic_attenuation(0.005f);
					obj_light->set_linear_attenuation(0.06f);
					obj_light->set_quadratic_attenuation(0.0f);
					obj_light->set_spot_direction(0.0f, 0.0f, 0.0f);
					obj_light->set_spot_cutoff(0.0f);
					obj_light->set_enabled(true);
					obj_light->set_spot_light(false);
					sce->add_light(obj_light);
					scene_lights.push_back(obj_light);
				}
			}
		}
	}
	cout << "#lights: " << scene_lights.size() << endl;
#endif

	// and finally:
	map_loaded = true;
	cur_map_num = map_num;
	a2e_debug("map #%u loaded!", map_num);
}

void map3d::unload() {
	if(wall_model != NULL) {
		sce->delete_model(wall_model);
		delete wall_model;
		wall_model = NULL;
		
		// can be NULLed now
		wall_vertices = NULL;
		wall_tex_coords = NULL;
		wall_indices = NULL;
	}
	if(fc_tiles_model != NULL) {
		sce->delete_model(fc_tiles_model);
		delete fc_tiles_model;
		fc_tiles_model = NULL;
		
		// can be NULLed now
		fc_vertices = NULL;
		fc_tex_coords = NULL;
		fc_indices = NULL;
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
	if(npcs_model != NULL) {
		sce->delete_model(npcs_model);
		delete npcs_model;
		npcs_model = NULL;
		
		// can be NULLed now
		npcs_vertices = NULL;
		npcs_ws_positions = NULL;
		npcs_tex_coords = NULL;
		npcs_indices = NULL;
		npc_object_count = 0;
	}
	if(wall_sides != NULL) {
		delete [] wall_sides;
		wall_sides = NULL;
	}
	
	//
	for(vector<npc3d*>::iterator npc_iter = npcs.begin(); npc_iter != npcs.end(); npc_iter++) {
		delete *npc_iter;
	}
	npcs.clear();
	if(mnpcs != NULL) {
		delete mnpcs;
		mnpcs = NULL;
	}
	
	for(auto liter = scene_lights.begin(); liter != scene_lights.end(); liter++) {
		sce->delete_light(*liter);
		delete *liter;
	}
	scene_lights.clear();
	
	mevents.unload();
	map_loaded = false;
	tile_size = std_tile_size;

	last_tile_animation = SDL_GetTicks();
	fc_ani_count = 0;
	fc_ani_offset = 0;
	wall_ani_count = 0;
	wall_ani_offset = 0;
	obj_ani_count = 0;
	obj_ani_offset = 0;
	animated_tiles.clear();
	cur_map_num = (~0);

	if(bg_loaded) {
		sce->delete_model(bg3d);
		bg_loaded = false;
	}
}

bool map3d::is_3d_map(const size_t& map_num) const {
	const xld::xld_object* object = xld::get_object(map_num, &map_xlds[0], 300);
	if(object == NULL) {
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
	
	// handle npcs
	const vector<map_npcs::map_npc*>& npc_data = mnpcs->get_npcs();
	size_t npc_num = 0, npc_index = 0;
	for(vector<npc3d*>::iterator npc_iter = npcs.begin(); npc_iter != npcs.end(); npc_iter++) {
		(*npc_iter)->handle();
		
		const labdata::lab_object* obj = lab_data->get_object(npc_data[npc_num]->object_num);
		for(size_t i = 0; i < obj->sub_object_count; i++) {
			const float y_size = float(obj->sub_objects[i]->y_scale)/32.0f;
			float3 offset = float3(obj->offset[i].x, -obj->offset[i].y, obj->offset[i].z)/32.0f;
			offset.z += (y_size/2.0f);
			float2 npc_pos = npcs[npc_num]->get_interpolated_pos();
			
			npcs_ws_positions[npc_index + 0] = float3(npc_pos.x*tile_size + offset.x, offset.z, offset.y + (npc_pos.y+1.0f)*tile_size);
			npcs_ws_positions[npc_index + 1] = float3(npc_pos.x*tile_size + offset.x, offset.z, offset.y + (npc_pos.y+1.0f)*tile_size);
			npcs_ws_positions[npc_index + 2] = float3(npc_pos.x*tile_size + offset.x, offset.z, offset.y + (npc_pos.y+1.0f)*tile_size);
			npcs_ws_positions[npc_index + 3] = float3(npc_pos.x*tile_size + offset.x, offset.z, offset.y + (npc_pos.y+1.0f)*tile_size);
			npc_index+=4;
		}
		npc_num++;
	}
	glBindBuffer(GL_ARRAY_BUFFER, npcs_model->get_vbo_ws_position_id());
	glBufferSubData(GL_ARRAY_BUFFER, 0, (npc_object_count * 4) * 3 * sizeof(float), npcs_ws_positions);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// handle animations
	if((SDL_GetTicks() - last_tile_animation) > TIME_PER_TILE_ANIMATION_FRAME) {
		last_tile_animation = SDL_GetTicks();

		lab_data->handle_animations();
		
		// map/tiles
		//size_t index = 0, ani_num = 0;
		size_t fc_index = 0, fc_ani_num = 0;
		size_t wall_index = 0, wall_ani_num = 0;
		size_t obj_index = 0, obj_ani_num = 0;
		for(auto ani_tile_iter = animated_tiles.begin(); ani_tile_iter != animated_tiles.end(); ani_tile_iter++) {
			if(get<0>(*ani_tile_iter) == 0) {
				fc_index = fc_ani_offset + fc_ani_num*4;
				fc_ani_num++;
			}
			else if(get<0>(*ani_tile_iter) == 1) {
				wall_index = wall_ani_offset + wall_ani_num*4;
				wall_ani_num++;
			}
			else if(get<0>(*ani_tile_iter) == 2) {
				obj_index = obj_ani_offset + obj_ani_num*4;
			}

			switch(get<0>(*ani_tile_iter)) {
				case 0: {
					const labdata::lab_floor* tile_data = lab_data->get_floor(get<1>(*ani_tile_iter));
					const float2& tc_b = tile_data->tex_info[tile_data->cur_ani].tex_coord_begin;
					const float2& tc_e = tile_data->tex_info[tile_data->cur_ani].tex_coord_end;
					fc_tex_coords[fc_index + 0].set(tc_b.x, tc_b.y);
					fc_tex_coords[fc_index + 1].set(tc_e.x, tc_b.y);
					fc_tex_coords[fc_index + 2].set(tc_e.x, tc_e.y);
					fc_tex_coords[fc_index + 3].set(tc_b.x, tc_e.y);
				}
				break;
				case 1: {
					const labdata::lab_wall* tile_data = lab_data->get_wall(get<1>(*ani_tile_iter));
					if(tile_data == NULL) break;
					const float2& tc_b = tile_data->tex_coord_begin[tile_data->cur_ani];
					const float2& tc_e = tile_data->tex_coord_end[tile_data->cur_ani];
					
					const uint2& pos = get<2>(*ani_tile_iter);
					const unsigned char& side = wall_sides[pos.y*map_size.x + pos.x];
					size_t side_count = 0;
					for(size_t i = 0; i < 4; i++) {
						if((side & (unsigned char)(1 << i)) != 0) side_count++;
					}
					if(side_count == 0) {
						wall_ani_num--; // decrease already increased ani num
						continue;
					}
					else wall_ani_num += side_count-1; // already inc'ed by one
					
					for(size_t i = 0; i < 4; i++) {
						// only add necessary walls
						if((side & (unsigned char)(1 << i)) == 0) continue;
						
						wall_tex_coords[wall_index + 0].set(tc_b.x, tc_e.y);
						wall_tex_coords[wall_index + 1].set(tc_e.x, tc_e.y);
						wall_tex_coords[wall_index + 2].set(tc_b.x, tc_b.y);
						wall_tex_coords[wall_index + 3].set(tc_e.x, tc_b.y);
						
						wall_index+=4;
					}
				}
				break;
				case 2: {
					const labdata::lab_object* tile_data = lab_data->get_object(get<1>(*ani_tile_iter));

					for(size_t i = 0; i < tile_data->sub_object_count; i++) {
						const float2& tc_b = tile_data->sub_objects[i]->tex_coord_begin[tile_data->sub_objects[i]->cur_ani];
						const float2& tc_e = tile_data->sub_objects[i]->tex_coord_end[tile_data->sub_objects[i]->cur_ani];

						obj_tex_coords[obj_index + 0].set(tc_e.x, tc_b.y);
						obj_tex_coords[obj_index + 1].set(tc_e.x, tc_e.y);
						obj_tex_coords[obj_index + 2].set(tc_b.x, tc_b.y);
						obj_tex_coords[obj_index + 3].set(tc_b.x, tc_e.y);
						
						obj_index+=4;
					}

					obj_ani_num+=tile_data->sub_object_count;
				}
				break;
				default: break;
			}
		}
		
		glBindBuffer(GL_ARRAY_BUFFER, fc_tiles_model->get_vbo_tex_coords());
		glBufferSubData(GL_ARRAY_BUFFER, fc_ani_offset * 2 * sizeof(float), (fc_ani_count * 4) * 2 * sizeof(float), &fc_tex_coords[fc_ani_offset]);
		
		glBindBuffer(GL_ARRAY_BUFFER, wall_model->get_vbo_tex_coords());
		glBufferSubData(GL_ARRAY_BUFFER, wall_ani_offset * 2 * sizeof(float), (wall_ani_count * 4) * 2 * sizeof(float), &wall_tex_coords[wall_ani_offset]);

		glBindBuffer(GL_ARRAY_BUFFER, objects_model->get_vbo_tex_coords());
		glBufferSubData(GL_ARRAY_BUFFER, obj_ani_offset * 2 * sizeof(float), (obj_ani_count * 4) * 2 * sizeof(float), &obj_tex_coords[obj_ani_offset]);
		
		// npcs
		npc_num = 0;
		npc_index = 0;
		for(vector<npc3d*>::iterator npc_iter = npcs.begin(); npc_iter != npcs.end(); npc_iter++) {
			const labdata::lab_object* npc_obj = lab_data->get_object(npc_data[npc_num]->object_num);
			
			for(size_t i = 0; i < npc_obj->sub_object_count; i++) {
				const float2& tc_b = npc_obj->sub_objects[i]->tex_coord_begin[npc_obj->sub_objects[i]->cur_ani];
				const float2& tc_e = npc_obj->sub_objects[i]->tex_coord_end[npc_obj->sub_objects[i]->cur_ani];
				
				npcs_tex_coords[npc_index + 0].set(tc_e.x, tc_b.y);
				npcs_tex_coords[npc_index + 1].set(tc_e.x, tc_e.y);
				npcs_tex_coords[npc_index + 2].set(tc_b.x, tc_b.y);
				npcs_tex_coords[npc_index + 3].set(tc_b.x, tc_e.y);
				npc_index+=4;
			}
			npc_num++;
		}
		
		glBindBuffer(GL_ARRAY_BUFFER, npcs_model->get_vbo_tex_coords());
		glBufferSubData(GL_ARRAY_BUFFER, 0, (npc_object_count * 4) * 2 * sizeof(float), &npcs_tex_coords[0]);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
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

bool4 map3d::collide(const MOVE_DIRECTION& direction, const size2& cur_position, const CHARACTER_TYPE& char_type) const {
	if(!map_loaded) return bool4(false);
	
	if(!conf::get<bool>("map.collision") && char_type == CT_PLAYER) return bool4(false);
	
	if(cur_position.x >= map_size.x) return bool4(true);
	if(cur_position.y >= map_size.y) return bool4(true);
	
	bool4 ret(false);
	static const size_t max_pos_count = 3;
	ssize2 next_position[max_pos_count];
	next_position[0].x = next_position[0].y = -1;
	next_position[1].x = next_position[1].y = -1;
	next_position[2].x = next_position[2].y = -1;
	size_t position_count = 0; // count == 1 if single direction, == 3 if sideways
	
	if(!((direction & MD_LEFT) && (direction & MD_RIGHT))) {
		if(direction & MD_LEFT) {
			if(cur_position.x == 0) {
				ret.x = true;
				ret.y = true;
			}
			else {
				next_position[0] = size2(cur_position.x-1, cur_position.y);
				position_count++;
			}
		}
		else if(direction & MD_RIGHT) {
			if(cur_position.x+1 >= map_size.x) {
				ret.x = true;
				ret.y = true;
			}
			else {
				next_position[0] = size2(cur_position.x+1, cur_position.y);
				position_count++;
			}
		}
	}
	
	if(!((direction & MD_UP) && (direction & MD_DOWN))) {
		if(direction & MD_UP) {
			if(cur_position.y == 0) {
				ret.x = true;
				ret.z = true;
			}
			else {
				next_position[1] = size2(cur_position.x, cur_position.y-1);
				position_count++;
			}
		}
		else if(direction & MD_DOWN) {
			if(cur_position.y+1 >= map_size.y) {
				ret.x = true;
				ret.z = true;
			}
			else {
				next_position[1] = size2(cur_position.x, cur_position.y+1);
				position_count++;
			}
		}
	}
	
	// check for sideways movement
	if(position_count == 2) {
		if((direction & MD_LEFT) && (direction & MD_UP)) {
			next_position[2] = size2(cur_position.x-1, cur_position.y-1);
			position_count++;
		}
		else if((direction & MD_LEFT) && (direction & MD_DOWN)) {
			next_position[2] = size2(cur_position.x-1, cur_position.y+1);
			position_count++;
		}
		else if((direction & MD_RIGHT) && (direction & MD_UP)) {
			next_position[2] = size2(cur_position.x+1, cur_position.y-1);
			position_count++;
		}
		else if((direction & MD_RIGHT) && (direction & MD_DOWN)) {
			next_position[2] = size2(cur_position.x+1, cur_position.y+1);
			position_count++;
		}
		
		if(position_count != max_pos_count) {
			a2e_error("impossible movement!");
			return bool4(true);
		}
	}
	
	//
	for(size_t i = 0; i < max_pos_count; i++) {
		if(next_position[i].x == -1 && next_position[i].y == -1) {
			ret[1+i] = false;
			continue;
		}
		
		const size_t index = next_position[i].y*map_size.x + next_position[i].x;
		
		const labdata::lab_floor* floor_data = NULL;
		const labdata::lab_floor* ceiling_data = NULL;
		const labdata::lab_wall* wall_data = NULL;
		const labdata::lab_object* obj_data = NULL;
		
		if(floor_tiles[index] > 0) floor_data = lab_data->get_floor(floor_tiles[index]);
		if(ceiling_tiles[index] > 0) ceiling_data = lab_data->get_floor(ceiling_tiles[index]);
		if(ow_tiles[index] >= 101) wall_data = lab_data->get_wall(ow_tiles[index]);
		// TODO: add correct object collision
		//if(ow_tiles[index] > 0 && ow_tiles[index] < 101) obj_data = lab_data->get_object(ow_tiles[index]);
		
		if(floor_data != NULL) {
			if(floor_data->collision & labdata::CT_BLOCK) {
				ret[1+i] = true;
			}
		}
		if(ceiling_data != NULL) {
			if(ceiling_data->collision & labdata::CT_BLOCK) {
				ret[1+i] = true;
			}
		}
		if(wall_data != NULL) {
			if(wall_data->collision & labdata::CT_BLOCK) {
				ret[1+i] = true;
			}
		}
		if(obj_data != NULL) {
			for(size_t so = 0; so < obj_data->sub_object_count; so++) {
				if(obj_data->sub_objects[so]->collision & labdata::CT_BLOCK) {
					ret[1+i] = true;
				}
			}
		}
		
		if(ret[1+i]) ret[0] = true;
	}

	return ret;
}

map_events& map3d::get_map_events() {
	return mevents;
}
