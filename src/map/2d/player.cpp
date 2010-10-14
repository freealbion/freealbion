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

#include "player.h"

/*! player constructor
 */
player::player(map2d* map2d_obj, npcgfx* npc_graphics) : map2d_obj(map2d_obj), npc_graphics(npc_graphics) {
	pos_interp = 0.0f;
	state = S_FRONT1;
	last_frame = SDL_GetTicks();
}

/*! player destructor
 */
player::~player() {
}

void player::draw() const {
	const float tile_size = map2d_obj->get_tile_size();
	const float2& screen_position = map2d_obj->get_screen_position();
	float2 player_pos = pos;
	float2 player_next_pos = next_pos;
	player_pos = player_pos*(1.0f-pos_interp) + player_next_pos*pos_interp;
	npc_graphics->draw_npc(202, (NPC_STATE)state,
						   (player_pos.x - screen_position.x)*tile_size,
						   (player_pos.y - 2.0f - screen_position.y)*tile_size);
	
	//
	if(conf::get<bool>("debug.player_pos")) {
		gfx::rect dbg_rect;
		dbg_rect.x1 = (player_pos.x - screen_position.x)*tile_size;
		dbg_rect.y1 = (player_pos.y - screen_position.y)*tile_size;
		dbg_rect.x2 = dbg_rect.x1 + tile_size;
		dbg_rect.y2 = dbg_rect.y1 + tile_size;
		egfx->draw_rectangle(&dbg_rect, 0xFF0000);
	}
}

void player::handle() {
	map2d_obj->set_pos(next_pos.x, next_pos.y);
	
	//
	/*pos_interp += 0.2f; // TODO: make this fps independent, bind to key repeat
	pos_interp = c->clamp(pos_interp, 0.0f, 1.0f);
	if(pos_interp >= 1.0f) pos = next_pos;*/
	float interp = float(SDL_GetTicks() - last_frame) / float(TIME_PER_TILE);
	last_frame = SDL_GetTicks();

	pos_interp += interp;
	pos_interp = c->clamp(pos_interp, 0.0f, 1.0f);
	if(pos_interp >= 1.0f) pos = next_pos;
	
	// animation (TODO: this is more of a hack right now, think of a better method)
	static const size_t time_per_frame = 150;
	static size_t last_frame = SDL_GetTicks();
	bool new_frame = (SDL_GetTicks() - last_frame >= time_per_frame) && (next_pos.x != pos.x || next_pos.y != pos.y);
	
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
		last_frame = SDL_GetTicks();
	}
}

void player::set_pos(const size_t& x, const size_t& y) {
	pos.set(x, y);
	next_pos.set(x, y);
}

void player::move(const MOVE_DIRECTION& direction) {
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

const size2& player::get_pos() const {
	//return pos;
	return next_pos;
}
