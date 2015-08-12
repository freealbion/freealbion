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

#include "map/3d/labdata.hpp"
#include "object_light.hpp"
#include <scene/model/a2ematerial.hpp>

/*! labdata constructor
 */
labdata::labdata() :
labdata_xlds({{ lazy_xld("LABDATA0.XLD"), lazy_xld("LABDATA1.XLD"), lazy_xld("LABDATA2.XLD") }}),
floor_xlds({{ lazy_xld("3DFLOOR0.XLD"), lazy_xld("3DFLOOR1.XLD"), lazy_xld("3DFLOOR2.XLD") }}),
object_xlds({{ lazy_xld("3DOBJEC0.XLD"), lazy_xld("3DOBJEC1.XLD"), lazy_xld("3DOBJEC2.XLD"), lazy_xld("3DOBJEC3.XLD") }}),
overlay_xlds({{ lazy_xld("3DOVERL0.XLD"), lazy_xld("3DOVERL1.XLD"), lazy_xld("3DOVERL2.XLD") }}),
wall_xlds({{ lazy_xld("3DWALLS0.XLD"), lazy_xld("3DWALLS1.XLD") }})
{
	tex_filtering = engine::get_filtering();
	
	// only use linear filtering (max!) when creating the texture (we'll add custom mip-maps later)
	custom_tex_filtering = std::min(TEXTURE_FILTERING::LINEAR, tex_filtering);

	floors_tex = t->get_dummy_texture();
	walls_tex = t->get_dummy_texture();
	objects_tex = t->get_dummy_texture();

#if 0
	for(size_t i = 0; i < 3; i++) {
		lazy_xld* x = &labdata_xlds[i];
		for(size_t j = 0; j < x->get_object_count(); j++) {
			if(x->get_object_size(j) == 0) continue;
			auto obj_data = x->get_object_data(j);
			if(obj_data != nullptr) {
				cout.width(3);
				cout << (i*100 + j);
				cout.width(1);
				cout << ": ";
				/*for(size_t k = 0; k < 38; k++) {
					cout.width(2);
					cout << right << hex << uppercase << (size_t)(*obj_data)[k] << dec << nouppercase;
					cout.width(1);
					cout << " ";
				}*/
				uint32_t s1 = uint32_t((*obj_data)[0]) + (uint32_t((*obj_data)[1]) << 8u);
				uint32_t s2 = uint32_t((*obj_data)[26]) + (uint32_t((*obj_data)[27]) << 8u);
				cout << right << hex << uppercase << setw(4) << s1 << ", " << s2 << setw(1) << ", ";
				cout << (1u << s2);
				cout << dec << nouppercase;
				cout << " (" << (1u << s2) << ")";
				cout.unsetf(ios::adjustfield);
				cout << endl;
			}
		}
	}
#endif
#if 0
	for(auto& wall_xld : wall_xlds) {
		for(size_t i = 0; i < wall_xld.get_object_count(); ++i) {
			cout << "wall #" << setw(3) << i << setw(1) << ": " << wall_xld.get_object_size(i) << endl;
			//if(wall_xld.get_object_size(i) == 0) continue;
			//auto wall_data = wall_xld.get_object_data(i);
		}
	}
#endif
	
	// init light object info	
	// beloveno (203), umajo (206)
	{
		auto ls = new map<unsigned int, object_light_type>();
		ls->insert(make_pair(8, object_light_type::STREET_LAMP));
		ls->insert(make_pair(9, object_light_type::STREET_LAMP));
		ls->insert(make_pair(10, object_light_type::STREET_LAMP));
		ls->insert(make_pair(39, object_light_type::GLOWING_LAMP));
		light_sets.push_back(ls);
		light_objects[203] = ls;
		light_objects[206] = ls;
	}
	// nakiridaani (109)
	{
		auto ls = new map<unsigned int, object_light_type>();
		ls->insert(make_pair(22, object_light_type::GLOWING_LAMP));
		light_sets.push_back(ls);
		light_objects[109] = ls;
	}
	// nak dungeons (106)
	{
		auto ls = new map<unsigned int, object_light_type>();
		ls->insert(make_pair(31, object_light_type::FIREFLY));
		ls->insert(make_pair(32, object_light_type::GLOWING_GRABBER));
		ls->insert(make_pair(51, object_light_type::ARGIM));
		ls->insert(make_pair(9, object_light_type::LIVING_WALL));
		ls->insert(make_pair(10, object_light_type::LIVING_WALL));
		ls->insert(make_pair(11, object_light_type::LIVING_WALL));
		ls->insert(make_pair(12, object_light_type::LIVING_WALL));
		ls->insert(make_pair(13, object_light_type::LIVING_WALL));
		light_sets.push_back(ls);
		light_objects[106] = ls;
	}
}

/*! labdata destructor
 */
labdata::~labdata() {
	unload();
	
	//
	for(auto& ls : light_sets) {
		delete ls;
	}
	light_sets.clear();
	light_objects.clear();
}

void labdata::load(const size_t& labdata_num, const size_t& palette) {
	if(cur_labdata_num == labdata_num) return;
	log_debug("loading labdata %u ...", labdata_num);
	unload();

	//
	if(labdata_num != 0 && labdata_num >= 300) {
		log_error("invalid labdata number: %u!", labdata_num);
		return;
	}
	auto labdata_obj = labdata_xlds[labdata_num / 100].get_object_data(labdata_num % 100);
	const auto data = labdata_obj->data();

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
	objects.resize(object_count);
	for(auto& obj : objects) {
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
		
		obj.type = (AUTOGFX_TYPE)data[offset];
		offset+=2;
		
		// get sub-objects
		obj.sub_object_count = 0;
		for(size_t i = 0; i < 8; i++) {
			size_t obj_num = AR_GET_USINT(data, offset+6);
			if(obj_num > 0) {
				// this also reorganizes the order in case there are "nullptr-objects" in between
				obj.object_num[obj.sub_object_count] = obj_num;
				obj.offset[obj.sub_object_count] = size3((size_t)AR_GET_SINT(data, offset),
														 (size_t)AR_GET_SINT(data, offset+2),
														 (size_t)AR_GET_SINT(data, offset+4));
				obj.sub_object_count++;
			}
			offset+=8;
		}
	}
	
	// floors
	offset = 40 + object_count*66;
	const size_t floor_count = AR_GET_USINT(data, offset);
	offset += 2;
	// keep a vector of all loaded/read floor data (these will be used again later when creating the texture)
	vector<shared_ptr<vector<unsigned char>>> floor_data_ptrs;
	floors.resize(floor_count);
	for(auto& floor : floors) {
		//cout << "#############" << endl << "## floor #" << i << endl;
		
		floor.collision = (COLLISION_TYPE)(data[offset]);
		//cout << "collision : " << hex << (size_t)floors.back()->collision << dec << endl;
		offset++;

		floor._unknown_collision = (unsigned int)((data[offset] << 8u) | data[offset+1]);
		//cout << "unknown collision: " << hex << (size_t)floors.back()->_unknown_collision << dec << endl;
		// TODO: additional collision data is used, too!
		offset+=2; // unknown
		
		//cout << "unknown #1: " << hex << (size_t)(data[offset] & 0xFF) << dec << endl;
		offset++; // unknown

		floor.animation = data[offset] & 0xFF;
		offset++;
		
		//cout << "unknown #2: " << hex << (size_t)(data[offset] & 0xFF) << dec << endl;
		offset++; // unknown

		floor.tex_num = AR_GET_USINT(data, offset);
		offset+=2;
		//cout << "unknown #3: " << hex << (size_t)(data[offset] & 0xFF) << dec << endl;
		//cout << "unknown #4: " << hex << (size_t)(data[offset+1] & 0xFF) << dec << endl;
		offset+=2; // unknown
		
		// !!!!!!!!!!!!!!
		// TODO: lazy_xld
		
		// check if tile uses colors that are animated ...
		const size_t& tex_num = floor.tex_num;
		auto floor_data_ptr = floor_xlds[tex_num/100].get_object_data(tex_num < 100 ? tex_num-1 : tex_num%100);
		floor_data_ptrs.emplace_back(floor_data_ptr);
		const auto floor_data = floor_data_ptr->data();
		const vector<size2>& animated_ranges = palettes->get_animated_ranges(palette);
		bool palette_shift = false;
		for(const auto& ani_range : animated_ranges) {
			for(size_t p = 0; p < 64*64; p++) {
				if(floor_data[p] >= ani_range.x && floor_data[p] <= ani_range.y) {
					// ... if so, set animation count accordingly
					floor.animation = (unsigned int)(ani_range.y - ani_range.x + 1);
					palette_shift = true;
					break;
				}
			}
		}

		//
		floor.tex_info = new albion_texture::albion_texture_multi_xld[floor.animation];
		for(unsigned int j = 0; j < floor.animation; j++) {
			floor.tex_info[j].tex_num = (unsigned int)tex_num;
			if(palette_shift) {
				floor.tex_info[j].palette_shift = j;
			}
			else {
				floor.tex_info[j].offset = j * (64*64);
			}
		}
	}
	
	// object info
	const size_t object_info_count = AR_GET_USINT(data, offset);
	offset+=2;
	object_infos.resize(object_info_count);
	for(auto& obj_info : object_infos) {
		//cout << ":object #" << i << ": ";
		/*for(size_t x = 0; x < 16; x++) {
			cout << (size_t)data[offset+x] << ", ";
		}
		cout << endl;*/

		obj_info.type = data[offset];
		offset++;

		//cout << "collision: " << hex << (size_t)(data[offset] & 0xFF) << " " << (size_t)(data[offset+1] & 0xFF) << " " << (size_t)(data[offset+2] & 0xFF) << dec << endl;
		obj_info.collision = (COLLISION_TYPE)(data[offset]);
		offset++;

		obj_info._unknown_collision = (unsigned int)((data[offset] << 8) | data[offset+1]);
		offset+=2;

		obj_info.texture = AR_GET_USINT(data, offset);
		offset+=2;

		obj_info.animation = data[offset] & 0xFF;
		offset++;

		offset++; // unknown (animation?)

		obj_info.x_size = AR_GET_USINT(data, offset);
		offset+=2;
		obj_info.y_size = AR_GET_USINT(data, offset);
		offset+=2;
		obj_info.x_scale = AR_GET_USINT(data, offset);
		offset+=2;
		obj_info.y_scale = AR_GET_USINT(data, offset);
		offset+=2;
	}
	
	// walls + overlays
	const size_t wall_count = AR_GET_USINT(data, offset);
	offset+=2;
	walls.resize(wall_count);
	for(auto& wall : walls) {
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
		
		//wall.type = data[offset] & 0xFF;
		wall.type = (data[offset] & 0xFF) & WT_JOINED;
		offset++;
		
		wall.collision = (COLLISION_TYPE)(data[offset]);
		offset++;

		wall._unknown_collision = uint32_t((data[offset] << 8u) | data[offset+1]);
		offset+=2;

		wall.texture = AR_GET_USINT(data, offset);
		offset+=2;
		
		wall.wall_animation = data[offset] & 0xFF;
		wall.animation = wall.wall_animation;
		offset++;
		wall.autogfx_type = (AUTOGFX_TYPE)data[offset];
		offset++;
		wall.palette_num = data[offset] & 0xFF;
		offset++;
		
		offset++; // unknown

		wall.x_size = AR_GET_USINT(data, offset);
		offset+=2;
		wall.y_size = AR_GET_USINT(data, offset);
		offset+=2;
		wall.overlay_count = AR_GET_USINT(data, offset);
		offset+=2;
		//cout << "overlay_count: " << (size_t)wall.overlay_count << endl;
		
		wall.overlays.resize(wall.overlay_count);
		for(auto& overlay : wall.overlays) {
			overlay.texture = AR_GET_USINT(data, offset);
			offset+=2;
			
			overlay.animation = data[offset] & 0xFF;
			// stupid max for now, might work always though (if not: mulitply)
			wall.animation = std::max(wall.animation, overlay.animation);
			offset++;
			
			overlay.write_zero = (data[offset] == 0);
			offset++;

			overlay.y_offset = AR_GET_USINT(data, offset);
			offset+=2;
			overlay.x_offset = AR_GET_USINT(data, offset);
			offset+=2;
			overlay.y_size = AR_GET_USINT(data, offset);
			offset+=2;
			overlay.x_size = AR_GET_USINT(data, offset);
			offset+=2;
		}
		
		wall.tex_coord_begin = new float2[wall.animation];
		wall.tex_coord_end = new float2[wall.animation];
	}

	//
	cout << "# walls: " << wall_count << endl;
	cout << "# objects: " << object_count << endl;
	cout << "# floors/ceilings: " << floor_count << endl;
	
	// object -> object_info associations
	for(auto& obj : objects) {
		for(size_t sub_obj = 0; sub_obj < obj.sub_object_count; sub_obj++) {
			if(obj.object_num[sub_obj] > object_infos.size()) {
				log_error("invalid object number %u!", obj.object_num[sub_obj]);
				continue;
			}
			obj.sub_objects[sub_obj] = &object_infos[obj.object_num[sub_obj]-1];
		}
	}

	// compute texture size dependent on necessary size
	const size_t max_tex_size = exts->get_max_texture_size();
	size2 floors_tex_size, walls_tex_size, objects_tex_size;
	const size_t scale_factor = scaling::get_scale_factor(conf::get<scaling::SCALE_TYPE>("map.3d.scale_type"));
	
	// compute floors tex size
	const size_t floor_spacing = 16 * scale_factor; // we sadly need that much spacing for floor tiles
	const size2 floor_size = (size2(64, 64) * scale_factor) + size2(2 * floor_spacing);
	for(const auto& floor : floors) {
		for(size_t j = 0; j < floor.animation; j++) {
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
	for(const auto& wall : walls) {
		const size2 wall_size = size2(wall.y_size, wall.x_size)*scale_factor + wall_spacing_vec;
		
		for(size_t j = 0; j < wall.animation; j++) {
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
	for(const auto& obj_info : object_infos) {
		const size2 object_size = size2(obj_info.x_size, obj_info.y_size)*scale_factor + object_spacing_vec;
		
		for(size_t j = 0; j < obj_info.animation; j++) {
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
	for(const auto& floor : floors) {
		for(size_t j = 0; j < floor.animation; j++) {
			floors_tex_info.push_back(&floor.tex_info[j]);
		}
	}
	cout << ":: creating floors texture " << floors_tex_size << " ..." << endl;
	if(floors_tex_size.x > 0 && floors_tex_size.y > 0) {
		floors_tex = albion_texture::create(MAP_TYPE::MAP_3D, floors_tex_size, size2(64, 64), palette, floors_tex_info,
											&floor_xlds[0], albion_texture::TEXTURE_SPACING::MIRROR, floor_spacing, true, tex_filtering);
	}
	else floors_tex = t->get_dummy_texture();
	// clear/delete floor xld data
	floor_data_ptrs.clear();
	//conf::set<a2e_texture>("debug.texture", floors_tex);
	
	// walls
	size2 tex_offset;
	float next_y_offset = 0.0f;
	if(walls_tex_size.x > 0 && walls_tex_size.y > 0) {
		unsigned int* walls_surface = new unsigned int[walls_tex_size.x*walls_tex_size.y];
		memset(walls_surface, 0, walls_tex_size.x*walls_tex_size.y*sizeof(unsigned int));
		for(const auto& wall : walls) {
			lazy_xld& wall_tex_xld = (wall.texture < 100 ? wall_xlds[0] : wall_xlds[1]);
			const size_t wall_tex_num = (wall.texture < 100 ? wall.texture-1 : wall.texture) % 100;
			auto wall_tex = wall_tex_xld.get_object_data(wall_tex_num);
			const auto wall_tex_data = wall_tex->data();
			bool write_zero = ((wall.type & WT_WRITE_OVERLAY_ZERO) != 0);
			
			//
			const size2 tile_size = size2(wall.y_size, wall.x_size);
			const size2 scaled_tile_size = tile_size * scale_factor;
			const size2 spaced_scaled_tile_size = scaled_tile_size + wall_spacing_vec;
			unsigned char* wall_data = new unsigned char[tile_size.x*tile_size.y];
			unsigned int* data_32bpp = new unsigned int[tile_size.x*tile_size.y];
			unsigned int* scaled_data = new unsigned int[scaled_tile_size.x*scaled_tile_size.y];
						
			//
			for(size_t j = 0; j < wall.animation; j++) {
				memcpy(wall_data, &wall_tex_data[(j%wall.wall_animation)*tile_size.x*tile_size.y], tile_size.x*tile_size.y);
				
				// add/apply wall overlays
				for(const auto& overlay : wall.overlays) {
					lazy_xld& overlay_tex_xld = (overlay.texture < 100 ? overlay_xlds[0] :
												 (overlay.texture < 200 ? overlay_xlds[1] : overlay_xlds[2]));
					const size_t overlay_tex_num = (overlay.texture < 100 ? overlay.texture-1 : overlay.texture) % 100;
					auto overlay_tex = overlay_tex_xld.get_object_data(overlay_tex_num);
					const auto overlay_tex_data = overlay_tex->data();
					
					const size_t anim_offset = (overlay.animation > 1 ? (j%overlay.animation)*overlay.x_size*overlay.y_size : 0);
					
					for(size_t y = 0; y < overlay.y_size; y++) {
						const size_t dst_offset = (overlay.y_offset + y)*tile_size.x + overlay.x_offset;
						const size_t src_offset = y*overlay.x_size + anim_offset;
						for(size_t x = 0; x < overlay.x_size; x++) {
							// ignore 0x00 bytes
							if((write_zero && overlay.write_zero) || overlay_tex_data[src_offset + x] > 0) {
								wall_data[dst_offset + x] = overlay_tex_data[src_offset + x];
							}
						}
					}
				}
				
				//
				unsigned int repl_alpha = 0xFF000000;
				if(wall.type & WT_CUT_ALPHA) repl_alpha = 0;
				if(wall.type & WT_TRANSPARENT) {
					// use 0x80 alpha for now, everything <= 0x7F will be alpha tested
					repl_alpha = 0x80000000 | (palettes->get_palette(palette)[wall.palette_num] & 0xFFFFFF);
				}
				
				gfxconv::convert_8to32(wall_data, data_32bpp, tile_size.x, tile_size.y, palette, 0, true, repl_alpha);
				scaling::scale(conf::get<scaling::SCALE_TYPE>("map.3d.scale_type"), data_32bpp, tile_size, scaled_data);
				
				// copy data into tiles surface
				if(tex_offset.x + spaced_scaled_tile_size.x > walls_tex_size.x) {
					tex_offset.x = 0;
					tex_offset.y = (size_t)next_y_offset;
				}
				if(tex_offset.y + spaced_scaled_tile_size.y > next_y_offset) {
					next_y_offset = tex_offset.y + spaced_scaled_tile_size.y;
				}
				
				for(size_t y = 0; y < scaled_tile_size.y; y++) {
					memcpy(&walls_surface[(tex_offset.y + y + wall_spacing) * walls_tex_size.x + (tex_offset.x + wall_spacing)],
						   &scaled_data[y*scaled_tile_size.x],
						   scaled_tile_size.x*sizeof(unsigned int));
				}
				albion_texture::add_spacing(walls_surface, walls_tex_size, scaled_data, scaled_tile_size,
											wall_spacing, albion_texture::TEXTURE_SPACING::MIRROR, tex_offset);
				
				wall.tex_coord_begin[j].set(float(tex_offset.x+wall_spacing)/float(walls_tex_size.x), float(tex_offset.y+wall_spacing)/float(walls_tex_size.y));
				wall.tex_coord_end[j] = wall.tex_coord_begin[j] + (float2(scaled_tile_size) / float2(walls_tex_size));
				
				tex_offset.x += spaced_scaled_tile_size.x;
			}
			
			delete [] data_32bpp;
			delete [] scaled_data;
			delete [] wall_data;
		}
		cout << ":: creating walls texture " << walls_tex_size << " ..." << endl;
		walls_tex = t->add_texture(walls_surface, (int)walls_tex_size.x, (int)walls_tex_size.y, GL_RGBA8, GL_RGBA,
								   custom_tex_filtering, engine::get_anisotropic(), GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_UNSIGNED_BYTE);
		albion_texture::build_mipmaps(walls_tex, walls_surface, tex_filtering);
		delete [] walls_surface;
		//conf::set<a2e_texture>("debug.texture", walls_tex);
	}
	else walls_tex = t->get_dummy_texture();

	// objects
	if(objects_tex_size.x > 0 && objects_tex_size.y > 0) {
		unsigned int* objects_surface = new unsigned int[objects_tex_size.x*objects_tex_size.y];
		memset(objects_surface, 0, objects_tex_size.x*objects_tex_size.y*sizeof(unsigned int));
		tex_offset = size2(0, 0);
		next_y_offset = 0.0f;
		for(auto& obj_info : object_infos) {
			auto obj_tex = object_xlds[obj_info.texture/100].get_object_data(obj_info.texture < 100 ?
																			 obj_info.texture-1 : obj_info.texture%100);
			const auto obj_tex_data = obj_tex->data();
			
			// check if tile uses colors that are animated ...
			const vector<size2>& animated_ranges = palettes->get_animated_ranges(palette);
			for(const auto& ani_range : animated_ranges) {
				const size_t obj_size = obj_info.x_size*obj_info.y_size;
				for(size_t p = 0; p < obj_size; p++) {
					if(obj_tex_data[p] >= ani_range.x && obj_tex_data[p] <= ani_range.y) {
						// ... if so, set animation count accordingly
						obj_info.animation = (unsigned int)(ani_range.y - ani_range.x + 1);
						obj_info.palette_shift = 1;
						break;
					}
				}
			}
			obj_info.tex_coord_begin = new float2[obj_info.animation];
			obj_info.tex_coord_end = new float2[obj_info.animation];
			
			//
			const size2 tile_size = size2(obj_info.x_size, obj_info.y_size);
			const size2 scaled_tile_size = tile_size * scale_factor;
			const size2 spaced_scaled_tile_size = scaled_tile_size + object_spacing_vec;
			unsigned int* data_32bpp = new unsigned int[tile_size.x*tile_size.y];
			unsigned int* scaled_data = new unsigned int[scaled_tile_size.x*scaled_tile_size.y];
			
			//
			for(size_t j = 0; j < obj_info.animation; j++) {
				if(obj_info.palette_shift > 0) {
					gfxconv::convert_8to32(obj_tex_data, data_32bpp, tile_size.x, tile_size.y, palette, j);
				}
				else gfxconv::convert_8to32(&obj_tex_data[j*tile_size.x*tile_size.y], data_32bpp, tile_size.x, tile_size.y, palette);
				scaling::scale(conf::get<scaling::SCALE_TYPE>("map.3d.scale_type"), data_32bpp, tile_size, scaled_data);
				
				// copy data into tiles surface
				if(tex_offset.x + spaced_scaled_tile_size.x > objects_tex_size.x) {
					tex_offset.x = 0;
					tex_offset.y = (size_t)next_y_offset;
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
				
				obj_info.tex_coord_begin[j].set(float(tex_offset.x+object_spacing)/float(objects_tex_size.x), float(tex_offset.y+object_spacing)/float(objects_tex_size.y));
				obj_info.tex_coord_end[j] = obj_info.tex_coord_begin[j] + (float2(scaled_tile_size) / float2(objects_tex_size));
				
				tex_offset.x += spaced_scaled_tile_size.x;
			}
			
			delete [] data_32bpp;
			delete [] scaled_data;
		}
		cout << ":: creating objects texture " << objects_tex_size << " ..." << endl;
		objects_tex = t->add_texture(objects_surface, (int)objects_tex_size.x, (int)objects_tex_size.y, GL_RGBA8, GL_RGBA,
									 custom_tex_filtering, engine::get_anisotropic(),
									 GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_UNSIGNED_BYTE);
		albion_texture::build_mipmaps(objects_tex, objects_surface, tex_filtering);
		//conf::set<a2e_texture>("debug.texture", objects_tex);
		delete [] objects_surface;
	}
	else objects_tex = t->get_dummy_texture();
	
	// update objects with new animation info
	for(auto& obj : objects) {
		for(size_t sub_obj = 0; sub_obj < obj.sub_object_count; sub_obj++) {
			if(obj.sub_objects[sub_obj] != nullptr &&
			   obj.sub_objects[sub_obj]->animation > 1) {
				obj.animated = true;
			}
		}
	}

	// create material and assign textures
	lab_fc_material = new a2ematerial();
	lab_fc_material->load_material(floor::data_path("3dmap_fc.a2mtl"));
	lab_wall_material = new a2ematerial();
	lab_wall_material->load_material(floor::data_path("3dmap_wall.a2mtl"));
	lab_obj_material = new a2ematerial();
	lab_obj_material->load_material(floor::data_path("3dmap_obj.a2mtl"));

	// assign textures
	for(size_t i = 0; i < 3; i++) {
		a2ematerial::material& mat = (i == 0 ? lab_fc_material->get_material(0) : (i == 1 ? lab_wall_material->get_material(0) : lab_obj_material->get_material(0)));
		a2ematerial::diffuse_material* diff_mat = (a2ematerial::diffuse_material*)mat.mat;
		diff_mat->diffuse_texture = (i == 0 ? floors_tex : (i == 1 ? walls_tex : objects_tex));
	}

	//
	cur_labdata_num = labdata_num;
	log_debug("labdata #%u loaded!", labdata_num);
}

void labdata::unload() {
	if(lab_fc_material != nullptr) {
		delete lab_fc_material;
		lab_fc_material = nullptr;
	}
	if(lab_wall_material != nullptr) {
		delete lab_wall_material;
		lab_wall_material = nullptr;
	}
	if(lab_obj_material != nullptr) {
		delete lab_obj_material;
		lab_obj_material = nullptr;
	}

	t->delete_texture(floors_tex);
	t->delete_texture(walls_tex);
	t->delete_texture(objects_tex);
	
	objects.clear();
	object_infos.clear();
	floors.clear();
	walls.clear();
}

const labdata::lab_object* labdata::get_object(const size_t& num) const {
	if(num == 0 || num > objects.size()) {
		log_error("invalid object number %u!", num);
		return &objects[0];
	}
	return &objects[num-1];
}

const labdata::lab_floor* labdata::get_floor(const size_t& num) const {
	if(num == 0 || num > floors.size()) {
		log_error("invalid floor number %u!", num);
		return &floors[0];
	}
	return &floors[num-1];
}

const labdata::lab_wall* labdata::get_wall(const size_t& num) const {
	if(num < 101 || num-101 > walls.size()) {
		return nullptr;
	}
	return &walls[num-101];
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
	
	for(auto& floor : floors) {
		if(floor.animation > 1) {
			floor.cur_ani++;
			if(floor.cur_ani >= floor.animation) {
				floor.cur_ani = 0;
			}
		}
	}

	for(auto& wall : walls) {
		if(wall.animation > 1) {
			wall.cur_ani++;
			if(wall.cur_ani >= wall.animation) {
				wall.cur_ani = 0;
			}
		}
	}

	for(auto& obj_info : object_infos) {
		if(obj_info.animation > 1) {
			obj_info.cur_ani++;
			if(obj_info.cur_ani >= obj_info.animation) {
				obj_info.cur_ani = 0;
			}
		}
	}
}

const map<unsigned int, labdata::light_info_container*>& labdata::get_light_objects() const {
	return light_objects;
}
