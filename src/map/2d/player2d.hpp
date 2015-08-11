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
 
#ifndef __AR_PLAYER2D_HPP__
#define __AR_PLAYER2D_HPP__

#include "ar_global.hpp"
#include "map/map_defines.hpp"
#include "map/2d/npcgfx.hpp"
#include "map/2d/npc2d.hpp"

/*! @class player
 *  @brief player class
 *  @author flo
 *  
 *  player class
 */

class map2d;
class player2d : public npc2d {
public:
	player2d(map2d* map2d_obj, npcgfx* npc_graphics);
	virtual ~player2d();

	virtual void draw(const NPC_DRAW_STAGE& draw_stage) const;
	virtual void handle();
	virtual void move(const MOVE_DIRECTION direction);
	virtual void set_view_direction(const MOVE_DIRECTION& vdirection);
	virtual float2 compute_screen_position_from_interpolated(const float2& interp_pos) const;

protected:
	
	virtual void compute_move();

};

#endif
