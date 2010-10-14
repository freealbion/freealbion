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

#include "map_handler.h"

map_handler::map_handler(pal* palettes) : palettes(palettes) {	
	tilesets = new tileset(palettes);
	maps2d = new map2d(tilesets);
	npc_graphics = new npcgfx(palettes);
	
	p = new player(maps2d, npc_graphics);
	p->set_pos(9, 8);
	
	//
	last_key_press = SDL_GetTicks();
	last_move = SDL_GetTicks();
	next_dir = MD_NONE;
	evt->add_event_callback(this, &map_handler::handle_key_press, event::KEY_DOWN, SDLK_w);
	evt->add_event_callback(this, &map_handler::handle_key_press, event::KEY_DOWN, SDLK_a);
	evt->add_event_callback(this, &map_handler::handle_key_press, event::KEY_DOWN, SDLK_s);
	evt->add_event_callback(this, &map_handler::handle_key_press, event::KEY_DOWN, SDLK_d);
	evt->add_event_callback(this, &map_handler::handle_key_press, event::KEY_DOWN, SDLK_UP);
	evt->add_event_callback(this, &map_handler::handle_key_press, event::KEY_DOWN, SDLK_DOWN);
	evt->add_event_callback(this, &map_handler::handle_key_press, event::KEY_DOWN, SDLK_LEFT);
	evt->add_event_callback(this, &map_handler::handle_key_press, event::KEY_DOWN, SDLK_RIGHT);
}

map_handler::~map_handler() {
	evt->delete_event_callbacks(this, &map_handler::handle_key_press, event::KEY_DOWN, SDLK_w);
	evt->delete_event_callbacks(this, &map_handler::handle_key_press, event::KEY_DOWN, SDLK_a);
	evt->delete_event_callbacks(this, &map_handler::handle_key_press, event::KEY_DOWN, SDLK_s);
	evt->delete_event_callbacks(this, &map_handler::handle_key_press, event::KEY_DOWN, SDLK_d);
	evt->delete_event_callbacks(this, &map_handler::handle_key_press, event::KEY_DOWN, SDLK_UP);
	evt->delete_event_callbacks(this, &map_handler::handle_key_press, event::KEY_DOWN, SDLK_DOWN);
	evt->delete_event_callbacks(this, &map_handler::handle_key_press, event::KEY_DOWN, SDLK_LEFT);
	evt->delete_event_callbacks(this, &map_handler::handle_key_press, event::KEY_DOWN, SDLK_RIGHT);
	
	delete tilesets;
	delete maps2d;
	delete npc_graphics;
	delete p;
}

void map_handler::handle() {
	if((SDL_GetTicks() - last_key_press) > TIME_PER_TILE) {
		next_dir = MD_NONE;
	}

	if(next_dir != MD_NONE && (SDL_GetTicks() - last_move) > TIME_PER_TILE) {
		last_move = SDL_GetTicks();
		p->move(next_dir);
	}

	maps2d->handle();
	p->handle();
}

void map_handler::draw() {
	e->start_2d_draw();
	
	//glEnable(GL_STENCIL_TEST);
	//glDisable(GL_STENCIL_TEST);
	
	glDisable(GL_DEPTH_TEST);
	//p->draw();
	
	if(conf::get<bool>("map.draw_underlay")) maps2d->draw(0);
	
	p->draw();
	
	if(conf::get<bool>("map.draw_overlay")) maps2d->draw(1);
	
	//p->draw();
	glEnable(GL_DEPTH_TEST);
	
	e->stop_2d_draw();
}

void map_handler::load_map(const size_t& map_num) {
	if(maps2d->is_2d_map(map_num)) {
		maps2d->load(map_num);
		npc_graphics->set_palette(maps2d->get_palette());
	}
	/*else if(maps3d->is_3d_map(map_num)) {
		maps3d->load(map_num);
	}*/
}

void map_handler::handle_key_press(event::GUI_EVENT_TYPE type, GUI_ID id) {
	//if(SDL_GetTicks() - last_key_press < time_per_tile) return;
	last_key_press = SDL_GetTicks();

	switch(id) {
		case SDLK_LEFT:
		case SDLK_a:
			next_dir = MD_LEFT;
			//p->move(MD_LEFT);
			break;
		case SDLK_RIGHT:
		case SDLK_d:
			next_dir = MD_RIGHT;
			//p->move(MD_RIGHT);
			break;
		case SDLK_UP:
		case SDLK_w:
			next_dir = MD_UP;
			//p->move(MD_UP);
			break;
		case SDLK_DOWN:
		case SDLK_s:
			next_dir = MD_DOWN;
			//p->move(MD_DOWN);
			break;
		default:
			next_dir = MD_NONE;
			break;
	}
}

const size2& map_handler::get_player_position() const {
	return p->get_pos();
}

tileset::tile_object* map_handler::get_tile(unsigned int type) {
	return maps2d->get_tile(type);
}

npcgfx* map_handler::get_npc_graphics() const {
	return npc_graphics;
}
