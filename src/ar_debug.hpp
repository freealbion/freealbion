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

#ifndef __AR_DEBUG_HPP__
#define __AR_DEBUG_HPP__

#include "ar_global.hpp"
#include <gui/gui.hpp>

#if defined(__APPLE__)
// for debugging/optimization purposes:
class gpu_perf_info : public thread_base {
public:
	gpu_perf_info() : thread_base("gpu_perf") {
		set_thread_delay(5);
		start();
	}
	virtual void run();
	
	unsigned int get_core_util() const { return core_util; }
	unsigned int get_mem_util() const { return mem_util; }
	unsigned int get_vram_free() const { return vram_free; }
	unsigned int get_vram_used() const { return vram_used; }
	unsigned int get_vram_total() const { return vram_free+vram_used; }
	void set_should_run(const bool state) {
		should_run.store(state);
	}
	
protected:
	unsigned int core_util = 0;
	unsigned int mem_util = 0;
	unsigned int vram_free = 0;
	unsigned int vram_used = 0;
	atomic<bool> should_run { false };
};
#endif

class font;
class ar_debug {
public:
	ar_debug();
	~ar_debug();
	
	void run();
	
protected:
	ui_draw_callback draw_cb;
	gui_simple_callback* cb_obj = nullptr;
	void draw_ui(const DRAW_MODE_UI draw_mode, rtt::fbo* buffer);
	
	font* fnt = nullptr;
	
#if defined(__APPLE__)
	gpu_perf_info* perf_info = nullptr;
#endif
	
};

#endif
