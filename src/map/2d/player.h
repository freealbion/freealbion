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
 
#ifndef __AR_PLAYER_H__
#define __AR_PLAYER_H__

#include "global.h"
#include "map_defines.h"
#include "map2d.h"
#include "npcgfx.h"

/*! @class player
 *  @brief player class
 *  @author flo
 *  
 *  player class
 */

class player {
public:
	player(map2d* map2d_obj, npcgfx* npc_graphics);
	~player();

	void draw() const;
	void handle();
	void move(const MOVE_DIRECTION& direction);

	void set_pos(const size_t& x, const size_t& y);
	const size2& get_pos() const;

protected:
	map2d* map2d_obj;
	npcgfx* npc_graphics;
	
	size2 pos;
	size2 next_pos;
	float pos_interp;
	
	//NPC_STATE state;
	size_t state;

	size_t last_frame;

};

#endif
