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

#include "npc3d.h"
#include "object_light.h"

/*! npc3d constructor
 */
npc3d::npc3d(map3d* map3d_obj_) : npc(), map3d_obj(map3d_obj_), l(NULL) {
	time_per_tile = TIME_PER_TILE_NPC3D;
	state = N3DS_FRONT1;
	anim_dir = false;
}

/*! npc3d destructor
 */
npc3d::~npc3d() {
}

void npc3d::draw() const {
	if(!enabled) return;
	if(npc_data == NULL) return;
}

void npc3d::handle() {
	if(!enabled) return;
	
	//
	compute_move();
	
	//
	float interp = float(SDL_GetTicks() - last_frame) / float(time_per_tile);
	last_frame = SDL_GetTicks();
	
	pos_interp += interp;
	pos_interp = core::clamp(pos_interp, 0.0f, 1.0f);
	if(pos_interp >= 1.0f) pos = next_pos;
	
	// animation (TODO: this is more of a hack right now, think of a better method)
	bool new_frame = (SDL_GetTicks() - last_anim_frame >= TIME_PER_ANIMATION_FRAME) && (next_pos.x != pos.x || next_pos.y != pos.y);
	
	if(new_frame && state > 0) {
		(!anim_dir) ? state++ : state--;
		
		if(state == N3DS_FRONT1 || state == N3DS_FRONT3) {
			anim_dir ^= true;
		}
		
		last_anim_frame = SDL_GetTicks();
	}
}

void npc3d::move(const MOVE_DIRECTION direction) {
	if(!enabled) return;
	
	pos = next_pos;
	pos_interp = 0.0f;

	if(!map3d_obj->collide(direction, pos, char_type).x) {
		if(direction & MD_LEFT) next_pos.x--;
		if(direction & MD_RIGHT) next_pos.x++;
		if(direction & MD_UP) next_pos.y--;
		if(direction & MD_DOWN) next_pos.y++;
	}
}

void npc3d::move(const size2& move_pos) {
	if(!enabled) return;
	
	pos = next_pos;
	next_pos = move_pos;
	pos_interp = 0.0f;
}

void npc3d::set_object_light(object_light_base* l_) {
	l = l_;
}

void npc3d::set_enabled(const bool& state_) {
	npc::set_enabled(state_);
	if(l == NULL) return;
	l->set_enabled(state_);
}

float3 npc3d::get_offset() const {
	const labdata::lab_object* obj = map3d_obj->get_active_lab_data()->get_object(npc_data->object_num);
	if(obj == NULL) return float3(0.0f);
	return obj->offset[0];
}

float npc3d::get_y_scale() const {
	const labdata::lab_object* obj = map3d_obj->get_active_lab_data()->get_object(npc_data->object_num);
	if(obj == NULL) return 0.0f;
	return obj->sub_objects[0]->y_scale;
}
