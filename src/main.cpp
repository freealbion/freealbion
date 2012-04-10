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

#include "main.h"
#include <a2e.h>

/*!
 * \mainpage
 *
 * \author flo
 *
 * \date April 2007 - April 2012
 *
 * Albion Remake
 */

//
static a2e_texture debug_tex;

int main(int argc, char *argv[]) {
	// initialize the engine
#if !defined(A2E_IOS) || !defined(A2E_DEBUG)
	e = new engine(argv[0], (const char*)"../data/");
#else
	e = new engine(argv[0], (const char*)"../../../../Documents/albion/data/");
#endif
	e->init();
	e->set_caption(APPLICATION_NAME);
	
	conf::init();
	xld::set_xld_path(e->data_path("xld/"));
	
	clck = new ar_clock();

	// init class pointers
	fio = e->get_file_io();
	eevt = e->get_event();
	t = e->get_texman();
	ocl = e->get_opencl();
	exts = e->get_ext();
	s = e->get_shader();
	sce = e->get_scene();
	ui = e->get_gui();
	
	conf::add<a2e_texture>("debug.texture", t->get_dummy_texture());
	
	cam = new camera(e);

	// initialize the camera
	cam->set_position(0.0f, -80.0f, 0.0f);
	cam->set_rotation(0.0f, 90.0f+45.0f, 0.0f);
	cam->set_rotation_speed(300.0f);
	cam->set_cam_speed(5.0f);
	cam->set_mouse_input(false);
	cam->set_keyboard_input(true);
	cam->set_wasd_input(true);

	// initialize the scene
	sce->set_eye_distance(-0.3f);
	
	// add shaders
	const string ar_shaders[][2] = {
		{ "AR_IR_GBUFFER_MAP_OBJECTS", "inferred/gp_gbuffer_map_objects.a2eshd" },
		{ "AR_IR_GBUFFER_MAP_TILES", "inferred/gp_gbuffer_map_tiles.a2eshd" },
		{ "AR_IR_MP_SKY", "inferred/mp_sky.a2eshd" },
		{ "AR_IR_MP_MAP_OBJECTS", "inferred/mp_map_objects.a2eshd" },
		{ "AR_IR_MP_MAP_TILES", "inferred/mp_map_tiles.a2eshd" },
	};
	for(const auto& shd : ar_shaders) {
		if(!s->add_a2e_shader(shd[0], shd[1])) {
			a2e_error("couldn't add a2e-shader \"%s\"!", shd[1]);
			done = true;
		}
	}
	
	// add event handlers
	event::handler key_handler_fnctr(&key_handler);
	eevt->add_event_handler(key_handler_fnctr, EVENT_TYPE::KEY_DOWN, EVENT_TYPE::KEY_UP, EVENT_TYPE::KEY_PRESSED);
	event::handler mouse_handler_fnctr(&mouse_handler);
	eevt->add_event_handler(mouse_handler_fnctr, EVENT_TYPE::MOUSE_RIGHT_CLICK);
	event::handler quit_handler_fnctr(&quit_handler);
	eevt->add_event_handler(quit_handler_fnctr, EVENT_TYPE::QUIT);
	event::handler window_handler_fnctr(&window_handler);
	eevt->add_internal_event_handler(window_handler_fnctr, EVENT_TYPE::WINDOW_RESIZE);

	// load/init stuff
	palettes = new pal();
	scaling::init();
	bin_gfx = new bin_graphics();

	mh = new map_handler();
	size_t inital_map_num = 99; // shortcut map
	if(argc > 1) {
		const string map_num_str = argv[1];
		if(!map_num_str.empty()) {
			const size_t arg_map_num = string2size_t(map_num_str);
			if((arg_map_num == 0 && map_num_str[0] == '0') ||
			   (arg_map_num != 0 && mh->get_map_type(arg_map_num) != MT_NONE)) {
				inital_map_num = arg_map_num;
			}
			else {
				a2e_error("invalid map number or map type!");
			}
		}
	}
	mh->load_map(inital_map_num);

	aui = new albion_ui(mh);
	if(conf::get<bool>("ui.display")) {
		aui->open_goto_map_wnd();
		aui->open_game_ui();
	}
	
	cam->set_keyboard_input(conf::get<bool>("debug.free_cam"));
	
	// debug window
	//a2e_debug_wnd::init(e, eui, s, ocl, cam);
	//a2e_debug_wnd::open();
	
	// for debugging purposes
	debug_tex = make_a2e_texture();
	debug_tex->width = e->get_width();
	debug_tex->height = e->get_height();
	
	// main loop
	while(!done) {
		// event handling
		eevt->handle_events();
		
		// stop drawing if window is inactive
		if(!(SDL_GetWindowFlags(e->get_window()) & SDL_WINDOW_INPUT_FOCUS)) {
			SDL_Delay(20);
			continue;
		}
		
		// set caption (app name and fps count)
		if(e->is_new_fps_count()) {
			//cout << "FPS: " << e->get_fps() << endl;
			caption << APPLICATION_NAME << " | FPS: " << e->get_fps();
			if(mh->get_tile(0) != NULL) {
				caption << " | ";
				
				stringstream tbytes;
				string byte_strs[6];
				tbytes.setf(ios::hex, ios::basefield);
				
				tbytes << mh->get_tile(0)->upper_bytes;
				byte_strs[0] = tbytes.str();
				core::reset(&tbytes);
				tbytes << mh->get_tile(0)->lower_bytes;
				byte_strs[1] = tbytes.str();
				core::reset(&tbytes);
				tbytes << mh->get_tile(1)->upper_bytes;
				byte_strs[2] = tbytes.str();
				core::reset(&tbytes);
				tbytes << mh->get_tile(1)->lower_bytes;
				byte_strs[3] = tbytes.str();
				core::reset(&tbytes);
				tbytes << mh->get_tile_num(0);
				byte_strs[4] = tbytes.str();
				core::reset(&tbytes);
				tbytes << mh->get_tile_num(1);
				byte_strs[5] = tbytes.str();
				core::reset(&tbytes);

				for(size_t i = 0; i < 6; i++) {
					size_t add_zeros = 8 - byte_strs[i].length();
					for(size_t j = 0; j < add_zeros; j++) {
						byte_strs[i] = "0"+byte_strs[i];
					}
				}
				
				caption << byte_strs[0] << " " << byte_strs[1] << " | ";
				caption << byte_strs[2] << " " << byte_strs[3] << " | ";
				caption << byte_strs[4] << " | ";
				caption << byte_strs[5] << " | ";
				caption << "c: " << (size_t)mh->get_tile(0)->collision << " " << (size_t)mh->get_tile(1)->collision;
			}

			if(mh->get_active_map_type() == MT_3D_MAP) {
				ssize3 tile_info = mh->get_3d_tile();
				if(tile_info.min_element() >= 0) {
					caption << " | " << tile_info.x << " " << tile_info.y << " " << tile_info.z;
				}
				caption << " | Pos: " << (float3(-*e->get_position())/std_tile_size).floored();
				caption << " | Cam: " << float3(-*e->get_position());
			}
			else if(mh->get_active_map_type() == MT_2D_MAP) {
				caption << " | Pos: " << mh->get_player_position();
			}

			e->set_caption(caption.str().c_str());
			core::reset(&caption);
		}

		e->start_draw();

		clck->run();
		mh->handle();
		aui->run();
		
		// TODO: move this into a function and add a draw callback
		/*if(conf::get<bool>("debug.display_debug_texture")) {
			a2e_texture tex = conf::get<a2e_texture>("debug.texture");
			if(tex->width > 0 && tex->height > 0) {
				e->start_2d_draw();
				size_t draw_width = tex->width, draw_height = tex->height;
				float ratio = float(draw_width) / float(draw_height);
				float scale = 1.0f;
				if(ratio >= 1.0f && draw_width > e->get_width()) {
					scale = float(e->get_width()) / float(draw_width);
				}
				else if(ratio < 1.0f && draw_height > e->get_height()) {
					scale = float(e->get_height()) / float(draw_height);
				}
				draw_width *= scale;
				draw_height *= scale;
				gfx2d::draw_rectangle_texture(rect(0, 0, (unsigned int)draw_width, (unsigned int)draw_height),
											  conf::get<a2e_texture>("debug.texture")->tex(),
											  float4(1.0f, 1.0f, 1.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 1.0f));
				e->stop_2d_draw();
			}
		}*/

		e->stop_draw();
	}
	debug_tex->tex_num = 0;
	
	//a2e_debug_wnd::close();
	
	eevt->remove_event_handler(key_handler_fnctr);
	eevt->remove_event_handler(mouse_handler_fnctr);
	eevt->remove_event_handler(quit_handler_fnctr);
	eevt->remove_event_handler(window_handler_fnctr);

	delete palettes;
	delete bin_gfx;

	delete aui;
	delete cam;
	delete e;

	return 0;
}


bool key_handler(EVENT_TYPE type, shared_ptr<event_object> obj) {
	// cast correctly
	if(type == EVENT_TYPE::KEY_DOWN) {
		const shared_ptr<key_down_event>& key_evt = (shared_ptr<key_down_event>&)obj;
		switch(key_evt->key) {
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				cam->set_cam_speed(0.2f);
				break;
			default:
				return false;
		}
	}
	else if(type == EVENT_TYPE::KEY_UP) {
		const shared_ptr<key_up_event>& key_evt = (shared_ptr<key_up_event>&)obj;
		switch(key_evt->key) {
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				cam->set_cam_speed(5.0f);
				break;
			default:
				return false;
		}
	}
	else { // EVENT_TYPE::KEY_PRESSED
		const shared_ptr<key_pressed_event>& key_evt = (shared_ptr<key_pressed_event>&)obj;
		switch(key_evt->key) {
			case SDLK_ESCAPE:
				done = true;
				break;
			/*case SDLK_CARET:
				if(!a2e_debug_wnd::is_open()) a2e_debug_wnd::open();
				else a2e_debug_wnd::close();
				break;*/
			case SDLK_g:
				conf::set<bool>("ui.display", conf::get<bool>("ui.display") ^ true);
				if(conf::get<bool>("ui.display")) {
					aui->open_goto_map_wnd();
					aui->open_game_ui();
				}
				else {
					aui->close_goto_map_wnd();
					aui->close_game_ui();
				}
				break;
			case SDLK_c:
				conf::set<bool>("map.collision", conf::get<bool>("map.collision") ^ true);
				break;
			case SDLK_t:
				conf::set<bool>("debug.player_pos", conf::get<bool>("debug.player_pos") ^ true);
				break;
			case SDLK_u:
				conf::set<bool>("map.draw_underlay", conf::get<bool>("map.draw_underlay") ^ true);
				break;
			case SDLK_o:
				conf::set<bool>("map.draw_overlay", conf::get<bool>("map.draw_overlay") ^ true);
				break;
			case SDLK_e:
				conf::set<bool>("debug.draw_events", conf::get<bool>("debug.draw_events") ^ true);
				break;
			case SDLK_q:
				conf::set<bool>("debug.display_debug_texture", conf::get<bool>("debug.display_debug_texture") ^ true);
				break;
			case SDLK_n:
				if(conf::get<size_t>("debug.npcgfx") > 0) {
					conf::set<size_t>("debug.npcgfx", conf::get<size_t>("debug.npcgfx")-1);
					cout << ":: " << conf::get<size_t>("debug.npcgfx") << endl;
				}
				break;
			case SDLK_m:
				if(conf::get<size_t>("debug.npcgfx") < 209) {
					conf::set<size_t>("debug.npcgfx", conf::get<size_t>("debug.npcgfx")+1);
					cout << ":: " << conf::get<size_t>("debug.npcgfx") << endl;
				}
				break;
			case SDLK_r:
				cam->set_position(0.0f, -8.0f, 0.0f);
				cam->set_rotation(0.0f, 90.0f+45.0f, 0.0f);
				break;
			case SDLK_1:
				debug_tex->tex_num = sce->_get_g_buffer(0)->tex_id[0];
				conf::set<a2e_texture>("debug.texture", debug_tex);
				break;
			case SDLK_2:
				debug_tex->tex_num = sce->_get_g_buffer(1)->tex_id[0];
				conf::set<a2e_texture>("debug.texture", debug_tex);
				break;
			case SDLK_3:
				debug_tex->tex_num = sce->_get_g_buffer(1)->tex_id[1];
				conf::set<a2e_texture>("debug.texture", debug_tex);
				break;
			case SDLK_4:
				debug_tex->tex_num = sce->_get_l_buffer(0)->tex_id[0];
				conf::set<a2e_texture>("debug.texture", debug_tex);
				break;
			case SDLK_5:
				debug_tex->tex_num = sce->_get_l_buffer(1)->tex_id[0];
				conf::set<a2e_texture>("debug.texture", debug_tex);
				break;
			case SDLK_6:
				debug_tex->tex_num = sce->_get_fxaa_buffer()->tex_id[0];
				conf::set<a2e_texture>("debug.texture", debug_tex);
				break;
			case SDLK_7:
				debug_tex->tex_num = sce->_get_scene_buffer()->tex_id[0];
				conf::set<a2e_texture>("debug.texture", debug_tex);
				break;
			case SDLK_v:
				cam->set_keyboard_input(cam->get_keyboard_input() ^ true);
				conf::set<bool>("debug.free_cam", cam->get_keyboard_input());
				break;
			case SDLK_MINUS:
			case SDLK_KP_MINUS:
				clck->set_ticks(((clck->get_ticks() + AR_TICKS_PER_DAY) - AR_TICKS_PER_HOUR/4) % AR_TICKS_PER_DAY);
				break;
			case SDLK_PLUS:
			case SDLK_KP_PLUS:
				clck->set_ticks(((clck->get_ticks() + AR_TICKS_PER_DAY) + AR_TICKS_PER_HOUR/4) % AR_TICKS_PER_DAY);
				break;
			case SDLK_F18:
			case SDLK_9:
				e->reload_shaders();
				break;
			case SDLK_F19:
			case SDLK_0:
				e->reload_kernels();
				break;
			// TODO: make this work
			/*case SDLK_F7:
				mh->load_map(99);
				break;*/
			default:
				return false;
		}
	}
	return true;
}

bool mouse_handler(EVENT_TYPE type, shared_ptr<event_object> obj) {
	if(type == EVENT_TYPE::MOUSE_RIGHT_CLICK) {
		const bool cur_state = cam->get_mouse_input();
		cam->set_mouse_input(cur_state ^ true);
		e->set_cursor_visible(cur_state);
	}
	return true;
}

bool quit_handler(EVENT_TYPE type, shared_ptr<event_object> obj) {
	done = true;
	return true;
}

bool window_handler(EVENT_TYPE type, shared_ptr<event_object> obj) {
	if(type == EVENT_TYPE::WINDOW_RESIZE) {
		const window_resize_event& evt = (const window_resize_event&)*obj;
		aui->load_game_ui(evt.size);
	}
	return true;
}
