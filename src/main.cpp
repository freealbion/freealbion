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

#include "main.h"

/*!
 * \mainpage
 *
 * \author flo
 *
 * \date April 2007 - October 2010
 *
 * Albion Remake
 */

int main(int argc, char *argv[]) {
	// initialize the engine
	e = new engine(argv[0], (const char*)"../../data/");
	e->init();
	e->set_caption(APPLICATION_NAME);
	
	conf::init();
	//xld::set_xld_path("/Users/flo/xld/");
	xld::set_xld_path("C:/xld/");

	// init class pointers
	c = e->get_core();
	fio = e->get_file_io();
	evt = e->get_event();
	egfx = e->get_gfx();
	t = e->get_texman();
	ocl = e->get_opencl();
	
	s = new shader(e);
	egui = new gui(e, s);
	eui = new a2eui(e, egui);
	gs = egui->get_gui_style();

	// initialize the a2e events
	evt->init(sevent);

	// initialize the gui
	egui->init();

	// load/init stuff
	scaling::init();
	pal* palettes = new pal();
	gfxconv::init(palettes);

	mh = new map_handler(palettes);
	mh->load_map(42);
	
	aui = new albion_ui(mh);
	aui->open_goto_map_wnd();

	// dbg img
	image* img = new image(e);
	img->set_scaling(true);
	img->set_texture((a2e_texture&)mh->get_npc_graphics()->get_npcgfx(202));
	
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
						case SDLK_g:
							conf::set<bool>("ui.display", conf::get<bool>("ui.display") ^ true);
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
				string byte_strs[4];
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
				
				for(size_t i = 0; i < 4; i++) {
					size_t add_zeros = 8 - byte_strs[i].length();
					for(size_t j = 0; j < add_zeros; j++) {
						byte_strs[i] = "0"+byte_strs[i];
					}
				}
				
				caption << byte_strs[0] << " " << byte_strs[1] << " | ";
				caption << byte_strs[2] << " " << byte_strs[3] << " |";
			}
			caption << " (" << mh->get_player_position().x << ", " << mh->get_player_position().y << ")";
			if(mh->get_tile(0) != NULL) caption << " | c: " << (size_t)mh->get_tile(0)->collision << " " << (size_t)mh->get_tile(1)->collision;
			e->set_caption(caption.str().c_str());
			c->reset(&caption);
		}

		mh->handle();

		e->start_draw();
		egui->handle_gui();

		mh->draw();
		
		/*e->start_2d_draw();
		img->draw();
		e->stop_2d_draw();*/
		
		if(conf::get<bool>("ui.display")) egui->draw();

		e->stop_draw();
	}

	delete palettes;

	delete img;
	delete aui;
	delete eui;
	delete egui;
	delete s;
	delete e;

	return 0;
}
