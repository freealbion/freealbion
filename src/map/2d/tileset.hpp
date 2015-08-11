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
 
#ifndef __AR_TILESET_HPP__
#define __AR_TILESET_HPP__

#include "ar_global.hpp"
#include "map/map_defines.hpp"
#include "gfx/palette.hpp"
#include "gfx/gfxconv.hpp"
#include "gfx/scaling.hpp"
#include "gfx/albion_texture.hpp"

/*! @class tileset
 *  @brief tileset loader
 *  @author flo
 *  
 *  loads and handles the tilesets
 */

class tileset {
public:
	tileset(const pal* palettes);
	~tileset();

	struct tile_object {
		unsigned char special_1 { 0 };
		unsigned char collision { 0 };
		unsigned short int special_2 { 0 };
		size_t num { 0 };
		size_t ani_num { 0 };
		size_t ani_tiles { 0 };

		float2 tex_coord;
		bool palette_shift { false };
		
		TILE_LAYER layer_type { TILE_LAYER::UNKNOWN };

		unsigned int upper_bytes { 0 };
		unsigned int lower_bytes { 0 };

		tile_object() = default;
	};

	struct tileset_object {
		size_t tile_count;
		tile_object* tiles { nullptr };
		size_t tile_obj_count { 0 };
		a2e_texture tileset {};
		bool loaded { false };
		
		albion_texture::albion_texture_single_object_ps tex_info_obj {};
		const vector<vector<float2>>* tex_coords { nullptr };
		float2 tile_tc_size {};
	};

	void load(const size_t& num, const size_t& palette);
	void handle_animations(set<unsigned int>& modified_tiles);
	
	const tileset_object& get_tileset(const size_t& num) const;
	const tileset_object& get_cur_tileset() const;
	TILE_LAYER get_layer_type(const unsigned char& ch) const;
	const float2 get_tile_tex_coord_size() const;
	const vector<vector<float2>>* get_tex_coords() const;
	
protected:
	const pal* palettes;

	vector<tileset_object> tilesets;
	size_t cur_tileset_num;
	const tileset_object* cur_tileset;
	a2e_texture cur_tileset_tex;

	lazy_xld icongfx;

};

#endif
