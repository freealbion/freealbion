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

#include "xld.hpp"

string xld::xld_path = "";

static bool xld_read_and_init(const string& filename,
							  vector<xld::xld_object>& objects,
							  unsigned int& data_block_length,
							  function<void(file_io&)> init_func = [](file_io&){}) {
	// open file
	file_io file(xld::make_xld_path(filename), file_io::OPEN_TYPE::READ_BINARY);
	if(!file.is_open()) {
		log_error("couldn't open xld \"%s\"!", filename);
		return false;
	}
	
	// check header
	static constexpr auto xld_header = CS("XLD0I");
	static constexpr auto header_size = xld_header.size();
	char header[header_size];
	file.get_block(header, header_size); // note, this also gets the \0
	if(header != xld_header) {
		log_error("\"%s\" is not a valid XLD file!", filename);
		return false;
	}
	
	// get object count
	const size_t object_count = file.get_swapped_usint();
	
	// get object lengths
	objects.reserve(object_count);
	data_block_length = 0;
	const unsigned int data_start_offset = (unsigned int)((size_t)file.get_current_read_offset() + object_count * sizeof(unsigned int));
	for(size_t i = 0; i < object_count; i++) {
		const unsigned int read_length = file.get_swapped_uint();
		objects.emplace_back(xld::xld_object { read_length, data_block_length + data_start_offset, nullptr });
		data_block_length += read_length;
	}
	objects.shrink_to_fit();
	
	init_func(file);
	
	return true;
}

xld::xld(const string& filename) {
	if(!xld_read_and_init(filename, objects, data_block_length,
						  [this](file_io& file) {
							  // read xld data block
#if defined(DEBUG)
							  // check block length against remaining file size
							  const size_t bytes_left = (size_t)(file.get_filesize() - file.get_current_read_offset());
							  if(bytes_left != data_block_length) {
								  log_error("XLD \"%s\" has an invalid size: %u bytes expected, %u bytes left!",
											data_block_length, bytes_left);
							  }
#endif
							  data_block = make_unique<unsigned char[]>(data_block_length);
							  file.get_block((char*)data_block.get(), data_block_length);
							  
							  // assign block pointers
							  unsigned char* block_ptr = data_block.get();
							  for(auto& obj : objects) {
								  obj.data = block_ptr;
								  block_ptr += obj.size;
							  }
						  })) {
		return;
	}
}

void xld::set_xld_path(const string& xld_path_) {
	xld::xld_path = xld_path_;
}

const string& xld::get_xld_path() {
	return xld::xld_path;
}

const string xld::make_xld_path(const string& filename) {
	return xld::xld_path + filename;
}

size_t xld::get_object_count() const {
	return objects.size();
}

const xld::xld_object* xld::get_object(const size_t& num) const {
	if(num >= objects.size()) {
		log_error("invalid xld object number %d!", num);
		return nullptr;
	}
	return &objects[num];
}

const vector<xld::xld_object>& xld::get_objects() const {
	return objects;
}

const xld::xld_object* xld::get_object(const size_t& num, const xld* const* xlds, const size_t max_value) {
	if(max_value != 0 && num >= max_value) {
		log_error("invalid object number: %u!", num);
		return nullptr;
	}
	return xlds[num / 100]->get_object(num % 100);
}

////////////////////////////////////////////////////////////////////////////////
// lazy_xld

lazy_xld::lazy_xld(const string& filename) {
	if(!xld_read_and_init(filename, objects, data_block_length,
						  [this](file_io& init_file) {
							  // move initial file object to this class
							  file = move(init_file);
						  })) {
		return;
	}
	data_ptrs.resize(objects.size());
}

lazy_xld::lazy_xld(lazy_xld&& lxld) :
objects(move(lxld.objects)), data_ptrs(move(lxld.data_ptrs)),
data_block_length(lxld.data_block_length), file(move(lxld.file)) {
}

size_t lazy_xld::get_object_count() const {
	return objects.size();
}

const unsigned int& lazy_xld::get_object_size(const size_t& num) const {
	return objects[num].size;
}

const unsigned int& lazy_xld::get_object_offset(const size_t& num) const {
	return objects[num].offset;
}

const shared_ptr<vector<unsigned char>> lazy_xld::get_object_data(const size_t& num) {
	// check if the data is currently loaded - if so, return a shared_ptr to it
	auto existing_data = data_ptrs[num].lock();
	if(existing_data) return existing_data;
	
	// we can't create a c array via make_shared/shared_ptr, so use a vector instead
	// (note: there is a proposal, but this isn't part of c++14 -> N3641)
	auto data = make_shared<vector<unsigned char>>(objects[num].size);
	
	// seek and read
	file.seek_read(objects[num].offset);
	file.get_block((char*)data.get()->data(), (streamsize)objects[num].size);
	
	// set weak_ptr to new data shared_ptr
	data_ptrs[num] = data;
	
	return data;
}

const vector<xld::xld_object>& lazy_xld::get_objects() const {
	return objects;
}

file_io& lazy_xld::get_file() {
	return file;
}
