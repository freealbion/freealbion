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

#include "background3d.h"

background3d::background3d() : a2estatic(::e, ::s, ::sce) {
	bg3d_xld = new xld("3DBCKGR0.XLD");
	bg_texture = t->get_dummy_texture();
	cur_bg_num = -1;
}

background3d::~background3d() {
	delete bg3d_xld;
}

void background3d::unload() {
	if(cur_bg_num >= 0) t->delete_texture(bg_texture);
	cur_bg_num = -1;
}

void background3d::load(const size_t& bg_num, const size_t& palette) {
	if(bg_num >= bg3d_xld->get_object_count()) {
		a2e_error("invalid background number #%u!", bg_num);
		return;
	}

	if(cur_bg_num >= 0 && (size_t)cur_bg_num != bg_num) {
		unload();
	}
	cur_bg_num = bg_num;
	
	// get background image
	const xld::xld_object* obj = bg3d_xld->get_object(bg_num);
	size2 bg_size = size2(AR_GET_USINT(obj->data, 0), AR_GET_USINT(obj->data, 2));
	size2 texture_size = bg_size*4;

	unsigned int* tex_surface = new unsigned int[texture_size.x*texture_size.y];
	memset(tex_surface, 0, texture_size.x*texture_size.y*sizeof(unsigned int));

	unsigned int* data_32bpp = new unsigned int[bg_size.x*bg_size.y];
	gfxconv::convert_8to32(&(obj->data[6]), data_32bpp, bg_size.x, bg_size.y, palette, 0);
	scaling::scale(conf::get<scaling::SCALE_TYPE>("map.3d.scale_type"), data_32bpp, bg_size, tex_surface);
	delete [] data_32bpp;

	// this is constant for all bg cubemaps
	const size2 cm_texture_size = size2(texture_size.x*2, texture_size.x*2);

	// allocate pixel data memory
	unsigned int** pixel_data = new unsigned int*[6];
	for(size_t i = 0; i < 6; i++) pixel_data[i] = new unsigned int[cm_texture_size.x*cm_texture_size.y];

	// create +x texture
	memset(pixel_data[0], 0, cm_texture_size.x*cm_texture_size.y*sizeof(unsigned int));
	const size_t y_offset = 300;
	const unsigned int repeat_val = tex_surface[0]; // "sky repeat value"
	// on the top it's just a repeated color
	for(size_t y = 0; y < y_offset; y++) {
		for(size_t x = 0; x < cm_texture_size.x; x++) {
			pixel_data[0][y*cm_texture_size.x + x] = repeat_val;
		}
	}
	// the actual background image is used twice for one side
	for(size_t y = 0; y < texture_size.y; y++) {
		memcpy(&pixel_data[0][(y+y_offset)*cm_texture_size.x], &tex_surface[y*texture_size.x], texture_size.x*sizeof(unsigned int));
		memcpy(&pixel_data[0][(y+y_offset)*cm_texture_size.x + texture_size.x], &tex_surface[y*texture_size.x], texture_size.x*sizeof(unsigned int));
	}
	
	// all other sides are equal to +x
	memcpy(pixel_data[1], pixel_data[0], cm_texture_size.x*cm_texture_size.y*sizeof(unsigned int)); //-x
	memcpy(pixel_data[4], pixel_data[0], cm_texture_size.x*cm_texture_size.y*sizeof(unsigned int)); //+z
	memcpy(pixel_data[5], pixel_data[0], cm_texture_size.x*cm_texture_size.y*sizeof(unsigned int)); //-z
	
	// fill +y with a sky color
	for(size_t y = 0; y < cm_texture_size.y; y++) {
		for(size_t x = 0; x < cm_texture_size.x; x++) {
			pixel_data[2][y*cm_texture_size.x + x] = repeat_val;
		}
	}
	memset(pixel_data[3], 0, cm_texture_size.x*cm_texture_size.y*sizeof(unsigned int)); // -y = black

	bg_texture = t->add_cubemap_texture((void**)pixel_data, cm_texture_size.x, cm_texture_size.y, GL_RGBA8, GL_RGBA, texture_object::TF_POINT, 0, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_UNSIGNED_BYTE, NULL);
	
	delete [] pixel_data;
	delete [] tex_surface;
	//conf::set<a2e_texture>("debug.texture", bg_texture);
}

a2e_texture& background3d::get_bg_texture() {
	return bg_texture;
}

void background3d::draw(const size_t draw_mode) {
	if(cur_bg_num < 0) return;
	
	gl2shader shd = (draw_mode == MDM_GEOMETRY_PASS ? s->get_gl2shader("AR_IR_GBUFFER_SKY") : s->get_gl2shader("AR_IR_MP_SKY"));
	
	matrix4f skybox_proj_mat = *e->get_projection_matrix();
	skybox_proj_mat.transpose();
	matrix4f IMVP = skybox_proj_mat * matrix4f().rotate_x(e->get_rotation()->x) * matrix4f().rotate_y(-e->get_rotation()->y);
	IMVP.invert();
	
	float3 sb_v0 = float3(0.0f, 2.0f, 1.0f);
	float3 sb_v1 = float3(3.0f, -1.0f, 1.0f);
	float3 sb_v2 = float3(-3.0f, -1.0f, 1.0f);
	sb_v0 *= IMVP;
	sb_v1 *= IMVP;
	sb_v2 *= IMVP;
	
	if(draw_mode == MDM_MATERIAL_PASS) {
		shd->texture("diffuse_texture", bg_texture);
		
		ir_mp_setup(shd);
	}
	
	glBegin(GL_TRIANGLES);
	glTexCoord3fv(&sb_v0.x);
	glVertex4f(0.0f, 2.0f, 1.0f, 1.0f);
	glTexCoord3fv(&sb_v1.x);
	glVertex4f(-3.0f, -1.0f, 1.0f, 1.0f);
	glTexCoord3fv(&sb_v2.x);
	glVertex4f(3.0f, -1.0f, 1.0f, 1.0f);
	glEnd();
	
	shd->disable();
}
