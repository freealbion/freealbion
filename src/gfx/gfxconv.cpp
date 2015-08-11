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

#include "gfx/gfxconv.hpp"
#include <floor/math/vector_lib.hpp>

void gfxconv::convert_8to32(const unsigned char* data_8bpp, unsigned int* data_32bpp,
							const size_t& width, const size_t& height, 
							const size_t& palette_num, const size_t palette_shift,
							const bool overwrite_alpha, const unsigned int replacement_alpha) {
	const unsigned int* const palette = palettes->get_palette(palette_num);
	const vector<size2>& animated_ranges = palettes->get_animated_ranges(palette_num);
	unsigned char index;

	// apply palette shift
	const unsigned char* data = data_8bpp;
	if(palette_shift != 0) {
		unsigned char* shifted_data = new unsigned char[width*height];
		memcpy(shifted_data, data, width*height);

		for(vector<size2>::const_iterator ani_range = animated_ranges.begin(); ani_range != animated_ranges.end(); ani_range++) {
			const size_t range = (ani_range->y - ani_range->x) + 1;
			for(size_t y = 0; y < height; y++) {
				for(size_t x = 0; x < width; x++) {
					index = shifted_data[y*width + x];
					if(index >= ani_range->x && index <= ani_range->y) {
						index = uint8_t(ani_range->x + ((range + size_t(ssize_t(index - ani_range->x) - ssize_t(palette_shift % range))) % range));
						shifted_data[y*width + x] = index;
					}
					else shifted_data[y*width + x] = index;
				}
			}
		}

		data = shifted_data;
	}

	if(!overwrite_alpha) {
		for(size_t i = 0; i < height; i++) {
			for(size_t j = 0; j < width; j++) {
				index = data[i*width+j];
				data_32bpp[i*width+j] = palette[index];
			}
		}
	}
	else {
		for(size_t i = 0; i < height; i++) {
			for(size_t j = 0; j < width; j++) {
				index = data[i*width+j];
				data_32bpp[i*width+j] = (palette[index] & 0xFF000000) == 0 ? replacement_alpha : palette[index];
			}
		}
	}

	if(palette_shift != 0) {
		delete [] data;
	}
}
