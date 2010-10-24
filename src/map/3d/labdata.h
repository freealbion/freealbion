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

#ifndef __AR_LABDATA_H__
#define __AR_LABDATA_H__

#include "global.h"
#include "map_defines.h"
#include "xld.h"
#include "palette.h"
#include "gfxconv.h"
#include "scaling.h"
#include "conf.h"

class labdata {
public:
	labdata();
	~labdata();
	
	void load(const size_t& labdata_num, const size_t& palette);
	void unload();
	
	enum AUTOGFX_TYPE {
		AGT_UNKNOWN,
		AGT_NORMAL,
		AGT_RIDDLE_MOUTH,
		AGT_TELEPORTER,
		AGT_SPINNER,
		AGT_TRAP,
		AGT_TRAP_DOOR,
		AGT_SPECIAL,
		AGT_MONSTER,
		AGT_CLOSED_DOOR,
		AGT_OPENED_DOOR,
		AGT_MERCHANT,
		AGT_TAVERN,
		AGT_CLOSED_CHEST,
		AGT_MAP_EXIT,
		AGT_OPENED_CHEST,
		AGT_TRASH_HEAP,
		AGT_NPC,
		AGT_GOTO_POINT,
		AGT_EVENT
	};
	
	struct lab_floor {
		bool collision;
		unsigned int texture;
		unsigned int animation;
		float2 tex_coord;
	};
	
	struct lab_object_info {
		unsigned char type;
		unsigned int collision; // 3 bytes
		unsigned int texture;
		unsigned int animation;
		unsigned int x_size;
		unsigned int y_size;
		unsigned int x_scale;
		unsigned int y_scale;

		float2 tex_coord_begin;
		float2 tex_coord_end;
	};
	
	struct lab_object {
		AUTOGFX_TYPE type;

		size_t sub_object_count;
		ssize3 offset[8];
		size_t object_num[8];
		lab_object_info* sub_objects[8];
	};
	
	struct lab_wall_overlay {
		unsigned int texture;
		unsigned int x_size;
		unsigned int y_size;
		unsigned int x_offset;
		unsigned int y_offset;
	};
	
	struct lab_wall {
		AUTOGFX_TYPE type;
		unsigned int texture;
		unsigned int animation; //?
		unsigned int palette_num; //?
		unsigned int x_size;
		unsigned int y_size;
		unsigned int overlay_count;
		vector<lab_wall_overlay*> overlays;
		float2 tex_coord_begin;
		float2 tex_coord_end;
	};
	
	const lab_object* get_object(const size_t& num) const;
	const lab_floor* get_floor(const size_t& num) const;
	const lab_wall* get_wall(const size_t& num) const;
	a2ematerial* get_material() const;
	a2ematerial* get_object_material() const;
	
protected:
	vector<lab_object*> objects;
	vector<lab_object_info*> object_infos;
	
	vector<lab_floor*> floors;
	vector<lab_wall*> walls;

	xld* labdata_xlds[3];
	size_t cur_labdata_num;
	
	a2e_texture floors_tex;
	a2e_texture walls_tex;
	a2e_texture objects_tex;
	a2ematerial* lab_material;
	a2ematerial* lab_obj_material;

	//
	xld* floor_xlds[3];
	xld* object_xlds[4];
	xld* overlay_xlds[3];
	xld* wall_xlds[2];
	
};

#endif
