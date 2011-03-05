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

#include "main.h"

/*!
 * \mainpage
 *
 * \author flo
 *
 * \date April 2007 - March 2011
 *
 * Albion Remake
 */

int main(int argc, char *argv[]) {
	// initialize the engine
	e = new engine(argv[0], (const char*)"../data/");
	e->init();
	e->set_caption(APPLICATION_NAME);
	
	conf::init();
	xld::set_xld_path(e->data_path("xld/"));
	
	clck = new ar_clock();

	// init class pointers
	c = e->get_core();
	fio = e->get_file_io();
	evt = e->get_event();
	egfx = e->get_gfx();
	t = e->get_texman();
	ocl = e->get_opencl();
	exts = e->get_ext();
	
	conf::add<a2e_texture>("debug.texture", t->get_dummy_texture());
	
	s = new shader(e);
	sce = new scene(e, s);
	cam = new camera(e);
	egui = new gui(e, s);
	eui = new a2eui(e, egui);
	gs = egui->get_gui_style();

	// initialize the a2e events
	evt->init(sevent);

	// initialize the gui
	egui->init();

	// initialize the camera
	cam->set_position(0.0f, -80.0f, 0.0f);
	cam->set_rotation(0.0f, 90.0f+45.0f, 0.0f);
	cam->set_mouse_input(false);
	cam->set_rotation_speed(300.0f);
	cam->set_cam_speed(5.0f);

	// initialize the scene
	sce->set_eye_distance(-0.3f);
	
	// add map objects shader
	map<string, string> ar_shaders;
	ar_shaders["AR_DR_GBUFFER_MAP_OBJECTS"] = "deferred/gbuffer_map_objects.a2eshd";
	ar_shaders["AR_DR_GBUFFER_MAP_TILES"] = "deferred/gbuffer_map_tiles.a2eshd";
	ar_shaders["AR_DR_GBUFFER_SKY"] = "deferred/gbuffer_sky.a2eshd";
	ar_shaders["AR_IR_GBUFFER_MAP_OBJECTS"] = "inferred/gp_gbuffer_map_objects.a2eshd";
	ar_shaders["AR_IR_GBUFFER_MAP_TILES"] = "inferred/gp_gbuffer_map_tiles.a2eshd";
	ar_shaders["AR_IR_GBUFFER_SKY"] = "inferred/gp_gbuffer_sky.a2eshd";
	ar_shaders["AR_IR_MP_SKY"] = "inferred/mp_sky.a2eshd";
	ar_shaders["AR_IR_MP_MAP_OBJECTS"] = "inferred/mp_map_objects.a2eshd";
	ar_shaders["AR_IR_MP_MAP_TILES"] = "inferred/mp_map_tiles.a2eshd";

	a2e_shader* a2e_shd = s->get_a2e_shader();
	for(map<string, string>::const_iterator ars_iter = ar_shaders.begin(); ars_iter != ar_shaders.end(); ars_iter++) {
		a2e_shd->add_a2e_shader(ars_iter->first);
		if(!a2e_shd->load_a2e_shader(ars_iter->first, e->shader_path(ars_iter->second.c_str()), a2e_shd->get_a2e_shader(ars_iter->first))) {
			a2e_error("couldn't load a2e-shader \"%s\"!", ars_iter->second);
			done = true;
		}
		else {
			if(!a2e_shd->preprocess_and_compile_a2e_shader(a2e_shd->get_a2e_shader(ars_iter->first))) {
				a2e_error("couldn't preprocess and/or compile a2e-shader \"%s\"!", ars_iter->second);
				done = true;
			}
		}
	}

	// load/init stuff
	palettes = new pal();
	scaling::init();

	mh = new map_handler();
	//mh->load_map(42);
	//mh->load_map(10);
	mh->load_map(50);
	//mh->load_map(11);
	//mh->load_map(22);
	//mh->load_map(183);
	
	aui = new albion_ui(mh);
	aui->open_goto_map_wnd();

	// dbg img
	image* img = new image(e);
	img->set_scaling(true);
	
	// debug window
	a2e_debug_wnd::init(e, eui, s, cam);

	//transtb* ttb = new transtb();
	
	while(!done) {
		/*static float gscale = conf::get<float>("global.scale");
		static float scale_dir = 1.0f;
		gscale += 0.01f * scale_dir;
		if(gscale >= 4.0f || gscale <= 1.0f) scale_dir *= -1.0f;
		conf::set<float>("global.scale", gscale);*/
		
		while(evt->is_event()) {
			evt->handle_events(evt->get_event().type);
			switch(evt->get_event().type) {
				case SDL_QUIT:
					done = true;
					break;
				case SDL_KEYDOWN:
					switch(evt->get_event().key.keysym.sym) {
						case SDLK_ESCAPE:
							done = true;
							break;
						case SDLK_CARET:
							if(!a2e_debug_wnd::is_open()) a2e_debug_wnd::open();
							else a2e_debug_wnd::close();
							break;
						case SDLK_g:
							conf::set<bool>("ui.display", conf::get<bool>("ui.display") ^ true);
							if(conf::get<bool>("ui.display")) aui->open_goto_map_wnd();
							else aui->close_goto_map_wnd();
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
							img->set_texture((a2e_texture&)conf::get<a2e_texture>("debug.texture"));
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
						case SDLK_LSHIFT:
						case SDLK_RSHIFT:
							cam->set_cam_speed(0.2f);
							break;
						case SDLK_LEFT:
						case SDLK_a:
							evt->set_key_left(true);
							mh->get_next_dir() |= MD_LEFT;
							break;
						case SDLK_RIGHT:
						case SDLK_d:
							evt->set_key_right(true);
							mh->get_next_dir() |= MD_RIGHT;
							break;
						case SDLK_UP:
						case SDLK_w:
							evt->set_key_up(true);
							mh->get_next_dir() |= MD_UP;
							break;
						case SDLK_DOWN:
						case SDLK_s:
							evt->set_key_down(true);
							mh->get_next_dir() |= MD_DOWN;
							break;
						default:
						break;
					}
					break;
				case SDL_KEYUP:
					switch(evt->get_event().key.keysym.sym) {
						case SDLK_LSHIFT:
						case SDLK_RSHIFT:
							cam->set_cam_speed(5.0f);
							break;
						case SDLK_LEFT:
						case SDLK_a:
							evt->set_key_left(false);
							mh->get_next_dir() ^= MD_LEFT;
							break;
						case SDLK_RIGHT:
						case SDLK_d:
							evt->set_key_right(false);
							mh->get_next_dir() ^= MD_RIGHT;
							break;
						case SDLK_UP:
						case SDLK_w:
							evt->set_key_up(false);
							mh->get_next_dir() ^= MD_UP;
							break;
						case SDLK_DOWN:
						case SDLK_s:
							evt->set_key_down(false);
							mh->get_next_dir() ^= MD_DOWN;
							break;
						default:
							break;
					}
					break;
				default:
				break;
			}
		}
		
		// stop drawing if window is inactive
		if(!(SDL_GetWindowFlags(e->get_window()) & SDL_WINDOW_INPUT_FOCUS)) {
			SDL_Delay(20);
			continue;
		}
		
		// set caption (app name and fps count)
		if(e->is_new_fps_count()) {
			caption << APPLICATION_NAME << " | FPS: " << e->get_fps();
			if(mh->get_tile(0) != NULL) {
				caption << " | ";
				
				stringstream tbytes;
				string byte_strs[6];
				tbytes.setf(ios::hex, ios::basefield);
				
				tbytes << mh->get_tile(0)->upper_bytes;
				byte_strs[0] = tbytes.str();
				c->reset(&tbytes);
				tbytes << mh->get_tile(0)->lower_bytes;
				byte_strs[1] = tbytes.str();
				c->reset(&tbytes);
				tbytes << mh->get_tile(1)->upper_bytes;
				byte_strs[2] = tbytes.str();
				c->reset(&tbytes);
				tbytes << mh->get_tile(1)->lower_bytes;
				byte_strs[3] = tbytes.str();
				c->reset(&tbytes);
				tbytes << mh->get_tile_num(0);
				byte_strs[4] = tbytes.str();
				c->reset(&tbytes);
				tbytes << mh->get_tile_num(1);
				byte_strs[5] = tbytes.str();
				c->reset(&tbytes);

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
				caption << " | Pos: " << -float3(*e->get_position());
			}
			else if(mh->get_active_map_type() == MT_2D_MAP) {
				caption << " | Pos: " << mh->get_player_position();
			}

			e->set_caption(caption.str().c_str());
			c->reset(&caption);
		}

		clck->run();
		mh->handle();

		e->start_draw();
		egui->handle_gui();

		mh->draw();
		
		if(conf::get<bool>("debug.display_debug_texture") && img->get_width() > 0 && img->get_height() > 0) {
			e->start_2d_draw();
			float ratio = img->get_width()/img->get_height();
			//float2 screen_size = float2(e->get_width(), e->get_height());
			//(ratio >= 1.0f) ? img->draw(screen_size.x, screen_size.y*(1.0f/ratio)) : img->draw(screen_size.x*ratio, screen_size.y);
			(ratio >= 1.0f) ? img->draw(720.0f, 720.0f*(1.0f/ratio)) : img->draw(720.0f*ratio, 720.0f);
			e->stop_2d_draw();
		}
		
		//if(conf::get<bool>("ui.display")) egui->draw();
		egui->draw();

		e->stop_draw();
	}
	
	a2e_debug_wnd::close();

	//delete ttb;

	delete palettes;

	delete img;
	delete aui;
	delete eui;
	delete egui;
	delete sce;
	delete cam;
	delete s;
	delete e;

	return 0;
}
