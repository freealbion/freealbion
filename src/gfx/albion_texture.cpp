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

#include "albion_texture.h"

albion_texture::albion_texture() {
}

albion_texture::~albion_texture() {
}

a2e_texture albion_texture::create(const size2& texture_size, const size2& tile_size, const size_t& palette, const vector<albion_texture_info*>& tex_info, xld** xlds, const texture_object::TEXTURE_FILTERING filtering) {
	if(tex_info.size() == 0) return t->get_dummy_texture();
	if(texture_size.x == 0 || texture_size.y == 0) return t->get_dummy_texture();
	if(tile_size.x == 0 || tile_size.y == 0) return t->get_dummy_texture();
	
	unsigned int* tex_surface = new unsigned int[texture_size.x*texture_size.y];
	memset(tex_surface, 0, texture_size.x*texture_size.y*sizeof(unsigned int));
	
	const size2 scaled_tile_size = tile_size * 4;
	unsigned int* data_32bpp = new unsigned int[tile_size.x*tile_size.y];
	unsigned int* scaled_data = new unsigned int[scaled_tile_size.x*scaled_tile_size.y];
	
	const float2 tile_tc_size = float2(float(scaled_tile_size.x)/float(texture_size.x), float(scaled_tile_size.y)/float(texture_size.y));

	a2e_texture ret_tex;
	switch(tex_info[0]->type) {
		case ATIT_MULTI_XLD:
			for(size_t i = 0; i < tex_info.size(); i++) {
				albion_texture_multi_xld* info = (albion_texture_multi_xld*)tex_info[i];

				const xld* tex_xld = xlds[(info->tex_num / 100)];
				const size_t tex_num = (info->tex_num < 100 ? info->tex_num-1 : info->tex_num) % 100;
				const xld::xld_object* tex_data = tex_xld->get_object(tex_num);
				gfxconv::convert_8to32(&tex_data->data[info->offset], data_32bpp, tile_size.x, tile_size.y, palette, info->palette_shift);
				scaling::scale_4x(scaling::ST_HQ4X, data_32bpp, tile_size, scaled_data);

				// copy data into tiles surface
				const size_t offset_x = (scaled_tile_size.x * i) % texture_size.x;
				const size_t offset_y = scaled_tile_size.y * (scaled_tile_size.x * i / texture_size.x);
				for(size_t y = 0; y < scaled_tile_size.y; y++) {
					memcpy(&tex_surface[(offset_y + y) * texture_size.x + offset_x], &scaled_data[y*scaled_tile_size.x], scaled_tile_size.x*sizeof(unsigned int));
				}

				info->tex_coord_begin.set(float(offset_x)/float(texture_size.x), float(offset_y)/float(texture_size.y));
				info->tex_coord_end = info->tex_coord_begin + tile_tc_size;
			}
			break;
		case ATIT_SINGLE_OBJECT: {
			albion_texture_single_object* info = (albion_texture_single_object*)tex_info[0];
			const size_t obj_size = tile_size.x * tile_size.y;
			for(size_t i = 0; i < info->object_count; i++) {
				gfxconv::convert_8to32(&(info->object->data[info->offset + i*obj_size]), data_32bpp, tile_size.x, tile_size.y, palette, info->palette_shift);
				scaling::scale_4x(scaling::ST_HQ4X, data_32bpp, tile_size, scaled_data);

				// copy data into tiles surface
				const size_t offset_x = (scaled_tile_size.x * i) % texture_size.x;
				const size_t offset_y = scaled_tile_size.y * (scaled_tile_size.x * i / texture_size.x);
				for(size_t y = 0; y < scaled_tile_size.y; y++) {
					memcpy(&tex_surface[(offset_y + y) * texture_size.x + offset_x], &scaled_data[y*scaled_tile_size.x], scaled_tile_size.x*sizeof(unsigned int));
				}
			}
		}
		break;
		case ATIT_SINGLE_OBJECT_PALETTE_SHIFT: {
			albion_texture_single_object_ps* info = (albion_texture_single_object_ps*)tex_info[0];
			const size_t obj_size = tile_size.x * tile_size.y;
			const vector<size2>& animated_ranges = palettes->get_animated_ranges(palette);
			size_t tile_num = 0;
			for(size_t i = 0; i < info->object_count; i++) {
				// check if tile contains animated colors
				size_t animations = 1;
				for(vector<size2>::const_iterator ani_range = animated_ranges.begin(); ani_range != animated_ranges.end(); ani_range++) {
					const size_t obj_size = tile_size.x*tile_size.y;
					const size_t p_offset = info->offset + i*obj_size;
					for(size_t p = 0; p < obj_size; p++) {
						if(info->object->data[p_offset + p] >= ani_range->x && info->object->data[p_offset + p] <= ani_range->y) {
							animations = std::max(ani_range->y - ani_range->x + 1, animations);
							break;
						}
					}
				}

				info->tex_coords.push_back(vector<float2>());
				for(size_t j = 0; j < animations; j++) {
					gfxconv::convert_8to32(&(info->object->data[info->offset + i*obj_size]), data_32bpp, tile_size.x, tile_size.y, palette, j);
					scaling::scale_4x(scaling::ST_HQ4X, data_32bpp, tile_size, scaled_data);

					// copy data into tiles surface
					const size_t offset_x = (scaled_tile_size.x * tile_num) % texture_size.x;
					const size_t offset_y = scaled_tile_size.y * (scaled_tile_size.x * tile_num / texture_size.x);
					for(size_t y = 0; y < scaled_tile_size.y; y++) {
						memcpy(&tex_surface[(offset_y + y) * texture_size.x + offset_x], &scaled_data[y*scaled_tile_size.x], scaled_tile_size.x*sizeof(unsigned int));
					}
					info->tex_coords.back().push_back(float2(offset_x, offset_y)/float2(texture_size));
					tile_num++;
				}
			}
		}
		break;
		default:
			ret_tex = t->get_dummy_texture();
			break;
	}

	delete [] data_32bpp;
	delete [] scaled_data;

	ret_tex = t->add_texture(tex_surface, texture_size.x, texture_size.y, GL_RGBA8, GL_RGBA, filtering, e->get_anisotropic(), GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_UNSIGNED_BYTE);
	delete [] tex_surface;

	return ret_tex;
}
