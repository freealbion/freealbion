/*
 *  Albion Remake
 *  Copyright (C) 2007 - 2012 Florian Ziesche
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

#ifndef __AR_ALBION_TEXTURE_H__
#define __AR_ALBION_TEXTURE_H__

#include "ar_global.h"
#include "conf.h"
#include "gfxconv.h"
#include "scaling.h"
#include "xld.h"
#include <rendering/texture_object.h>

class albion_texture {
public:
	albion_texture();
	~albion_texture();

	enum ALBION_TEXTURE_INFO_TYPE {
		ATIT_NONE,
		ATIT_MULTI_XLD,
		ATIT_SINGLE_OBJECT,
		ATIT_SINGLE_OBJECT_PALETTE_SHIFT,
	};
	enum TEXTURE_SPACING_TYPE {
		TST_NONE,
		TST_MIRROR,
		TST_TILE,
		TST_TRANSPARENT
	};
	
	struct albion_texture_info {
		ALBION_TEXTURE_INFO_TYPE type;
		unsigned int palette_shift;
		unsigned int offset;
		albion_texture_info() : type(ATIT_NONE), palette_shift(0), offset(0) {}
	};
	struct albion_texture_multi_xld : public albion_texture_info {
		unsigned int tex_num;
		float2 tex_coord_begin;
		float2 tex_coord_end;
		albion_texture_multi_xld() : albion_texture_info(), tex_num(0), tex_coord_begin(), tex_coord_end() {
			type = ATIT_MULTI_XLD;
		}
	};
	struct albion_texture_single_object : public albion_texture_info {
		unsigned int object_count;
		const xld::xld_object* object;
		albion_texture_single_object() : albion_texture_info(), object_count(0), object(NULL) {
			type = ATIT_SINGLE_OBJECT;
		}
	};
	struct albion_texture_single_object_ps : public albion_texture_single_object {
		vector<vector<float2>> tex_coords;
		albion_texture_single_object_ps() : albion_texture_single_object(), tex_coords() {
			type = ATIT_SINGLE_OBJECT_PALETTE_SHIFT;
		}
	};

	static a2e_texture create(const size_t& map_type, const size2& texture_size, const size2& tile_size, const size_t& palette, const vector<albion_texture_info*>& tex_info, xld** xlds, const TEXTURE_SPACING_TYPE spacing_type = TST_NONE, const size_t spacing_size = 0, bool custom_mipmaps = false, const texture_object::TEXTURE_FILTERING filtering = texture_object::TF_POINT);
	
	static void build_mipmaps(const a2e_texture& tex, const unsigned int* tex_data, const texture_object::TEXTURE_FILTERING filtering);
	
	static void add_spacing(unsigned int* surface, const size2& texture_size, const unsigned int* tile_data, const size2& tile_size, const size_t& spacing, const TEXTURE_SPACING_TYPE& spacing_type, const size2 offset);

protected:

};

#endif
