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

#include "map_handler.h"

map_handler::map_handler() {
	// load maps
	maps1 = new xld("MAPDATA1.XLD");
	maps2 = new xld("MAPDATA2.XLD");
	maps3 = new xld("MAPDATA3.XLD");

	//
	tilesets = new tileset(palettes);
	npc_graphics = new npcgfx(palettes);
	maps2d = new map2d(tilesets, npc_graphics, maps1, maps2, maps3);
	lab_data = new labdata();
	maps3d = new map3d(lab_data, maps1, maps2, maps3);

	/*cout << "#########" << endl;
	for(size_t i = 0; i < 3; i++) {
		xld* maps = (i == 0 ? maps1 : (i == 1 ? maps2 : maps3));
		for(size_t map_num = 0; map_num < maps->get_object_count(); map_num++) {
			const xld::xld_object* obj = maps->get_object(map_num);
			const size_t rmap_num = (i*100)+map_num;
			if(obj->length == 0) cout << rmap_num << " NULL" << endl;
			else if(maps2d->is_2d_map(rmap_num)) cout << rmap_num << " 2D" << endl;
			else if(maps3d->is_3d_map(rmap_num)) cout << rmap_num << " 3D" << endl;
			else cout << rmap_num << " UNKNOWN" << endl;
		}
	}
	cout << "#########" << endl;*/

	//
	p = new player2d(maps2d, npc_graphics);
	p->set_pos(9, 8);
	
	active_map_type = MT_NONE;
	
	sce->add_draw_callback("map_handler", this, &map_handler::debug_draw);

	//
	last_key_press = SDL_GetTicks();
	last_move = SDL_GetTicks();
	next_dir = MD_NONE;
	evt->add_event_callback(this, &map_handler::handle_key_down, event::KEY_DOWN, SDLK_w);
	evt->add_event_callback(this, &map_handler::handle_key_down, event::KEY_DOWN, SDLK_a);
	evt->add_event_callback(this, &map_handler::handle_key_down, event::KEY_DOWN, SDLK_s);
	evt->add_event_callback(this, &map_handler::handle_key_down, event::KEY_DOWN, SDLK_d);
	evt->add_event_callback(this, &map_handler::handle_key_down, event::KEY_DOWN, SDLK_UP);
	evt->add_event_callback(this, &map_handler::handle_key_down, event::KEY_DOWN, SDLK_DOWN);
	evt->add_event_callback(this, &map_handler::handle_key_down, event::KEY_DOWN, SDLK_LEFT);
	evt->add_event_callback(this, &map_handler::handle_key_down, event::KEY_DOWN, SDLK_RIGHT);
	evt->add_event_callback(this, &map_handler::handle_key_up, event::KEY_PRESSED, SDLK_w);
	evt->add_event_callback(this, &map_handler::handle_key_up, event::KEY_PRESSED, SDLK_a);
	evt->add_event_callback(this, &map_handler::handle_key_up, event::KEY_PRESSED, SDLK_s);
	evt->add_event_callback(this, &map_handler::handle_key_up, event::KEY_PRESSED, SDLK_d);
	evt->add_event_callback(this, &map_handler::handle_right_click, event::RIGHT_MOUSE_CLICK, 0);
}

map_handler::~map_handler() {
	evt->delete_event_callbacks(this, &map_handler::handle_key_down, event::KEY_DOWN, SDLK_w);
	evt->delete_event_callbacks(this, &map_handler::handle_key_down, event::KEY_DOWN, SDLK_a);
	evt->delete_event_callbacks(this, &map_handler::handle_key_down, event::KEY_DOWN, SDLK_s);
	evt->delete_event_callbacks(this, &map_handler::handle_key_down, event::KEY_DOWN, SDLK_d);
	evt->delete_event_callbacks(this, &map_handler::handle_key_down, event::KEY_DOWN, SDLK_UP);
	evt->delete_event_callbacks(this, &map_handler::handle_key_down, event::KEY_DOWN, SDLK_DOWN);
	evt->delete_event_callbacks(this, &map_handler::handle_key_down, event::KEY_DOWN, SDLK_LEFT);
	evt->delete_event_callbacks(this, &map_handler::handle_key_down, event::KEY_DOWN, SDLK_RIGHT);
	evt->delete_event_callbacks(this, &map_handler::handle_key_up, event::KEY_PRESSED, SDLK_w);
	evt->delete_event_callbacks(this, &map_handler::handle_key_up, event::KEY_PRESSED, SDLK_a);
	evt->delete_event_callbacks(this, &map_handler::handle_key_up, event::KEY_PRESSED, SDLK_s);
	evt->delete_event_callbacks(this, &map_handler::handle_key_up, event::KEY_PRESSED, SDLK_d);
	evt->delete_event_callbacks(this, &map_handler::handle_right_click, event::RIGHT_MOUSE_CLICK, 0);
	
	delete maps1;
	delete maps2;
	delete maps3;

	delete tilesets;
	delete maps2d;
	delete maps3d;
	delete npc_graphics;
	delete lab_data;
	delete p;
}

void map_handler::handle() {
	if(next_dir != MD_NONE && (SDL_GetTicks() - last_move) > TIME_PER_TILE) {
		last_move = SDL_GetTicks();
		p->move((MOVE_DIRECTION)next_dir);
	}
	
	maps2d->handle();
	maps3d->handle();
	p->handle();
}

void map_handler::draw() {
	if(active_map_type == MT_2D_MAP) {
		e->start_2d_draw();
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
	
		maps2d->draw(MDS_NPCS, NDS_PRE_UNDERLAY);
		p->draw(NDS_PRE_UNDERLAY);
	
		if(conf::get<bool>("map.draw_underlay")) maps2d->draw(MDS_UNDERLAY, NDS_NONE);
	
		maps2d->draw(MDS_NPCS, NDS_PRE_OVERLAY);
		p->draw(NDS_PRE_OVERLAY);
	
		if(conf::get<bool>("map.draw_overlay")) maps2d->draw(MDS_OVERLAY, NDS_NONE);

		maps2d->draw(MDS_NPCS, NDS_POST_OVERLAY);
		p->draw(NDS_POST_OVERLAY);

		// clear depth (so it doesn't interfere with the gui) and draw debug stuff
		glClear(GL_DEPTH_BUFFER_BIT);
		maps2d->draw(MDS_DEBUG, NDS_NONE);
	
		glDisable(GL_BLEND);
		e->stop_2d_draw();
	}
	else if(active_map_type == MT_3D_MAP) {
		cam->run();
		sce->draw();
	}
}

void map_handler::load_map(const size_t& map_num) {
	if(maps2d->is_2d_map(map_num)) {
		p->set_pos(9, 8);
		maps3d->unload();
		maps2d->load(map_num);
		npc_graphics->set_palette(maps2d->get_palette()-1);
		active_map_type = MT_2D_MAP;
	}
	else if(maps3d->is_3d_map(map_num)) {
		maps2d->unload();
		maps3d->load(map_num);
		active_map_type = MT_3D_MAP;
	}
}

void map_handler::handle_key_down(event::GUI_EVENT_TYPE type, GUI_ID id) {
	last_key_press = SDL_GetTicks();
	
	/*switch(id) {
		default:
			break;
	}*/
}

void map_handler::handle_key_up(event::GUI_EVENT_TYPE type, GUI_ID id) {
	/*switch(id) {
		default:
			break;
	}*/
}

void map_handler::handle_right_click(event::GUI_EVENT_TYPE type, GUI_ID id) {
	if(active_map_type == MT_3D_MAP) {
		cam->set_mouse_input(cam->get_mouse_input() ^ true);
		e->set_cursor_visible(cam->get_mouse_input() ^ true);
	}
}

const size2& map_handler::get_player_position() const {
	return p->get_pos();
}

npcgfx* map_handler::get_npc_graphics() const {
	return npc_graphics;
}

tileset::tile_object* map_handler::get_tile(unsigned int type) {
	return maps2d->get_tile(type);
}

unsigned int map_handler::get_tile_num(unsigned int type) {
	return maps2d->get_tile_num(type);
}

const ssize3 map_handler::get_3d_tile() const {
	return maps3d->get_tile();
}

const MAP_TYPE& map_handler::get_active_map_type() const {
	return active_map_type;
}

const MAP_TYPE map_handler::get_map_type(const size_t& map_num) const {
	if(maps2d->is_2d_map(map_num)) return MT_2D_MAP;
	if(maps3d->is_3d_map(map_num)) return MT_3D_MAP;
	return MT_NONE;
}

void map_handler::debug_draw() {
	if(conf::get<bool>("debug.player_pos")) {
		ssize3 tile_info = get_3d_tile();
		if(tile_info.min_element() >= 0) {
			const float3 cam_pos = -float3(e->get_position());
			float3 tmin = ((cam_pos/16.0f).floored())*16.0f;
			float3 tmax = ((cam_pos/16.0f).ceiled())*16.0f;
			tmin.y = tmax.y = 0.0f;

			extbbox box(tmin, tmax, float3(0.0f), matrix4f().identity());
			egfx->draw_bbox(&box, 0x7FFF0000);
		}
	}
}

size_t& map_handler::get_next_dir() {
	return next_dir;
}
