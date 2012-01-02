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

#include "albion_texture.h"
#include <engine.h>
#include <rendering/texman.h>

albion_texture::albion_texture() {
}

albion_texture::~albion_texture() {
}

a2e_texture albion_texture::create(const size_t& map_type, const size2& texture_size, const size2& tile_size, const size_t& palette, const vector<albion_texture_info*>& tex_info, xld** xlds, const TEXTURE_SPACING_TYPE spacing_type, const size_t spacing_size, bool custom_mipmaps, const texture_object::TEXTURE_FILTERING filtering) {
	if(tex_info.size() == 0) return t->get_dummy_texture();
	if(texture_size.x == 0 || texture_size.y == 0) return t->get_dummy_texture();
	if(tile_size.x == 0 || tile_size.y == 0) return t->get_dummy_texture();
	
	unsigned int* tex_surface = new unsigned int[texture_size.x*texture_size.y];
	memset(tex_surface, 0, texture_size.x*texture_size.y*sizeof(unsigned int));
	
	const scaling::SCALE_TYPE scale_type = conf::get<scaling::SCALE_TYPE>(map_type == 0 ? "map.2d.scale_type" : "map.3d.scale_type");
	const size_t scale_factor = scaling::get_scale_factor(scale_type);
	const size2 scaled_tile_size = tile_size * scale_factor;
	const size2 spaced_scaled_tile_size = scaled_tile_size + size2(2*spacing_size);
	unsigned int* data_32bpp = new unsigned int[tile_size.x*tile_size.y];
	unsigned int* scaled_data = new unsigned int[scaled_tile_size.x*scaled_tile_size.y];
	
	const float2 tile_tc_size = float2(float(scaled_tile_size.x)/float(texture_size.x), float(scaled_tile_size.y)/float(texture_size.y));
	
	a2e_texture ret_tex;
	switch(tex_info[0]->type) {
		case ATIT_MULTI_XLD: {
			size_t offset_x = 0, offset_y = 0;
			for(size_t i = 0; i < tex_info.size(); i++) {
				albion_texture_multi_xld* info = (albion_texture_multi_xld*)tex_info[i];

				const xld* tex_xld = xlds[(info->tex_num / 100)];
				const size_t tex_num = (info->tex_num < 100 ? info->tex_num-1 : info->tex_num) % 100;
				const xld::xld_object* tex_data = tex_xld->get_object(tex_num);
				gfxconv::convert_8to32(&tex_data->data[info->offset], data_32bpp, tile_size.x, tile_size.y, palette, info->palette_shift);
				scaling::scale(scale_type, data_32bpp, tile_size, scaled_data);

				// copy data into tiles surface
				const size_t cur_x = offset_x + spaced_scaled_tile_size.x;
				const bool overflow = (cur_x >= texture_size.x-spaced_scaled_tile_size.x);
				offset_x = (overflow ? 0 : cur_x);
				if(overflow) offset_y += spaced_scaled_tile_size.y;
				
				for(size_t y = 0; y < scaled_tile_size.y; y++) {
					memcpy(&tex_surface[(offset_y + y + spacing_size) * texture_size.x + (offset_x + spacing_size)], &scaled_data[y*scaled_tile_size.x], scaled_tile_size.x*sizeof(unsigned int));
				}
				
				//
				add_spacing(tex_surface, texture_size, scaled_data, scaled_tile_size, spacing_size, spacing_type, size2(offset_x, offset_y));
				
				info->tex_coord_begin.set(float(offset_x+spacing_size)/float(texture_size.x), float(offset_y+spacing_size)/float(texture_size.y));
				info->tex_coord_end = info->tex_coord_begin + tile_tc_size;
			}
		}
		break;
		case ATIT_SINGLE_OBJECT: {
			albion_texture_single_object* info = (albion_texture_single_object*)tex_info[0];
			const size_t obj_size = tile_size.x * tile_size.y;
			for(size_t i = 0; i < info->object_count; i++) {
				gfxconv::convert_8to32(&(info->object->data[info->offset + i*obj_size]), data_32bpp, tile_size.x, tile_size.y, palette, info->palette_shift);
				scaling::scale(scale_type, data_32bpp, tile_size, scaled_data);

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
					scaling::scale(scale_type, data_32bpp, tile_size, scaled_data);

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

	if(custom_mipmaps) {
		ret_tex = t->add_texture(tex_surface, (unsigned int)texture_size.x, (unsigned int)texture_size.y, GL_RGBA8, GL_RGBA, std::min(filtering, texture_object::TF_LINEAR), e->get_anisotropic(), GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_UNSIGNED_BYTE);
		build_mipmaps(ret_tex, tex_surface, filtering);
	}
	else {
		ret_tex = t->add_texture(tex_surface, (unsigned int)texture_size.x, (unsigned int)texture_size.y, GL_RGBA8, GL_RGBA, filtering, e->get_anisotropic(), GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_UNSIGNED_BYTE);
	}
	delete [] tex_surface;

	return ret_tex;
}

void albion_texture::build_mipmaps(const a2e_texture& tex, const unsigned int* tex_data, const texture_object::TEXTURE_FILTERING filtering) {
	if(filtering < texture_object::TF_BILINEAR) return;
	if(tex_data == NULL) return;
	
	//
	const ssize_t scale_factor = (ssize_t)scaling::get_scale_factor(conf::get<scaling::SCALE_TYPE>("map.3d.scale_type"));
	const ssize_t scale_passes = (scale_factor == 4 ? 3 : (scale_factor == 2 ? 2 : 1)) - 1;
	
	glBindTexture(GL_TEXTURE_2D, tex->tex());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (filtering == texture_object::TF_BILINEAR ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR));
	if(e->get_anisotropic() > 0) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (GLint)e->get_anisotropic());
	}
	
	// scale down to 0.5x
	// for each level, we will use the texture data of the previous level (starting with tex_data)
	const unsigned int* last_data = tex_data;
	size2 last_size = size2(tex->width, tex->height);
	size_t level = 1;
	for(ssize_t scale = scale_passes; scale >= 0; scale--, level++) {
		const size2 cur_size = size2(tex->width >> level, tex->height >> level);
		unsigned int* scaled_data = new unsigned int[cur_size.x*cur_size.y];
		
		// scale down
		for(size_t y = 0; y < last_size.y; y+=2) {
			// y size might be odd (note: x will always be even, b/c we're using max_tex_size == pot)
			if(y/2 >= cur_size.y) break;
			const size_t cur_offset = (y/2) * cur_size.x;
			const size_t offset_1 = y*last_size.x;
			const size_t offset_2 = (y + (y == last_size.y-1 ? 0 : 1))*last_size.x;
			for(size_t x = 0; x < last_size.x; x+=2) {
				// TODO: keep max alpha?
				size4 colors(last_data[offset_1 + x], last_data[offset_1 + x + 1],
							 last_data[offset_2 + x], last_data[offset_2 + x + 1]);
				unsigned int avg = 0;
				for(unsigned int col = 0; col < 4; col++) {
					const size_t shift = col*8;
					size_t ch = (colors.x & (0xFF << shift)) >> shift;
					ch += (colors.y & (0xFF << shift)) >> shift;
					ch += (colors.z & (0xFF << shift)) >> shift;
					ch += (colors.w & (0xFF << shift)) >> shift;
					ch /= 4;
					avg |= std::min(ch, (size_t)0xFF) << shift; 
				}
				scaled_data[cur_offset + (x/2)] = avg;
			}
		}
		
		glTexImage2D(GL_TEXTURE_2D, (unsigned int)level, tex->internal_format, (unsigned int)cur_size.x, (unsigned int)cur_size.y, 0, tex->format, tex->type, scaled_data);
		
		if(last_data != tex_data) delete [] last_data;
		
		last_data = scaled_data;
		last_size = cur_size;
	}
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, (unsigned int)(level-1));
	
	if(last_data != tex_data) delete [] last_data;
}

void albion_texture::add_spacing(unsigned int* surface, const size2& texture_size, const unsigned int* tile_data, const size2& tile_size, const size_t& spacing, const TEXTURE_SPACING_TYPE& spacing_type, const size2 offset) {
	if(spacing == 0) return;
	
	switch(spacing_type) {
		case TST_MIRROR:
			// +/- y
			for(size_t y = 0; y < spacing; y++) {
				// up
				memcpy(&surface[(offset.y + y) * texture_size.x + (offset.x + spacing)],
					   &tile_data[(spacing-y)*tile_size.x],
					   tile_size.x*sizeof(unsigned int));
				
				// down
				memcpy(&surface[(offset.y + tile_size.y + spacing + y) * texture_size.x + (offset.x + spacing)],
					   &tile_data[(tile_size.y-1 - y)*tile_size.x],
					   tile_size.x*sizeof(unsigned int));
			}
			
			// +/- x
			for(size_t y = 0; y < tile_size.y; y++) {
				unsigned int* left_x = &surface[(offset.y + spacing + y) * texture_size.x + offset.x];
				unsigned int* right_x = &surface[(offset.y + spacing + y) * texture_size.x + offset.x + tile_size.x + spacing];
				for(size_t x = 0; x < spacing; x++) {
					// left
					*left_x = tile_data[(y*tile_size.x) + (spacing-x)];
					left_x++;
					
					// right
					*right_x = tile_data[(y*tile_size.x) + (tile_size.x-1 - x)];
					right_x++;
				}
			}
			
			// corners
			for(size_t y = 0; y < spacing; y++) {
				// up left can copy from left spacing (mirroring it vertically)
				memcpy(&surface[(offset.y + (spacing-1 - y)) * texture_size.x + offset.x],
					   &surface[(offset.y + spacing + y) * texture_size.x + offset.x],
					   spacing*sizeof(unsigned int));
				
				// up right
				memcpy(&surface[(offset.y + (spacing-1 - y)) * texture_size.x + (offset.x + spacing + tile_size.x)],
					   &surface[(offset.y + spacing + y) * texture_size.x + (offset.x + spacing + tile_size.x)],
					   spacing*sizeof(unsigned int));
				
				// down left
				memcpy(&surface[(offset.y + tile_size.y + spacing + y) * texture_size.x + offset.x],
					   &surface[(offset.y + tile_size.y + (spacing-1 - y)) * texture_size.x + offset.x],
					   spacing*sizeof(unsigned int));
				
				// down right
				memcpy(&surface[(offset.y + tile_size.y + spacing + y) * texture_size.x + (offset.x + spacing + tile_size.x)],
					   &surface[(offset.y + tile_size.y + (spacing-1 - y)) * texture_size.x + (offset.x + spacing + tile_size.x)],
					   spacing*sizeof(unsigned int));
			}
			
			break;
		case TST_TILE:
			// TODO: !
			break;
		case TST_TRANSPARENT: {
			// upper and lower stripe
			const size_t width = tile_size.x + spacing*2;
			for(size_t y = 0; y < spacing; y++) {
				memset(&surface[(offset.y + y) * texture_size.x + offset.x], 0, width*sizeof(unsigned int));
				memset(&surface[(offset.y + spacing + tile_size.y + y) * texture_size.x + offset.x], 0, width*sizeof(unsigned int));
			}
			
			// left and right stripe
			for(size_t y = 0; y < tile_size.y; y++) {
				memset(&surface[(offset.y + spacing + y) * texture_size.x + offset.x], 0, spacing*sizeof(unsigned int));
				memset(&surface[(offset.y + spacing + y) * texture_size.x + (offset.x + tile_size.x + spacing)], 0, spacing*sizeof(unsigned int));
			}
		}
		break;
		case TST_NONE:
		default:
			break;
	}
}
