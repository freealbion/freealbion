/*
 *  Albion Remake
 *  Copyright (C) 2007 - 2014 Florian Ziesche
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

#include "xld.hpp"
#include <core/file_io.hpp>

string xld::xld_path = "";

#define AR_SWAP_USINT(us) (((us & 0xFF00) >> 8) + ((us & 0xFF) << 8))
#define AR_SWAP_UINT(ui) (((ui >> 24) & 0xFF) + (((ui >> 16) & 0xFF) << 8) + (((ui >> 8) & 0xFF) << 16) + ((ui & 0xFF) << 24))

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

void xld::set_xld_path(const string& xld_path_) {
	xld::xld_path = xld_path_;
}

const string xld::get_xld_path() {
	return xld::xld_path;
}

const string xld::make_xld_path(const string& filename) {
	return xld::xld_path+filename;
}

void xld::load(const string& filename) {
	// open file
	if(!fio->open(make_xld_path(filename).c_str(), file_io::OPEN_TYPE::READ_BINARY)) {
		log_error("couldn't open xld \"%s\"!", filename);
		return;
	}

	// check header
	fio->get_terminated_block(data, (char)0x00);
	if(data != "XLD0I") {
		fio->close();
		log_error("\"%s\" is no valid XLD file!", filename);
		return;
	}

	// get object count
	const unsigned short int read_obj_count = fio->get_usint();
	const size_t object_count = AR_SWAP_USINT(read_obj_count);

	// get object lengths
	objects.reserve(object_count);
	for(size_t i = 0; i < object_count; i++) {
		objects.push_back(new xld_object());
		const unsigned int read_length = fio->get_uint();
		objects.back()->length = AR_SWAP_UINT(read_length);
		objects.back()->data = new unsigned char[objects.back()->length];
	}

	// get objects
	for(vector<xld_object*>::iterator obj_iter = objects.begin(); obj_iter != objects.end(); obj_iter++) {
		fio->get_block((char*)(*obj_iter)->data, (*obj_iter)->length);
	}

	// close file
	fio->close();
}

void xld::close() {
	for(vector<xld_object*>::iterator obj_iter = objects.begin(); obj_iter != objects.end(); obj_iter++) {
		delete [] (*obj_iter)->data;
	}
	objects.clear();
}

size_t xld::get_object_count() const {
	return objects.size();
}

const xld::xld_object* xld::get_object(const size_t& num) const {
	if(num >= objects.size()) {
		log_error("invalid xld object number %d!", num);
		return nullptr;
	}
	return objects[num];
}

const vector<xld::xld_object*>& xld::get_objects() const {
	return objects;
}

const xld::xld_object* xld::get_object(const size_t& num, const xld* const* xlds, const size_t max_value) {
	if(max_value != 0 && num >= max_value) {
		log_error("invalid object number: %u!", num);
		return nullptr;
	}

	return xlds[num / 100]->get_object(num % 100);
}
