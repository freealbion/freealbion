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

#ifndef __AR_MAP3D_H__
#define __AR_MAP3D_H__

#include "ar_global.h"
#include "conf.h"
#include "map_defines.h"
#include "xld.h"
#include "labdata.h"
#include "map_npcs.h"
#include "map_events.h"
#include "map_objects.h"
#include "map_tiles.h"
#include "background3d.h"

class npc3d;
class map3d {
public:
	map3d(labdata* lab_data, xld* maps1, xld* maps2, xld* maps3);
	~map3d();

	void load(const size_t& map_num);
	void unload();
	bool is_3d_map(const size_t& map_num) const;
	
	void handle();
	void draw() const;
	
	bool collide(const MOVE_DIRECTION& direction, const size2& cur_position, const CHARACTER_TYPE& char_type) const;

	// DEBUG: for debugging purposes
	const ssize3 get_tile() const;
	
protected:
	labdata* lab_data;
	const xld* map_xlds[3];

	background3d* bg3d;
	bool bg_loaded;
	
	// npcs data
	vector<npc3d*> npcs;
	map_npcs* mnpcs;
	map_events mevents;
	float3* npcs_vertices;
	float3* npcs_ws_positions;
	float2* npcs_tex_coords;
	float4* npcs_tc_restrict;
	index3** npcs_indices;
	size_t npc_object_count;
	map_objects* npcs_model;
	
	unsigned int* ow_tiles;
	unsigned int* floor_tiles;
	unsigned int* ceiling_tiles;

	// gl
	// floors/ceilings
	float3* fc_vertices;
	float2* fc_tex_coords;
	float4* fc_tc_restrict;
	index3** fc_indices;
	size_t fc_tile_count;
	map_tiles* fc_tiles_model;
	
	// walls
	float3* wall_vertices;
	float2* wall_tex_coords;
	float4* wall_tc_restrict;
	index3** wall_indices;
	size_t wall_tile_count;
	map_tiles* wall_model;

	// objects
	float3* obj_vertices;
	float3* obj_ws_positions;
	float2* obj_tex_coords;
	float4* obj_tc_restrict;
	index3** obj_indices;
	map_objects* objects_model;
	
	light* player_light;

	//
	bool map_loaded;
	size_t cur_map_num;
	size_t cur_labdata_num;
	size2 map_size;
	size_t map_palette;
	float tile_size;
	
	//
	size_t last_tile_animation;
	size_t fc_ani_count;
	size_t fc_ani_offset;
	size_t wall_ani_count;
	size_t wall_ani_offset;
	size_t obj_ani_count;
	size_t obj_ani_offset;
	// <type, tile number> (type: 0 = floor/ceiling, 1 = wall, 2 = object)
	vector<pair<unsigned int, unsigned int> > animated_tiles;

};

#endif
