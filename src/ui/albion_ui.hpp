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
 
#ifndef __AR_ALBION_UI_HPP__
#define __AR_ALBION_UI_HPP__

#include "ar_global.hpp"
#include "map/map_handler.hpp"
#include <gui/gui.hpp>

class image;

class gui_window;
class gui_button;
class gui_input_box;
class gui_list_box;

class albion_ui {
public:
	albion_ui(map_handler* mh);
	~albion_ui();
	
	void run();
	void draw(const DRAW_MODE_UI draw_mode, rtt::fbo* buffer);
	
	void open_goto_map_wnd();
	void close_goto_map_wnd();
	
	void open_game_ui();
	void close_game_ui();
	void load_game_ui(const size2& size);
	
protected:
	map_handler* mh { nullptr };
	ui_draw_callback draw_cb;
	gui_simple_callback* draw_cb_obj { nullptr };
	
	//
	a2e_image* clock_img_obj;
	struct clock_number {
		a2e_image* img;
		float2 num_pos;
	} clock_numbers[4];
	float2 clock_img_pos;
	float2 clock_img_size;
	float2 clock_num_size;
	
	float2 compass_img_pos;
	float2 compass_img_size;
	float2 compass_dot_img_pos;
	float2 compass_dot_img_size;
	a2e_image* compass_img_obj;
	a2e_image* compass_dot_img_obj;
	size_t cur_dot_img;
	size_t dot_timer;
	bool game_ui_opened;
	bool game_ui_loaded;
	bool game_ui_hidden { false };
	
	void delete_game_ui();
	
	//
	gui_window* map_wnd { nullptr };
	gui_button* goto_map_button { nullptr };
	gui_input_box* goto_map_input { nullptr };
	gui_list_box* goto_map_list { nullptr };
	
	//
	clock_callback clock_cb;
	void clock_tick(size_t ticks);
	
	//
	static const map<size_t, string> albion_maps;

};

#endif
