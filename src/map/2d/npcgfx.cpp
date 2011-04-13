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

#include "npcgfx.h"

/*! npcgfx constructor
 */
npcgfx::npcgfx(const pal* palettes) : palettes(palettes), cur_palette(6) {
	// load npc graphics
	npcgfx_xlds[0] = new xld("NPCGR0.XLD");
	npcgfx_xlds[1] = new xld("NPCGR1.XLD");
	npcgfx_xlds[2] = new xld("PARTGR0.XLD");
	npcgfx_xlds[3] = new xld("TACTICO0.XLD");
}

/*! npcgfx destructor
 */
npcgfx::~npcgfx() {
	clear();
	delete npcgfx_xlds[0];
	delete npcgfx_xlds[1];
	delete npcgfx_xlds[2];
	delete npcgfx_xlds[3];
}

void npcgfx::load_npcgfx(const size_t& npc_num) {
	const xld::xld_object* object = NULL;
	if(npc_num < 100 && npc_num <= npcgfx_xlds[0]->get_object_count()) {
		object = npcgfx_xlds[0]->get_object(npc_num-1); // TODO: check if only npcgfx nums have to be decremented in the sub-100 range
	}
	else if(npc_num < 200 && (npc_num - 100) < npcgfx_xlds[1]->get_object_count()) {
		object = npcgfx_xlds[1]->get_object(npc_num - 100);
	}
	else if(npc_num < 300 && (npc_num - 200) < npcgfx_xlds[2]->get_object_count()) {
		object = npcgfx_xlds[2]->get_object(npc_num - 200);
	}
	else if(npc_num < 400 && (npc_num - 300) < npcgfx_xlds[3]->get_object_count()) {
		object = npcgfx_xlds[3]->get_object(npc_num - 300);
	}
	
	if(object == NULL) {
		a2e_error("invalid npc num #%u!", npc_num);
		object = npcgfx_xlds[0]->get_object(0);
	}

	const size2 npc_size = size2(object->data[0], object->data[2]);
	const size_t object_count = object->length / (npc_size.x * npc_size.y);
	const size_t offset = (object_count > 1 ? 6 : 0);

	albion_texture::albion_texture_single_object tex_info_obj;
	tex_info_obj.object_count = object_count;
	tex_info_obj.object = object;
	tex_info_obj.offset = offset;
	vector<albion_texture::albion_texture_info*> tex_info;
	tex_info.push_back(&tex_info_obj);

	npc_graphics[npc_num] = albion_texture::create(MT_2D_MAP, size2(npc_size.x*4*4, 1024), npc_size, cur_palette, tex_info, NULL, texture_object::TF_TRILINEAR);
}

const a2e_texture& npcgfx::get_npcgfx(const size_t& npc_num) {
	if(npc_num > 399) {
		a2e_error("invalid npc graphic #%u!", npc_num);
		//throw exception();
		return get_npcgfx(0);
	}
	
	if(npc_graphics.count(npc_num) == 0) {
		load_npcgfx(npc_num);
	}

	return npc_graphics.find(npc_num)->second;
}

void npcgfx::draw_npc(const size_t& npc_num, const size_t& frame, const float2& screen_position, const float2& position, const float depth_overwrite) {
	const float scale = conf::get<float>("map.scale");

	//
	size_t npc_frame = 0;
	switch(frame & 0xF7) {
		case S_BACK1: npc_frame = 0; break;
		case S_BACK2: npc_frame = 1; break;
		case S_BACK3: npc_frame = 2; break;
		case S_RIGHT1: npc_frame = 3; break;
		case S_RIGHT2: npc_frame = 4; break;
		case S_RIGHT3: npc_frame = 5; break;
		case S_FRONT1: npc_frame = 6; break;
		case S_FRONT2: npc_frame = 7; break;
		case S_FRONT3: npc_frame = 8; break;
		case S_LEFT1: npc_frame = 9; break;
		case S_LEFT2: npc_frame = 10; break;
		case S_LEFT3: npc_frame = 11; break;
		case S_SIT_BACK: npc_frame = 12; break;
		case S_SIT_RIGHT: npc_frame = 13; break;
		case S_SIT_FRONT: npc_frame = 14; break;
		case S_SIT_LEFT: npc_frame = 15; break;
		case S_LAY: npc_frame = 16; break;
		default: npc_frame = 17; break;
	}
	
	float ty = (float(npc_frame / 4) * 48.0f * 4.0f) / 1024.0f;
	float tx = (float(npc_frame % 4) * 32.0f * 4.0f) / 512.0f;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, get_npcgfx(npc_num)->tex());
	glColor3f(1.0f, 1.0f, 1.0f);

	// offset position a bit (prefer player/party)
	float depth_val = (position.y - (npc_num < 200 ? 0.2f : 0.1f))/255.0f;
	// depth overwrite
	if(depth_overwrite != -1.0f) depth_val = depth_overwrite;

	glFrontFace(GL_CCW);
	float2 draw_size = float2(32.0f * scale, 48.0f * scale);
	float2 tc_size = float2(32.0f/128.0f, 48.0f/256.0f);
	glBegin(GL_QUADS);
		glTexCoord2f(tx, ty);
		glVertex3f(screen_position.x, screen_position.y, depth_val);
		glTexCoord2f(tx, ty + tc_size.y);
		glVertex3f(screen_position.x, screen_position.y + draw_size.y, depth_val);
		glTexCoord2f(tx + tc_size.x, ty + tc_size.y);
		glVertex3f(screen_position.x + draw_size.x, screen_position.y + draw_size.y, depth_val);
		glTexCoord2f(tx + tc_size.x, ty);
		glVertex3f(screen_position.x + draw_size.x, screen_position.y, depth_val);
	glEnd();

	// --leave this here for the moment
	/*float3 depth_vals = float3(
		(position.y-2.0f)/255.0f,
		(position.y-1.0f)/255.0f,
		(position.y)/255.0f);
	if(depth_overwrite != -1.0f) depth_vals = float3(depth_overwrite);

	float draw_offset;
	float tex_offset;
	float2 tex_coord;
	float2 draw_size = float2(32.0f * scale, 16.0f * scale);
	for(size_t i = 0; i < 3; i++) {
		draw_offset = float(i) * 16.0f * scale;
		tex_offset = (16.0f*float(i))/256.0f;
		tex_coord.set(32.0f/128.0f, (16.0f*float(i+1))/256.0f);

		glBegin(GL_QUADS);
			glTexCoord2f(tx, ty + tex_offset);
			glVertex3f(screen_position.x, screen_position.y + draw_offset, depth_vals[i]);
			glTexCoord2f(tx, ty + tex_coord.y);
			glVertex3f(screen_position.x, screen_position.y + draw_size.y + draw_offset, depth_vals[i]);
			glTexCoord2f(tx + tex_coord.x, ty + tex_coord.y);
			glVertex3f(screen_position.x + draw_size.x, screen_position.y + draw_size.y + draw_offset, depth_vals[i]);
			glTexCoord2f(tx + tex_coord.x, ty + tex_offset);
			glVertex3f(screen_position.x + draw_size.x, screen_position.y + draw_offset, depth_vals[i]);
		glEnd();
	}*/

	glDisable(GL_TEXTURE_2D);
}

void npcgfx::clear() {
	npc_graphics.clear();
}

void npcgfx::set_palette(const size_t& palette_num) {
	if(palette_num >= 56) {
		a2e_error("invalid palette #%u!", palette_num);
		return;
	}
	
	if(cur_palette != palette_num) clear();
	
	cur_palette = palette_num;
}
