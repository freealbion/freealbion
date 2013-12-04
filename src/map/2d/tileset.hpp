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
 
#ifndef __AR_TILESET_HPP__
#define __AR_TILESET_HPP__

#include "ar_global.hpp"
#include "map_defines.hpp"
#include "xld.hpp"
#include "palette.hpp"
#include "gfxconv.hpp"
#include "scaling.hpp"
#include "albion_texture.hpp"

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
		unsigned char special_1;
		unsigned char collision;
		unsigned short int special_2;
		size_t num;
		size_t ani_num;
		size_t ani_tiles;

		float2 tex_coord;
		bool palette_shift;
		
		TILE_LAYER layer_type;

		unsigned int upper_bytes;
		unsigned int lower_bytes;

		tile_object() : tex_coord(), palette_shift(false) {}
	};

	struct tileset_object {
		size_t tile_count;
		unsigned char* tile_data;
		tile_object* tiles;
		size_t tile_obj_count;
		a2e_texture tileset;
		bool loaded;
		
		albion_texture::albion_texture_single_object_ps tex_info_obj;
		const vector<vector<float2>>* tex_coords;
		float2 tile_tc_size;

		tileset_object() : tile_count(0), tile_data(nullptr), tiles(nullptr), tile_obj_count(0), tileset(), loaded(false), tex_info_obj(), tex_coords(nullptr), tile_tc_size() {}
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

	vector<tileset_object*> tilesets;
	size_t cur_tileset_num;
	const tileset_object* cur_tileset;
	a2e_texture cur_tileset_tex;

	xld* icongfx;

};

#endif
