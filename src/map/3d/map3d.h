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

#include "global.h"
#include "conf.h"
#include "map_defines.h"
#include "xld.h"
#include "labdata.h"
#include "map_npcs.h"
#include "map_events.h"
#include "map_objects.h"

class map3d {
public:
	map3d(labdata* lab_data, xld* maps1, xld* maps2, xld* maps3);
	~map3d();

	void load(const size_t& map_num);
	void unload();
	bool is_3d_map(const size_t& map_num) const;
	
	void handle();
	void draw() const;

	// DEBUG: for debugging purposes
	const ssize3 get_tile() const;
	
protected:
	labdata* lab_data;
	xld* maps1;
	xld* maps2;
	xld* maps3;
	
	//vector<npc2d*> npcs;
	map_npcs* mnpcs;
	map_events mevents;
	
	unsigned int* ow_tiles;
	unsigned int* floor_tiles;
	unsigned int* ceiling_tiles;

	// gl
	float3* vertices;
	float2* tex_coords;
	index3** indices;
	size_t tile_count;
	a2estatic* map_model;
	
	float3* obj_vertices;
	float3* obj_ws_positions;
	float2* obj_tex_coords;
	index3** obj_indices;
	map_objects* objects_model;
	
	light* player_light;

	//
	bool map_loaded;
	size_t cur_map_num;
	size_t cur_labdata_num;
	size2 map_size;
	size_t map_palette;

};

#endif
