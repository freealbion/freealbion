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

#ifndef __AR_MAP_DEFINES_H__
#define __AR_MAP_DEFINES_H__

#include "ar_global.h"

// experimental: 5 - 6 tiles per second (-> at least 166 - 200ms between move cmds)
#define TILES_PER_SECOND (6) // assume 6 tiles/s for the moment, looks smoother
#define TIME_PER_TILE (1000 / TILES_PER_SECOND)
#define TILES_PER_SECOND_NPC (3) // assume 3 tiles/s for the moment
#define TIME_PER_TILE_NPC (1000 / TILES_PER_SECOND_NPC)
#define TILES_PER_SECOND_NPC3D (1) // assume 3 tiles/s for the moment
#define TIME_PER_TILE_NPC3D (1000 / TILES_PER_SECOND_NPC3D)
#define TIME_PER_ANIMATION_FRAME (150)
#define TIME_PER_TILE_ANIMATION_FRAME (100)

enum NPC_STATE {
	S_BACK1		= 0x01,
	S_BACK2		= 0x02,
	S_BACK3		= 0x04,
	S_RIGHT1	= 0x11,
	S_RIGHT2	= 0x12,
	S_RIGHT3	= 0x14,
	S_FRONT1	= 0x21,
	S_FRONT2	= 0x22,
	S_FRONT3	= 0x24,
	S_LEFT1		= 0x31,
	S_LEFT2		= 0x32,
	S_LEFT3		= 0x34,
	S_SIT_BACK	= 0x40,
	S_SIT_RIGHT	= 0x50,
	S_SIT_FRONT	= 0x60,
	S_SIT_LEFT	= 0x70,
	S_LAY		= 0x80
};

enum NPC3D_STATE {
	N3DS_NONE,
	N3DS_FRONT1,
	N3DS_FRONT2,
	N3DS_FRONT3,
};

enum MOVE_DIRECTION {
	MD_NONE		= 0,
	MD_LEFT		= 1,
	MD_RIGHT	= 2,
	MD_UP		= 4,
	MD_DOWN		= 8
};

enum MOVEMENT_TYPE {
	MT_RANDOM,
	MT_TRACK
};

enum TILE_DEBUG_TYPE {
	TBT_TRANSPARENT_1 = 0,
	TBT_TRANSPARENT_2 = 2,
	TBT_ORANGE_DOT = 4,
	TBT_ORANGE = 6,
};

enum MAP_DRAW_STAGE {
	MDS_UNDERLAY,
	MDS_OVERLAY,
	MDS_NPCS,
	MDS_DEBUG
};

enum NPC_DRAW_STAGE {
	NDS_NONE,
	NDS_PRE_UNDERLAY,
	NDS_PRE_OVERLAY,
	NDS_POST_OVERLAY,
	NDS_DEBUG
};

enum TILE_LAYER {
	TL_UNKNOWN,
	TL_UNDERLAY,	//! always underlay
	TL_DYNAMIC_1,	//! underlay if player y > tile y, overlay if player y <= tile y
	TL_DYNAMIC_2,	//! same as TL_DYNAMIC_1, but tile y + 1
	TL_DYNAMIC_3,	//! same as TL_DYNAMIC_1, but tile y + 2
	TL_OVERLAY		//! always overlay
};

enum CHARACTER_TYPE {
	CT_PLAYER,
	CT_NPC
};

enum MAP_TYPE {
	MT_2D_MAP,
	MT_3D_MAP,
	MT_NONE
};

#endif
