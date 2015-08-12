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

#ifndef __AR_MAP_DEFINES_HPP__
#define __AR_MAP_DEFINES_HPP__

#include "ar_global.hpp"
#include <floor/core/util.hpp>

// experimental: 5 - 6 tiles per second (-> at least 166 - 200ms between move cmds)
#define TILES_PER_SECOND (6) // assume 6 tiles/s for the moment, looks smoother
#define TIME_PER_TILE (1000 / TILES_PER_SECOND)
#define TILES_PER_SECOND_NPC (3) // assume 3 tiles/s for the moment
#define TIME_PER_TILE_NPC (1000 / TILES_PER_SECOND_NPC)
#define TILES_PER_SECOND_NPC3D (1) // assume 3 tiles/s for the moment
#define TIME_PER_TILE_NPC3D (1000 / TILES_PER_SECOND_NPC3D)
#define TIME_PER_ANIMATION_FRAME (150)
#define TIME_PER_TILE_ANIMATION_FRAME (100)

#define MAX_MAP_NUMBER (300u)
#define MAX_NPC_NUMBER (600u)

static const float std_tile_size = 16.0f;
static const float std_half_tile_size = std_tile_size/2.0f;

enum class NPC_STATE : unsigned char {
	BACK1		= 0x01,
	BACK2		= 0x02,
	BACK3		= 0x04,
	RIGHT1		= 0x11,
	RIGHT2		= 0x12,
	RIGHT3		= 0x14,
	FRONT1		= 0x21,
	FRONT2		= 0x22,
	FRONT3		= 0x24,
	LEFT1		= 0x31,
	LEFT2		= 0x32,
	LEFT3		= 0x34,
	SIT_BACK	= 0x40,
	SIT_RIGHT	= 0x50,
	SIT_FRONT	= 0x60,
	SIT_LEFT	= 0x70,
	LAY			= 0x80
};

enum class NPC3D_STATE {
	NONE,
	FRONT1,
	FRONT2,
	FRONT3,
};

enum class MOVE_DIRECTION : unsigned int {
	NONE	= (0u),
	LEFT	= (1u << 0u),
	RIGHT	= (1u << 1u),
	UP		= (1u << 2u),
	DOWN	= (1u << 3u)
};
floor_global_enum_ext(MOVE_DIRECTION)
atomic<MOVE_DIRECTION>& operator|=(atomic<MOVE_DIRECTION>& e0, const MOVE_DIRECTION& e1);
atomic<MOVE_DIRECTION>& operator&=(atomic<MOVE_DIRECTION>& e0, const MOVE_DIRECTION& e1);

enum class MOVEMENT_TYPE {
	RANDOM,
	TRACK
};

enum class TILE_DEBUG_TYPE : unsigned char {
	TRANSPARENT_1 = 0,
	TRANSPARENT_2 = 2,
	ORANGE_DOT = 4,
	ORANGE = 6,
};

enum class MAP_DRAW_STAGE {
	UNDERLAY,
	OVERLAY,
	NPCS,
	DEBUGGING
};

enum class NPC_DRAW_STAGE {
	NONE,
	PRE_OVERLAY,
	POST_OVERLAY,
	DEBUGGING
};

enum class TILE_LAYER {
	UNKNOWN,
	UNDERLAY,	//!< always underlay
	DYNAMIC_1,	//!< underlay if player y > tile y, overlay if player y <= tile y
	DYNAMIC_2,	//!< same as TILE_LAYER::DYNAMIC_1, but tile y + 1
	DYNAMIC_3,	//!< same as TILE_LAYER::DYNAMIC_1, but tile y + 2
	OVERLAY		//!< always overlay
};

enum class CHARACTER_TYPE {
	PLAYER,
	NPC
};

enum class MAP_TYPE : unsigned int {
	NONE = 0u,
	MAP_2D,
	MAP_3D,
};

#endif
