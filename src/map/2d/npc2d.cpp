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

#include "npc2d.h"

/*! npc2d constructor
 */
npc2d::npc2d(map2d* map2d_obj, npcgfx* npc_graphics) : map2d_obj(map2d_obj), npc_graphics(npc_graphics) {
	time_per_tile = TIME_PER_TILE_NPC;
	pos_interp = 0.0f;
	state = S_FRONT1;
	last_frame = last_anim_frame = last_move = SDL_GetTicks();
	npc_data = NULL;
	enabled = true;
}

/*! npc2d destructor
 */
npc2d::~npc2d() {
}

void npc2d::set_npc_data(const map_npcs::map_npc* npc_data) {
	npc2d::npc_data = npc_data;

	// set start position from map data (-1, b/c we start at 0)
	if(npc_data->position[0].x == 0 || npc_data->position[0].y == 0) {
		// pos (0, 0) = disabled npc
		set_enabled(false);
		set_pos(0, 0);
	}
	else set_pos(npc_data->position[0].x-1, npc_data->position[0].y-1);
}

void npc2d::draw(const NPC_DRAW_STAGE& draw_stage) const {
	if(!enabled) return;
	if(npc_data == NULL) return;

	const float tile_size = map2d_obj->get_tile_size();
	float2 npc_pos = pos;
	float2 npc_next_pos = next_pos;
	npc_pos = npc_pos*(1.0f-pos_interp) + npc_next_pos*pos_interp;
	
	float depth_overwrite = (draw_stage == NDS_PRE_UNDERLAY || draw_stage == NDS_PRE_OVERLAY) ? 0.0f : -1.0f;
	npc_graphics->draw_npc(npc_data->object_num, (NPC_STATE)state,
						   float2((npc_pos.x)*tile_size, (npc_pos.y - 2.0f)*tile_size),
						   npc_pos, depth_overwrite);
}

void npc2d::handle() {
	if(!enabled) return;

	//
	compute_move();

	//
	float interp = float(SDL_GetTicks() - last_frame) / float(time_per_tile);
	last_frame = SDL_GetTicks();

	pos_interp += interp;
	pos_interp = c->clamp(pos_interp, 0.0f, 1.0f);
	if(pos_interp >= 1.0f) pos = next_pos;
	
	// animation (TODO: this is more of a hack right now, think of a better method)
	bool new_frame = (SDL_GetTicks() - last_anim_frame >= TIME_PER_ANIMATION_FRAME) && (next_pos.x != pos.x || next_pos.y != pos.y);
	
	const size_t cur_frame = (state & 0x7);
	const size_t cur_frame_dir = (state & 0x8) >> 3;
	if(new_frame && cur_frame > 0 && (state & 0xFF) < 0x40) {
		state &= ~(0xF);
		if(cur_frame_dir == 0) {
			state |= (cur_frame < 4 ? (cur_frame << 1) : 0xA);
		}
		else {
			state |= (cur_frame > 1 ? ((cur_frame >> 1) | 0x8) : 0x2);
		}
		last_anim_frame = SDL_GetTicks();
	}
}

void npc2d::set_pos(const size_t& x, const size_t& y) {
	pos.set(x, y);
	next_pos.set(x, y);
}

void npc2d::move(const MOVE_DIRECTION& direction) {
	if(!enabled) return;

	pos = next_pos;
	pos_interp = 0.0f;
	switch(direction) {
		case MD_LEFT:
			if(!map2d_obj->collide(MD_LEFT, pos)) {
				if(pos.x != 0) next_pos.x--;
			}
			state = (S_LEFT1 & 0xF0) | (state & 0xF);
			break;
		case MD_RIGHT:
			if(!map2d_obj->collide(MD_RIGHT, pos)) {
				if(pos.x < map2d_obj->get_size().x-1) next_pos.x++;
			}
			state = (S_RIGHT1 & 0xF0) | (state & 0xF);
			break;
		case MD_UP:
			if(!map2d_obj->collide(MD_UP, pos)) {
				if(pos.y != 0) next_pos.y--;
			}
			state = (S_BACK1 & 0xF0) | (state & 0xF);
			break;
		case MD_DOWN:
			if(!map2d_obj->collide(MD_DOWN, pos)) {
				if(pos.y < map2d_obj->get_size().y-1) next_pos.y++;
			}
			state = (S_FRONT1 & 0xF0) | (state & 0xF);
			break;
		default:
			break;
	}
}

const size2& npc2d::get_pos() const {
	return next_pos;
}

void npc2d::compute_move() {
	if(!enabled) return;

	// TODO: compute next move
	if(npc_data->movement_type == MT_RANDOM) {
		// TODO: think of a better method ;)
		if(SDL_GetTicks() - last_move < time_per_tile) return;
		last_move = SDL_GetTicks();

		MOVE_DIRECTION dir = (MOVE_DIRECTION)((rand() % 4)+1);
		move(dir);
	}
	else {
		// TODO: follow track dependent on game time
	}
}

void npc2d::set_enabled(const bool& state) {
	enabled = state;
}

bool npc2d::is_enabled() const {
	return enabled;
}
