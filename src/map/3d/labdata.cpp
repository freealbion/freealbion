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

#include "labdata.h"

/*! labdata constructor
 */
labdata::labdata() {
	tex_filtering = e->get_filtering();
	
	// only use linear filtering (max!) when creating the texture (we'll add custom mip-maps later)
	custom_tex_filtering = std::min(texture_object::TF_LINEAR, tex_filtering);

	cur_labdata_num = (~0);
	lab_fc_material = NULL;
	lab_wall_material = NULL;
	lab_obj_material = NULL;

	floors_tex = t->get_dummy_texture();
	walls_tex = t->get_dummy_texture();
	objects_tex = t->get_dummy_texture();

	// load labdata
	labdata_xlds[0] = new xld("LABDATA0.XLD");
	labdata_xlds[1] = new xld("LABDATA1.XLD");
	labdata_xlds[2] = new xld("LABDATA2.XLD");

	/*for(size_t i = 0; i < 3; i++) {
		xld* x = labdata_xlds[i];
		for(size_t j = 0; j < x->get_object_count(); j++) {
			const xld::xld_object* obj = x->get_object(j);
			if(obj != NULL) {
				cout << (i*100 + j) << ": ";
				for(size_t k = 0; k < 38; k++) {
					cout.width(2);
					cout << right << hex << (size_t)obj->data[k] << dec;
					cout.width(1);
					cout << " ";
				}
				cout.unsetf(ios::adjustfield);
				cout << endl;
			}
		}
	}*/
	
	// 3d textures
	floor_xlds[0] = new xld("3DFLOOR0.XLD");
	floor_xlds[1] = new xld("3DFLOOR1.XLD");
	floor_xlds[2] = new xld("3DFLOOR2.XLD");
	object_xlds[0] = new xld("3DOBJEC0.XLD");
	object_xlds[1] = new xld("3DOBJEC1.XLD");
	object_xlds[2] = new xld("3DOBJEC2.XLD");
	object_xlds[3] = new xld("3DOBJEC3.XLD");
	overlay_xlds[0] = new xld("3DOVERL0.XLD");
	overlay_xlds[1] = new xld("3DOVERL1.XLD");
	overlay_xlds[2] = new xld("3DOVERL2.XLD");
	wall_xlds[0] = new xld("3DWALLS0.XLD");
	wall_xlds[1] = new xld("3DWALLS1.XLD");
}

/*! labdata destructor
 */
labdata::~labdata() {
	unload();

	delete labdata_xlds[0];
	delete labdata_xlds[1];
	delete labdata_xlds[2];
	
	delete floor_xlds[0];
	delete floor_xlds[1];
	delete floor_xlds[2];
	delete object_xlds[0];
	delete object_xlds[1];
	delete object_xlds[2];
	delete object_xlds[3];
	delete overlay_xlds[0];
	delete overlay_xlds[1];
	delete overlay_xlds[2];
	delete wall_xlds[0];
	delete wall_xlds[1];
}

void labdata::load(const size_t& labdata_num, const size_t& palette) {
	if(cur_labdata_num == labdata_num) return;
	a2e_debug("loading labdata %u ...", labdata_num);
	unload();

	//
	const xld::xld_object* object = xld::get_object(labdata_num, labdata_xlds, 300);
	if(object == NULL) {
		a2e_error("invalid labdata number: %u!", labdata_num);
		return;
	}
	const unsigned char* data = object->data;

	// read lab header
	info.scale_1 = data[1];
	info.camera_height = data[2];
	info.camera_target_height = data[3];
	info.background = data[6];
	info.fog_distance = data[10];
	info.max_light_strength = data[24];
	info.scale_2 = data[26];
	info.max_visible_tiles = data[30];
	info.lighting = data[34];

	/*for(size_t x = 0; x < 38; x++) {
		cout << "#" << x << ": " << (size_t)data[x] << endl;
	}
	
	cout << "lighting: " << info.lighting << endl;
	cout << "lab_control: " << info.lab_control_1 << ", " << info.lab_control_2 << endl;*/
	
	// objects
	size_t offset = 38;
	const size_t object_count = AR_GET_USINT(data, offset);
	offset += 2;
	for(size_t i = 0; i < object_count; i++) {
		//offset = 40 + i*66;

		//
		/*cout << "#" << (i+1) << ": " << endl;
		for(size_t x = 0; x < 8; x++) {
			for(size_t y = 0; y < 8; y++) {
				cout << (y == 0?"\t":"") << (size_t)data[offset+2+x*8+y] << " ";
			}
			cout << endl;
		}
		cout << endl << endl;*/
		//
		
		objects.push_back(new lab_object());
		objects.back()->type = (AUTOGFX_TYPE)data[offset];
		offset+=2;
		
		// get sub-objects
		objects.back()->sub_object_count = 0;
		for(size_t obj = 0; obj < 8; obj++) {
			size_t obj_num = AR_GET_USINT(data, offset+6);
			if(obj_num > 0) {
				// this also reorganizes the order in case there are "NULL-objects" in between
				objects.back()->object_num[objects.back()->sub_object_count] = obj_num;
				objects.back()->sub_objects[objects.back()->sub_object_count] = NULL;
				objects.back()->offset[objects.back()->sub_object_count] = size3(AR_GET_SINT(data, offset), AR_GET_SINT(data, offset+2), AR_GET_SINT(data, offset+4));
				objects.back()->sub_object_count++;
			}
			offset+=8;
		}
	}
	
	// floors
	offset = 40 + object_count*66;
	const size_t floor_count = AR_GET_USINT(data, offset);
	offset += 2;
	for(size_t i = 0; i < floor_count; i++) {
		floors.push_back(new lab_floor());

		//cout << "#############" << endl << "## floor #" << i << endl;
		
		floors.back()->collision = (COLLISION_TYPE)(data[offset]);
		//cout << "collision : " << hex << (size_t)floors.back()->collision << dec << endl;
		offset++;

		floors.back()->_unknown_collision = (data[offset] << 8) | (data[offset+1]);
		//cout << "unknown collision: " << hex << (size_t)floors.back()->_unknown_collision << dec << endl;
		// TODO: additional collision data is used, too!
		offset+=2; // unknown
		
		//cout << "unknown #1: " << hex << (size_t)(data[offset] & 0xFF) << dec << endl;
		offset++; // unknown

		floors.back()->animation = data[offset] & 0xFF;
		offset++;
		
		//cout << "unknown #2: " << hex << (size_t)(data[offset] & 0xFF) << dec << endl;
		offset++; // unknown

		floors.back()->tex_num = AR_GET_USINT(data, offset);
		offset+=2;
		//cout << "unknown #3: " << hex << (size_t)(data[offset] & 0xFF) << dec << endl;
		//cout << "unknown #4: " << hex << (size_t)(data[offset+1] & 0xFF) << dec << endl;
		offset+=2; // unknown
		
		// check if tile uses colors that are animated ...
		const size_t& tex_num = floors.back()->tex_num;
		const xld::xld_object* obj = floor_xlds[tex_num/100]->get_object(tex_num < 100 ? tex_num-1 : tex_num%100);
		const vector<size2>& animated_ranges = palettes->get_animated_ranges(palette);
		bool palette_shift = false;
		for(vector<size2>::const_iterator ani_range = animated_ranges.begin(); ani_range != animated_ranges.end(); ani_range++) {
			for(size_t p = 0; p < 64*64; p++) {
				if(obj->data[p] >= ani_range->x && obj->data[p] <= ani_range->y) {
					// ... if so, set animation count accordingly
					floors.back()->animation = (unsigned int)(ani_range->y - ani_range->x + 1);
					palette_shift = true;
					break;
				}
			}
		}

		//
		floors.back()->tex_info = new albion_texture::albion_texture_multi_xld[floors.back()->animation];
		for(unsigned int j = 0; j < floors.back()->animation; j++) {
			floors.back()->tex_info[j].tex_num = (unsigned int)tex_num;
			if(palette_shift) {
				floors.back()->tex_info[j].palette_shift = j;
			}
			else {
				floors.back()->tex_info[j].offset = j * (64*64);
			}
		}

	}
	
	// object info
	const size_t object_info_count = AR_GET_USINT(data, offset);
	offset+=2;
	for(size_t i = 0; i < object_info_count; i++) {
		//cout << ":object #" << i << ": ";
		/*for(size_t x = 0; x < 16; x++) {
			cout << (size_t)data[offset+x] << ", ";
		}
		cout << endl;*/

		object_infos.push_back(new lab_object_info());
		object_infos.back()->type = data[offset];
		offset++;

		//cout << "collision: " << hex << (size_t)(data[offset] & 0xFF) << " " << (size_t)(data[offset+1] & 0xFF) << " " << (size_t)(data[offset+2] & 0xFF) << dec << endl;
		object_infos.back()->collision = (COLLISION_TYPE)(data[offset]);
		offset++;

		object_infos.back()->_unknown_collision = (data[offset] << 8) | data[offset+1];
		offset+=2;

		object_infos.back()->texture = AR_GET_USINT(data, offset);
		offset+=2;

		object_infos.back()->animation = data[offset] & 0xFF;
		//cout << "object ani: " << object_infos.back()->animation << endl;
		offset++;

		offset++; // unknown (animation?)

		object_infos.back()->x_size = AR_GET_USINT(data, offset);
		offset+=2;
		object_infos.back()->y_size = AR_GET_USINT(data, offset);
		offset+=2;
		object_infos.back()->x_scale = AR_GET_USINT(data, offset);
		offset+=2;
		object_infos.back()->y_scale = AR_GET_USINT(data, offset);
		offset+=2;

		// check if tile uses colors that are animated ...
		const size_t& tex_num = object_infos.back()->texture;
		const xld::xld_object* obj = object_xlds[tex_num/100]->get_object(tex_num < 100 ? tex_num-1 : tex_num%100);
		const vector<size2>& animated_ranges = palettes->get_animated_ranges(palette);
		for(vector<size2>::const_iterator ani_range = animated_ranges.begin(); ani_range != animated_ranges.end(); ani_range++) {
			const size_t obj_size = object_infos.back()->x_size*object_infos.back()->y_size;
			for(size_t p = 0; p < obj_size; p++) {
				if(obj->data[p] >= ani_range->x && obj->data[p] <= ani_range->y) {
					// ... if so, set animation count accordingly
					object_infos.back()->animation = (unsigned int)(ani_range->y - ani_range->x + 1);
					object_infos.back()->palette_shift = 1;
					break;
				}
			}
		}

		object_infos.back()->tex_coord_begin = new float2[object_infos.back()->animation];
		object_infos.back()->tex_coord_end = new float2[object_infos.back()->animation];
	}
	
	// walls + overlays
	const size_t wall_count = AR_GET_USINT(data, offset);
	offset+=2;
	for(size_t i = 0; i < wall_count; i++) {
		walls.push_back(new lab_wall());

		//cout << "#############" << endl << "## wall #" << i << endl;
		/*cout << "ani: " << (size_t)(data[offset+6] & 0xFF) << endl;
		cout << "type: " << (size_t)(data[offset] & 0xFF) << endl;*/
		//cout << "collision: " << hex << (size_t)((data[offset+1] << 16) | (data[offset+2] << 8) | data[offset+3]) << dec << endl;
		/*cout << "tex: " << (size_t)AR_GET_USINT(data, offset+4) << endl;
		cout << "type: " << (size_t)(data[offset+7] & 0xFF) << endl;
		cout << "pal: " << (size_t)(data[offset+8] & 0xFF) << endl;
		cout << "#9: " << (size_t)(data[offset+9] & 0xFF) << endl;
		cout << "x: " << (size_t)AR_GET_USINT(data, offset+10) << endl;
		cout << "y: " << (size_t)AR_GET_USINT(data, offset+12) << endl;
		cout << "#over: " << (size_t)AR_GET_USINT(data, offset+14) << endl;*/
		
		//walls.back()->type = data[offset] & 0xFF;
		walls.back()->type = (data[offset] & 0xFF) & WT_JOINED;
		offset++;
		
		walls.back()->collision = (COLLISION_TYPE)(data[offset]);
		offset++;

		walls.back()->_unknown_collision = (data[offset] << 8) | data[offset+1];
		offset+=2;

		walls.back()->texture = AR_GET_USINT(data, offset);
		offset+=2;
		
		walls.back()->wall_animation = data[offset] & 0xFF;
		walls.back()->animation = walls.back()->wall_animation;
		offset++;
		walls.back()->autogfx_type = (AUTOGFX_TYPE)data[offset];
		offset++;
		walls.back()->palette_num = data[offset] & 0xFF;
		offset++;
		
		offset++; // unknown

		walls.back()->x_size = AR_GET_USINT(data, offset);
		offset+=2;
		walls.back()->y_size = AR_GET_USINT(data, offset);
		offset+=2;
		walls.back()->overlay_count = AR_GET_USINT(data, offset);
		offset+=2;
		//cout << "overlay_count: " << (size_t)walls.back()->overlay_count << endl;
		
		for(unsigned int j = 0; j < walls.back()->overlay_count; j++) {
			walls.back()->overlays.push_back(new lab_wall_overlay());
			walls.back()->overlays.back()->texture = AR_GET_USINT(data, offset);
			offset+=2;
			//cout << "overlay tex: " << walls.back()->overlays.back()->texture << endl;
			
			walls.back()->overlays.back()->animation = data[offset] & 0xFF;
			// stupid max for now, might work always though (if not: mulitply)
			walls.back()->animation = std::max(walls.back()->animation, walls.back()->overlays.back()->animation);
			//cout << "overlay animation: " << walls.back()->overlays.back()->animation << endl;
			offset++;
			
			walls.back()->overlays.back()->write_zero = (data[offset] == 0);
			offset++;

			walls.back()->overlays.back()->y_offset = AR_GET_USINT(data, offset);
			offset+=2;
			walls.back()->overlays.back()->x_offset = AR_GET_USINT(data, offset);
			offset+=2;
			walls.back()->overlays.back()->y_size = AR_GET_USINT(data, offset);
			offset+=2;
			walls.back()->overlays.back()->x_size = AR_GET_USINT(data, offset);
			offset+=2;
			
			//cout << "overlay x: " << walls.back()->overlays.back()->x_size << endl;
			//cout << "overlay y: " << walls.back()->overlays.back()->y_size << endl;
		}
		
		walls.back()->tex_coord_begin = new float2[walls.back()->animation];
		walls.back()->tex_coord_end = new float2[walls.back()->animation];
	}

	//
	cout << "# walls: " << wall_count << endl;
	cout << "# objects: " << object_count << endl;
	cout << "# floors/ceilings: " << floor_count << endl;
	
	// object -> object_info associations
	for(vector<lab_object*>::iterator obj_iter = objects.begin(); obj_iter != objects.end(); obj_iter++) {
		for(size_t sub_obj = 0; sub_obj < (*obj_iter)->sub_object_count; sub_obj++) {
			if((*obj_iter)->object_num[sub_obj] > object_infos.size()) {
				a2e_error("invalid object number %u!", (*obj_iter)->object_num[sub_obj]);
				continue;
			}
			(*obj_iter)->sub_objects[sub_obj] = object_infos[(*obj_iter)->object_num[sub_obj]-1];
			
			if((*obj_iter)->sub_objects[sub_obj]->animation > 1) {
				(*obj_iter)->animated = true;
			}
		}
	}


	// compute texture size dependent on necessary size
	const size_t max_tex_size = exts->get_max_texture_size();
	size2 floors_tex_size, walls_tex_size, objects_tex_size;
	const size_t scale_factor = scaling::get_scale_factor(conf::get<scaling::SCALE_TYPE>("map.3d.scale_type"));
	//const size_t aa_spacing_scale = (e->get_anti_aliasing() >= rtt::TAA_MSAA_2 ? 2 : 1);
	
	// compute floors tex size
	const size_t floor_spacing = 16 * scale_factor; // we sadly need that much spacing for floor tiles
	const size2 floor_size = (size2(64, 64) * scale_factor) + size2(2 * floor_spacing);
	for(size_t i = 0; i < floor_count; i++) {
		for(size_t j = 0; j < floors[i]->animation; j++) {
			floors_tex_size.x += floor_size.x;
			if(floors_tex_size.x > max_tex_size) {
				floors_tex_size.y += floor_size.y;
				floors_tex_size.x = floor_size.x;
			}
		}
	}
	floors_tex_size.x = max_tex_size;
	floors_tex_size.y += floor_size.y;
	
	// walls tex size
	const size_t wall_spacing = 16 * scale_factor;
	const size2 wall_spacing_vec = size2(wall_spacing*2);
	size_t next_y_coord = 0;
	for(size_t i = 0; i < wall_count; i++) {
		const size2 wall_size = size2(walls[i]->y_size, walls[i]->x_size)*scale_factor + wall_spacing_vec;
		
		for(size_t j = 0; j < walls[i]->animation; j++) {
			if(wall_size.y > next_y_coord) next_y_coord = wall_size.y;
			
			walls_tex_size.x += wall_size.x;
			if(walls_tex_size.x > max_tex_size) {
				walls_tex_size.y += next_y_coord;
				walls_tex_size.x = wall_size.x;
				next_y_coord = wall_size.y;
			}
		}
	}
	walls_tex_size.x = max_tex_size;
	walls_tex_size.y += next_y_coord;
	
	// objects tex size
	const size_t object_spacing = 4 * scale_factor; // seems to be enough
	const size2 object_spacing_vec = size2(object_spacing*2);
	next_y_coord = 0;
	for(size_t i = 0; i < object_info_count; i++) {
		const size2 object_size = size2(object_infos[i]->x_size, object_infos[i]->y_size)*scale_factor + object_spacing_vec;
		
		for(size_t j = 0; j < object_infos[i]->animation; j++) {
			if(object_size.y > next_y_coord) next_y_coord = object_size.y;
			
			objects_tex_size.x += object_size.x;
			if(objects_tex_size.x > max_tex_size) {
				objects_tex_size.y += next_y_coord;
				objects_tex_size.x = object_size.x;
				next_y_coord = object_size.y;
			}
		}
	}
	objects_tex_size.x = max_tex_size;
	objects_tex_size.y += next_y_coord;

	//// create textures
	// floors
	vector<albion_texture::albion_texture_info*> floors_tex_info;
	for(size_t i = 0; i < floor_count; i++) {
		for(size_t j = 0; j < floors[i]->animation; j++) {
			floors_tex_info.push_back(&floors[i]->tex_info[j]);
		}
	}
	cout << ":: creating floors texture " << floors_tex_size << " ..." << endl;
	if(floors_tex_size.x > 0 && floors_tex_size.y > 0) {
		floors_tex = albion_texture::create(MT_3D_MAP, floors_tex_size, size2(64, 64), palette, floors_tex_info, floor_xlds, albion_texture::TST_MIRROR, floor_spacing, true, tex_filtering);
	}
	else floors_tex = t->get_dummy_texture();
	conf::set<a2e_texture>("debug.texture", floors_tex);
	
	// walls
	size2 tex_offset;
	float next_y_offset = 0.0f;
	if(walls_tex_size.x > 0 && walls_tex_size.y > 0) {
		unsigned int* walls_surface = new unsigned int[walls_tex_size.x*walls_tex_size.y];
		memset(walls_surface, 0, walls_tex_size.x*walls_tex_size.y*sizeof(unsigned int));
		for(size_t i = 0; i < wall_count; i++) {
			const xld* wall_tex_xld = (walls[i]->texture < 100 ? wall_xlds[0] : wall_xlds[1]);
			const size_t wall_tex_num = (walls[i]->texture < 100 ? walls[i]->texture-1 : walls[i]->texture) % 100;
			const xld::xld_object* wall_tex_data = wall_tex_xld->get_object(wall_tex_num);
			bool write_zero = ((walls[i]->type & WT_WRITE_OVERLAY_ZERO) != 0);
			
			//
			const size2 tile_size = size2(walls[i]->y_size, walls[i]->x_size);
			const size2 scaled_tile_size = tile_size * scale_factor;
			const size2 spaced_scaled_tile_size = scaled_tile_size + wall_spacing_vec;
			unsigned char* wall_data = new unsigned char[tile_size.x*tile_size.y];
			unsigned int* data_32bpp = new unsigned int[tile_size.x*tile_size.y];
			unsigned int* scaled_data = new unsigned int[scaled_tile_size.x*scaled_tile_size.y];
						
			//
			for(size_t j = 0; j < walls[i]->animation; j++) {
				memcpy(wall_data, &wall_tex_data->data[(j%walls[i]->wall_animation)*tile_size.x*tile_size.y], tile_size.x*tile_size.y);
				
				// add/apply wall overlays
				for(vector<lab_wall_overlay*>::iterator overlay_iter = walls[i]->overlays.begin(); overlay_iter != walls[i]->overlays.end(); overlay_iter++) {
					const xld* overlay_tex_xld = ((*overlay_iter)->texture < 100 ? overlay_xlds[0] : ((*overlay_iter)->texture < 200 ? overlay_xlds[1] : overlay_xlds[2]));
					const size_t overlay_tex_num = ((*overlay_iter)->texture < 100 ? (*overlay_iter)->texture-1 :(*overlay_iter)->texture) % 100;
					const xld::xld_object* overlay_tex_data = overlay_tex_xld->get_object(overlay_tex_num);
					
					const size_t anim_offset = ((*overlay_iter)->animation > 1 ? (j%(*overlay_iter)->animation)*(*overlay_iter)->x_size*(*overlay_iter)->y_size : 0);
					
					for(size_t y = 0; y < (*overlay_iter)->y_size; y++) {
						const size_t dst_offset = ((*overlay_iter)->y_offset + y)*tile_size.x + (*overlay_iter)->x_offset;
						const size_t src_offset = y*(*overlay_iter)->x_size + anim_offset;
						for(size_t x = 0; x < (*overlay_iter)->x_size; x++) {
							// ignore 0x00 bytes
							if((write_zero && (*overlay_iter)->write_zero) || overlay_tex_data->data[src_offset + x] > 0) {
								wall_data[dst_offset + x] = overlay_tex_data->data[src_offset + x];
							}
						}
					}
				}
				
				//
				unsigned int repl_alpha = 0xFF000000;
				if(walls[i]->type & WT_CUT_ALPHA) repl_alpha = 0;
				if(walls[i]->type & WT_TRANSPARENT) {
					// use 0x80 alpha for now, everything <= 0x7F will be alpha tested
					repl_alpha = 0x80000000 | (palettes->get_palette(palette)[walls[i]->palette_num] & 0xFFFFFF);
				}
				
				gfxconv::convert_8to32(wall_data, data_32bpp, tile_size.x, tile_size.y, palette, 0, true, repl_alpha);
				scaling::scale(conf::get<scaling::SCALE_TYPE>("map.3d.scale_type"), data_32bpp, tile_size, scaled_data);
				
				// copy data into tiles surface
				if(tex_offset.x + spaced_scaled_tile_size.x > walls_tex_size.x) {
					tex_offset.x = 0;
					tex_offset.y = next_y_offset;
				}
				if(tex_offset.y + spaced_scaled_tile_size.y > next_y_offset) {
					next_y_offset = tex_offset.y + spaced_scaled_tile_size.y;
				}
				
				for(size_t y = 0; y < scaled_tile_size.y; y++) {
					memcpy(&walls_surface[(tex_offset.y + y + wall_spacing) * walls_tex_size.x + (tex_offset.x + wall_spacing)],
						   &scaled_data[y*scaled_tile_size.x],
						   scaled_tile_size.x*sizeof(unsigned int));
				}
				albion_texture::add_spacing(walls_surface, walls_tex_size, scaled_data, scaled_tile_size, wall_spacing, albion_texture::TST_MIRROR, tex_offset);
				
				walls[i]->tex_coord_begin[j].set(float(tex_offset.x+wall_spacing)/float(walls_tex_size.x), float(tex_offset.y+wall_spacing)/float(walls_tex_size.y));
				walls[i]->tex_coord_end[j] = walls[i]->tex_coord_begin[j] + (float2(scaled_tile_size) / float2(walls_tex_size));
				
				tex_offset.x += spaced_scaled_tile_size.x;
			}
			
			delete [] data_32bpp;
			delete [] scaled_data;
			delete [] wall_data;
		}
		cout << ":: creating walls texture " << walls_tex_size << " ..." << endl;
		walls_tex = t->add_texture(walls_surface, (unsigned int)walls_tex_size.x, (unsigned int)walls_tex_size.y, GL_RGBA8, GL_RGBA, custom_tex_filtering, e->get_anisotropic(), GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_UNSIGNED_BYTE);
		albion_texture::build_mipmaps(walls_tex, walls_surface, tex_filtering);
		delete [] walls_surface;
		conf::set<a2e_texture>("debug.texture", walls_tex);
	}
	else walls_tex = t->get_dummy_texture();

	// objects
	if(objects_tex_size.x > 0 && objects_tex_size.y > 0) {
		unsigned int* objects_surface = new unsigned int[objects_tex_size.x*objects_tex_size.y];
		memset(objects_surface, 0, objects_tex_size.x*objects_tex_size.y*sizeof(unsigned int));
		tex_offset = size2(0, 0);
		next_y_offset = 0.0f;
		for(size_t i = 0; i < object_info_count; i++) {
			const xld* obj_tex_xld = (object_infos[i]->texture < 100 ? object_xlds[0] :
									  (object_infos[i]->texture < 200 ? object_xlds[1] :
									   (object_infos[i]->texture < 300 ? object_xlds[2] : object_xlds[3])));
			const size_t obj_tex_num = (object_infos[i]->texture < 100 ? object_infos[i]->texture-1 : object_infos[i]->texture) % 100;
			const xld::xld_object* obj_tex_data = obj_tex_xld->get_object(obj_tex_num);
			
			//
			const size2 tile_size = size2(object_infos[i]->x_size, object_infos[i]->y_size);
			const size2 scaled_tile_size = tile_size * scale_factor;
			const size2 spaced_scaled_tile_size = scaled_tile_size + object_spacing_vec;
			unsigned int* data_32bpp = new unsigned int[tile_size.x*tile_size.y];
			unsigned int* scaled_data = new unsigned int[scaled_tile_size.x*scaled_tile_size.y];
			
			//
			for(size_t j = 0; j < object_infos[i]->animation; j++) {
				if(object_infos[i]->palette_shift > 0) {
					gfxconv::convert_8to32(obj_tex_data->data, data_32bpp, tile_size.x, tile_size.y, palette, j);
				}
				else gfxconv::convert_8to32(&obj_tex_data->data[j*tile_size.x*tile_size.y], data_32bpp, tile_size.x, tile_size.y, palette);
				scaling::scale(conf::get<scaling::SCALE_TYPE>("map.3d.scale_type"), data_32bpp, tile_size, scaled_data);
				
				// copy data into tiles surface
				if(tex_offset.x + spaced_scaled_tile_size.x > objects_tex_size.x) {
					tex_offset.x = 0;
					tex_offset.y = next_y_offset;
				}
				if(tex_offset.y + spaced_scaled_tile_size.y > next_y_offset) {
					next_y_offset = tex_offset.y + spaced_scaled_tile_size.y;
				}
				
				for(size_t y = 0; y < scaled_tile_size.y; y++) {
					memcpy(&objects_surface[(tex_offset.y + y + object_spacing) * objects_tex_size.x + (tex_offset.x + object_spacing)],
						   &scaled_data[y*scaled_tile_size.x],
						   scaled_tile_size.x*sizeof(unsigned int));
				}
				// no need to add transparent spacing here, since the texture already is transparent
				
				object_infos[i]->tex_coord_begin[j].set(float(tex_offset.x+object_spacing)/float(objects_tex_size.x), float(tex_offset.y+object_spacing)/float(objects_tex_size.y));
				object_infos[i]->tex_coord_end[j] = object_infos[i]->tex_coord_begin[j] + (float2(scaled_tile_size) / float2(objects_tex_size));
				
				tex_offset.x += spaced_scaled_tile_size.x;
			}
			
			delete [] data_32bpp;
			delete [] scaled_data;
		}
		cout << ":: creating objects texture " << objects_tex_size << " ..." << endl;
		objects_tex = t->add_texture(objects_surface, (unsigned int)objects_tex_size.x, (unsigned int)objects_tex_size.y, GL_RGBA8, GL_RGBA, custom_tex_filtering, e->get_anisotropic(), GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_UNSIGNED_BYTE);
		albion_texture::build_mipmaps(objects_tex, objects_surface, tex_filtering);
		conf::set<a2e_texture>("debug.texture", objects_tex);
		delete [] objects_surface;
	}
	else objects_tex = t->get_dummy_texture();

	// create material and assign textures
	lab_fc_material = new a2ematerial(e);
	lab_fc_material->load_material(e->data_path("3dmap_fc.a2mtl"));
	lab_wall_material = new a2ematerial(e);
	lab_wall_material->load_material(e->data_path("3dmap_wall.a2mtl"));
	lab_obj_material = new a2ematerial(e);
	lab_obj_material->load_material(e->data_path("3dmap_obj.a2mtl"));

	// assign textures
	for(size_t i = 0; i < 3; i++) {
		a2ematerial::material* mat = (i == 0 ? lab_fc_material->get_material(0) : (i == 1 ? lab_wall_material->get_material(0) : lab_obj_material->get_material(0)));
		a2ematerial::diffuse_material* diff_mat = (a2ematerial::diffuse_material*)mat->mat;
		diff_mat->diffuse_texture = (i == 0 ? floors_tex : (i == 1 ? walls_tex : objects_tex));
	}

	//
	cur_labdata_num = labdata_num;
	a2e_debug("labdata #%u loaded!", labdata_num);
}

void labdata::unload() {
	if(lab_fc_material != NULL) {
		delete lab_fc_material;
		lab_fc_material = NULL;
	}
	if(lab_wall_material != NULL) {
		delete lab_wall_material;
		lab_wall_material = NULL;
	}
	if(lab_obj_material != NULL) {
		delete lab_obj_material;
		lab_obj_material = NULL;
	}

	t->delete_texture(floors_tex);
	t->delete_texture(walls_tex);
	t->delete_texture(objects_tex);
	
	for(vector<lab_object*>::iterator obj_iter = objects.begin(); obj_iter != objects.end(); obj_iter++) {
		delete *obj_iter;
	}
	for(vector<lab_object_info*>::iterator obj_iter = object_infos.begin(); obj_iter != object_infos.end(); obj_iter++) {
		delete *obj_iter;
	}
	for(vector<lab_floor*>::iterator floor_iter = floors.begin(); floor_iter != floors.end(); floor_iter++) {
		delete *floor_iter;
	}
	for(vector<lab_wall*>::iterator wall_iter = walls.begin(); wall_iter != walls.end(); wall_iter++) {
		delete *wall_iter;
	}
	objects.clear();
	object_infos.clear();
	floors.clear();
	walls.clear();
}

const labdata::lab_object* labdata::get_object(const size_t& num) const {
	if(num == 0 || num > objects.size()) {
		a2e_error("invalid object number %u!", num);
		return objects[0];
	}
	return objects[num-1];
}

const labdata::lab_floor* labdata::get_floor(const size_t& num) const {
	if(num == 0 || num > floors.size()) {
		a2e_error("invalid floor number %u!", num);
		return floors[0];
	}
	return floors[num-1];
}

const labdata::lab_wall* labdata::get_wall(const size_t& num) const {
	if(num < 101 || num-101 > walls.size()) {
		return NULL;
	}
	return walls[num-101];
}

a2ematerial* labdata::get_fc_material() const {
	return lab_fc_material;
}

a2ematerial* labdata::get_wall_material() const {
	return lab_wall_material;
}

a2ematerial* labdata::get_object_material() const {
	return lab_obj_material;
}

void labdata::handle_animations() {
	if(cur_labdata_num == (~(size_t)0)) return;
	
	for(vector<lab_floor*>::iterator floor_iter = floors.begin(); floor_iter != floors.end(); floor_iter++) {
		if((*floor_iter)->animation > 1) {
			(*floor_iter)->cur_ani++;
			if((*floor_iter)->cur_ani >= (*floor_iter)->animation) {
				(*floor_iter)->cur_ani = 0;
			}
		}
	}

	for(vector<lab_wall*>::iterator wall_iter = walls.begin(); wall_iter != walls.end(); wall_iter++) {
		if((*wall_iter)->animation > 1) {
			(*wall_iter)->cur_ani++;
			if((*wall_iter)->cur_ani >= (*wall_iter)->animation) {
				(*wall_iter)->cur_ani = 0;
			}
		}
	}

	for(vector<lab_object_info*>::iterator obj_iter = object_infos.begin(); obj_iter != object_infos.end(); obj_iter++) {
		if((*obj_iter)->animation > 1) {
			(*obj_iter)->cur_ani++;
			if((*obj_iter)->cur_ani >= (*obj_iter)->animation) {
				(*obj_iter)->cur_ani = 0;
			}
		}
	}
}
