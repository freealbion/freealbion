/*
 *  Albion Remake
 *  Copyright (C) 2007 - 2014 Florian Ziesche
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
#include <a2e.hpp>

/*!
 * \mainpage
 *
 * \author flo
 *
 * \date April 2007 - December 2013
 *
 * Albion Remake
 */

//
static const float3 cam_speed(2.0f, 5.0f, 0.2f);
static a2estatic* debug_model = nullptr;
static a2ematerial* debug_mat = nullptr;
static vector<light*> debug_lights;

int main(int argc, char *argv[]) {
	// initialize the engine
#if !defined(A2E_IOS) || !defined(A2E_DEBUG)
	e = new engine(argv[0], (const char*)"../data/");
#else
	e = new engine(argv[0], (const char*)"../../../../Documents/albion/data/");
#endif
	e->init();
	e->set_caption(APPLICATION_NAME);
	e->acquire_gl_context();
	
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
	fm = ui->get_font_manager();
	
	// initialize the camera
	cam = new camera(e);
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

	mh = new map_handler();
	//size_t inital_map_num = 99; // shortcut map
	size_t inital_map_num = 183; // shortcut map
	if(argc > 1) {
		const string arg_1_str = argv[1];
		if(arg_1_str != "") {
			if(arg_1_str.find(".a2m") == string::npos) {
				const size_t arg_map_num = string2size_t(arg_1_str);
				if((arg_map_num == 0 && arg_1_str[0] == '0') ||
				   (arg_map_num != 0 && mh->get_map_type(arg_map_num) != MT_NONE)) {
					inital_map_num = arg_map_num;
				}
				else {
					a2e_error("invalid map number or map type!");
				}
			}
			else {
				const string model_name = arg_1_str.substr(0, arg_1_str.rfind(".a2m"));
				const string mtl_name = model_name + ".a2mtl";
				debug_mat = new a2ematerial(e);
				debug_mat->load_material(e->data_path(mtl_name));
				debug_model = sce->create_a2emodel<a2estatic>();
				debug_model->load_model(e->data_path(arg_1_str));
				debug_model->set_material(debug_mat);
				debug_model->set_hard_scale(100.0f, 100.0f, 100.0f);
				extbbox* bbox = debug_model->get_bounding_box();
				const float pos_offset = -(bbox->min.y + (bbox->max.y - bbox->min.y) * 0.5f);
				debug_model->set_position(0.0f, pos_offset, 0.0f);
				sce->add_model(debug_model);
				
				debug_lights = {
					new light(bbox->min.x, pos_offset, 0.0f),
					new light(bbox->max.x, pos_offset, 0.0f),
					new light(0.0f, bbox->min.y + pos_offset, 0.0f),
					new light(0.0f, bbox->max.y + pos_offset, 0.0f),
					new light(0.0f, pos_offset, bbox->min.z),
					new light(0.0f, pos_offset, bbox->max.z),
				};
				const float light_radius = bbox->diagonal().length() * 3.0f;
				for(const auto& li : debug_lights) {
					li->set_color(core::rand(1.0f), core::rand(1.0f), core::rand(1.0f));
					li->set_radius(light_radius);
					sce->add_light(li);
				}
				
				light* directional_light = new light(0.0f, bbox->max.y + pos_offset + 20.0f, 0.0f);
				directional_light->set_color(1.0f, 1.0f, 1.0f);
				directional_light->set_ambient(0.1f, 0.1f, 0.1f);
				directional_light->set_type(light::LIGHT_TYPE::DIRECTIONAL);
				sce->add_light(directional_light);
				debug_lights.push_back(directional_light);
				
				cam->set_position(0.0f, 0.0f, bbox->min.z * 4.0f);
				cam->set_rotation(0.0f, 0.0f, 0.0f);
			}
		}
	}
	if(debug_model == nullptr) mh->load_map(inital_map_num);

	aui = new albion_ui(mh);
	if(conf::get<bool>("ui.display")) {
		aui->open_goto_map_wnd();
		aui->open_game_ui();
	}
	
	cam->set_keyboard_input(conf::get<bool>("debug.free_cam"));
	
	// for debugging purposes
	ar_debug* debug_ui = new ar_debug();
	
	e->release_gl_context();
	
	// main loop
	while(!done) {
		// event handling
		eevt->handle_events();
		
		// stop drawing if window is inactive
		if(!(SDL_GetWindowFlags(e->get_window()) & SDL_WINDOW_INPUT_FOCUS)) {
			SDL_Delay(20);
			continue;
		}
		
		debug_ui->run();
		
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
				core::reset(tbytes);
				tbytes << mh->get_tile(0)->lower_bytes;
				byte_strs[1] = tbytes.str();
				core::reset(tbytes);
				tbytes << mh->get_tile(1)->upper_bytes;
				byte_strs[2] = tbytes.str();
				core::reset(tbytes);
				tbytes << mh->get_tile(1)->lower_bytes;
				byte_strs[3] = tbytes.str();
				core::reset(tbytes);
				tbytes << mh->get_tile_num(0);
				byte_strs[4] = tbytes.str();
				core::reset(tbytes);
				tbytes << mh->get_tile_num(1);
				byte_strs[5] = tbytes.str();
				core::reset(tbytes);

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
			core::reset(caption);
		}

		e->start_draw();

		clck->run();
		mh->handle();
		aui->run();

		e->stop_draw();
	}
	
	eevt->remove_event_handler(key_handler_fnctr);
	eevt->remove_event_handler(mouse_handler_fnctr);
	eevt->remove_event_handler(quit_handler_fnctr);
	eevt->remove_event_handler(window_handler_fnctr);
	
	if(debug_model != nullptr) {
		sce->delete_model(debug_model);
		delete debug_model;
	}
	if(debug_mat != nullptr) delete debug_mat;
	
	delete debug_ui;

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

bool mouse_handler(EVENT_TYPE type, shared_ptr<event_object> obj a2e_unused) {
	if(type == EVENT_TYPE::MOUSE_RIGHT_CLICK) {
		const bool cur_state = cam->get_mouse_input();
		cam->set_mouse_input(cur_state ^ true);
		e->set_cursor_visible(cur_state);
	}
	return true;
}

bool quit_handler(EVENT_TYPE type a2e_unused, shared_ptr<event_object> obj a2e_unused) {
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
