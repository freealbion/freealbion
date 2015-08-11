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
 
#ifndef __AR_XLD_HPP__
#define __AR_XLD_HPP__

#if !defined(XLD_UNPACK)
#include "ar_global.hpp"
#endif
#include <floor/core/cpp_headers.hpp>
#include <floor/core/file_io.hpp>

#define AR_GET_USINT(data_, offset_) ((unsigned short int)((data_[offset_+1u] << 8u) | data_[offset_]))
#define AR_GET_SINT(data_, offset_) ((short int)((data_[offset_+1u] << 8u) | data_[offset_]))
#define AR_GET_UINT(data_, offset_) ((unsigned int)((data_[offset_+3u] << 24u) | (data_[offset_+2u] << 16u) | \
													(data_[offset_+1u] << 8u) | data_[offset_]))

/*! @class xld
 *  @brief xld loader
 *  @author flo
 *  
 *  loads and handles xld files
 */

class xld {
public:
	xld(const string& filename);

	struct xld_object {
		const unsigned int size;
		const unsigned int offset;
		const unsigned char* data;
	};
	
	size_t get_object_count() const;
	const xld_object* get_object(const size_t& num) const;
	const vector<xld_object>& get_objects() const;
	
	//
	static void set_xld_path(const string& xld_path);
	static const string& get_xld_path();
	static const string make_xld_path(const string& filename);

	static const xld_object* get_object(const size_t& num, const xld* const* xlds, const size_t max_value = 0);
	
protected:
	static string xld_path;
	
	//! contains all xld data
	unique_ptr<unsigned char[]> data_block { nullptr };
	//! contains the <length, data ptr> pair to each xld object
	vector<xld_object> objects;
	//! the size of the xlds data block (post xld object size table -> eof)
	unsigned int data_block_length { 0u };

};

//! lazy xld loader that only reads the xld header.
//! all object data is only loaded when requested.
class lazy_xld {
public:
	lazy_xld() noexcept {}
	lazy_xld(const string& filename);
	lazy_xld(lazy_xld&& lxld);
	lazy_xld& operator=(lazy_xld&&) = default;
	
	size_t get_object_count() const;
	const unsigned int& get_object_size(const size_t& num) const;
	const unsigned int& get_object_offset(const size_t& num) const;
	const shared_ptr<vector<unsigned char>> get_object_data(const size_t& num);
	const vector<xld::xld_object>& get_objects() const;
	
	file_io& get_file();
	
protected:
	//! contains the <length, data ptr> pair to each xld object
	vector<xld::xld_object> objects;
	vector<weak_ptr<vector<unsigned char>>> data_ptrs;
	//! the size of the xlds data block (post xld object size table -> eof)
	unsigned int data_block_length { 0u };
	
	//! the xld file needs to be opened the whole time
	file_io file;
	
};

#endif
