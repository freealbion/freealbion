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

#ifndef __AR_MAP_HANDLER_H__
#define __AR_MAP_HANDLER_H__

#include "ar_global.h"
#include "conf.h"
#include "map_defines.h"
#include "map2d.h"
#include "map3d.h"
#include "npc2d.h"
#include "player2d.h"
#include "player3d.h"
#include "npcgfx.h"
#include "labdata.h"
#include <gui/gui.h>

class map_handler {
public:
	map_handler();
	~map_handler();
	
	void handle();
	void draw(const gui::DRAW_MODE_UI draw_mode);
	
	void load_map(const size_t& map_num, const size2 player_pos = size2(0, 0), const MOVE_DIRECTION player_direction = MD_NONE);
	MAP_TYPE get_map_type(const size_t& map_num) const;
	
	//
	npcgfx* get_npc_graphics() const;
	
	//
	const size2& get_player_position() const;
	
	// DEBUG: for debugging purposes
	tileset::tile_object* get_tile(unsigned int type);
	unsigned int get_tile_num(unsigned int type);
	const MAP_TYPE& get_active_map_type() const;
	const ssize3 get_3d_tile() const;
	void debug_draw(const DRAW_MODE);

protected:
	tileset* tilesets;
	map2d* maps2d;
	map3d* maps3d;
	npcgfx* npc_graphics;
	player2d* p2d;
	player3d* p3d;
	labdata* lab_data;

	xld* maps1;
	xld* maps2;
	xld* maps3;
	
	gui::draw_callback draw_cb;
	
	//
	size_t last_key_press;
	size_t last_move;
	atomic_t next_dir;
	event::handler key_handler_fnctr;
	bool key_handler(EVENT_TYPE type, shared_ptr<event_object> obj);
	
	MAP_TYPE active_map_type;

};

#endif
