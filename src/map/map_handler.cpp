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

#include "map/map_handler.hpp"
#include "ui/albion_ui.hpp"
#include <scene/camera.hpp>
#include <gui/style/gui_surface.hpp>

map_handler::map_handler() :
maps({{ lazy_xld("MAPDATA1.XLD"), lazy_xld("MAPDATA2.XLD"), lazy_xld("MAPDATA3.XLD") }}),
draw_cb(bind(&map_handler::draw, this, placeholders::_1, placeholders::_2)),
scene_draw_cb(bind(&map_handler::debug_draw, this, placeholders::_1)),
input_handler_fnctr(bind(&map_handler::input_handler, this, placeholders::_1, placeholders::_2))
{
	//
	tilesets = new tileset(palettes);
	npc_graphics = new npcgfx(palettes);
	maps2d = new map2d(tilesets, npc_graphics, maps);
	lab_data = new labdata();
	maps3d = new map3d(lab_data, maps);
	
	// read map types
	for(unsigned int i = 0; i < (MAX_MAP_NUMBER/100); ++i) {
		auto& mxld = maps[i];
		auto& file = mxld.get_file();
		for(size_t num = 0, max_num = mxld.get_object_count(); num < max_num; ++num) {
			if(mxld.get_object_size(num) == 0) continue;
			
			file.seek_read(mxld.get_object_offset(num) + 2);
			const auto type = file.get_char();
			map_types[i*100 + num] = (type == 0x01 ? MAP_TYPE::MAP_3D : (type == 0x02 ? MAP_TYPE::MAP_2D : MAP_TYPE::NONE));
		}
	}

	/*cout << "#########" << endl;
	for(size_t i = 0; i < 3; i++) {
		xld* maps = (i == 0 ? maps1 : (i == 1 ? maps2 : maps3));
		for(size_t map_num = 0; map_num < maps->get_object_count(); map_num++) {
			const xld::xld_object* obj = maps->get_object(map_num);
			const size_t rmap_num = (i*100)+map_num;
			if(obj->length == 0) cout << rmap_num << " nullptr" << endl;
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
	
	active_map_type = MAP_TYPE::NONE;
	
	sce->add_draw_callback("map_handler", scene_draw_cb);

	//
	last_key_press = SDL_GetTicks();
	last_move = SDL_GetTicks();
	
	eevt->add_event_handler(input_handler_fnctr,
							EVENT_TYPE::KEY_DOWN, EVENT_TYPE::KEY_UP,
							EVENT_TYPE::MOUSE_LEFT_DOWN, EVENT_TYPE::MOUSE_LEFT_UP, EVENT_TYPE::MOUSE_MOVE,
							EVENT_TYPE::FINGER_DOWN, EVENT_TYPE::FINGER_UP, EVENT_TYPE::FINGER_MOVE);
}

map_handler::~map_handler() {
	eevt->remove_event_handler(input_handler_fnctr);
	
	if(draw_cb_obj != nullptr) {
		ui->delete_draw_callback(draw_cb);
	}
	sce->delete_draw_callback(scene_draw_cb);

	delete tilesets;
	delete maps2d;
	delete maps3d;
	delete npc_graphics;
	delete lab_data;
	delete p2d;
	delete p3d;
}

bool map_handler::input_handler(EVENT_TYPE type, shared_ptr<event_object> obj) {
	// key input shouldn't be handled on 3d maps if free_cam is active (-> it is already handled by cam)
	const bool free_cam_3d = (conf::get<bool>("debug.free_cam") && active_map_type == MAP_TYPE::MAP_3D);
	
	lock_guard<mutex> lock(input_lock);
	
	/*const auto touch_norm_to_abs = [](float2 norm_coord) {
		SDL_Rect disp_bounds;
		SDL_GetDisplayBounds(SDL_GetWindowDisplayIndex(floor::get_window()), &disp_bounds);
		cout << endl << "########################>>>>>>>>>>>>>>" << endl;
		cout << "norm_coord: " << norm_coord << endl;
		cout << "bounds: @(" << disp_bounds.x << ", " << disp_bounds.y << "), size " << disp_bounds.w << " x " << disp_bounds.h << endl;
		int2 wnd_pos;
		SDL_GetWindowPosition(floor::get_window(), &wnd_pos.x, &wnd_pos.y);
		cout << "wnd pos: " << wnd_pos << endl;
		norm_coord *= float2 { disp_bounds.w, disp_bounds.h };
		cout << "scaled: " << norm_coord << endl;
		norm_coord -= float2 { wnd_pos };
		cout << "sub: " << norm_coord << endl;
		cout << "ret: " << int2 { norm_coord } << endl;
		cout << "########################<<<<<<<<<<<<<<" << endl;
		return int2 { norm_coord };
	};*/
	
	// handle input event
	switch(type) {
		case EVENT_TYPE::KEY_DOWN: {
			if(free_cam_3d) return false;
			const shared_ptr<key_down_event>& key_evt = (shared_ptr<key_down_event>&)obj;
			switch(key_evt->key) {
				case SDLK_LEFT:
				case SDLK_a:
					next_dir |= MOVE_DIRECTION::LEFT;
					break;
				case SDLK_RIGHT:
				case SDLK_d:
					next_dir |= MOVE_DIRECTION::RIGHT;
					break;
				case SDLK_UP:
				case SDLK_w:
					next_dir |= MOVE_DIRECTION::UP;
					break;
				case SDLK_DOWN:
				case SDLK_s:
					next_dir |= MOVE_DIRECTION::DOWN;
					break;
				default:
					return false;
			}
			return true;
		}
		case EVENT_TYPE::KEY_UP: {
			if(free_cam_3d) return false;
			const shared_ptr<key_up_event>& key_evt = (shared_ptr<key_up_event>&)obj;
			switch(key_evt->key) {
				case SDLK_LEFT:
				case SDLK_a:
					next_dir &= ~MOVE_DIRECTION::LEFT;
					break;
				case SDLK_RIGHT:
				case SDLK_d:
					next_dir &= ~MOVE_DIRECTION::RIGHT;
					break;
				case SDLK_UP:
				case SDLK_w:
					next_dir &= ~MOVE_DIRECTION::UP;
					break;
				case SDLK_DOWN:
				case SDLK_s:
					next_dir &= ~MOVE_DIRECTION::DOWN;
					break;
				default:
					return false;
			}
			return true;
		}
			
		case EVENT_TYPE::MOUSE_LEFT_DOWN: {
			if(input_type != ACTIVE_INPUT_TYPE::NONE) return false;
			input_type = ACTIVE_INPUT_TYPE::MOUSE;
			const auto& evt_obj = (shared_ptr<mouse_left_down_event>&)obj;
			click_position = evt_obj->position;
			break;
		}
		case EVENT_TYPE::MOUSE_LEFT_UP: {
			if(input_type != ACTIVE_INPUT_TYPE::MOUSE) return false;
			input_type = ACTIVE_INPUT_TYPE::NONE;
			next_dir = MOVE_DIRECTION::NONE;
			return true;
		}
		case EVENT_TYPE::MOUSE_MOVE: {
			if(input_type != ACTIVE_INPUT_TYPE::MOUSE) return false;
			const auto& evt_obj = (shared_ptr<mouse_move_event>&)obj;
			click_position = evt_obj->position;
			break;
		}
		
#if defined(FLOOR_IOS) // touch handling is disabled on the desktop for now
		case EVENT_TYPE::FINGER_DOWN: {
			if(input_type != ACTIVE_INPUT_TYPE::NONE) return false;
			input_type = ACTIVE_INPUT_TYPE::FINGER;
#if !defined(FLOOR_IOS)
			click_position = int2 { eevt->get_mouse_pos() };
#else
			const auto& evt_obj = (shared_ptr<finger_down_event>&)obj;
			click_position = int2 { evt_obj->normalized_position * float2 { floor::get_width(), floor::get_height() } };
#endif
			break;
		}
		case EVENT_TYPE::FINGER_UP: {
			if(input_type != ACTIVE_INPUT_TYPE::FINGER) return false;
			input_type = ACTIVE_INPUT_TYPE::NONE;
			next_dir = MOVE_DIRECTION::NONE;
			return true;
		}
		case EVENT_TYPE::FINGER_MOVE: {
			if(input_type != ACTIVE_INPUT_TYPE::FINGER) return false;
#if !defined(FLOOR_IOS)
			click_position = int2 { eevt->get_mouse_pos() };
#else
			const auto& evt_obj = (shared_ptr<finger_down_event>&)obj;
			click_position = int2 { evt_obj->normalized_position * float2 { floor::get_width(), floor::get_height() } };
#endif
			click_position = int2 { eevt->get_mouse_pos() };
			break;
		}
#endif
			
		default: return false;
	}
	
	// handle new click_position
	update_next_dir();
	
	return true;
}

void map_handler::update_next_dir() {
	const float2 fclick_position { click_position };
	const float2 player_screen_position { p2d->compute_screen_position() + float2(1.0f, 1.5f) * maps2d->get_tile_size() };
	
	// player pos -> click pos vector
	const float2 pos_diff { fclick_position - player_screen_position };
	if(pos_diff.length() < maps2d->get_tile_size() * 0.75f) {
		// click is too close to the player -> stop moving to prevent jittering
		next_dir = MOVE_DIRECTION::NONE;
		return;
	}
	
	// angle to the x axis
	const float angle { (float)const_math::rad_to_deg(std::atan2f(pos_diff.y, pos_diff.x)) };
	
	MOVE_DIRECTION new_dir = MOVE_DIRECTION::NONE;
	static constexpr float overlap = 67.5f;
	if(angle >= -overlap && angle <= overlap) new_dir |= MOVE_DIRECTION::RIGHT;
	if(angle >= 90.0f - overlap && angle <= 90.0f + overlap) new_dir |= MOVE_DIRECTION::DOWN;
	if(angle >= -90.0f - overlap && angle <= -90.0f + overlap) new_dir |= MOVE_DIRECTION::UP;
	// or because of -180 <-> +180 transition
	if(angle >= 180.0f - overlap || angle <= -180.0f + overlap) new_dir |= MOVE_DIRECTION::LEFT;
	 
	next_dir = new_dir;
}

void map_handler::handle() {
	const events::map_exit_event* me_evt = nullptr;
	const MOVE_DIRECTION cur_next_dir = next_dir;
	size2 player_pos(0, 0);
	if(active_map_type == MAP_TYPE::MAP_2D) {
		if(cur_next_dir != MOVE_DIRECTION::NONE && (SDL_GetTicks() - last_move) > TIME_PER_TILE) {
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
	else if(active_map_type == MAP_TYPE::MAP_3D) {
		p3d->move(cur_next_dir);
		if(p3d->has_moved()) {
			p3d->set_moved(false);
			player_pos = p3d->get_pos();
			maps3d->get_map_events();
			me_evt = maps3d->get_map_events().get_map_exit_event(player_pos);
		}
	}
	
	// map change
	if(me_evt != nullptr) {
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
	
	if(active_map_type != MAP_TYPE::MAP_2D) {
		cam->run();
	}
	
	// update next dir continuously while the mouse/finger input is active
	if(input_type != ACTIVE_INPUT_TYPE::NONE) {
		update_next_dir();
	}
}

void map_handler::draw(const DRAW_MODE_UI draw_mode floor_unused, rtt::fbo* buffer floor_unused) {
	if(active_map_type != MAP_TYPE::MAP_2D) return;
	
	if(conf::get<bool>("map.draw_underlay")) maps2d->draw(MAP_DRAW_STAGE::UNDERLAY, NPC_DRAW_STAGE::NONE);
	
	glDepthMask(GL_FALSE);
	maps2d->draw(MAP_DRAW_STAGE::NPCS, NPC_DRAW_STAGE::PRE_OVERLAY);
	p2d->draw(NPC_DRAW_STAGE::PRE_OVERLAY);
	glDepthMask(GL_TRUE);
	
	if(conf::get<bool>("map.draw_overlay")) maps2d->draw(MAP_DRAW_STAGE::OVERLAY, NPC_DRAW_STAGE::NONE);
	
	maps2d->draw(MAP_DRAW_STAGE::NPCS, NPC_DRAW_STAGE::POST_OVERLAY);
	p2d->draw(NPC_DRAW_STAGE::POST_OVERLAY);
	
	// clear depth (so it doesn't interfere with the gui) and draw debug stuff
	glClear(GL_DEPTH_BUFFER_BIT);
	maps2d->draw(MAP_DRAW_STAGE::DEBUGGING, NPC_DRAW_STAGE::NONE);
	
#if 0 // click/touch debug drawing
	static const auto dpi_scale = float(floor::get_dpi()) / 72.0f;
	if(input_lock.try_lock()) {
		if(input_type != ACTIVE_INPUT_TYPE::NONE) {
			gfx2d::draw_circle_gradient(float2(click_position), 16.0f * dpi_scale, 16.0f * dpi_scale,
										gfx2d::GRADIENT_TYPE::CENTER_ROUND,
										float4 { 0.0f, 0.40f, 0.45f, 0.5f },
										vector<float4> {
											float4 { 1.0f, 0.0f, 0.0f, 0.05f },
											float4 { 1.0f, 0.0f, 0.0f, 0.5f },
											float4 { 1.0f, 0.0f, 0.0f, 1.0f },
											float4 { 1.0f, 0.0f, 0.0f, 0.0f }
										});
			gfx2d::draw_circle_gradient(p2d->compute_screen_position() + float2(1.0f, 1.5f) * maps2d->get_tile_size(),
										16.0f * dpi_scale, 16.0f * dpi_scale,
										gfx2d::GRADIENT_TYPE::CENTER_ROUND,
										float4 { 0.0f, 0.40f, 0.45f, 0.5f },
										vector<float4> {
											float4 { 0.0f, 1.0f, 0.0f, 0.05f },
											float4 { 0.0f, 1.0f, 0.0f, 0.5f },
											float4 { 0.0f, 1.0f, 0.0f, 1.0f },
											float4 { 0.0f, 1.0f, 0.0f, 0.0f }
										});
		}
		input_lock.unlock();
	}
#endif
}

void map_handler::load_map(const size_t& map_num, const size2 player_pos, const MOVE_DIRECTION player_direction) {
	if(map_types[map_num] == MAP_TYPE::MAP_2D) {
		// unload old 3d map and disable scene, since we don't need it
		maps3d->unload();
		if(sce->is_enabled()) {
			sce->set_enabled(false);
		}
		
		// add 2d ui draw callback (if it doesn't exist yet or has been deleted previously)
		if(draw_cb_obj == nullptr) {
			draw_cb_obj = ui->add_draw_callback(DRAW_MODE_UI::PRE_UI, draw_cb, float2(1.0f), float2(0.0f),
												gui_surface::SURFACE_FLAGS::NO_ANTI_ALIASING);
		}
		
		// use another default for the 2d player for now
		size2 ppos2d = player_pos;
		if(ppos2d.x == 0 && ppos2d.y == 0) {
			ppos2d.set(9, 8);
		}
		p2d->set_pos(ppos2d.x, ppos2d.y);
		p2d->set_view_direction(player_direction);
		maps2d->load(map_num);
		maps2d->set_initial_position(ppos2d);
		npc_graphics->set_palette(maps2d->get_palette()-1);
		active_map_type = MAP_TYPE::MAP_2D;
		
		// disable 3d cam input, show cursor again
		cam->set_mouse_input(false);
		floor::set_cursor_visible(true);
	}
	else if(map_types[map_num] == MAP_TYPE::MAP_3D) {
		// kill 2d ui draw callback
		if(draw_cb_obj != nullptr) {
			ui->delete_draw_callback(draw_cb);
			draw_cb_obj = nullptr;
		}
		maps2d->unload();
		
		// enable scene (again), because we obviously need it
		if(!sce->is_enabled()) {
			sce->set_enabled(true);
		}
		maps3d->load(map_num);
		p3d->set_pos(player_pos.x, player_pos.y);
		p3d->set_view_direction(player_direction);
		active_map_type = MAP_TYPE::MAP_3D;
	}
	else return;
	
	active_map_num = map_num;
	
	// reset move direction
	next_dir = MOVE_DIRECTION::NONE;
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

const size_t& map_handler::get_active_map_num() const {
	return active_map_num;
}

MAP_TYPE map_handler::get_map_type(const size_t& map_num) const {
	return map_types[map_num];
}

void map_handler::debug_draw(const DRAW_MODE) {
	if(conf::get<bool>("debug.player_pos")) {
		ssize3 tile_info = get_3d_tile();
		if(tile_info.min_element() >= 0) {
			const float3 cam_pos = -float3(engine::get_position());
			float3 tmin = ((cam_pos/16.0f).floored())*16.0f;
			float3 tmax = ((cam_pos/16.0f).ceiled())*16.0f;
			tmin.y = tmax.y = 0.0f;

			extbbox box(tmin, tmax, float3(0.0f), matrix4f().identity());
			//gfx2d::draw_bbox(box, 0x7FFF0000);
		}
	}
}
