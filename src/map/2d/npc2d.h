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
 
#ifndef __AR_NPC2D_H__
#define __AR_NPC2D_H__

#include "ar_global.h"
#include "map_defines.h"
#include "map2d.h"
#include "npcgfx.h"
#include "map_npcs.h"
#include "npc.h"

class npc2d : public npc {
public:
	npc2d(map2d* map2d_obj, npcgfx* npc_graphics);
	~npc2d();

	virtual void draw(const NPC_DRAW_STAGE& draw_stage) const;
	virtual void handle();
	virtual void move(const MOVE_DIRECTION& direction);
	virtual void move(const size2& move_pos);

protected:
	map2d* map2d_obj;
	npcgfx* npc_graphics;

	size_t state;

};

#endif
