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
 
#ifndef __AR_XLD_H__
#define __AR_XLD_H__

#include "global.h"

#define AR_GET_USINT(data_, offset_) ((data_[offset_+1] << 8) + data_[offset_])
#define AR_GET_UINT(data_, offset_) ((data_[offset_+3] << 24) + (data_[offset_+2] << 16) + (data_[offset_+1] << 8) + data_[offset_])

/*! @class xld
 *  @brief xld loader
 *  @author flo
 *  
 *  loads and handles xld files
 */

class xld {
public:
	xld(const string& filename);
	~xld();

	struct xld_object {
		size_t length;
		unsigned char* data;
		
		xld_object() : length(0), data(NULL) {}
		xld_object(const size_t& length_, unsigned char* data_) : length(length_), data(data_) {}
	};

	void close();
	
	const size_t get_object_count() const;
	const xld_object* get_object(const size_t& num) const;
	const vector<xld_object*>& get_objects() const;
	
	//
	static void set_xld_path(const string& xld_path);
	static const string get_xld_path();
	static const string make_xld_path(const string& filename);
	
protected:
	string data;
	vector<xld_object*> objects;
	static string xld_path;
	
	void load(const string& filename);

};

#endif
