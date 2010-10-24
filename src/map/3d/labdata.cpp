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

#include "labdata.h"

/*! labdata constructor
 */
labdata::labdata() {
	cur_labdata_num = (~0);
	lab_material = NULL;
	lab_obj_material = NULL;

	floors_tex = t->get_dummy_texture();
	walls_tex = t->get_dummy_texture();
	objects_tex = t->get_dummy_texture();

	// load labdata
	labdata_xlds[0] = new xld("LABDATA0.XLD");
	labdata_xlds[1] = new xld("LABDATA1.XLD");
	labdata_xlds[2] = new xld("LABDATA2.XLD");
	
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
	const xld::xld_object* object;
	if(labdata_num < 100) {
		object = labdata_xlds[0]->get_object(labdata_num);
	}
	else if(labdata_num >= 100 && labdata_num < 200) {
		object = labdata_xlds[1]->get_object(labdata_num-100);
	}
	else if(labdata_num >= 200 && labdata_num < 300) {
		object = labdata_xlds[2]->get_object(labdata_num-200);
	}
	else {
		a2e_error("invalid labdata number: %u!", labdata_num);
		return;
	}
	const unsigned char* data = object->data;
	
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

		/*cout << "floor " << i << " bytes: ";
		for(size_t x = 0; x < 10; x++) {
			cout << (size_t)data[offset+x] << ", ";
		}
		cout << endl;*/
		
		floors.back()->collision = false;
		for(unsigned int j = 0; j < 3; j++) {
			if((data[offset] & 0xFF) != 0) {
				floors.back()->collision = true;
			}
			offset++;
		}
		
		offset++; // unknown

		floors.back()->animation = data[offset] & 0xFF;
		offset++;

		offset++; // unknown

		floors.back()->texture = AR_GET_USINT(data, offset);
		offset+=2;

		offset+=2; // unknown
	}
	
	// object info
	const size_t object_info_count = AR_GET_USINT(data, offset);
	offset+=2;
	for(size_t i = 0; i < object_info_count; i++) {
		/*cout << ":object #" << i << ": ";
		for(size_t x = 0; x < 16; x++) {
			cout << (size_t)data[offset+x] << ", ";
		}
		cout << endl;*/

		object_infos.push_back(new lab_object_info());
		object_infos.back()->type = data[offset];
		offset++;
		object_infos.back()->collision = (data[offset] << 16) | (data[offset+1] << 8) | data[offset+2];
		offset+=3;

		object_infos.back()->texture = AR_GET_USINT(data, offset);
		offset+=2;

		object_infos.back()->animation = data[offset] & 0xFF;
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
	}
	
	// walls + overlays
	const size_t wall_count = AR_GET_USINT(data, offset);
	offset+=2;
	for(size_t i = 0; i < wall_count; i++) {
		walls.push_back(new lab_wall());
		offset+=2; // unknown
		offset+=2; // unknown

		walls.back()->texture = AR_GET_USINT(data, offset);
		offset+=2;

		walls.back()->animation = data[offset] & 0xFF;
		offset++;
		walls.back()->type = (AUTOGFX_TYPE)data[offset];
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
		
		for(unsigned int j = 0; j < walls.back()->overlay_count; j++) {
			walls.back()->overlays.push_back(new lab_wall_overlay());
			walls.back()->overlays.back()->texture = AR_GET_USINT(data, offset);
			offset+=2;

			offset+=2; // unknown

			walls.back()->overlays.back()->y_offset = AR_GET_USINT(data, offset);
			offset+=2;
			walls.back()->overlays.back()->x_offset = AR_GET_USINT(data, offset);
			offset+=2;
			walls.back()->overlays.back()->y_size = AR_GET_USINT(data, offset);
			offset+=2;
			walls.back()->overlays.back()->x_size = AR_GET_USINT(data, offset);
			offset+=2;
		}
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
		}
	}


	// create textures

	// TODO: texture size dependent on necessary size

	// floors
	const size2 floors_tex_size = size2(4096, 4096);
	unsigned int* floors_surface = new unsigned int[floors_tex_size.x*floors_tex_size.y];
	memset(floors_surface, 0, floors_tex_size.x*floors_tex_size.y*sizeof(unsigned int));
	
	const size2 tile_size = size2(64, 64);
	const size2 scaled_tile_size = tile_size * 4;
	unsigned int* data_32bpp = new unsigned int[tile_size.x*tile_size.y];
	unsigned int* scaled_data = new unsigned int[scaled_tile_size.x*scaled_tile_size.y];
	for(size_t i = 0; i < floor_count; i++) {
		const xld* floor_tex_xld = (floors[i]->texture < 100 ? floor_xlds[0] : (floors[i]->texture < 200 ? floor_xlds[1] : floor_xlds[2]));
		const size_t floor_tex_num = (floors[i]->texture < 100 ? floors[i]->texture-1 : floors[i]->texture) % 100;
		const xld::xld_object* floor_tex_data = floor_tex_xld->get_object(floor_tex_num);
		gfxconv::convert_8to32(floor_tex_data->data, data_32bpp, tile_size.x, tile_size.y, palette);
		scaling::scale_4x(scaling::ST_HQ4X, data_32bpp, tile_size, scaled_data);

		// copy data into tiles surface
		const size_t offset_x = (scaled_tile_size.x * i) % floors_tex_size.x;
		const size_t offset_y = scaled_tile_size.y * (scaled_tile_size.x * i / floors_tex_size.x);
		for(size_t y = 0; y < scaled_tile_size.y; y++) {
			memcpy(&floors_surface[(offset_y + y) * floors_tex_size.x + offset_x], &scaled_data[y*scaled_tile_size.x], scaled_tile_size.x*sizeof(unsigned int));
		}

		floors[i]->tex_coord.set(float(offset_x)/float(floors_tex_size.x), float(offset_y)/float(floors_tex_size.y));
	}
	delete [] data_32bpp;
	delete [] scaled_data;

	// don't use any filtering here! this will only produce artifacts!
	floors_tex = t->add_texture(floors_surface, floors_tex_size.x, floors_tex_size.y, GL_RGBA8, GL_RGBA, texture_object::TF_POINT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_UNSIGNED_BYTE);
	//conf::set<a2e_texture>("debug.texture", floors_tex);
	delete [] floors_surface;

	// walls
	const size2 walls_tex_size = size2(4096, 4096);
	unsigned int* walls_surface = new unsigned int[walls_tex_size.x*walls_tex_size.y];
	memset(walls_surface, 0, walls_tex_size.x*walls_tex_size.y*sizeof(unsigned int));
	size2 tex_offset;
	float next_y_offset = 0.0f;
	for(size_t i = 0; i < wall_count; i++) {
		const xld* wall_tex_xld = (walls[i]->texture < 100 ? wall_xlds[0] : wall_xlds[1]);
		const size_t wall_tex_num = (walls[i]->texture < 100 ? walls[i]->texture-1 : walls[i]->texture) % 100;
		const xld::xld_object* wall_tex_data = wall_tex_xld->get_object(wall_tex_num);

		//
		const size2 tile_size = size2(walls[i]->y_size, walls[i]->x_size);
		const size2 scaled_tile_size = tile_size * 4;
		unsigned char* wall_data = new unsigned char[tile_size.x*tile_size.y];
		memcpy(wall_data, wall_tex_data->data, tile_size.x*tile_size.y);
		//cout << "wall #" << i << ": " << tile_size << " -> " << scaled_tile_size << endl;

		// add/apply wall overlays
		for(vector<lab_wall_overlay*>::iterator overlay_iter = walls[i]->overlays.begin(); overlay_iter != walls[i]->overlays.end(); overlay_iter++) {
			const xld* overlay_tex_xld = ((*overlay_iter)->texture < 100 ? overlay_xlds[0] : ((*overlay_iter)->texture < 200 ? overlay_xlds[1] : overlay_xlds[2]));
			const size_t overlay_tex_num = ((*overlay_iter)->texture < 100 ? (*overlay_iter)->texture-1 :(*overlay_iter)->texture) % 100;
			const xld::xld_object* overlay_tex_data = overlay_tex_xld->get_object(overlay_tex_num);
			//cout << "\toverlay: " << (*overlay_iter)->x_size << ", " << (*overlay_iter)->y_size << " / " << (*overlay_iter)->x_offset << ", " << (*overlay_iter)->y_offset << endl;

			for(size_t y = 0; y < (*overlay_iter)->y_size; y++) {
				//memcpy(&wall_data[((*overlay_iter)->y_offset + y)*tile_size.x + (*overlay_iter)->x_offset], &overlay_tex_data->data[y*(*overlay_iter)->x_size], (*overlay_iter)->x_size);
				// memcpy won't work, b/c 0x00 bytes must be ignored ...
				const size_t dst_offset = ((*overlay_iter)->y_offset + y)*tile_size.x + (*overlay_iter)->x_offset;
				const size_t src_offset = y*(*overlay_iter)->x_size;
				for(size_t x = 0; x < (*overlay_iter)->x_size; x++) {
					if(overlay_tex_data->data[src_offset + x] > 0) {
						wall_data[dst_offset + x] = overlay_tex_data->data[src_offset + x];
					}
				}
			}
		}

		unsigned int* data_32bpp = new unsigned int[tile_size.x*tile_size.y];
		unsigned int* scaled_data = new unsigned int[scaled_tile_size.x*scaled_tile_size.y];

		gfxconv::convert_8to32(wall_data, data_32bpp, tile_size.x, tile_size.y, palette);
		scaling::scale_4x(scaling::ST_HQ4X, data_32bpp, tile_size, scaled_data);

		// copy data into tiles surface
		if(tex_offset.x + scaled_tile_size.x > walls_tex_size.x) {
			tex_offset.x = 0;
			tex_offset.y = next_y_offset;
		}
		if(tex_offset.y + scaled_tile_size.y > next_y_offset) {
			next_y_offset = tex_offset.y + scaled_tile_size.y;
		}
		//cout << "\t@ " << tex_offset << " (" << next_y_offset << ")" << endl;

		for(size_t y = 0; y < scaled_tile_size.y; y++) {
			memcpy(&walls_surface[(tex_offset.y + y) * walls_tex_size.x + tex_offset.x], &scaled_data[y*scaled_tile_size.x], scaled_tile_size.x*sizeof(unsigned int));
		}
		
		delete [] data_32bpp;
		delete [] scaled_data;
		
		walls[i]->tex_coord_begin.set(float(tex_offset.x)/float(walls_tex_size.x), float(tex_offset.y)/float(walls_tex_size.y));
		walls[i]->tex_coord_end.set(float(tex_offset.x+scaled_tile_size.x)/float(walls_tex_size.x), float(tex_offset.y+scaled_tile_size.y)/float(walls_tex_size.y));

		tex_offset.x += scaled_tile_size.x;
	}
	walls_tex = t->add_texture(walls_surface, walls_tex_size.x, walls_tex_size.y, GL_RGBA8, GL_RGBA, texture_object::TF_POINT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_UNSIGNED_BYTE);
	//conf::set<a2e_texture>("debug.texture", walls_tex);
	delete [] walls_surface;
	
	// objects
	const size2 objects_tex_size = size2(4096, 4096);
	unsigned int* objects_surface = new unsigned int[objects_tex_size.x*objects_tex_size.y];
	memset(objects_surface, 0, objects_tex_size.x*objects_tex_size.y*sizeof(unsigned int));
	//size2 tex_offset;
	//float next_y_offset = 0.0f;
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
		const size2 scaled_tile_size = tile_size * 4;
		//cout << "object #" << (i+1) << ": " << tile_size << " -> " << scaled_tile_size << " :: " << object_infos[i]->x_scale << ", " << object_infos[i]->y_scale << endl;
		//cout << "#### " << hex << (size_t)object_infos[i]->type << dec << endl;
		//cout << "#### " << objects[i]->info->x_offset << ", " << objects[i]->info->y_offset << endl;
		unsigned int* data_32bpp = new unsigned int[tile_size.x*tile_size.y];
		unsigned int* scaled_data = new unsigned int[scaled_tile_size.x*scaled_tile_size.y];

		gfxconv::convert_8to32(obj_tex_data->data, data_32bpp, tile_size.x, tile_size.y, palette);
		scaling::scale_4x(scaling::ST_HQ4X, data_32bpp, tile_size, scaled_data);

		// copy data into tiles surface
		if(tex_offset.x + scaled_tile_size.x > objects_tex_size.x) {
			tex_offset.x = 0;
			tex_offset.y = next_y_offset;
		}
		if(tex_offset.y + scaled_tile_size.y > next_y_offset) {
			next_y_offset = tex_offset.y + scaled_tile_size.y;
		}
		//cout << "\t@ " << tex_offset << " (" << next_y_offset << ")" << endl;

		for(size_t y = 0; y < scaled_tile_size.y; y++) {
			memcpy(&objects_surface[(tex_offset.y + y) * objects_tex_size.x + tex_offset.x], &scaled_data[y*scaled_tile_size.x], scaled_tile_size.x*sizeof(unsigned int));
		}
		
		delete [] data_32bpp;
		delete [] scaled_data;
		
		object_infos[i]->tex_coord_begin.set(float(tex_offset.x)/float(objects_tex_size.x), float(tex_offset.y)/float(objects_tex_size.y));
		object_infos[i]->tex_coord_end.set(float(tex_offset.x+scaled_tile_size.x)/float(objects_tex_size.x), float(tex_offset.y+scaled_tile_size.y)/float(objects_tex_size.y));

		tex_offset.x += scaled_tile_size.x;
	}
	objects_tex = t->add_texture(objects_surface, objects_tex_size.x, objects_tex_size.y, GL_RGBA8, GL_RGBA, texture_object::TF_POINT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_UNSIGNED_BYTE);
	conf::set<a2e_texture>("debug.texture", objects_tex);
	delete [] objects_surface;

	// create material and assign textures
	lab_material = new a2ematerial(e);
	lab_material->load_material(e->data_path("3dmap.a2mtl"));
	lab_obj_material = new a2ematerial(e);
	lab_obj_material->load_material(e->data_path("3dmap_obj.a2mtl"));

	// assign textures
	for(size_t i = 0; i < 2; i++) {
		a2ematerial::material* mat = lab_material->get_material(i);
		a2ematerial::diffuse_material* diff_mat = (a2ematerial::diffuse_material*)mat->mat;
		diff_mat->diffuse_texture = (i == 0 ? floors_tex : walls_tex);
	}
	a2ematerial::material* mat = lab_obj_material->get_material(0);
	a2ematerial::diffuse_material* diff_mat = (a2ematerial::diffuse_material*)mat->mat;
	diff_mat->diffuse_texture = objects_tex;

	//
	cur_labdata_num = labdata_num;
	a2e_debug("labdata #%u loaded!", labdata_num);
}

void labdata::unload() {
	if(lab_material != NULL) {
		delete lab_material;
		lab_material = NULL;
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
		a2e_error("invalid wall number %u!", num);
		return walls[0];
	}
	return walls[num-101];
}

a2ematerial* labdata::get_material() const {
	return lab_material;
}

a2ematerial* labdata::get_object_material() const {
	return lab_obj_material;
}
