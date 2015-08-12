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

#include "main.hpp"

/*!
 * \mainpage
 *
 * \author flo
 *
 * \date April 2007 - August 2015
 *
 * Albion Remake
 */

//
static const float3 cam_speed(2.0f, 5.0f, 0.2f);
static vector<light*> debug_lights;

static map_handler* mh { nullptr };
static bool done { false };

int main(int argc, char *argv[]) {
	// initialize the engine
#if !defined(FLOOR_IOS)
	engine::init(argv[0], (const char*)"../data/");
#else
	engine::init(argv[0], (const char*)"data/");
#endif
	floor::set_caption(APPLICATION_NAME);
	floor::acquire_context();
	
	conf::init();
	xld::set_xld_path(floor::data_path("xld/"));
	
	clck = new ar_clock();

	// init class pointers
	eevt = floor::get_event();
	t = engine::get_texman();
	exts = engine::get_ext();
	s = engine::get_shader();
	sce = engine::get_scene();
	ui = engine::get_gui();
	fm = ui->get_font_manager();
	
	// initialize the camera
	cam = new camera();
	cam->set_position(0.0f, -80.0f, 0.0f);
	cam->set_rotation(0.0f, 90.0f+45.0f, 0.0f);
	cam->set_rotation_speed(300.0f);
	cam->set_mouse_input(false);
	cam->set_keyboard_input(true);
	cam->set_wasd_input(true);
	cam->set_cam_speed(cam_speed.x);

	// initialize the scene
	sce->set_eye_distance(-0.3f);
	
	// add shaders
	const string ar_shaders[][2] {
		{ "AR_IR_GBUFFER_MAP_OBJECTS", "inferred/gp_gbuffer_map_objects.a2eshd" },
		{ "AR_IR_GBUFFER_MAP_TILES", "inferred/gp_gbuffer_map_tiles.a2eshd" },
		{ "AR_IR_MP_SKY", "inferred/mp_sky.a2eshd" },
		{ "AR_IR_MP_MAP_OBJECTS", "inferred/mp_map_objects.a2eshd" },
		{ "AR_IR_MP_MAP_TILES", "inferred/mp_map_tiles.a2eshd" },
	};
	for(const auto& shd : ar_shaders) {
		if(!s->add_a2e_shader(shd[0], shd[1])) {
			log_error("couldn't add a2e-shader \"%s\"!", shd[1]);
			done = true;
		}
	}
	
	// add event handlers
	event::handler key_handler_fnctr(&key_handler);
	eevt->add_event_handler(key_handler_fnctr, EVENT_TYPE::KEY_DOWN, EVENT_TYPE::KEY_UP);
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
	script = new albion_script();
	audio_handler::init();

	mh = new map_handler();
	//size_t inital_map_num = 99; // shortcut map
	size_t inital_map_num = 181; // beloveno
	if(argc > 1) {
		const string arg_1_str = argv[1];
		if(arg_1_str.size() > 0 && isdigit(arg_1_str[0])) {
			const size_t arg_map_num = stosize(arg_1_str);
			if((arg_map_num == 0 && arg_1_str[0] == '0') ||
			   (arg_map_num != 0 && mh->get_map_type(arg_map_num) != MAP_TYPE::NONE)) {
				inital_map_num = arg_map_num;
			}
			else {
				log_error("invalid map number or map type!");
			}
		}
	}
	mh->load_map(inital_map_num);

	aui = new albion_ui(mh);
	aui->open_game_ui();
	
	cam->set_keyboard_input(conf::get<bool>("debug.free_cam"));
	
	// for debugging purposes
	ar_debug* debug_ui = new ar_debug();
	
	floor::release_context();
	
	// main loop
	while(!done) {
		// event handling
		eevt->handle_events();
		
#if !defined(FLOOR_IOS)
		// stop drawing if window is inactive
		if(!(SDL_GetWindowFlags(floor::get_window()) & SDL_WINDOW_INPUT_FOCUS)) {
			SDL_Delay(20);
			continue;
		}
#endif
		
		debug_ui->run();

		engine::start_draw();
		
		// set caption (app name and fps count)
		if(floor::is_new_fps_count()) {
#if defined(FLOOR_IOS)
			cout << "FPS: " << floor::get_fps() << endl;
#endif
			stringstream caption;
			caption << APPLICATION_NAME << " | FPS: " << floor::get_fps();
			if(mh->get_tile(0) != nullptr) {
				caption << " | ";
				
				stringstream tbytes;
				string byte_strs[6];
				tbytes.setf(ios::hex, ios::basefield);
				
				tbytes << mh->get_tile(0)->upper_bytes;
				byte_strs[0] = tbytes.str();
				tbytes.str("");
				tbytes << mh->get_tile(0)->lower_bytes;
				byte_strs[1] = tbytes.str();
				tbytes.str("");
				tbytes << mh->get_tile(1)->upper_bytes;
				byte_strs[2] = tbytes.str();
				tbytes.str("");
				tbytes << mh->get_tile(1)->lower_bytes;
				byte_strs[3] = tbytes.str();
				tbytes.str("");
				tbytes << mh->get_tile_num(0);
				byte_strs[4] = tbytes.str();
				tbytes.str("");
				tbytes << mh->get_tile_num(1);
				byte_strs[5] = tbytes.str();
				tbytes.str("");

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

			if(mh->get_active_map_type() == MAP_TYPE::MAP_3D) {
				ssize3 tile_info = mh->get_3d_tile();
				if(tile_info.min_element() >= 0) {
					caption << " | " << tile_info.x << " " << tile_info.y << " " << tile_info.z;
				}
				caption << " | Pos: " << (float3(-*engine::get_position())/std_tile_size).floored();
				caption << " | Cam: " << float3(-*engine::get_position());
			}
			else if(mh->get_active_map_type() == MAP_TYPE::MAP_2D) {
				caption << " | Pos: " << mh->get_player_position();
			}

			floor::set_caption(caption.str());
		}

		clck->run();
		mh->handle();
		aui->run();
		
		engine::stop_draw();
	}
	
	eevt->remove_event_handler(key_handler_fnctr);
	eevt->remove_event_handler(mouse_handler_fnctr);
	eevt->remove_event_handler(quit_handler_fnctr);
	eevt->remove_event_handler(window_handler_fnctr);
	
	delete debug_ui;
	
	audio_handler::destroy();
	delete palettes;
	delete bin_gfx;
	delete script;

	delete aui;
	delete cam;
	
	engine::destroy();

	return 0;
}


bool key_handler(EVENT_TYPE type, shared_ptr<event_object> obj) {
	// cast correctly
	if(type == EVENT_TYPE::KEY_DOWN) {
		const shared_ptr<key_down_event>& key_evt = (shared_ptr<key_down_event>&)obj;
		switch(key_evt->key) {
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				cam->set_cam_speed(cam_speed.y);
				break;
			case SDLK_LCTRL:
			case SDLK_RCTRL:
				cam->set_cam_speed(cam_speed.z);
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
				cam->set_cam_speed(cam_speed.x);
				break;
			case SDLK_LCTRL:
			case SDLK_RCTRL:
				cam->set_cam_speed(cam_speed.x);
				break;
			case SDLK_ESCAPE:
				done = true;
				break;
			case SDLK_g:
				conf::set<bool>("ui.display", conf::get<bool>("ui.display") ^ true);
				floor::acquire_context();
				if(conf::get<bool>("ui.display")) {
					aui->open_goto_map_wnd();
					//aui->open_game_ui();
				}
				else {
					aui->close_goto_map_wnd();
					//aui->close_game_ui();
				}
				floor::release_context();
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
			case SDLK_q: {
				const bool new_debug_mode = conf::get<bool>("debug.show_texture") ^ true;
				conf::set<bool>("debug.ui", new_debug_mode);
				conf::set<bool>("debug.show_texture", new_debug_mode);
			}
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
				conf::set<size_t>("debug.texture", sce->get_geometry_buffer(0)->tex[0]);
				break;
			case SDLK_2:
				conf::set<size_t>("debug.texture", sce->get_light_buffer(0)->tex[0]);
				break;
			case SDLK_3:
				conf::set<size_t>("debug.texture", sce->get_light_buffer(0)->tex[1]);
				break;
			case SDLK_4:
				conf::set<size_t>("debug.texture", sce->get_scene_buffer()->tex[0]);
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
				engine::reload_shaders();
				break;
			case SDLK_F19:
			case SDLK_0:
				floor::reload_kernels();
				break;
			case SDLK_F7:
				floor::start_draw();
				mh->load_map(99);
				floor::stop_draw();
				break;
			default:
				return false;
		}
	}
	return true;
}

bool mouse_handler(EVENT_TYPE type, shared_ptr<event_object> obj floor_unused) {
	if(type == EVENT_TYPE::MOUSE_RIGHT_CLICK) {
		const bool cur_state = cam->get_mouse_input();
		cam->set_mouse_input(cur_state ^ true);
		floor::set_cursor_visible(cur_state);
	}
	return true;
}

bool quit_handler(EVENT_TYPE type floor_unused, shared_ptr<event_object> obj floor_unused) {
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
