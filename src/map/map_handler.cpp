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

#include "map_handler.h"
#include <scene/camera.h>
#include <gui/style/gui_surface.h>

map_handler::map_handler() :
draw_cb(this, &map_handler::draw),
scene_draw_cb(this, &map_handler::debug_draw),
key_handler_fnctr(this, &map_handler::key_handler)
{
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
	p2d = new player2d(maps2d, npc_graphics);
	p2d->set_pos(9, 8);
	p3d = new player3d(maps3d);
	p3d->set_pos(0, 0);
	
	active_map_type = MT_NONE;
	
	sce->add_draw_callback("map_handler", scene_draw_cb);
	draw_cb_obj = ui->add_draw_callback(DRAW_MODE_UI::PRE_UI, draw_cb, float2(1.0f), float2(0.0f));

	//
	last_key_press = SDL_GetTicks();
	last_move = SDL_GetTicks();
	
	eevt->add_event_handler(key_handler_fnctr, EVENT_TYPE::KEY_DOWN, EVENT_TYPE::KEY_UP);
}

map_handler::~map_handler() {
	eevt->remove_event_handler(key_handler_fnctr);
	
	ui->delete_draw_callback(draw_cb);
	sce->delete_draw_callback(scene_draw_cb);
	
	delete maps1;
	delete maps2;
	delete maps3;

	delete tilesets;
	delete maps2d;
	delete maps3d;
	delete npc_graphics;
	delete lab_data;
	delete p2d;
	delete p3d;
}

bool map_handler::key_handler(EVENT_TYPE type, shared_ptr<event_object> obj) {
	if(conf::get<bool>("debug.free_cam") &&
	   active_map_type == MT_3D_MAP) {
		return false;
	}
	
	if(type == EVENT_TYPE::KEY_DOWN) {
		const shared_ptr<key_down_event>& key_evt = (shared_ptr<key_down_event>&)obj;
		switch(key_evt->key) {
			case SDLK_LEFT:
			case SDLK_a:
				next_dir |= MD_LEFT;
				break;
			case SDLK_RIGHT:
			case SDLK_d:
				next_dir |= MD_RIGHT;
				break;
			case SDLK_UP:
			case SDLK_w:
				next_dir |= MD_UP;
				break;
			case SDLK_DOWN:
			case SDLK_s:
				next_dir |= MD_DOWN;
				break;
			default:
				return false;
		}
	}
	else if(type == EVENT_TYPE::KEY_UP) {
		const shared_ptr<key_up_event>& key_evt = (shared_ptr<key_up_event>&)obj;
		switch(key_evt->key) {
			case SDLK_LEFT:
			case SDLK_a:
				next_dir &= ~MD_LEFT;
				break;
			case SDLK_RIGHT:
			case SDLK_d:
				next_dir &= ~MD_RIGHT;
				break;
			case SDLK_UP:
			case SDLK_w:
				next_dir &= ~MD_UP;
				break;
			case SDLK_DOWN:
			case SDLK_s:
				next_dir &= ~MD_DOWN;
				break;
			default:
				return false;
		}
	}
	else return false;
	return true;
}

void map_handler::handle() {
	const events::map_exit_event* me_evt = NULL;
	const MOVE_DIRECTION cur_next_dir = (MOVE_DIRECTION)next_dir.load();
	size2 player_pos(0, 0);
	if(active_map_type == MT_2D_MAP) {
		if(cur_next_dir != MD_NONE && (SDL_GetTicks() - last_move) > TIME_PER_TILE) {
			last_move = SDL_GetTicks();
			p2d->move(cur_next_dir);
		}
		if(p2d->has_moved()) {
			p2d->set_moved(false);
			player_pos = p2d->get_pos();
			me_evt = maps2d->get_map_events().get_map_exit_event(player_pos);
		}
		
		draw_cb_obj->redraw();
	}
	else if(active_map_type == MT_3D_MAP) {
		p3d->move(cur_next_dir);
		if(p3d->has_moved()) {
			p3d->set_moved(false);
			player_pos = p3d->get_pos();
			maps3d->get_map_events();
			me_evt = maps3d->get_map_events().get_map_exit_event(player_pos);
		}
	}
	
	// map change
	if(me_evt != NULL) {
		size2 map_pos = size2(me_evt->map_x-1, me_evt->map_y-1);
		// the map change event pos might be (0, 0), in which case the player pos should be used
		if(me_evt->map_x == 0 && me_evt->map_y == 0) {
			map_pos = player_pos;
		}
		// if only one is 0, this is a continent map change (keep player pos for this coordinate)
		else if(me_evt->map_x == 0 || me_evt->map_y == 0) {
			if(me_evt->map_x == 0) {
				map_pos.x = player_pos.x;
				map_pos.y = me_evt->map_y - 1;
			}
			else if(me_evt->map_y == 0) {
				map_pos.x = me_evt->map_x - 1;
				map_pos.y = player_pos.y;
			}
		}
		load_map(me_evt->next_map-100, map_pos, me_evt->direction);
		return;
	}
	
	maps2d->handle();
	maps3d->handle();
	p2d->handle();
	p3d->handle();
	
	if(active_map_type == MT_3D_MAP) {
		cam->run();
	}
}

void map_handler::draw(const DRAW_MODE_UI draw_mode a2e_unused, rtt::fbo* buffer a2e_unused) {
	if(active_map_type != MT_2D_MAP) return;
	
	maps2d->draw(MDS_NPCS, NDS_PRE_UNDERLAY);
	p2d->draw(NDS_PRE_UNDERLAY);
	
	if(conf::get<bool>("map.draw_underlay")) maps2d->draw(MDS_UNDERLAY, NDS_NONE);
	
	maps2d->draw(MDS_NPCS, NDS_PRE_OVERLAY);
	p2d->draw(NDS_PRE_OVERLAY);
	
	if(conf::get<bool>("map.draw_overlay")) maps2d->draw(MDS_OVERLAY, NDS_NONE);
	
	maps2d->draw(MDS_NPCS, NDS_POST_OVERLAY);
	p2d->draw(NDS_POST_OVERLAY);
	
	// clear depth (so it doesn't interfere with the gui) and draw debug stuff
	glClear(GL_DEPTH_BUFFER_BIT);
	maps2d->draw(MDS_DEBUG, NDS_NONE);
}

void map_handler::load_map(const size_t& map_num, const size2 player_pos, const MOVE_DIRECTION player_direction) {
	if(maps2d->is_2d_map(map_num)) {
		// use another default for the 2d player for now
		size2 ppos2d = player_pos;
		if(ppos2d.x == 0 && ppos2d.y == 0) {
			ppos2d.set(9, 8);
		}
		p2d->set_pos(ppos2d.x, ppos2d.y);
		p2d->set_view_direction(player_direction);
		maps3d->unload();
		maps2d->load(map_num);
		maps2d->set_initial_position(ppos2d);
		npc_graphics->set_palette(maps2d->get_palette()-1);
		active_map_type = MT_2D_MAP;
		
		// disable 3d cam input, show cursor again
		cam->set_mouse_input(false);
		e->set_cursor_visible(true);
	}
	else if(maps3d->is_3d_map(map_num)) {
		maps2d->unload();
		maps3d->load(map_num);
		p3d->set_pos(player_pos.x, player_pos.y);
		p3d->set_view_direction(player_direction);
		active_map_type = MT_3D_MAP;
	}
	
	// reset move direction
	next_dir = MD_NONE;
}

const size2& map_handler::get_player_position() const {
	return p2d->get_pos();
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

MAP_TYPE map_handler::get_map_type(const size_t& map_num) const {
	if(maps2d->is_2d_map(map_num)) return MT_2D_MAP;
	if(maps3d->is_3d_map(map_num)) return MT_3D_MAP;
	return MT_NONE;
}

void map_handler::debug_draw(const DRAW_MODE) {
	if(conf::get<bool>("debug.player_pos")) {
		ssize3 tile_info = get_3d_tile();
		if(tile_info.min_element() >= 0) {
			const float3 cam_pos = -float3(e->get_position());
			float3 tmin = ((cam_pos/16.0f).floored())*16.0f;
			float3 tmax = ((cam_pos/16.0f).ceiled())*16.0f;
			tmin.y = tmax.y = 0.0f;

			extbbox box(tmin, tmax, float3(0.0f), matrix4f().identity());
			//gfx2d::draw_bbox(box, 0x7FFF0000);
		}
	}
}
