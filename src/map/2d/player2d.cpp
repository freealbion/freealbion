/*
 *  Albion Remake
 *  Copyright (C) 2007 - 2014 Florian Ziesche
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

#include "player2d.hpp"
#include "map2d.hpp"

/*! player constructor
 */
player2d::player2d(map2d* map2d_obj, npcgfx* npc_graphics) : npc2d(map2d_obj, npc_graphics) {
	char_type = CHARACTER_TYPE::PLAYER;
	time_per_tile = TIME_PER_TILE;
	map2d_obj->set_player(this);
}

/*! player destructor
 */
player2d::~player2d() {
}

void player2d::draw(const NPC_DRAW_STAGE& draw_stage) const {
	//
	const float tile_size = map2d_obj->get_tile_size();
	const float2& screen_position = map2d_obj->get_screen_position();
	float2 player_pos = pos;
	float2 player_next_pos = next_pos;
	player_pos = player_pos*(1.0f-pos_interp) + player_next_pos*pos_interp;

	float depth_overwrite = (draw_stage == NPC_DRAW_STAGE::PRE_UNDERLAY || draw_stage == NPC_DRAW_STAGE::PRE_OVERLAY) ? 0.0f : -1.0f;
	//static const size_t continent_npcgfx_offset = 500;
	npc_graphics->draw_npc((continent ? 500 : conf::get<size_t>("debug.npcgfx")),
						   state,
						   float2((player_pos.x - screen_position.x)*tile_size,
								  (player_pos.y - 2.0f - screen_position.y)*tile_size),
						   player_pos, depth_overwrite);
}

void player2d::handle() {
	map2d_obj->set_pos(next_pos.x, next_pos.y);
	
	npc2d::handle();
}

void player2d::compute_move() {
	// do nothing
}

void player2d::move(const MOVE_DIRECTION direction) {
	npc2d::move(direction);
	
	// player moved to the next tile
	set_moved(true);
}

void player2d::set_view_direction(const MOVE_DIRECTION& vdirection) {
	npc2d::set_view_direction(vdirection);
	
	switch(vdirection) {
		case MOVE_DIRECTION::UP:
			state = (size_t)NPC_STATE::BACK1;
			break;
		case MOVE_DIRECTION::RIGHT:
			state = (size_t)NPC_STATE::RIGHT1;
			break;
		case MOVE_DIRECTION::DOWN:
			state = (size_t)NPC_STATE::FRONT1;
			break;
		case MOVE_DIRECTION::LEFT:
			state = (size_t)NPC_STATE::LEFT1;
			break;
		default: break;
	}
}
