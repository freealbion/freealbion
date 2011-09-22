/*
 *  Albion Remake
 *  Copyright (C) 2007 - 2011 Florian Ziesche
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

#ifndef __AR_NPC_H__
#define __AR_NPC_H__

#include "ar_global.h"
#include "map_defines.h"
#include "map_npcs.h"
#include "clock.h"

class npc {
public:
	npc();
	virtual ~npc();
	
	virtual void handle() = 0;
	virtual void move(const MOVE_DIRECTION& direction) = 0;
	virtual void move(const size2& move_pos) = 0;
	
	virtual void set_pos(const size_t& x, const size_t& y);
	virtual const size2& get_pos() const;
	virtual const float2 get_interpolated_pos() const;
	
	virtual void set_view_direction(const MOVE_DIRECTION& vdirection);
	
	virtual void set_npc_data(const map_npcs::map_npc* npc_data);
	virtual unsigned int get_object_num() const;
	
	virtual void set_enabled(const bool& state);
	virtual bool is_enabled() const;
	
	// used for player event handling
	virtual bool has_moved() const;
	virtual void set_moved(const bool& state);
	
protected:
	const map_npcs::map_npc* npc_data;
	
	size2 pos;
	size2 next_pos;
	float pos_interp;
	
	CHARACTER_TYPE char_type;
	MOVE_DIRECTION view_direction;
	
	size_t time_per_tile;
	size_t last_frame;
	size_t last_anim_frame;
	size_t last_move;
	
	bool enabled;
	bool moved;
	
	virtual void compute_move();
	
	virtual void clock_tick_cb(size_t tick);
	clock_callback* clock_cb;
	
};

#endif
