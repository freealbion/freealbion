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

#include "map_objects.hpp"

map_objects::map_objects() : map_tiles() {
	ws_positions = nullptr;
	vbo_ws_position_id = 0;
}

map_objects::~map_objects() {
	if(ws_positions != nullptr) {
		delete [] ws_positions;
		ws_positions = nullptr;
	}
	if(glIsBuffer(vbo_ws_position_id)) { glDeleteBuffers(1, &vbo_ws_position_id); }
}

void map_objects::set_ws_positions(float3* ws_positions_, GLenum usage) {
	map_objects::ws_positions = ws_positions_;
	
	glGenBuffers(1, &vbo_ws_position_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_ws_position_id);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * 3 * sizeof(float), ws_positions, usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

const string map_objects::select_shader(const DRAW_MODE& draw_mode) const {
	if(draw_mode == DRAW_MODE::GEOMETRY_PASS || draw_mode == DRAW_MODE::GEOMETRY_ALPHA_PASS) return "AR_IR_GBUFFER_MAP_OBJECTS";
	else if(draw_mode == DRAW_MODE::MATERIAL_PASS || draw_mode == DRAW_MODE::MATERIAL_ALPHA_PASS) return "AR_IR_MP_MAP_OBJECTS";
	return "";
}

void map_objects::pre_draw_geometry(gl3shader& shd, VERTEX_ATTRIBUTE& attr_array_mask, a2ematerial::TEXTURE_TYPE& texture_mask) {
	pre_draw_material(shd, attr_array_mask, texture_mask);
	attr_array_mask |= a2emodel::VERTEX_ATTRIBUTE::TEXTURE_COORD;
	texture_mask |= a2ematerial::TEXTURE_TYPE::DIFFUSE;
}

void map_objects::pre_draw_material(gl3shader& shd, VERTEX_ATTRIBUTE& attr_array_mask, a2ematerial::TEXTURE_TYPE& texture_mask floor_unused) {
	shd->uniform("cam_position", -float3(*e->get_position()));
	
	shd->attribute_array("ws_position", vbo_ws_position_id, 3);

	attr_array_mask &= (a2emodel::VERTEX_ATTRIBUTE)(~(unsigned int)a2emodel::VERTEX_ATTRIBUTE::NORMAL);
}
