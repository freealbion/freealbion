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
#define LB_MAP_NAMES_ID 503

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
	lb_map_names = albion_dbg->get<gui_list>("lb_map_names");
	evt->add_event_callback(this, &albion_ui::handle_b_goto_map_button, event::BUTTON_PRESSED, b_goto_map->get_id());
	evt->add_event_callback(this, &albion_ui::handle_i_goto_map_selected, event::INPUT_SELECTED, i_goto_map->get_id());
	evt->add_event_callback(this, &albion_ui::handle_lb_map_names_list, event::LISTBOX_ITEM_SELECTED, lb_map_names->get_id());
	
	// position
	gui_window* wnd = (gui_window*)albion_dbg->get_object();
	gfx::rect* wnd_rect = wnd->get_rectangle();
	unsigned int wnd_height = wnd->get_height();
	wnd_rect->y1 = e->get_height() - 1 - wnd_height;
	wnd_rect->y2 = e->get_height() - 1;
	wnd->set_rectangle(wnd_rect);

	// fill map list
	struct _albion_dbg_maps {
		size_t map_num;
		string map_name;
	};
	const _albion_dbg_maps albion_maps[] = {
		{ 100, "Test-Map Iskai" },
		{ 101, "Test-Map Toronto" },
		{ 102, "Test-Map Toronto" },
		{ 103, "Test-Map Srimalinar" },
		{ 104, "Test-Map Drinno" },
		{ 105, "Test-Map teilweise Absturz" },
		{ 106, "Test-Map Argim" },
		{ 107, "Test-Map Argim" },
		{ 108, "teilweise eingerichtete Keltenhütte" },
		{ 109, "Test-Map Khamulon" },
		{ 110, "3D-Karte Jirinaar" },
		{ 111, "Haus des Jägerclans" },
		{ 112, "Untergeschoss Jägerclan" },
		{ 113, "Rathaus Jirinaar" },
		{ 114, "Dji-Kas Gildenhaus" },
		{ 115, "Dji-Kas Untergeschoss" },
		{ 116, "Dji-Fadh Gildenhaus" },
		{ 117, "Dji-Kas Untergeschoss 2" },
		{ 118, "Snird, Haus der Waffen" },
		{ 119, "Gewürzhändler" },
		{ 120, "Haus der nützlichen Vielfalt" },
		{ 121, "Haus der Winde" },
		{ 122, "Altes Formergebäude" },
		{ 123, "Keller des Jägerclanhauses" },
		{ 124, "Leere Keltenhütte" },
		{ 125, "Leere Keltenhütte breit" },
		{ 126, "Leere Keltenhütte hoch" },
		{ 127, "Sarenas Hütte" },
		{ 128, "Peleitos Hütte" },
		{ 129, "Garris" },
		{ 130, "Bragona" },
		{ 131, "Tharnos" },
		{ 132, "Winion" },
		{ 133, "Oibelos" },
		{ 134, "Tamno" },
		{ 135, "Dranbar" },
		{ 136, "Bennos, Nahrung" },
		{ 137, "Aretha" },
		{ 138, "Rifrako" },
		{ 139, "Ferina" },
		{ 140, "Arjano Hütte" },
		{ 141, "Arjano Unten 2D" },
		{ 142, "Bibliothek" },
		{ 143, "Drinno 2D" },
		{ 144, "Drinno" },
		{ 145, "Drinno" },
		{ 146, "Drinno" },
		{ 147, "Drinno" },
		{ 148, "Drinno" },
		{ 149, "Beros Raum" },
		{ 150, "Toronto Teil 1" },
		{ 151, "Toronto Teil 2" },
		{ 152, "Toronto Teil 2" },
		{ 153, "Toronto Teil 3" },
		{ 154, "Kenget 3D" },
		{ 155, "Kenget 3D" },
		{ 156, "Kenget 3D" },
		{ 157, "Kenget 3D" },
		{ 158, "Kenget 3D" },
		{ 159, "Kenget 3D" },
		{ 160, "Kenget 3D" },
		{ 162, "Kenget 3D" },
		{ 163, "Test-Map Kenget" },
		{ 164, "Formergebäude nach Kampf gegen Argim" },
		{ 166, "Sequenz nach Landung auf Albion" },
		{ 167, "Kampftrainer Jirinaar" },
		{ 168, "Transporterhöhle Jirinaar" },
		{ 169, "Transporterhöhle Gratogel" },
		{ 170, "Transporterhöhle Maini Süd" },
		{ 171, "Transporterhöhle Maini Nord" },
		{ 172, "Transporterhöhle Umajo" },
		{ 173, "Transporterhöhle Dji Cantos" },
		{ 174, "Endszenario (nicht Sequenz)" },
		{ 195, "Test-Map" },
		{ 197, "Test-Map" },
		{ 198, "Test-Map" },
		{ 199, "ALBION SHORTCUT-MAP" },
		{ 200, "Nakiridaani Kontinentkarte" },
		{ 201, "Gratogel Kontinentkarte Norden" },
		{ 202, "Gratogel Kontinentkarte Süden" },
		{ 203, "Maini Kontinentkarte" },
		{ 204, "Maini Kontinentkarte" },
		{ 205, "Maini Kontinentkarte" },
		{ 206, "Maini Kontinentkarte" },
		{ 207, "Maini Kontinentkarte" },
		{ 210, "Test-Map Kontinentkarte" },
		{ 211, "Test-Map Grafik" },
		{ 212, "Iskai-Heiligtum" },
		{ 213, "Kontos" },
		{ 214, "Test-Map Grafik" },
		{ 215, "Umajo Kontinentkarte" },
		{ 216, "Umajo Kontinentkarte" },
		{ 217, "Umajo Kontinentkarte" },
		{ 219, "Umajo Kontinentkarte" },
		{ 230, "Gerätemacher Gildenhaus" },
		{ 231, "Diamantschleifer Gildenhaus" },
		{ 232, "Waffenschmiede Gildenhaus" },
		{ 233, "Bergarbeiter Gildenhaus" },
		{ 234, "2D-Gefängnis" },
		{ 235, "Umajo Kenta Karte" },
		{ 236, "Nahrungsmittel Kyla" },
		{ 237, "Gemischtwarenladen Umajo" },
		{ 238, "Gefängnis Umajo" },
		{ 239, "Gasthaus Erzmine" },
		{ 240, "Sojekos" },
		{ 241, "Haus Kyla" },
		{ 242, "Gebirgspassage 3D" },
		{ 243, "Gerätemacherdungeon 2D" },
		{ 244, "Gerätemacher 3D Lvl 2" },
		{ 245, "Gerätemacher 3D Lvl 1" },
		{ 246, "Gerätemacher 3D Kossotto" },
		{ 247, "Eingang ins Dungeon Gerätemacherkammer" },
		{ 248, "Stollen Bergarbeiter (Spitzel)" },
		{ 252, "Kounos Höhle Lvl 1" },
		{ 253, "Kounos Höhle Lvl 2" },
		{ 254, "Kounos Höhle Lvl 3" },
		{ 255, "Kounos Höhle Lvl 4" },
		{ 256, "Kounos Höhle Lvl 5" },
		{ 260, "Herberge Beloveno" },
		{ 261, "Siobhan" },
		{ 262, "Siobhan Keller" },
		{ 263, "Wohnhaus Süden" },
		{ 264, "Kariah" },
		{ 265, "Rathaus Beloveno" },
		{ 266, "Wohnhaus Nordwesten" },
		{ 267, "Nahrungshandel Dolo" },
		{ 268, "Rüstungen Bagga" },
		{ 269, "Poschs Waffenladen" },
		{ 270, "Gemischtwaren Riolea" },
		{ 271, "Heilerin Ramina" },
		{ 273, "Händlerin Kounos" },
		{ 274, "Darios" },
		{ 275, "Gasthaus Kounos" },
		{ 276, "Nahrungshandel Dolo" },
		{ 277, "Kontos Labyrinth 3D" },
		{ 278, "Waffenladen Nadje" },
		{ 279, "Magiergilde Srimalinar" },
		{ 280, "Arrim" },
		{ 281, "Edjirr" },
		{ 282, "Heiligtum Untergeschoß" },
		{ 283, "Beloveno Karte" },
		{ 284, "Srimalinar Karte" },
		{ 290, "Test-Map Wüste" },
		{ 291, "Test-Map Dji Cantos" },
		{ 293, "Test Items" },
		{ 294, "Test-Map Kenget Kamulos" },
		{ 295, "Test-Map Mahinohaus" },
		{ 299, "Testmap Haus" },
		{ 300, "Toronto 2D Gesamtkarte Spielbeginn" },
		{ 301, "Reaktorraum der Toronto" },
		{ 302, "Ankunft auf der Toronto Teil 2" },
		{ 303, "Toronto Erkundung (Shuttle)" },
		{ 304, "Toronto Erkundung (Shuttle) mit Joe" },
		{ 305, "Reaktorraum mit AI und Brandt" },
		{ 310, "Gefängnis Kenget" },
		{ 311, "Kenget Kamulos 2D-Bereich" },
		{ 312, "Sklavenquartiere der Kenget" },
		{ 320, "Insel des Friedens" },
		{ 322, "Cantos Haus"}
	};
	const size_t map_count = A2E_ARRAY_LENGTH(albion_maps);
	for(size_t i = 0; i < map_count; i++) {
		string item_str = size_t2string(albion_maps[i].map_num-100) + " - " + albion_maps[i].map_name;
		lb_map_names->add_item(item_str.c_str());
	}
}

void albion_ui::handle_b_goto_map_button(event::GUI_EVENT_TYPE type, GUI_ID id) {
	string map_num_str = "";

	if(*i_goto_map->get_text() != "" && *i_goto_map->get_text() != "go to map ...") {
		map_num_str = *i_goto_map->get_text();
	}
	else if(lb_map_names->get_selected_item() != NULL) {
		map_num_str = lb_map_names->get_selected_item()->text;
		map_num_str = map_num_str.substr(0, map_num_str.find(" "));

		// unselect, but keep position
		int pos = lb_map_names->get_position();
		lb_map_names->set_selected_id(0);
		lb_map_names->set_position(pos);
	}
	i_goto_map->set_text("");

	mh->load_map(string2size_t(map_num_str));
}

void albion_ui::handle_i_goto_map_selected(event::GUI_EVENT_TYPE type, GUI_ID id) {
	i_goto_map->set_text("");
}

void albion_ui::handle_lb_map_names_list(event::GUI_EVENT_TYPE type, GUI_ID id) {
}
