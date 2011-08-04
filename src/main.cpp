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
 * \date April 2007 - August 2011
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
	eevt = e->get_event();
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
	eevt->init(sevent);

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
	
	// add shaders
	const string ar_shaders[][2] = {
		{ "AR_IR_GBUFFER_MAP_OBJECTS", "inferred/gp_gbuffer_map_objects.a2eshd" },
		{ "AR_IR_GBUFFER_MAP_TILES", "inferred/gp_gbuffer_map_tiles.a2eshd" },
		{ "AR_IR_GBUFFER_SKY", "inferred/gp_gbuffer_sky.a2eshd" },
		{ "AR_IR_MP_SKY", "inferred/mp_sky.a2eshd" },
		{ "AR_IR_MP_MAP_OBJECTS", "inferred/mp_map_objects.a2eshd" },
		{ "AR_IR_MP_MAP_TILES", "inferred/mp_map_tiles.a2eshd" },
	};
	for(size_t i = 0; i < A2E_ARRAY_LENGTH(ar_shaders); i++) {
		if(!s->add_a2e_shader(ar_shaders[i][0], ar_shaders[i][1])) {
			a2e_error("couldn't add a2e-shader \"%s\"!", ar_shaders[i][1]);
			done = true;
		}
	}

	// load/init stuff
	palettes = new pal();
	scaling::init();
	bin_gfx = new bin_graphics();

	mh = new map_handler();
	//mh->load_map(42);
	//mh->load_map(10);
	//mh->load_map(50);
	//mh->load_map(11);
	//mh->load_map(22);
	mh->load_map(183);
	//mh->load_map(45);
	//mh->load_map(47);
	//mh->load_map(100);

	aui = new albion_ui(mh);
	if(conf::get<bool>("ui.display")) {
		aui->open_goto_map_wnd();
		aui->open_game_ui();
	}

	// dbg img
	image* img = new image(e);
	img->set_scaling(true);
	
	// debug window
	a2e_debug_wnd::init(e, eui, s, ocl, cam);
	//a2e_debug_wnd::open();

	//transtb* ttb = new transtb();
	
	a2e_texture tmp_tex = new texture_object();
	tmp_tex->width = e->get_width();
	tmp_tex->height = e->get_height();
	while(!done) {
		while(eevt->is_event()) {
			eevt->handle_events(eevt->get_event().type);
			switch(eevt->get_event().type) {
				case SDL_QUIT:
					done = true;
					break;
				case SDL_KEYDOWN:
					switch(eevt->get_event().key.keysym.sym) {
						case SDLK_ESCAPE:
							done = true;
							break;
						case SDLK_CARET:
							if(!a2e_debug_wnd::is_open()) a2e_debug_wnd::open();
							else a2e_debug_wnd::close();
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
						case SDLK_1:
							tmp_tex->tex_num = sce->_get_g_buffer()->tex_id[0];
							conf::set<a2e_texture>("debug.texture", tmp_tex);
							break;
						case SDLK_2:
							tmp_tex->tex_num = sce->_get_g_buffer()->tex_id[1];
							conf::set<a2e_texture>("debug.texture", tmp_tex);
							break;
						case SDLK_3:
							tmp_tex->tex_num = sce->_get_l_buffer()->tex_id[0];
							conf::set<a2e_texture>("debug.texture", tmp_tex);
							break;
						case SDLK_4:
							tmp_tex->tex_num = sce->_get_fxaa_buffer()->tex_id[0];
							conf::set<a2e_texture>("debug.texture", tmp_tex);
							break;
						case SDLK_5:
							tmp_tex->tex_num = sce->_get_scene_buffer()->tex_id[0];
							conf::set<a2e_texture>("debug.texture", tmp_tex);
							break;
						case SDLK_v:
							cam->set_cam_input(cam->get_cam_input() ^ true);
							conf::set<bool>("debug.free_cam", cam->get_cam_input());
							break;
						case SDLK_LSHIFT:
						case SDLK_RSHIFT:
							cam->set_cam_speed(0.2f);
							break;
						case SDLK_LEFT:
						case SDLK_a:
							eevt->set_key_left(true);
							mh->get_next_dir() |= MD_LEFT;
							break;
						case SDLK_RIGHT:
						case SDLK_d:
							eevt->set_key_right(true);
							mh->get_next_dir() |= MD_RIGHT;
							break;
						case SDLK_UP:
						case SDLK_w:
							eevt->set_key_up(true);
							mh->get_next_dir() |= MD_UP;
							break;
						case SDLK_DOWN:
						case SDLK_s:
							eevt->set_key_down(true);
							mh->get_next_dir() |= MD_DOWN;
							break;
						default:
						break;
					}
					break;
				case SDL_KEYUP:
					switch(eevt->get_event().key.keysym.sym) {
						case SDLK_LSHIFT:
						case SDLK_RSHIFT:
							cam->set_cam_speed(5.0f);
							break;
						case SDLK_LEFT:
						case SDLK_a:
							eevt->set_key_left(false);
							mh->get_next_dir() ^= MD_LEFT;
							break;
						case SDLK_RIGHT:
						case SDLK_d:
							eevt->set_key_right(false);
							mh->get_next_dir() ^= MD_RIGHT;
							break;
						case SDLK_UP:
						case SDLK_w:
							eevt->set_key_up(false);
							mh->get_next_dir() ^= MD_UP;
							break;
						case SDLK_DOWN:
						case SDLK_s:
							eevt->set_key_down(false);
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
				caption << " | Pos: " << (float3(-*e->get_position())/std_tile_size).floored();
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
		aui->run();

		mh->draw();
		
		if(conf::get<bool>("debug.display_debug_texture") && img->get_width() > 0 && img->get_height() > 0) {
			e->start_2d_draw();
			size_t draw_width = img->get_width(), draw_height = img->get_height();
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
			img->draw((unsigned int)draw_width, (unsigned int)draw_height, true);
			e->stop_2d_draw();
		}
		
		//if(conf::get<bool>("ui.display")) egui->draw();
		egui->draw();

		e->stop_draw();
	}
	tmp_tex->tex_num = 0;
	
	a2e_debug_wnd::close();

	//delete ttb;

	delete palettes;
	delete bin_gfx;

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
