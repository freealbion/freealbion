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

#include "map/2d/npcgfx.hpp"
#include <rendering/gfx2d.hpp>

/*! npcgfx constructor
 */
npcgfx::npcgfx(const pal* palettes_) : palettes(palettes_), cur_palette(6),
npcgfx_xlds({{
	lazy_xld("NPCGR0.XLD"), lazy_xld("NPCGR1.XLD"),
	lazy_xld("PARTGR0.XLD"), lazy_xld("TACTICO0.XLD"),
	lazy_xld("NPCKL0.XLD"), lazy_xld("PARTKL0.XLD") }})
{}

/*! npcgfx destructor
 */
npcgfx::~npcgfx() {
	clear();
}

void npcgfx::load_npcgfx(const size_t& npc_num) {
	//
	if(npc_num >= MAX_NPC_NUMBER) {
		log_error("invalid npc number: %u!", npc_num);
		return;
	}
	
	// TODO: check if only npcgfx nums have to be decremented in the sub-100 range
	const auto object_num = (npc_num % 100) - (npc_num < 100 ? 1 : 0);
	const auto npc_data_size = npcgfx_xlds[npc_num/100].get_object_size(object_num);
	if(npc_data_size == 0) {
		log_error("npc data %u is empty!", npc_num);
		return;
	}
	
	auto object_data_ptr = npcgfx_xlds[npc_num/100].get_object_data(object_num);
	const auto object_data = object_data_ptr->data();

	const size2 npc_size = size2(object_data[0], object_data[2]);
	const size_t object_count = npc_data_size / (npc_size.x * npc_size.y);
	const size_t offset = (object_count > 1 ? 6 : 0);

	albion_texture::albion_texture_single_object tex_info_obj;
	tex_info_obj.object_count = (unsigned int)object_count;
	tex_info_obj.data = object_data;
	tex_info_obj.offset = (unsigned int)offset;
	vector<albion_texture::albion_texture_info*> tex_info;
	tex_info.push_back(&tex_info_obj);
	
	size2 texture_size(npc_size.x*4*4, 1024);
	if(npc_num >= 400 && npc_num < 600) {
		texture_size.y = npc_size.y*4*(object_count/4);
	}
	
	npc_graphics[npc_num] = albion_texture::create(MAP_TYPE::MAP_2D, texture_size, npc_size, cur_palette, tex_info, nullptr,
												   albion_texture::TEXTURE_SPACING::NONE, 0, false,
												   // this should never use mip-maps!
												   TEXTURE_FILTERING::LINEAR);
}

const a2e_texture& npcgfx::get_npcgfx(const size_t& npc_num) {
	if(npc_num >= MAX_NPC_NUMBER) {
		log_error("invalid npc graphic #%u!", npc_num);
		//throw exception();
		return get_npcgfx(0);
	}
	
	if(npc_graphics.count(npc_num) == 0) {
		load_npcgfx(npc_num);
	}

	return npc_graphics.find(npc_num)->second;
}

void npcgfx::draw_npc(const size_t& npc_num, const size_t& frame,
					  const float2& screen_position, const float2& position) {
	const float scale = conf::get<float>("map.scale");

	//
	size_t npc_frame = 0;
	switch((NPC_STATE)(frame & 0xF7)) {
		case NPC_STATE::BACK1: npc_frame = 0; break;
		case NPC_STATE::BACK2: npc_frame = 1; break;
		case NPC_STATE::BACK3: npc_frame = 2; break;
		case NPC_STATE::RIGHT1: npc_frame = 3; break;
		case NPC_STATE::RIGHT2: npc_frame = 4; break;
		case NPC_STATE::RIGHT3: npc_frame = 5; break;
		case NPC_STATE::FRONT1: npc_frame = 6; break;
		case NPC_STATE::FRONT2: npc_frame = 7; break;
		case NPC_STATE::FRONT3: npc_frame = 8; break;
		case NPC_STATE::LEFT1: npc_frame = 9; break;
		case NPC_STATE::LEFT2: npc_frame = 10; break;
		case NPC_STATE::LEFT3: npc_frame = 11; break;
		case NPC_STATE::SIT_BACK: npc_frame = 12; break;
		case NPC_STATE::SIT_RIGHT: npc_frame = 13; break;
		case NPC_STATE::SIT_FRONT: npc_frame = 14; break;
		case NPC_STATE::SIT_LEFT: npc_frame = 15; break;
		case NPC_STATE::LAY: npc_frame = 16; break;
	}
	
	const a2e_texture& tex = get_npcgfx(npc_num);
	
	float tx = 0.0f, ty = 0.0f;
	float2 draw_size, tc_size;
	if(npc_num < 400) {
		ty = (float(npc_frame / 4) * 48.0f * 4.0f) / 1024.0f;
		tx = (float(npc_frame % 4) * 32.0f * 4.0f) / 512.0f;
		draw_size = float2(32.0f * scale, 48.0f * scale);
		tc_size = float2(32.0f/128.0f, 48.0f/256.0f);
	}
	else if(npc_num >= 400 && npc_num < 600) {
		if(npc_frame > 11) return; // npckl only has 12 frames
		
		ty = (float(npc_frame / 4) / 3.0f);
		tx = (float(npc_frame % 4) / 4.0f);
		draw_size = float2(float(tex->width)/4.0f * scale, float(tex->height)/3.0f * scale)/4.0f;
		tc_size = float2(1.0f/4.0f, 1.0f/3.0f);
	}

	// offset position a bit (prefer player/party)
	float depth_val = (position.y - (npc_num < 200 ? 0.2f : 0.1f))/255.0f;
	
	const uint2 rect_start { screen_position.floored() };
	const uint2 rect_end { rect_start + uint2 { draw_size.rounded() } };
	gfx2d::draw_rectangle_texture(rect(rect_start.x, rect_start.y,
									   rect_end.x, rect_end.y),
								  tex->tex(),
								  float2(tx, ty),
								  float2(tx + tc_size.x, ty + tc_size.y),
								  depth_val);
}

void npcgfx::clear() {
	npc_graphics.clear();
}

void npcgfx::set_palette(const size_t& palette_num) {
	if(palette_num >= 56) {
		log_error("invalid palette #%u!", palette_num);
		return;
	}
	
	if(cur_palette != palette_num) clear();
	
	cur_palette = palette_num;
}
