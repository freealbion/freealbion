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
 
#ifndef __AR_MAP2D_H__
#define __AR_MAP2D_H__

#include "ar_global.h"
#include "conf.h"
#include "map_defines.h"
#include "xld.h"
#include "tileset.h"
#include "npcgfx.h"
#include "map_npcs.h"
#include "map_events.h"
#include "player2d.h"
#include <rendering/gl_funcs.h>

/*! @class map2d
 *  @brief displays 2d maps
 *  @author flo
 *  
 *  opens and displays 2d maps
 */

class npc2d;
class map2d {
public:
	map2d(tileset* tilesets, npcgfx* npc_graphics, xld* maps1, xld* maps2, xld* maps3);
	~map2d();

	void load(const size_t& map_num);
	void unload();
	bool is_2d_map(const size_t& map_num) const;
	
	void handle();
	void draw(const MAP_DRAW_STAGE& draw_stage, const NPC_DRAW_STAGE& npc_draw_stage) const;

	//void add_npc(const size_t& id, const size_t& x, const size_t& y);
	//void move_npc(const size_t& id, const size_t& x, const size_t& y);


	void set_pos(const size_t& x, const size_t& y);
	void set_initial_position(const size2& init_pos);
	const size2& get_size() const;
	float get_tile_size() const;
	const size_t& get_palette() const;
	const float2& get_screen_position() const;

	bool collide(const MOVE_DIRECTION& direction, const size2& cur_position, const CHARACTER_TYPE& char_type) const;
	
	map_events& get_map_events();
	
	void set_player(player2d* player);

	// DEBUG: for debugging purposes
	tileset::tile_object* get_tile(unsigned int type);
	unsigned int get_tile_num(unsigned int type);

protected:
	player2d* p2d;
	tileset* tilesets;
	npcgfx* npc_graphics;

	xld* maps1;
	xld* maps2;
	xld* maps3;

	vector<npc2d*> npcs;
	map_npcs* mnpcs;
	map_events mevents;

	//
	size_t cur_map_num;
	size_t cur_tileset_num;
	size2 map_size;
	size_t map_palette;
	unsigned char* cur_map_data;
	unsigned int* underlay_tiles;
	unsigned int* overlay_tiles;
	
	// ogl buffers
	struct map_layer {
		GLuint vertices_vbo;
		GLuint tex_coords_vbo;
		GLuint indices_vbo;
		float3* vertices;
		float2* tex_coords;
		index3* indices;
		unsigned int* tile_nums;
		size_t index_count;
		size_t ani_offset;
		
		void clear() {
			if(glIsBuffer(vertices_vbo)) glDeleteBuffers(1, &vertices_vbo);
			if(glIsBuffer(tex_coords_vbo)) glDeleteBuffers(1, &tex_coords_vbo);
			if(glIsBuffer(indices_vbo)) glDeleteBuffers(1, &indices_vbo);
			
			if(vertices != NULL) {
				delete [] vertices;
				vertices = NULL;
			}
			if(tex_coords != NULL) {
				delete [] tex_coords;
				tex_coords = NULL;
			}
			if(indices != NULL) {
				delete [] indices;
				indices = NULL;
			}
			if(tile_nums != NULL) {
				delete [] tile_nums;
				tile_nums = NULL;
			}
			index_count = 0;
		}
		
		map_layer() : vertices_vbo(0), tex_coords_vbo(0), indices_vbo(0), vertices(NULL), tex_coords(NULL), indices(NULL), tile_nums(NULL), index_count(0), ani_offset(0) {}
		~map_layer() { clear(); }
	};
	map_layer layers[2]; // under/overlay
	size_t last_tile_animation;

	//
	float2 compute_target_position();
	ssize2 next_position;
	float2 screen_position;
	
	ssize2 normal_player_offset;

	bool map_loaded;
	bool continent_map;

};

#endif
