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
 
#ifndef __AR_ALBION_UI_H__
#define __AR_ALBION_UI_H__

#include "ar_global.h"
#include "map_handler.h"

/*! @class agui
 *  @brief albion gui
 *  @author flo
 *  
 *  albion gui
 */

class albion_ui {
public:
	albion_ui(map_handler* mh);
	~albion_ui();
	
	void open_goto_map_wnd();
	void close_goto_map_wnd();
	
protected:
	map_handler* mh;

	//
	a2eui_window* albion_dbg;
	gui_button* b_goto_map;
	gui_input* i_goto_map;
	gui_list* lb_map_names;
	gui_text* t_time;
	gui_text* t_time_value;
	
	//
	void handle_b_goto_map_button(event::GUI_EVENT_TYPE type, GUI_ID id);
	void handle_i_goto_map_selected(event::GUI_EVENT_TYPE type, GUI_ID id);
	void handle_lb_map_names_list(event::GUI_EVENT_TYPE type, GUI_ID id);
	
	//
	clock_callback* clock_cb;
	void clock_tick(size_t ticks);

};

#endif
