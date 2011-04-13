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

#include "player3d.h"

player3d::player3d(map3d* map3d_obj) : npc3d(map3d_obj) {
	char_type = CT_PLAYER;
}

player3d::~player3d() {
}

void player3d::draw() const {
	return;
}

void player3d::handle() {
	// TODO: !
}

void player3d::move(const MOVE_DIRECTION& direction) {
	if(conf::get<bool>("debug.free_cam")) return;
	// TODO: !
	
	// handle player
	const float3 orig_cam_pos = *cam->get_position();
	float3 cam_pos = orig_cam_pos;
	float3 cam_offset(0.0f);
	float3 cam_rot = *cam->get_rotation();

	// allow player to move two tiles per second
	float cam_speed = (TILES_PER_SECOND_NPC3D * 2.0f * float(SDL_GetTicks() - last_move)/1000.0f) * std_tile_size;
	last_move = SDL_GetTicks();
	
	// compute new camera position
	if(direction & MD_RIGHT) {
		cam_offset.x += (float)sin((cam_rot.y - 90.0f) * PIOVER180) * cam_speed;
		cam_offset.z -= (float)cos((cam_rot.y - 90.0f) * PIOVER180) * cam_speed;
	}
	
	if(direction & MD_LEFT) {
		cam_offset.x -= (float)sin((cam_rot.y - 90.0f) * PIOVER180) * cam_speed;
		cam_offset.z += (float)cos((cam_rot.y - 90.0f) * PIOVER180) * cam_speed;
	}
	
	if(direction & MD_UP) {
		cam_offset.x -= (float)sin(cam_rot.y * PIOVER180) * cam_speed;
		cam_offset.z += (float)cos(cam_rot.y * PIOVER180) * cam_speed;
	}
	
	if(direction & MD_DOWN) {
		cam_offset.x += (float)sin(cam_rot.y * PIOVER180) * cam_speed;
		cam_offset.z -= (float)cos(cam_rot.y * PIOVER180) * cam_speed;
	}
	
	cam_pos += cam_offset;
	
	// collision check
	/*static const float cam_collision_offset = 1.3f; // offset position a bit, so we don't end up on or inside a poly
	float3 map_pos = -cam_pos / std_tile_size;
	float3 old_pos = (-orig_cam_pos / std_tile_size).floor() + float3(0.5f);
	float2 dir = ssize2(float2(map_pos.x - old_pos.x, map_pos.z - old_pos.z)*2.0f * cam_collision_offset);
	const MOVE_DIRECTION cur_dir = (MOVE_DIRECTION)((dir.x < 0 ? MD_LEFT : (dir.x > 0 ? MD_RIGHT : 0)) | (dir.y < 0 ? MD_UP : (dir.y > 0 ? MD_DOWN : 0)));
	cout << "cur pos: " << size2(map_pos.x, map_pos.z) << endl;
	if(cur_dir != 0) {
		cout << "dir: " << cur_dir << ": ";
		if(map3d_obj->collide(cur_dir, size2(old_pos.x, old_pos.z), char_type)) {
			cout << "COLLISION" << endl;
			//return; // return on collision
			cam_pos = orig_cam_pos;
			switch(cur_dir) {
				case MD_LEFT:
				case MD_RIGHT:
					cam_pos.z += cam_offset.z;
					break;
				case MD_UP:
				case MD_DOWN:
					cam_pos.x += cam_offset.x;
					break;
				default: break;
			}
		}
		cout << "NOPE" << endl;
	}*/
	
	// collision check
	static const float cam_collision_offset = 1.3f; // offset position a bit, so we don't end up on or inside a poly
	float3 map_pos = -cam_pos / std_tile_size;
	float3 old_pos = (-orig_cam_pos / std_tile_size).floor() + float3(0.5f);
	float2 fdir = float2(map_pos.x - old_pos.x, map_pos.z - old_pos.z)*2.0f * cam_collision_offset;
	ssize2 sdir = ssize2(fdir);
	const MOVE_DIRECTION cur_dir_x = (MOVE_DIRECTION)(sdir.x < 0 ? MD_LEFT : (sdir.x > 0 ? MD_RIGHT : 0));
	const MOVE_DIRECTION cur_dir_y = (MOVE_DIRECTION)(sdir.y < 0 ? MD_UP : (sdir.y > 0 ? MD_DOWN : 0));
	const MOVE_DIRECTION cur_dir = (MOVE_DIRECTION)(cur_dir_x|cur_dir_y);
	
	/*cout << "#########" << endl;
	cout << "pos old/new: " << size3((-orig_cam_pos / std_tile_size).floor()) << ", " << size3((-cam_pos / std_tile_size).floor()) << endl;
	cout << "f/s: " << fdir << ", " << sdir << endl;
	cout << "dir: " << cur_dir_x << " | " << cur_dir_y << " => " << cur_dir << endl;*/
	
	if(cur_dir != 0) {
		bool4 col_data = map3d_obj->collide(cur_dir, size2(old_pos.x, old_pos.z), char_type);
		//cout << "col_data: " << col_data << endl;
		
		// special case: diagonal tile is a hit and we're moving there
		if(col_data.w && cur_dir_x > 0 && cur_dir_y > 0) {
			// if x/y tiles are also a hit, don't move at all
			if(col_data.y && col_data.z) {
				cam_offset.x = 0.0f;
				cam_offset.z = 0.0f;
			}
			else {
				// else: 
				/*if(fabs(fdir.x) >= fabs(fdir.y)) {
					cam_offset.x = 0.0f;
				}
				else {
					cam_offset.z = 0.0f;
				}*/
				cam_offset.x = 0.0f;
				cam_offset.z = 0.0f;
			}
		}
		else {
			// on x/y hit: set new cam offset to 0
			if(col_data.y) cam_offset.x = 0.0f;
			if(col_data.z) cam_offset.z = 0.0f;
		}
	}
	
	//
	float3 new_cam_pos = orig_cam_pos;
	new_cam_pos.x += cam_offset.x;
	new_cam_pos.z += cam_offset.z;
	
	// set new pos
	cam->set_position(new_cam_pos.x, -8.0f, new_cam_pos.z);
}

void player3d::move(const size2& move_pos) {
	return;
}
