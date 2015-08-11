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

#ifndef __AR_NPC3D_HPP__
#define __AR_NPC3D_HPP__

#include "ar_global.hpp"
#include "map/map_defines.hpp"
#include "map/3d/map3d.hpp"
#include "map/map_npcs.hpp"
#include "map/npc.hpp"

class npc3d : public npc {
public:
	npc3d(map3d* map3d_obj);
	virtual ~npc3d();
	
	virtual void draw() const;
	virtual void handle();
	virtual void move(const MOVE_DIRECTION direction);
	virtual void move(const size2& move_pos);
	
	virtual void set_object_light(object_light_base* l);
	virtual void set_enabled(const bool& state);
	virtual float3 get_offset() const;
	virtual float get_y_scale() const;
	
protected:
	map3d* map3d_obj;
	object_light_base* l;
	
	size_t state;
	bool anim_dir; // 0 = +, 1 = -
	
};

#endif
