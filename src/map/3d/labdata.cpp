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

	// load labdata
	labdata_xlds[0] = new xld("LABDATA0.XLD");
	labdata_xlds[1] = new xld("LABDATA1.XLD");
	labdata_xlds[2] = new xld("LABDATA2.XLD");
}

/*! labdata destructor
 */
labdata::~labdata() {
	delete labdata_xlds[0];
	delete labdata_xlds[1];
	delete labdata_xlds[2];
}

void labdata::load(const size_t& labdata_num) {
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
	for(size_t i = 0; i < object_count; i++) {
		offset = 40 + i*66;
		
		objects.push_back(new lab_object());
		objects.back()->type = (AUTOGFX_TYPE)data[offset];
		
		offset = 40 + i*66 + 8;
		objects.back()->object_num = AR_GET_USINT(data, offset);
		objects.back()->object_info = NULL;
	}
	
	// floors
	offset = 40 + object_count*66;
	const size_t floor_count = AR_GET_USINT(data, offset);
	offset += 2;
	for(size_t i = 0; i < floor_count; i++) {
		floors.push_back(new lab_floor());
		
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
		object_infos.push_back(new lab_object_info());
		offset+=2; // unknown
		offset+=2; // unknown

		object_infos.back()->texture = AR_GET_USINT(data, offset);
		offset+=2;

		object_infos.back()->animation = data[offset] & 0xFF;
		offset++;

		offset++; // unknown

		object_infos.back()->x_size = AR_GET_USINT(data, offset);
		offset+=2;
		object_infos.back()->y_size = AR_GET_USINT(data, offset);
		offset+=2;
		object_infos.back()->x_offset = AR_GET_USINT(data, offset);
		offset+=2;
		object_infos.back()->y_offset = AR_GET_USINT(data, offset);
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
	
	// object -> object_info associations
	for(vector<lab_object*>::iterator obj_iter = objects.begin(); obj_iter != objects.end(); obj_iter++) {
		if((*obj_iter)->object_num > object_infos.size()) {
			a2e_error("invalid object number %u!", (*obj_iter)->object_num);
			continue;
		}
		(*obj_iter)->object_info = object_infos[(*obj_iter)->object_num-1];
	}

	cur_labdata_num = labdata_num;
	a2e_debug("labdata #%u loaded!", labdata_num);
}

void labdata::unload() {
}

const labdata::lab_object* labdata::get_object(const size_t& num) const {
	if(num == 0 || num > objects.size()) {
		a2e_error("invalid object number %u!", num);
		return NULL;
	}
	return objects[num-1];
}

const labdata::lab_floor* labdata::get_floor(const size_t& num) const {
	if(num == 0 || num > floors.size()) {
		a2e_error("invalid floor number %u!", num);
		return NULL;
	}
	return floors[num-1];
}

const labdata::lab_wall* labdata::get_wall(const size_t& num) const {
	if(num < 102 || num-102 > walls.size()) {
		a2e_error("invalid wall number %u!", num);
		return NULL;
	}
	return walls[num-102];
}
