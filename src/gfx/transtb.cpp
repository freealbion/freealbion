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

#include "transtb.hpp"

transtb::transtb() {
	// i don't really have any idea where those are used for yet ...
	/*xld* ttb = new xld("TRANSTB0.XLD");

	// floors
	const size2 ttb_tex_size = size2(1536, 1536);
	unsigned int* ttb_surface = new unsigned int[ttb_tex_size.x*ttb_tex_size.y];
	memset(ttb_surface, 0, ttb_tex_size.x*ttb_tex_size.y*sizeof(unsigned int));
	
	const size2 tile_size = size2(16, 16);
	const size2 scaled_tile_size = tile_size * 4;
	unsigned int* data_32bpp = new unsigned int[tile_size.x*tile_size.y];
	unsigned int* scaled_data = new unsigned int[scaled_tile_size.x*scaled_tile_size.y];
	const xld::xld_object* xld_obj = ttb->get_object(33);
	const size_t size_per_obj = tile_size.x*tile_size.y;
	for(size_t i = 0; i < xld_obj->length/size_per_obj; i++) {
		gfxconv::convert_8to32(&xld_obj->data[i*size_per_obj], data_32bpp, tile_size.x, tile_size.y, 1);
		scaling::scale_4x(scaling::ST_NEAREST, data_32bpp, tile_size, scaled_data);

		// copy data into tiles surface
		const size_t offset_x = (scaled_tile_size.x * i) % ttb_tex_size.x;
		const size_t offset_y = scaled_tile_size.y * (scaled_tile_size.x * i / ttb_tex_size.x);
		if(offset_y+scaled_tile_size.y >= ttb_tex_size.y) break;
		for(size_t y = 0; y < scaled_tile_size.y; y++) {
			memcpy(&ttb_surface[(offset_y + y) * ttb_tex_size.x + offset_x], &scaled_data[y*scaled_tile_size.x], scaled_tile_size.x*sizeof(unsigned int));
		}
	}
	delete [] data_32bpp;

	ttb_tex = t->add_texture(ttb_surface, ttb_tex_size.x, ttb_tex_size.y, GL_RGBA8, GL_RGBA, TEXTURE_FILTERING::POINT, 0, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_UNSIGNED_BYTE);
	//conf::set<a2e_texture>("debug.texture", ttb_tex);

	delete ttb;*/
}

transtb::~transtb() {
}
