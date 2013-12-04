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

#ifndef __AR_MAP_NPCS_H__
#define __AR_MAP_NPCS_H__

#include "ar_global.h"
#include "map_defines.h"
#include "xld.h"

class map_npcs {
public:
	map_npcs();
	~map_npcs();
	
	struct map_npc {
		unsigned int npc_num;
		unsigned int object_num;
		unsigned int event_num; //?
		MOVEMENT_TYPE movement_type;
		
		size2* position;
	};
	
	void load(const unsigned char* data, const size_t& event_end_offset);
	const vector<map_npc*>& get_npcs() const;
	
protected:
	vector<map_npc*> npcs;
	
};

#endif
