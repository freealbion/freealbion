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

#ifndef __AR_MAP_HANDLER_HPP__
#define __AR_MAP_HANDLER_HPP__

#include "ar_global.hpp"
#include "conf.hpp"
#include "map/map_defines.hpp"
#include "map/2d/map2d.hpp"
#include "map/3d/map3d.hpp"
#include "map/2d/npc2d.hpp"
#include "map/2d/player2d.hpp"
#include "map/3d/player3d.hpp"
#include "map/2d/npcgfx.hpp"
#include "map/3d/labdata.hpp"
#include <gui/gui.hpp>
#include <scene/scene.hpp>

class map_handler {
public:
	map_handler();
	~map_handler();
	
	void handle();
	void draw(const DRAW_MODE_UI draw_mode, rtt::fbo* buffer);
	
	void load_map(const size_t& map_num, const size2 player_pos = size2(0, 0),
				  const MOVE_DIRECTION player_direction = MOVE_DIRECTION::NONE);
	MAP_TYPE get_map_type(const size_t& map_num) const;
	
	//
	npcgfx* get_npc_graphics() const;
	
	//
	const size2& get_player_position() const;
	
	// DEBUG: for debugging purposes
	tileset::tile_object* get_tile(unsigned int type);
	unsigned int get_tile_num(unsigned int type);
	const size_t& get_active_map_num() const;
	const MAP_TYPE& get_active_map_type() const;
	const ssize3 get_3d_tile() const;
	void debug_draw(const DRAW_MODE);

protected:
	tileset* tilesets;
	map2d* maps2d;
	map3d* maps3d;
	npcgfx* npc_graphics;
	player2d* p2d;
	player3d* p3d;
	labdata* lab_data;

	array<lazy_xld, 3> maps;
	
	ui_draw_callback draw_cb;
	gui_simple_callback* draw_cb_obj { nullptr };
	scene::draw_callback scene_draw_cb;
	
	//
	size_t last_key_press;
	size_t last_move;
	atomic<MOVE_DIRECTION> next_dir { MOVE_DIRECTION::NONE };
	
	// these will make sure that only one input type is active at a time
	enum class ACTIVE_INPUT_TYPE {
		NONE,
		MOUSE,
		FINGER
	};
	ACTIVE_INPUT_TYPE input_type = ACTIVE_INPUT_TYPE::NONE;
	int2 click_position;
	event::handler input_handler_fnctr;
	mutex input_lock;
	bool input_handler(EVENT_TYPE type, shared_ptr<event_object> obj);
	void update_next_dir();
	
	//
	size_t active_map_num { 0 };
	MAP_TYPE active_map_type;
	
	array<MAP_TYPE, MAX_MAP_NUMBER> map_types {};

};

#endif
