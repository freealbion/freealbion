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

#include "albion_ui.h"

// ui defines
#define ALBION_DBG_ID 500
#define B_GOTO_MAP_ID 501
#define I_GOTO_MAP_ID 502

/*! albion_ui constructor
 */
albion_ui::albion_ui(map_handler* mh) : mh(mh) {
}

/*! albion_ui destructor
 */
albion_ui::~albion_ui() {
}

void albion_ui::open_goto_map_wnd() {
	if(egui->exists(ALBION_DBG_ID)) return;
	
	albion_dbg = eui->load(e->data_path("albion_dbg.a2ui"), e->get_width()-1, 0, false, true, 0, 0, 0);
	b_goto_map = albion_dbg->get<gui_button>("b_goto_map");
	i_goto_map = albion_dbg->get<gui_input>("i_goto_map");
	evt->add_event_callback(this, &albion_ui::handle_b_goto_map_button, event::BUTTON_PRESSED, b_goto_map->get_id());
	evt->add_event_callback(this, &albion_ui::handle_i_goto_map_selected, event::INPUT_SELECTED, i_goto_map->get_id());
	
	// position
	gui_window* wnd = (gui_window*)albion_dbg->get_object();
	gfx::rect* wnd_rect = wnd->get_rectangle();
	unsigned int wnd_height = wnd->get_height();
	wnd_rect->y1 = e->get_height() - 1 - wnd_height;
	wnd_rect->y2 = e->get_height() - 1;
	wnd->set_rectangle(wnd_rect);
}

void albion_ui::handle_b_goto_map_button(event::GUI_EVENT_TYPE type, GUI_ID id) {
	mh->load_map(string2size_t(*i_goto_map->get_text()));
	i_goto_map->set_text("");
}

void albion_ui::handle_i_goto_map_selected(event::GUI_EVENT_TYPE type, GUI_ID id) {
	i_goto_map->set_text("");
}
