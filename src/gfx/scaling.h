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

#ifndef __AR_SCALING_H__
#define __AR_SCALING_H__

#include "ar_global.h"
#include <core/vector2.hpp>

class scaling {
public:
	static void init();
	
	enum SCALE_TYPE {
		ST_NEAREST_1X = 0,
		ST_NEAREST_2X = 1,
		ST_HQ2X = 2,
		ST_NEAREST_4X = 3,
		ST_HQ4X = 4
	};
	static void scale(const SCALE_TYPE scale_type, const unsigned int* input, const size2& input_size, unsigned int* output);
	static size_t get_scale_factor(const SCALE_TYPE& scale_type);
	
protected:
	//static opencl::kernel_object* hq4x;
	//static cl::NDRange hq4x_range;
	static unsigned int RGBtoYUV[16777216];
	
	static void hq2x_32(const unsigned int* sp, unsigned int* dp, const size2& input_size);
	static void hq4x_32(const unsigned int* sp, unsigned int* dp, const size2& input_size);
	static inline int Diff(unsigned int w1, unsigned int w2);
	
	scaling() {}
	~scaling() {}
	scaling& operator=(const scaling& s);
};

#endif
