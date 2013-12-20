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

#include "map_npcs.hpp"

/*! map_npcs constructor
 */
map_npcs::map_npcs() {
}

/*! map_npcs destructor
 */
map_npcs::~map_npcs() {
	for(vector<map_npc*>::iterator niter = npcs.begin(); niter != npcs.end(); niter++) {
		if((*niter)->movement_type == MOVEMENT_TYPE::RANDOM) delete (*niter)->position;
		else delete [] (*niter)->position;

		(*niter)->position = nullptr;
	}
	npcs.clear();
}

void map_npcs::load(const unsigned char* data, const size_t& event_end_offset) {
	const size_t npc_count = data[1];
	size_t offset = 10;
	for(size_t i = 0; i < npc_count; i++) {
		const unsigned int npc_num = data[offset]; offset++;
		if(npc_num == 0) {
			offset += 9;
			continue;
		}

		/*cout << "npc data: ";
		for(size_t x = 0; x < 10; x++) {
			cout << (size_t)data[offset-1+x] << ", ";
		}*/
		
		npcs.push_back(new map_npc());
		npcs.back()->npc_num = npc_num;
		offset++; // unknown
		npcs.back()->event_num = AR_GET_USINT(data, offset);
		offset += 2;
		npcs.back()->object_num = data[offset]; offset++;
		offset++; // unknown
		offset++; // unknown
		
		//cout << " mt: " << (size_t)data[offset];
		//cout << endl;
		if(data[offset] & 0x3) npcs.back()->movement_type = MOVEMENT_TYPE::RANDOM;
		else npcs.back()->movement_type = MOVEMENT_TYPE::TRACK;
		offset++;

		offset++; // unknown
		offset++; // unknown
	}
	
	offset = event_end_offset;
	for(vector<map_npc*>::iterator niter = npcs.begin(); niter != npcs.end(); niter++) {
		if((*niter)->movement_type == MOVEMENT_TYPE::RANDOM) {
			(*niter)->position = new size2();
			(*niter)->position->x = data[offset]; offset++;
			(*niter)->position->y = data[offset]; offset++;
		}
		else { // MOVEMENT_TYPE::TRACK
			(*niter)->position = new size2[0x480];
			for(size_t i = 0; i < 0x480; i++) {
				(*niter)->position[i].set(data[offset], data[offset+1]);
				offset+=2;
			}
		}
		
		cout << "NPC @" << hex << offset << dec << ": " << (*niter)->npc_num << ", " << (*niter)->object_num << ", " << (*niter)->event_num << ", " << (size_t)npcs.back()->movement_type << ", " << *(*niter)->position << endl;
	}
}

const vector<map_npcs::map_npc*>& map_npcs::get_npcs() const {
	return npcs;
}
