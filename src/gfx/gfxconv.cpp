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

#include "gfxconv.h"

const pal* gfxconv::palettes = NULL;

void gfxconv::init(const pal* palettes) {
	gfxconv::palettes = palettes;
}

void gfxconv::convert_8to32(const unsigned char* data_8bpp, unsigned int* data_32bpp, const size_t& width, const size_t& height, const size_t& palette_num) {
	const unsigned int* const palette = palettes->get_palette(palette_num);
	for(size_t i = 0; i < height; i++) {
		for(size_t j = 0; j < width; j++) {
			data_32bpp[i*width+j] = palette[data_8bpp[i*width+j]];
		}
	}
}
