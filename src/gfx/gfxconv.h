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
 
#ifndef __AR_GFXCONV_H__
#define __AR_GFXCONV_H__

#include "ar_global.h"

/*! @class gfxconv
 *  @brief gfx converter
 *  @author flo
 *  
 *  converts 8bpp pixel data to 32bpp using a specific palette
 */

class gfxconv {
public:
	static void convert_8to32(const unsigned char* data_8bpp, unsigned int* data_32bpp, const size_t& width, const size_t& height, const size_t& palette_num,
							  const size_t palette_shift = 0, const bool overwrite_alpha = false, const unsigned int replacement_alpha = 0);
	
protected:
	gfxconv() {}
	~gfxconv() {}
	gfxconv& operator=(const gfxconv& gfxc);

};

#endif
