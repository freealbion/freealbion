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

#include "palette.h"

/*! palette constructor
 */
pal::pal() {
	// load palette 000
	fio->open_file(xld::make_xld_path("PALETTE.000").c_str(), file_io::OT_READ_BINARY);
	unsigned char* palette_000 = new unsigned char[192];
	fio->get_block((char*)palette_000, 192);
	fio->close_file();

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
}

/*! palette destructor
 */
pal::~pal() {
	for(vector<unsigned int*>::iterator piter = palettes.begin(); piter != palettes.end(); piter++) {
		delete [] (*piter);
	}
	palettes.clear();
}

const unsigned int* const pal::get_palette(const size_t& num) const {
	if(num >= palettes.size()) {
		a2e_error("invalid palette number %d!", num);
		return NULL;
	}
	return palettes[num];
}
