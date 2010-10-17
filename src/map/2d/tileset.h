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
 
#ifndef __AR_TILESET_H__
#define __AR_TILESET_H__

#include "global.h"
#include "map_defines.h"
#include "xld.h"
#include "palette.h"
#include "gfxconv.h"
#include "scaling.h"

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
		TILE_LAYER layer_type;

		unsigned int upper_bytes;
		unsigned int lower_bytes;
	};

	struct tileset_object {
		size_t tile_count;
		unsigned char* tile_data;
		tile_object* tiles;
		size_t tile_obj_count;
		a2e_texture tileset;
		bool loaded;
		
		tileset_object() : tile_count(0), tile_data(NULL), tiles(NULL), tile_obj_count(0), tileset(), loaded(false) {}
	};

	void load(const size_t& num, const size_t& palette);
	void handle_animations(set<unsigned int>& modified_tiles);
	
	const tileset_object& get_tileset(const size_t& num) const;
	const tileset_object& get_cur_tileset() const;
	const TILE_LAYER get_layer_type(const unsigned char& ch) const;
	const float get_tile_tex_coord_size() const;
	
protected:
	const pal* palettes;

	vector<tileset_object*> tilesets;
	size_t cur_tileset_num;
	const tileset_object* cur_tileset;
	a2e_texture cur_tileset_tex;

};

#endif
