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

#ifndef __AR_MAP_HANDLER_H__
#define __AR_MAP_HANDLER_H__

#include "global.h"
#include "conf.h"
#include "map_defines.h"
#include "map2d.h"
#include "map3d.h"
#include "npc2d.h"
#include "player2d.h"
#include "npcgfx.h"
#include "labdata.h"

class map_handler {
public:
	map_handler(pal* palettes);
	~map_handler();
	
	void handle();
	void draw();
	
	void load_map(const size_t& map_num);
	
	//
	npcgfx* get_npc_graphics() const;
	
	//
	const size2& get_player_position() const;
	
	// DEBUG: for debugging purposes
	tileset::tile_object* get_tile(unsigned int type);
	unsigned int get_tile_num(unsigned int type);

protected:
	pal* palettes;
	tileset* tilesets;
	map2d* maps2d;
	//map3d* maps3d;
	npcgfx* npc_graphics;
	player2d* p;
	labdata* lab_data;
	
	//
	size_t last_key_press;
	size_t last_move;
	MOVE_DIRECTION next_dir;
	void handle_key_press(event::GUI_EVENT_TYPE type, GUI_ID id);
	
};

#endif
