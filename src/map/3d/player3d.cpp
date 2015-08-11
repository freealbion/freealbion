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

#include "map/3d/player3d.hpp"
#include <scene/camera.hpp>

player3d::player3d(map3d* map3d_obj) : npc3d(map3d_obj), last_pos(0.0f, 0.0f, 0.0f) {
	char_type = CHARACTER_TYPE::PLAYER;
}

player3d::~player3d() {
}

void player3d::draw() const {
	return;
}

void player3d::handle() {
}

void player3d::set_pos(const size_t& x, const size_t& y) {
	npc3d::set_pos(x, y);
	last_pos.set(x*std_tile_size + std_half_tile_size,
				 -8.0f, 
				 y*std_tile_size + std_half_tile_size);
	cam->set_position(-last_pos.x, last_pos.y, -last_pos.z);
	
	// reset timer (so the player won't move one complete tile in the next move)
	last_move = SDL_GetTicks();
}

void player3d::move(const MOVE_DIRECTION direction) {
	if(conf::get<bool>("debug.free_cam")) return;
	
	// handle player
	const float3 orig_cam_pos = -(cam->get_position());
	float3 new_cam_pos = orig_cam_pos;
	float3 cam_pos = orig_cam_pos;
	float3 cam_offset(0.0f);
	float3 cam_rot = cam->get_rotation();

	// allow player to move two tiles per second ...
	float cam_speed = (TILES_PER_SECOND_NPC3D * 2.0f * float(SDL_GetTicks() - last_move)/1000.0f) * std_tile_size;
	cam_speed = std::min(cam_speed, std_tile_size); // ... but no more than 1 tile per "tick"
	last_move = SDL_GetTicks();
	
	// compute new camera position
	if((direction & MOVE_DIRECTION::RIGHT) != MOVE_DIRECTION::NONE) {
		cam_offset.x -= (float)sin((cam_rot.y - 90.0f) * const_math::PI_DIV_180<float>) * cam_speed;
		cam_offset.z += (float)cos((cam_rot.y - 90.0f) * const_math::PI_DIV_180<float>) * cam_speed;
	}
	
	if((direction & MOVE_DIRECTION::LEFT) != MOVE_DIRECTION::NONE) {
		cam_offset.x += (float)sin((cam_rot.y - 90.0f) * const_math::PI_DIV_180<float>) * cam_speed;
		cam_offset.z -= (float)cos((cam_rot.y - 90.0f) * const_math::PI_DIV_180<float>) * cam_speed;
	}
	
	if((direction & MOVE_DIRECTION::UP) != MOVE_DIRECTION::NONE) {
		cam_offset.x += (float)sin(cam_rot.y * const_math::PI_DIV_180<float>) * cam_speed;
		cam_offset.z -= (float)cos(cam_rot.y * const_math::PI_DIV_180<float>) * cam_speed;
	}
	
	if((direction & MOVE_DIRECTION::DOWN) != MOVE_DIRECTION::NONE) {
		cam_offset.x -= (float)sin(cam_rot.y * const_math::PI_DIV_180<float>) * cam_speed;
		cam_offset.z += (float)cos(cam_rot.y * const_math::PI_DIV_180<float>) * cam_speed;
	}
	
	cam_pos += cam_offset;
	
	// compute direction
	float2 fdir = float2(last_pos.x - cam_pos.x, last_pos.z - cam_pos.z).normalize();
	const MOVE_DIRECTION cur_dir_x = (MOVE_DIRECTION)(fdir.x > 0.0f ? MOVE_DIRECTION::LEFT :
                                                      (fdir.x < 0.0f ? MOVE_DIRECTION::RIGHT : MOVE_DIRECTION::NONE));
	const MOVE_DIRECTION cur_dir_y = (MOVE_DIRECTION)(fdir.y > 0.0f ? MOVE_DIRECTION::UP :
                                                      (fdir.y < 0.0f ? MOVE_DIRECTION::DOWN : MOVE_DIRECTION::NONE));
	const MOVE_DIRECTION cur_dir = (MOVE_DIRECTION)(cur_dir_x|cur_dir_y);
	
	if(cur_dir != MOVE_DIRECTION::NONE) {
		// collision check
		bool4 col_data = map3d_obj->collide(cur_dir, pos, char_type);
		
		static const float collision_offset = std_tile_size * 0.15f; // 15%
		static const float neg_collision_offset = std_tile_size - collision_offset;
		
		float2 mod_pos = float2(fmodf(cam_pos.x, std_tile_size), fmodf(cam_pos.z, std_tile_size));
		if((col_data.y || false) &&
		   ((mod_pos.x < collision_offset && cur_dir_x == MOVE_DIRECTION::LEFT) ||
			(mod_pos.x > neg_collision_offset && cur_dir_x == MOVE_DIRECTION::RIGHT))) {
			cam_offset.x = 0.0f;
			// clip to collision/hit point
			new_cam_pos.x = (float(pos.x)*std_tile_size + (cur_dir_x == MOVE_DIRECTION::LEFT ? collision_offset : neg_collision_offset));
		}
		if((col_data.z || false) &&
		   ((mod_pos.y < collision_offset && cur_dir_y == MOVE_DIRECTION::UP) ||
			(mod_pos.y > neg_collision_offset && cur_dir_y == MOVE_DIRECTION::DOWN))) {
			cam_offset.z = 0.0f;
			// clip to collision/hit point
			new_cam_pos.z = (float(pos.y)*std_tile_size + (cur_dir_y == MOVE_DIRECTION::UP ? collision_offset : neg_collision_offset));
		}
	}
	new_cam_pos.x += cam_offset.x;
	new_cam_pos.z += cam_offset.z;
	
	// set new pos
	cam->set_position(-new_cam_pos.x, -8.0f, -new_cam_pos.z);
	last_pos = orig_cam_pos;
	
	size2 new_pos { (size_t)floorf(new_cam_pos.x/std_tile_size), (size_t)floorf(new_cam_pos.z/std_tile_size) };
	if(new_pos.x != pos.x || new_pos.y != pos.y) {
		// player moved to the next tile
		set_moved(true);
	}
	pos = next_pos = new_pos;
}

void player3d::move(const size2& move_pos floor_unused) {
	return;
}

void player3d::set_view_direction(const MOVE_DIRECTION& vdirection) {
	npc3d::set_view_direction(vdirection);
	
	switch(vdirection) {
		case MOVE_DIRECTION::UP:
			cam->set_rotation(0.0f, 0.0f, 0.0f);
			break;
		case MOVE_DIRECTION::RIGHT:
			cam->set_rotation(0.0f, 90.0f, 0.0f);
			break;
		case MOVE_DIRECTION::DOWN:
			cam->set_rotation(0.0f, 180.0f, 0.0f);
			break;
		case MOVE_DIRECTION::LEFT:
			cam->set_rotation(0.0f, 270.0f, 0.0f);
			break;
		default: break;
	}
}
