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

#ifndef __AR_ALBION_TEXTURE_HPP__
#define __AR_ALBION_TEXTURE_HPP__

#include "ar_global.hpp"
#include "conf.hpp"
#include "gfx/gfxconv.hpp"
#include "gfx/scaling.hpp"
#include "map/map_defines.hpp"
#include <rendering/texture_object.hpp>

class albion_texture {
public:
	albion_texture();
	~albion_texture();

	enum class TEXTURE_INFO {
		NONE,
		MULTI_XLD,
		SINGLE_OBJECT,
		SINGLE_OBJECT_PALETTE_SHIFT,
	};
	enum class TEXTURE_SPACING {
		NONE,
		MIRROR,
		TILE,
		TRANSPARENT
	};
	
	struct albion_texture_info {
		TEXTURE_INFO type { TEXTURE_INFO::NONE };
		unsigned int palette_shift { 0 };
		unsigned int offset { 0 };
		bool overwrite_alpha { false };
		unsigned int replacement_alpha { 0 };
	};
	struct albion_texture_multi_xld : public albion_texture_info {
		unsigned int tex_num { 0 };
		float2 tex_coord_begin {};
		float2 tex_coord_end {};
		albion_texture_multi_xld() : albion_texture_info() {
			type = TEXTURE_INFO::MULTI_XLD;
		}
	};
	struct albion_texture_single_object : public albion_texture_info {
		unsigned int object_count { 0 };
		const unsigned char* data { nullptr };
		albion_texture_single_object() : albion_texture_info() {
			type = TEXTURE_INFO::SINGLE_OBJECT;
		}
	};
	struct albion_texture_single_object_ps : public albion_texture_single_object {
		vector<vector<float2>> tex_coords;
		albion_texture_single_object_ps() : albion_texture_single_object() {
			type = TEXTURE_INFO::SINGLE_OBJECT_PALETTE_SHIFT;
		}
	};

	static a2e_texture create(const MAP_TYPE& map_type,
							  const size2& texture_size,
							  const size2& tile_size,
							  const size_t& palette,
							  const vector<albion_texture_info*>& tex_info,
							  lazy_xld* xlds,
							  const TEXTURE_SPACING spacing_type = TEXTURE_SPACING::NONE,
							  const size_t spacing_size = 0,
							  bool custom_mipmaps = false,
							  const TEXTURE_FILTERING filtering = TEXTURE_FILTERING::POINT);
	
	static void build_mipmaps(const a2e_texture& tex, const unsigned int* tex_data, const TEXTURE_FILTERING filtering);
	
	static void add_spacing(unsigned int* surface,
							const size2& texture_size,
							const unsigned int* tile_data,
							const size2& tile_size,
							const size_t& spacing,
							const TEXTURE_SPACING& spacing_type,
							const size2 offset);

protected:

};

#endif
