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

#include "npc2d.h"
#include "map2d.h"

/*! npc2d constructor
 */
npc2d::npc2d(map2d* map2d_obj, npcgfx* npc_graphics) : npc(), map2d_obj(map2d_obj), npc_graphics(npc_graphics), continent(false) {
	state = S_FRONT1;
}

/*! npc2d destructor
 */
npc2d::~npc2d() {
}

void npc2d::draw(const NPC_DRAW_STAGE& draw_stage) const {
	if(!enabled) return;
	if(npc_data == NULL) return;

	const float tile_size = map2d_obj->get_tile_size();
	float2 npc_pos = pos;
	float2 npc_next_pos = next_pos;
	npc_pos = npc_pos*(1.0f-pos_interp) + npc_next_pos*pos_interp;
	
	float depth_overwrite = (draw_stage == NDS_PRE_UNDERLAY || draw_stage == NDS_PRE_OVERLAY) ? 0.0f : -1.0f;
	static const size_t continent_npcgfx_offset = 399;
	npc_graphics->draw_npc((continent ? continent_npcgfx_offset : 0) + npc_data->object_num,
						   (NPC_STATE)state,
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

void npc2d::move(const MOVE_DIRECTION& direction) {
	if(!enabled) return;

	pos = next_pos;
	pos_interp = 0.0f;
	
	size_t final_dir = 0;
	MOVE_DIRECTION dirs[4];
	size_t dir_count = 0;
	if(direction & MD_UP) dirs[dir_count++] = MD_UP;
	if(direction & MD_DOWN) dirs[dir_count++] = MD_DOWN;
	if(direction & MD_LEFT) dirs[dir_count++] = MD_LEFT;
	if(direction & MD_RIGHT) dirs[dir_count++] = MD_RIGHT;
	
	for(size_t i = 0; i < dir_count+1; i++) {
		if(i == 0 && !map2d_obj->collide(direction, pos, char_type)) {
			final_dir = direction;
			break;
		}
		else if(i > 0 && !map2d_obj->collide(dirs[i-1], pos, char_type)) {
			final_dir = dirs[i-1];
			break;
		}
	}
	
	if(final_dir & MD_LEFT) {
		if(pos.x != 0) next_pos.x--;
	}
	if(final_dir & MD_RIGHT)  {
		if(pos.x < map2d_obj->get_size().x-1) next_pos.x++;
	}
	if(final_dir & MD_UP)  {
		if(pos.y != 0) next_pos.y--;
	}
	if(final_dir & MD_DOWN)  {
		if(pos.y < map2d_obj->get_size().y-1) next_pos.y++;
	}
	
	// prioritize left/right when selecting the npc state
	if(final_dir & MD_UP) state = (S_BACK1 & 0xF0) | (state & 0xF);
	if(final_dir & MD_DOWN) state = (S_FRONT1 & 0xF0) | (state & 0xF);
	if(final_dir & MD_LEFT) state = (S_LEFT1 & 0xF0) | (state & 0xF);
	if(final_dir & MD_RIGHT) state = (S_RIGHT1 & 0xF0) | (state & 0xF);
}

void npc2d::move(const size2& move_pos) {
	if(!enabled) return;
	
	ssize2 dir = ssize2(move_pos) - ssize2(next_pos);
	size_t final_dir = (dir.x < 0 ? MD_LEFT : (dir.x > 0 ? MD_RIGHT : 0)) | (dir.y < 0 ? MD_UP : (dir.y > 0 ? MD_DOWN : 0));
	pos = next_pos;
	next_pos = move_pos;
	pos_interp = 0.0f;
	
	// prioritize left/right when selecting the npc state
	if(final_dir & MD_UP) state = (S_BACK1 & 0xF0) | (state & 0xF);
	if(final_dir & MD_DOWN) state = (S_FRONT1 & 0xF0) | (state & 0xF);
	if(final_dir & MD_LEFT) state = (S_LEFT1 & 0xF0) | (state & 0xF);
	if(final_dir & MD_RIGHT) state = (S_RIGHT1 & 0xF0) | (state & 0xF);
}

void npc2d::set_continent(const bool& state) {
	continent = state;
}
