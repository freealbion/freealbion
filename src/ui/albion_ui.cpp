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

#include "ui/albion_ui.hpp"
#include "ui/bin_graphics.hpp"
#include <gui/image.hpp>
#include <rendering/gfx2d.hpp>
#include <gui/style/gui_surface.hpp>
#include <unordered_map>

#include <gui/font_manager.hpp>
#include <gui/font.hpp>
#include <gui/objects/gui_window.hpp>
#include <gui/objects/gui_button.hpp>
#include <gui/objects/gui_text.hpp>
#include <gui/objects/gui_pop_up_button.hpp>
#include <gui/objects/gui_toggle_button.hpp>
#include <gui/objects/gui_slider.hpp>
#include <gui/objects/gui_list_box.hpp>
#include <gui/objects/gui_input_box.hpp>


// fill map list (TODO: should be put somewhere else + add english list)
const map<size_t, string> albion_ui::albion_maps {
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
	{ 161, "-----" },
	{ 162, "Kenget 3D" },
	{ 163, "Test-Map Kenget" },
	{ 164, "Formergebäude nach Kampf gegen Argim" },
	{ 165, "-----" },
	{ 166, "Sequenz nach Landung auf Albion" },
	{ 167, "Kampftrainer Jirinaar" },
	{ 168, "Transporterhöhle Jirinaar" },
	{ 169, "Transporterhöhle Gratogel" },
	{ 170, "Transporterhöhle Maini Süd" },
	{ 171, "Transporterhöhle Maini Nord" },
	{ 172, "Transporterhöhle Umajo" },
	{ 173, "Transporterhöhle Dji Cantos" },
	{ 174, "Endszenario (nicht Sequenz)" },
	{ 190, "00s (don't load)" },
	{ 195, "Test-Map" },
	{ 196, "-----" },
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
	{ 218, "-----" },
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
	{ 249, "-----" },
	{ 250, "-----" },
	{ 251, "-----" },
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
	{ 272, "-----" },
	{ 273, "Händlerin Kounos" },
	{ 274, "Darios" },
	{ 275, "Gasthaus Kounos" },
	{ 276, "Kontos Labyrinth 3D Upper" },
	{ 277, "Kontos Labyrinth 3D Lower" },
	{ 278, "Waffenladen Nadje" },
	{ 279, "Magiergilde Srimalinar" },
	{ 280, "Arrim" },
	{ 281, "Edjirr" },
	{ 282, "Heiligtum Untergeschoß" },
	{ 283, "Beloveno Karte" },
	{ 284, "Srimalinar Karte" },
	{ 290, "Test-Map Wüste" },
	{ 291, "Test-Map Dji Cantos" },
	{ 292, "-----" },
	{ 293, "Test Items" },
	{ 294, "Test-Map Kenget Kamulos" },
	{ 295, "Test-Map Mahinohaus" },
	{ 297, "-----" },
	{ 298, "-----" },
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
	{ 313, "-----" },
	{ 320, "Insel des Friedens" },
	{ 322, "Cantos Haus"},
	{ 388, "-----" },
	{ 389, "-----" },
	{ 390, "-----" },
	{ 398, "-----" },
	{ 399, "-----" },
};

/*! albion_ui constructor
 */
albion_ui::albion_ui(map_handler* mh_) :
mh(mh_),
draw_cb(bind(&albion_ui::draw, this, placeholders::_1, placeholders::_2)),
game_ui_opened(false), game_ui_loaded(false),
clock_cb(bind(&albion_ui::clock_tick, this, placeholders::_1)) {
	clck->add_tick_callback(ar_clock::CCBT_TICK, clock_cb);
	
	//game_ui = nullptr;
	compass_dot_img_obj = nullptr;
	cur_dot_img = 0;
}

/*! albion_ui destructor
 */
albion_ui::~albion_ui() {
	game_ui_opened = false;
	clck->delete_tick_callback(clock_cb);
	
	if(game_ui_loaded) {
		delete_game_ui();
	}
}

void albion_ui::delete_game_ui() {
	delete clock_img_obj;
	for(size_t i = 0; i < 4; i++) {
		delete clock_numbers[i].img;
	}
	delete compass_img_obj;
	delete compass_dot_img_obj;
}

void albion_ui::run() {
	if(draw_cb_obj == nullptr) return;
	
	// not nice, but it's just bools ...
	bool map_3d = (mh->get_active_map_type() == MAP_TYPE::MAP_3D);
	if(!map_3d) return;
	
	// update compass dot every 33ms (~30fps)
	if(compass_dot_img_obj != nullptr && SDL_GetTicks() - dot_timer > 33) {
		// blinking
		compass_dot_img_obj->set_texture(bin_gfx->get_bin_graphic((bin_graphics::BIN_GRAPHIC_TYPE)(bin_graphics::COMPASS_DOT_0 + cur_dot_img)));
		cur_dot_img = (cur_dot_img+1) % 8;
		dot_timer = SDL_GetTicks();
		draw_cb_obj->redraw();
		
		// position
		const float y_rot = 180.0f - engine::get_rotation()->y;
		compass_dot_img_pos = float2(sinf(const_math::deg_to_rad(y_rot)), cosf(const_math::deg_to_rad(y_rot)));
		compass_dot_img_pos *= (compass_img_size.x / 2.0f) - compass_dot_img_size.x*0.8f;
		compass_dot_img_pos = compass_img_pos + compass_img_size/2.0f + compass_dot_img_pos - compass_dot_img_size/2.0f;
	}
}

void albion_ui::draw(const DRAW_MODE_UI draw_mode floor_unused, rtt::fbo* buffer floor_unused) {
	if(!game_ui_opened) return;
	if(game_ui_hidden) return;
	
	// draw the clock
	gfx2d::draw_rectangle_texture(rect(uint32_t(clock_img_pos.x), uint32_t(clock_img_pos.y),
									   uint32_t(clock_img_pos.x+clock_img_size.x), uint32_t(clock_img_pos.y+clock_img_size.y)),
								  clock_img_obj->get_texture()->tex());
	for(size_t i = 0; i < 4; i++) {
		gfx2d::draw_rectangle_texture(rect(uint32_t(clock_numbers[i].num_pos.x),
										   uint32_t(clock_numbers[i].num_pos.y),
										   uint32_t(clock_numbers[i].num_pos.x+clock_num_size.x),
										   uint32_t(clock_numbers[i].num_pos.y+clock_num_size.y)),
									  clock_numbers[i].img->get_texture()->tex());
	}
	
	// draw the compass (only on 3d maps)
	bool map_3d = (mh->get_active_map_type() == MAP_TYPE::MAP_3D);
	if(map_3d) {
		gfx2d::draw_rectangle_texture(rect(uint32_t(compass_img_pos.x), uint32_t(compass_img_pos.y),
										   uint32_t(compass_img_pos.x+compass_img_size.x),
										   uint32_t(compass_img_pos.y+compass_img_size.y)),
									  compass_img_obj->get_texture()->tex());
		gfx2d::draw_rectangle_texture(rect(uint32_t(compass_dot_img_pos.x), uint32_t(compass_dot_img_pos.y),
										   uint32_t(compass_dot_img_pos.x+compass_dot_img_size.x),
										   uint32_t(compass_dot_img_pos.y+compass_dot_img_size.y)),
									  compass_dot_img_obj->get_texture()->tex());
	}
}

void albion_ui::clock_tick(size_t ticks) {
	if(!game_ui_opened) return;
	
	const size_t hours = ticks / AR_TICKS_PER_HOUR;
	const size_t mins = (size_t)floorf(float(ticks % AR_TICKS_PER_HOUR) * 1.25f);
	
	clock_numbers[0].img->set_texture(bin_gfx->get_bin_graphic((bin_graphics::BIN_GRAPHIC_TYPE)(bin_graphics::CLOCK_NUM_0 + hours/10)));
	clock_numbers[1].img->set_texture(bin_gfx->get_bin_graphic((bin_graphics::BIN_GRAPHIC_TYPE)(bin_graphics::CLOCK_NUM_0 + hours%10)));
	clock_numbers[2].img->set_texture(bin_gfx->get_bin_graphic((bin_graphics::BIN_GRAPHIC_TYPE)(bin_graphics::CLOCK_NUM_0 + mins/10)));
	clock_numbers[3].img->set_texture(bin_gfx->get_bin_graphic((bin_graphics::BIN_GRAPHIC_TYPE)(bin_graphics::CLOCK_NUM_0 + mins%10)));
	
	draw_cb_obj->redraw();
}

void albion_ui::open_game_ui() {
	if(game_ui_opened) return;
	game_ui_opened = true;
	draw_cb_obj = ui->add_draw_callback(DRAW_MODE_UI::POST_UI, draw_cb, float2(1.0f), float2(0.0f),
										gui_surface::SURFACE_FLAGS::NO_ANTI_ALIASING |
										gui_surface::SURFACE_FLAGS::NO_DEPTH);
	
	if(game_ui_loaded) return;
	load_game_ui(floor::get_physical_screen_size());
}

void albion_ui::close_game_ui() {
	if(!game_ui_opened) return;
	game_ui_opened = false;
	floor::acquire_context();
	ui->delete_draw_callback(draw_cb);
	draw_cb_obj = nullptr;
	floor::release_context();
}

void albion_ui::load_game_ui(const size2& size) {
	// delete old ui if it has been loaded already
	if(game_ui_loaded) {
		delete_game_ui();
	}
	
	//
	game_ui_loaded = false;
	const float2 wnd_size(size.x/12, size.y);
	
	// clock
	clock_img_obj = new a2e_image();
	clock_img_obj->set_scaling(true);
	const a2e_texture& clock_tex = bin_gfx->get_bin_graphic(bin_graphics::CLOCK);
	clock_img_obj->set_texture(clock_tex);
	clock_img_obj->set_gui_image(true);
	
	clock_img_pos = float2(wnd_size.x * 0.05f, wnd_size.y * 0.31f);
	clock_img_size = float2(wnd_size.x * 0.9f, 0.0f);
	float img_scale = clock_img_size.x / float(clock_tex->width);
	clock_img_size.y = img_scale * float(clock_tex->height);
	
	// clock numbers
	const a2e_texture& clock_num_tex = bin_gfx->get_bin_graphic(bin_graphics::CLOCK_NUM_0);
	clock_num_size = float2(clock_num_tex->width, clock_num_tex->height);
	clock_num_size *= img_scale;
	float x_per_num = (clock_img_size.x - clock_num_size.x*4.0f) / 4.0f;
	for(size_t i = 0; i < 4; i++) {
		clock_numbers[i].img = new a2e_image();
		clock_numbers[i].img->set_scaling(true);
		clock_numbers[i].img->set_texture(clock_num_tex);
		clock_numbers[i].img->set_gui_image(true);
		
		clock_numbers[i].num_pos = float2(clock_img_pos);
		clock_numbers[i].num_pos += float2(clock_num_size.x * float(i),
										   clock_img_size.y/2.0f - clock_num_size.y*0.3333f);
		clock_numbers[i].num_pos.x += x_per_num * (i < 2 ? 1.0f : 2.0f);
	}
	
	// compass
	const a2e_texture& compass_tex = bin_gfx->get_bin_graphic(bin_graphics::COMPASS_EN);
	const a2e_texture& compass_dot_tex = bin_gfx->get_bin_graphic(bin_graphics::COMPASS_DOT_0);
	compass_img_obj = new a2e_image();
	compass_img_obj->set_scaling(true);
	compass_img_obj->set_texture(compass_tex);
	compass_img_obj->set_gui_image(true);
	compass_dot_img_obj = new a2e_image();
	compass_dot_img_obj->set_scaling(true);
	compass_dot_img_obj->set_texture(compass_dot_tex);
	compass_dot_img_obj->set_gui_image(true);
	
	compass_img_pos = float2(wnd_size.x * 0.05f);
	compass_img_size = float2(wnd_size.x * 0.9f, 0.0f);
	float compass_img_scale = clock_img_size.x / float(compass_tex->width);
	compass_img_size.y = compass_img_scale * float(compass_tex->height);
	compass_dot_img_size = float2(compass_img_scale * compass_dot_tex->width,
								  compass_img_scale * compass_dot_tex->height);
	compass_dot_img_pos = float2(compass_img_pos.x, compass_img_pos.y);
	
	dot_timer = SDL_GetTicks();
	
	game_ui_loaded = true;
}

void albion_ui::open_goto_map_wnd() {
	game_ui_hidden = true;
	if(map_wnd != nullptr) return;
	
	map_wnd = ui->add<gui_window>(float2(0.3f, 1.0f), float2(0.0f, 0.0f));
	
	static constexpr float margin_x = 0.025f, margin_x_double = margin_x * 2.0f;
	static constexpr float margin_y = 0.0125f, margin_y_double = margin_y * 2.0f;
	static constexpr float x_full_width = 1.0f - margin_x_double;
	const float elem_height = 0.03f;
	const float list_height = 1.0f - elem_height * 2.0f - margin_y_double - margin_y;
	
	goto_map_input = ui->add<gui_input_box>(float2(x_full_width, elem_height), float2(margin_x, margin_y));
	goto_map_list = ui->add<gui_list_box>(float2(x_full_width, list_height), float2(margin_x, margin_y + margin_y * 0.5f + elem_height));
	goto_map_button = ui->add<gui_button>(float2(x_full_width, elem_height), float2(margin_x, 1.0f - margin_y - elem_height));
	
	goto_map_input->set_input("go to map ...");
	goto_map_button->set_label("Go to Map");
	
	for(const auto& entry : albion_maps) {
		string item_str = to_string(entry.first - 100) + " - " + entry.second;
		switch(mh->get_map_type(entry.first - 100)) {
			case MAP_TYPE::MAP_2D: item_str += " (2D)"; break;
			case MAP_TYPE::MAP_3D: item_str += " (3D)"; break;
			default: break;
		}
		goto_map_list->add_item(to_string(entry.first), item_str);
	}
	const auto cur_map_id = to_string(100 + mh->get_active_map_num());
	goto_map_list->set_selected_item(cur_map_id);
	goto_map_list->scroll_to_item(cur_map_id);
	
	map_wnd->add_child(goto_map_input);
	map_wnd->add_child(goto_map_list);
	map_wnd->add_child(goto_map_button);
	
	//"go to map ..."
	static enum class LAST_ACTIVE {
		INPUT,
		LIST
	} last_active = LAST_ACTIVE::LIST;
	goto_map_input->add_handler([this](GUI_EVENT, gui_object&) {
		// TODO: !
		last_active = LAST_ACTIVE::INPUT;
	}, GUI_EVENT::INPUT_BOX_ENTER);
	
	goto_map_input->add_handler([this](GUI_EVENT, gui_object&) {
		last_active = LAST_ACTIVE::INPUT;
		// TODO: list box update
	}, GUI_EVENT::INPUT_BOX_INPUT);
	
	goto_map_input->add_handler([this](GUI_EVENT, gui_object&) {
		last_active = LAST_ACTIVE::INPUT;
	}, GUI_EVENT::INPUT_BOX_ACTIVATION);
	
	goto_map_list->add_handler([this](GUI_EVENT, gui_object&) {
		last_active = LAST_ACTIVE::LIST;
	}, GUI_EVENT::LIST_BOX_SELECT);
	
	goto_map_list->add_handler([this](GUI_EVENT, gui_object&) {
		last_active = LAST_ACTIVE::LIST;
		
		const auto selected_item = goto_map_list->get_selected_item();
		if(selected_item == nullptr) return;
		size_t map_num = stosize(selected_item->first);
		
		floor::acquire_context();
		mh->load_map(map_num - 100);
		floor::release_context();
	}, GUI_EVENT::LIST_BOX_SELECT_EXECUTE);
	
	goto_map_button->add_handler([this](GUI_EVENT, gui_object&) {
		size_t map_num = 0;
		if(last_active == LAST_ACTIVE::INPUT) {
			map_num = stosize(goto_map_input->get_input());
		}
		else {
			const auto selected_item = goto_map_list->get_selected_item();
			if(selected_item == nullptr) return;
			map_num = stosize(selected_item->first);
		}
		floor::acquire_context();
		mh->load_map(map_num - 100);
		floor::release_context();
	}, GUI_EVENT::BUTTON_PRESS);
}

void albion_ui::close_goto_map_wnd() {
	game_ui_hidden = false;
	ui->remove(map_wnd);
	map_wnd = nullptr;
}
