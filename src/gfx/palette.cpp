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

#include "palette.hpp"
#include "ar_global.hpp"
#include "xld.hpp"
#include <core/vector2.hpp>
#include <core/file_io.hpp>

/*! palette constructor
 */
pal::pal() {
	// load palette 000
	fio->open(xld::make_xld_path("PALETTE.000").c_str(), file_io::OPEN_TYPE::READ_BINARY);
	unsigned char* palette_000 = new unsigned char[192];
	fio->get_block((char*)palette_000, 192);
	fio->close();

	// load palettes
	xld* palette_xld = new xld("PALETTE0.XLD");
	const size_t palette_count = palette_xld->get_object_count();
	palettes.reserve(palette_count);
	for(size_t i = 0; i < palette_count; i++) {
		palettes.push_back(new unsigned int[256]);
		unsigned int* palette = palettes.back();
		const xld::xld_object* object = palette_xld->get_object(i);
		palette[0] = 0;
		for(unsigned int j = 1; j < 192; j++) {
			palette[j] = 0xFF000000 + (object->data[j*3+2] << 16) + (object->data[j*3+1] << 8) + object->data[j*3];
			if(palette[j] == 0xFF000000) palette[j] |= 0x010101;
		}
		for(unsigned int j = 192; j < 256; j++) {
			palette[j] = 0xFF000000 + (palette_000[(j-192)*3+2] << 16) + (palette_000[(j-192)*3+1] << 8) + palette_000[(j-192)*3];
			if(palette[j] == 0xFF000000) palette[j] |= 0x010101;
		}
	}
	delete palette_xld;

	// set animated ranges (aka palette color rotation)
	for(size_t i = 0; i < palette_count; i++) {
		animated_ranges.push_back(vector<size2>());
	}

	/*  3D: 1, 3, 7, 8, 13, 14, 15, 18, 22, 25, 29, 30, 51

		1  :  1 : y 0x99-0x9F, 0xB0-0xBF
		3  :  3 : y 0x40-0x43, 0x43-0x4F
		7  :  7 : -
		8  :  8 : -
		13 :  d : -
		14 :  e : y 0xB0-0xB3, 0xB4-0xBF
		15 :  f : y 0x58-0x5F
		18 : 12 : -
		22 : 16 : -
		25 : 19 : y 0xB0-0xB3, 0xB4-0xBF
		29 : 1d : -
		30 : 1e : -
		51 : 33 : y 0xB0-0xB3, 0xB4-0xBF

		2D: 1, 2, 4, 5, 6, 9, 16, 20, 26, 28, 31, 45, 56

		1  :  1 : y 0x99-0x9F, 0xB0-0xBF
		2  :  2 : y 0x99-0x9F, 0xB0-0xB4, 0xB5-0xBF
		4  :  4 : -
		5  :  5 : -
		6  :  6 : y 0xB0-0xB4, 0xB5-0xBF
		9  :  9 : -
		16 : 10 : -
		20 : 14 : -
		26 : 1a : y 0xB4-0xB7, 0xB8-0xBB, 0xBC-0xBF
		28 : 1c : -
		31 : 1f : y 0x10-0x4F
		45 : 2d : -
		56 : 38 : -
	*/

	animated_ranges[0].push_back(size2(0x99, 0x9F));
	animated_ranges[0].push_back(size2(0xB0, 0xBF));
	
	animated_ranges[1].push_back(size2(0x99, 0x9F));
	animated_ranges[1].push_back(size2(0xB0, 0xB4));
	animated_ranges[1].push_back(size2(0xB5, 0xBF));
	
	animated_ranges[2].push_back(size2(0x40, 0x43));
	animated_ranges[2].push_back(size2(0x44, 0x4F));
	
	animated_ranges[5].push_back(size2(0xB0, 0xB4));
	animated_ranges[5].push_back(size2(0xB5, 0xBF));
	
	animated_ranges[13].push_back(size2(0xB0, 0xB3));
	animated_ranges[13].push_back(size2(0xB4, 0xBF));

	animated_ranges[14].push_back(size2(0x58, 0x5F));

	animated_ranges[24].push_back(size2(0xB0, 0xB3));
	animated_ranges[24].push_back(size2(0xB4, 0xBF));
	
	animated_ranges[25].push_back(size2(0xB4, 0xB7));
	animated_ranges[25].push_back(size2(0xB8, 0xBB));
	animated_ranges[25].push_back(size2(0xBC, 0xBF));

	//animated_ranges[30].push_back(size2(0x10, 0x4F)); // don't use, will crash! (it's only used on a debug/test map anyways)

	animated_ranges[50].push_back(size2(0xB0, 0xB3));
	animated_ranges[50].push_back(size2(0xB4, 0xBF));
}

/*! palette destructor
 */
pal::~pal() {
	for(vector<unsigned int*>::iterator piter = palettes.begin(); piter != palettes.end(); piter++) {
		delete [] (*piter);
	}
	palettes.clear();
}

const unsigned int* pal::get_palette(const size_t& num) const {
	if(num >= palettes.size()) {
		a2e_error("invalid palette number %d!", num);
		return NULL;
	}
	return palettes[num];
}

const vector<size2>& pal::get_animated_ranges(const size_t& num) const {
	if(num >= palettes.size()) {
		a2e_error("invalid palette number %d!", num);
		return animated_ranges[0];
	}
	return animated_ranges[num];
}
