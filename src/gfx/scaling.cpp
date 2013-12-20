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

#include "scaling.hpp"
#include <core/platform.hpp>

//opencl::kernel_object* scaling::hq4x = nullptr;
//cl::NDRange scaling::hq4x_range;
unsigned int scaling::RGBtoYUV[16777216];

void scaling::init() {
	//////// taken from hqx library, LGPL 2.1+
	// Initalize RGB to YUV lookup table
	unsigned int col, r, g, b, y, u, v;
	RGBtoYUV[0] = (128 << 8) + 128;
	for (col = 1; col < 16777215; col++) {
		r = (col & 0xFF0000) >> 16;
		g = (col & 0x00FF00) >> 8;
		b = col & 0x0000FF;
		y = (unsigned int)(0.299*r + 0.587*g + 0.114*b);
		u = (unsigned int)(-0.169*r - 0.331*g + 0.5*b) + 128;
		v = (unsigned int)(0.5*r - 0.419*g - 0.081*b) + 128;
		RGBtoYUV[col] = 0xFF000000 + (y << 16) + (u << 8) + v;
	}
	////////
}

size_t scaling::get_scale_factor(const SCALE_TYPE& scale_type) {
	switch(scale_type) {
		case ST_NEAREST_1X:
			return 1;
		case ST_NEAREST_2X:
		case ST_HQ2X:
			return 2;
		case ST_NEAREST_4X:
		case ST_HQ4X:
			return 4;
	}
	floor_unreachable();
}

void scaling::scale(const SCALE_TYPE scale_type, const unsigned int* input, const size2& input_size, unsigned int* output) {
	// scale
	switch(scale_type) {
		case ST_NEAREST_1X:
			memcpy(output, input, input_size.x*input_size.y*sizeof(unsigned int));
			break;
		case ST_NEAREST_2X:
		case ST_NEAREST_4X: {
			// cpu/c++ scaling
			const size_t scale_factor = (scale_type == ST_NEAREST_2X ? 2 : 4);
			const size_t scale_size_x = input_size.x * scale_factor;
			for(size_t y = 0; y < input_size.y; y++) {
				for(size_t x = 0; x < input_size.x; x++) {
					for(size_t sy = 0; sy < scale_factor; sy++) {
						for(size_t sx = 0; sx < scale_factor; sx++) {
							output[(y*scale_factor + sy) * scale_size_x + x*scale_factor + sx] = input[y*input_size.x + x];
						}
					}
				}
			}
		}
		break;
		case ST_HQ2X:
			hq2x_32(input, output, input_size);
			break;
		case ST_HQ4X:
			hq4x_32(input, output, input_size);
			break;
	}
}
