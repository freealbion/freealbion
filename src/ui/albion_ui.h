/*
 *  Albion Remake
 *  Copyright (C) 2007 - 2012 Florian Ziesche
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
 
#ifndef __AR_ALBION_UI_H__
#define __AR_ALBION_UI_H__

#include "ar_global.h"
#include "map_handler.h"
#include <gui/gui.h>

class image;

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
	map_handler* mh;
	ui_draw_callback draw_cb;
	gui_simple_callback* draw_cb_obj = nullptr;
	
	//
	image* clock_img_obj;
	struct clock_number {
		image* img;
		float2 num_pos;
	} clock_numbers[4];
	float2 clock_img_pos;
	float2 clock_img_size;
	float2 clock_num_size;
	
	float2 compass_img_pos;
	float2 compass_img_size;
	float2 compass_dot_img_pos;
	float2 compass_dot_img_size;
	image* compass_img_obj;
	image* compass_dot_img_obj;
	size_t cur_dot_img;
	size_t dot_timer;
	bool game_ui_opened;
	bool game_ui_loaded;
	
	void delete_game_ui();
	
	//
	clock_callback* clock_cb;
	void clock_tick(size_t ticks);

};

#endif
