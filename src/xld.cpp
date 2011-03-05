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

#include "xld.h"

string xld::xld_path = "";

/*! xld constructor
 */
xld::xld(const string& filename) {
	load(filename);
}

/*! xld destructor
 */
xld::~xld() {
	close();
}

void xld::set_xld_path(const string& xld_path) {
	xld::xld_path = xld_path;
}

const string xld::get_xld_path() {
	return xld::xld_path;
}

const string xld::make_xld_path(const string& filename) {
	return xld::xld_path+filename;
}

void xld::load(const string& filename) {
	// open file
	if(!fio->open_file(make_xld_path(filename).c_str(), file_io::OT_READ_BINARY)) {
		a2e_error("couldn't open xld \"%s\"!", filename);
		return;
	}

	// check header
	fio->get_terminated_block(&data, (char)0x00);
	if(data != "XLD0I") {
		fio->close_file();
		a2e_error("\"%s\" is no valid XLD file!", filename);
		return;
	}

	// get object count
	const size_t object_count = c->swap_usint(fio->get_usint());

	// get object lengths
	objects.reserve(object_count);
	for(size_t i = 0; i < object_count; i++) {
		objects.push_back(new xld_object());
		objects.back()->length = c->swap_uint(fio->get_uint());
		objects.back()->data = new unsigned char[objects.back()->length];
	}

	// get objects
	for(vector<xld_object*>::iterator obj_iter = objects.begin(); obj_iter != objects.end(); obj_iter++) {
		fio->get_block((char*)(*obj_iter)->data, (*obj_iter)->length);
	}

	// close file
	fio->close_file();
}

void xld::close() {
	for(vector<xld_object*>::iterator obj_iter = objects.begin(); obj_iter != objects.end(); obj_iter++) {
		delete [] (*obj_iter)->data;
	}
	objects.clear();
}

const size_t xld::get_object_count() const {
	return objects.size();
}

const xld::xld_object* xld::get_object(const size_t& num) const {
	if(num >= objects.size()) {
		a2e_error("invalid xld object number %d!", num);
		return NULL;
	}
	return objects[num];
}

const vector<xld::xld_object*>& xld::get_objects() const {
	return objects;
}

const xld::xld_object* xld::get_object(const size_t& num, const xld* const* xlds, const size_t max_value) {
	if(max_value != 0 && num >= max_value) {
		a2e_error("invalid object number: %u!", num);
		return NULL;
	}

	return xlds[num / 100]->get_object(num % 100);
}
