/*
 *  Albion Remake
 *  Copyright (C) 2007 - 2012 Florian Ziesche
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
	cur_bg_palette = -1;
	light_color = float3(0.0f);
	
	const float fullscreen_triangle[] = { 0.0f, 2.0f, -3.0f, -1.0f, 3.0f, -1.0f };
	const float _dummy_coords[] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	
	glGenBuffers(1, &vbo_fs_triangle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_fs_triangle);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float2), fullscreen_triangle, GL_STATIC_DRAW);
	
	glGenBuffers(1, &vbo_fs_coords);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_fs_coords);
	// only allocate the necessary memory (init with dummy data)
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float3), _dummy_coords, GL_STREAM_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

background3d::~background3d() {
	delete bg3d_xld;
	
	if(glIsBuffer(vbo_fs_triangle)) glDeleteBuffers(1, &vbo_fs_triangle);
	if(glIsBuffer(vbo_fs_coords)) glDeleteBuffers(1, &vbo_fs_coords);
}

void background3d::unload() {
	if(cur_bg_num >= 0) t->delete_texture(bg_texture);
	cur_bg_num = -1;
	cur_bg_palette = -1;
}

void background3d::load(const size_t& bg_num, const size_t& palette) {
	if(bg_num >= bg3d_xld->get_object_count()) {
		a2e_error("invalid background number #%u!", bg_num);
		return;
	}
	
	if(cur_bg_num >= 0) {
		if((size_t)cur_bg_num == bg_num && (size_t)cur_bg_palette == palette) {
			// bg is the same and already loaded -> return
			return;
		}
		// else: this is a different background, unload and load new one
		unload();
	}
	cur_bg_num = bg_num;
	cur_bg_palette = palette;
	
	// background image is always scaled up by 4, however, nearest filtering or hq4x may be used
	const size_t scale_factor = 4;
	const scaling::SCALE_TYPE conf_scale_type = conf::get<scaling::SCALE_TYPE>("map.3d.scale_type");
	const scaling::SCALE_TYPE scale_type = (conf_scale_type == scaling::ST_HQ2X || conf_scale_type == scaling::ST_HQ4X ? scaling::ST_HQ4X : scaling::ST_NEAREST_4X);
	
	// get background image
	const xld::xld_object* obj = bg3d_xld->get_object(bg_num);
	size2 bg_size = size2(AR_GET_USINT(obj->data, 0), AR_GET_USINT(obj->data, 2));
	size2 texture_size = bg_size * scale_factor;

	unsigned int* tex_surface = new unsigned int[texture_size.x*texture_size.y];
	memset(tex_surface, 0, texture_size.x*texture_size.y*sizeof(unsigned int));

	unsigned int* data_32bpp = new unsigned int[bg_size.x*bg_size.y];
	gfxconv::convert_8to32(&(obj->data[6]), data_32bpp, bg_size.x, bg_size.y, palette, 0);
	scaling::scale(scale_type, data_32bpp, bg_size, tex_surface);
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
	
	bg_texture = t->add_cubemap_texture((void**)pixel_data, (unsigned int)cm_texture_size.x, (unsigned int)cm_texture_size.y, GL_RGBA8, GL_RGBA, texture_object::TF_TRILINEAR, e->get_anisotropic(), GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_UNSIGNED_BYTE, NULL);
	
	for(size_t i = 0; i < 6; i++) delete [] pixel_data[i];
	delete [] pixel_data;
	delete [] tex_surface;
	//conf::set<a2e_texture>("debug.texture", bg_texture);
}

a2e_texture& background3d::get_bg_texture() {
	return bg_texture;
}

void background3d::draw(const DRAW_MODE draw_mode) {
	if(cur_bg_num < 0) return;
	
	// we will write directly into the final material pass, so no geometry
	// and light pass are necessary (-> return)
	if(draw_mode != DRAW_MODE::MATERIAL_PASS) return;
	
	gl3shader shd = s->get_gl3shader("AR_IR_MP_SKY");
	
	matrix4f skybox_proj_mat = *e->get_projection_matrix();
	matrix4f IMVP = matrix4f().rotate_y(-e->get_rotation()->y) * matrix4f().rotate_x(e->get_rotation()->x) * skybox_proj_mat;
	IMVP.invert();
	
	float3 sb_tc[3] = {
		float3(0.0f, 2.0f, 1.0f),
		float3(3.0f, -1.0f, 1.0f),
		float3(-3.0f, -1.0f, 1.0f)
	};
	sb_tc[0] *= IMVP;
	sb_tc[1] *= IMVP;
	sb_tc[2] *= IMVP;
	
	shd->texture("diffuse_texture", bg_texture);
	shd->uniform("light_color", light_color);
	
	// update coords buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo_fs_coords);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float3)*3, sb_tc);
	
	//
	shd->attribute_array("in_vertex", vbo_fs_triangle, 2, GL_FLOAT);
	shd->attribute_array("in_cube_tex_coord", vbo_fs_coords, 3, GL_FLOAT);
	
	glDrawArrays(GL_TRIANGLES, 0, 3);
	
	shd->disable();
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void background3d::set_light_color(const float3& color) {
	light_color = color;
}
