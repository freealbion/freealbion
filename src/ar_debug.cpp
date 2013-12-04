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

#include "ar_debug.h"
#include "conf.h"
#include <rendering/gfx2d.hpp>
#include <gui/font.hpp>
#include <gui/font_manager.hpp>
#include <rendering/gl_timer.hpp>
#include <atomic>

#if defined(__APPLE__)
void gpu_perf_info::run() {
	if(!should_run) return;
	const auto extract_data = [](const string& input, const string& search_token, bool trim_0) -> unsigned int {
		const size_t len(search_token.size());
		const size_t pos(input.find(search_token));
		if(pos == string::npos) return 0;
		const string ext_str_0(input.substr(pos+len, input.find("\n", pos+len) - pos - len));
		if(trim_0) return string2uint(ext_str_0.substr(0, ext_str_0.size() - 7)); // trim '0000000'
		return string2uint(ext_str_0);
	};
	
	//
	string output = "";
	core::system("ioreg -p IOService -c NVKernel -r | grep 'PerformanceStatistics' | tr ',' '\n'", output);
	core_util = extract_data(output, "\"GPU Core Utilization\"=", true);
	mem_util = extract_data(output, "\"GPU Memory Utilization\"=", true);
	vram_used = extract_data(output, "\"vramUsedBytes\"=", false);
	vram_free = extract_data(output, "\"vramFreeBytes\"=", false);
}
#endif

//
ar_debug::ar_debug() :
draw_cb(this, &ar_debug::draw_ui)
{
	fnt = fm->get_font("SYSTEM_SANS_SERIF");
#if defined(__APPLE__)
	perf_info = new gpu_perf_info();
#endif
}

ar_debug::~ar_debug() {
	if(cb_obj != nullptr) {
		cb_obj = nullptr;
		ui->delete_draw_callback(draw_cb);
	}
#if defined(__APPLE__)
	delete perf_info;
#endif
}

void ar_debug::run() {
	if(conf::get<bool>("debug.ui")) {
		if(cb_obj == nullptr) {
			e->acquire_gl_context();
			cb_obj = ui->add_draw_callback(DRAW_MODE_UI::POST_UI, draw_cb, float2(1.0f), float2(0.0f), gui_surface::SURFACE_FLAGS::NO_ANTI_ALIASING);
			e->release_gl_context();
		}
		cb_obj->redraw();
	}
	else if(cb_obj != nullptr) {
		e->acquire_gl_context();
		ui->delete_draw_callback(draw_cb);
		e->release_gl_context();
		cb_obj = nullptr;
	}
}

void ar_debug::draw_ui(const DRAW_MODE_UI draw_mode a2e_unused, rtt::fbo* buffer) {
	const uint2 size(buffer->width, buffer->height);
#if defined(__APPLE__)
	if(!conf::get<bool>("debug.osx")) {
		perf_info->set_should_run(false);
	}
	else {
		perf_info->set_should_run(true);
		
		const uint2 bar_extents(250, 16);
		uint2 offset(10, size.y - 32 - (bar_extents.y + 2) * 3);
		const auto draw_bar = [&offset, &bar_extents](const unsigned y_offset, const unsigned int value, const unsigned int max_value) {
			const uint2 inset(2);
			gfx2d::draw_rectangle_fill(rect(offset.x+1, offset.y+1 + y_offset,
											offset.x+1 + bar_extents.x, offset.y+1 + bar_extents.y + y_offset),
									   float4(1.0f));
			gfx2d::draw_rectangle_fill(rect(offset.x, offset.y + y_offset,
											offset.x + bar_extents.x, offset.y + bar_extents.y + y_offset),
									   float4(0.0f, 0.0f, 0.0f, 1.0f));
			gfx2d::draw_rectangle_fill(rect(offset.x + inset.x, offset.y + inset.y + y_offset,
											offset.x + inset.x + float(bar_extents.x - inset.x*2) * (float(value) / float(max_value)),
											offset.y + bar_extents.y - inset.y + y_offset),
									   float4(1.0f));
		};
		
		draw_bar(0, perf_info->get_core_util(), 100);
		draw_bar(bar_extents.y + 2, perf_info->get_mem_util(), 100);
		draw_bar((bar_extents.y + 2) * 2, perf_info->get_vram_used(), perf_info->get_vram_total());
	}
#endif
	
	if(conf::get<bool>("debug.timer")) {
		const auto frame = gl_timer::get_last_available_frame();
		if(frame != nullptr) {
			const unsigned long long int time_start = frame->queries[0].time, time_end = frame->queries[frame->queries.size() - 1].time, time_len = time_end - time_start;
			const float ftime_len = 1.0f / float(time_len);
			uint2 offset(10, 10);
			const uint2 inner_offset(1, 1);
			const unsigned int inner_bar_height = fnt->get_display_size() + inner_offset.y * 2;
			const unsigned int outer_bar_height = inner_bar_height + inner_offset.y * 2;
			unsigned int qry_offset = 0;
			for(size_t i = 1; i < frame->queries.size(); i++) {
				const auto& qry(frame->queries[i]);
				const float ratio = float(qry.time - frame->queries[i-1].time) * ftime_len;
				
				const unsigned int x_len = floorf(ratio * float(size.x - (offset.x + inner_offset.x) * 2)) - 2;
				const rect qry_rect(offset.x + inner_offset.x + qry_offset,
									offset.y + inner_offset.y,
									offset.x + inner_offset.x + qry_offset + x_len,
									offset.y + inner_offset.y + inner_bar_height);
				gfx2d::draw_rectangle_fill(qry_rect, float4(qry.color, 1.0f));
				const float2 font_offset(float(qry_rect.x1), float(offset.y) - float(fnt->get_display_size()) * 0.1f);
				
				const float ftime = float((qry.time - frame->queries[i-1].time) / 100000) / 10.0f;
				const string ftime_str = float2string(ftime);
				const string text(qry.identifier+" ("+ftime_str.substr(0, ftime_str.find(".")+2)+")");
				fnt->draw(text, font_offset + 1.0f, float4(1.0f));
				fnt->draw(text, font_offset, float4(0.0f, 0.0f, 0.0f, 1.0f));
				
				offset.y += fnt->get_display_size();
			}
			const float ftotal_time = float(time_len / 100000) / 10.0f;
			const string ftotal_time_str = float2string(ftotal_time);
			const string total_time_text("Total: "+ftotal_time_str.substr(0, ftotal_time_str.find(".")+2)+"ms");
			fnt->draw(total_time_text, float2(offset.x, offset.y + outer_bar_height), float4(1.0f));
		}
	}
	
	// misc
	if(conf::get<bool>("debug.fps")) {
		const string fps_str("FPS: "+uint2string(e->get_fps()));
		const float2 fps_pos(10.0f, e->get_height() - fnt->get_display_size() - 10);
		fnt->draw(fps_str, fps_pos + 1.0f, float4(0.0f, 0.0f, 0.0f, 1.0f));
		fnt->draw(fps_str, fps_pos);
	}
	
	if(conf::get<bool>("debug.show_texture")) {
		const GLuint debug_tex_num = (GLuint)conf::get<size_t>("debug.texture");
		if(debug_tex_num == 0) return;
		if(!glIsTexture(debug_tex_num)) return;
		
		gfx2d::draw_rectangle_texture(rect(0, 0, e->get_width(), e->get_height()),
									  debug_tex_num,
									  float4(1.0f, 1.0f, 1.0f, 0.0f),
									  float4(0.0f, 0.0f, 0.0f, 1.0f),
									  coord(0.0f, 1.0f),
									  coord(1.0f, 0.0f));
	}
}
