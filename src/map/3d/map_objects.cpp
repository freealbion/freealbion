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

#include "map_objects.h"

map_objects::map_objects() : map_tiles() {
	ws_positions = NULL;
	vbo_ws_position_id = 0;
}

map_objects::~map_objects() {
	if(ws_positions != NULL) {
		delete [] ws_positions;
		ws_positions = NULL;
	}
	if(glIsBuffer(vbo_ws_position_id)) { glDeleteBuffers(1, &vbo_ws_position_id); }
}

/*void map_objects::draw(const size_t draw_mode) {
	pre_draw_setup();
	
	gl2shader shd = s->get_gl2shader("AR_DR_GBUFFER_MAP_OBJECTS");
	
	shd->uniform("local_mview", mview_mat);
	shd->uniform("local_scale", scale_mat);
	shd->uniform("Nuv", 16.0f, 16.0f);
	shd->uniform("cam_position", -float3(*e->get_position()));

	const a2e_texture& tex = ((a2ematerial::diffuse_material*)material->get_material(0)->mat)->diffuse_texture;
	float2 texel_size = float2(1.0f) / float2(tex->width, tex->height);
	//float2 texel_size = (float2(1.0f) / float2(tex->width, tex->height)) * float2(0.5f);
	shd->uniform("texel_size", texel_size);
	
	material->enable_textures(0, shd);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices_id);
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	
	shd->attribute_array("ws_position", vbo_ws_position_id, 3);
	shd->attribute_array("normal", vbo_normals_id, 3);
	shd->attribute_array("texture_coord", vbo_tex_coords_id, 2);
	shd->attribute_array("tc_restrict", vbo_tc_restrict_id, 4);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, vbo_indices_ids[0]);
	glDrawElements(GL_TRIANGLES, (GLsizei)(index_count[0] * 3), GL_UNSIGNED_INT, NULL);
	glDisableClientState(GL_VERTEX_ARRAY);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	material->disable_textures(0);
	shd->disable();
	
	post_draw_setup();
}*/

void map_objects::set_ws_positions(float3* ws_positions, GLenum usage) {
	map_objects::ws_positions = ws_positions;
	
	glGenBuffers(1, &vbo_ws_position_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_ws_position_id);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * 3 * sizeof(float), ws_positions, usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

const string map_objects::select_shader(const size_t& draw_mode) const {
	if(draw_mode == a2emodel::MDM_GEOMETRY_PASS) return "AR_IR_GBUFFER_MAP_OBJECTS";
	else if(draw_mode == a2emodel::MDM_MATERIAL_PASS) return "AR_IR_MP_MAP_OBJECTS";
	return "";
}

void map_objects::pre_draw_geometry(gl2shader& shd, size_t& attr_array_mask, size_t& texture_mask) {
	pre_draw_material(shd, attr_array_mask, texture_mask);
	attr_array_mask |= VA_TEXTURE_COORD;
	texture_mask |= a2ematerial::TT_DIFFUSE;
}

void map_objects::pre_draw_material(gl2shader& shd, size_t& attr_array_mask, size_t& texture_mask) {
	shd->uniform("cam_position", -float3(*e->get_position()));
	
	const a2e_texture& tex = ((a2ematerial::diffuse_material*)material->get_material(0)->mat)->diffuse_texture;
	float2 texel_size = float2(1.0f) / float2(tex->width, tex->height);
	//float2 texel_size = (float2(1.0f) / float2(tex->width, tex->height)) * float2(0.5f);
	shd->uniform("diffuse_texel_size", texel_size);
	
	shd->attribute_array("ws_position", vbo_ws_position_id, 3);
	shd->attribute_array("tc_restrict", vbo_tc_restrict_id, 4);

	attr_array_mask &= ~VA_NORMAL;
}
