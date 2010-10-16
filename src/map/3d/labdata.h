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

class labdata {
public:
	labdata();
	~labdata();
	
	void load(const size_t& labdata_num);
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
	};
	
	struct lab_object_info {
		unsigned int texture;
		unsigned int animation;
		unsigned int x_size;
		unsigned int y_size;
		unsigned int x_offset; //?
		unsigned int y_offset; //?
	};
	
	struct lab_object {
		AUTOGFX_TYPE type;
		unsigned int object_num;
		lab_object_info* object_info;
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
	};
	
	const lab_object* get_object(const size_t& num) const;
	const lab_floor* get_floor(const size_t& num) const;
	const lab_wall* get_wall(const size_t& num) const;
	
protected:
	vector<lab_object*> objects;
	vector<lab_object_info*> object_infos;
	
	vector<lab_floor*> floors;
	vector<lab_wall*> walls;

	xld* labdata_xlds[3];
	size_t cur_labdata_num;
	
};

#endif
