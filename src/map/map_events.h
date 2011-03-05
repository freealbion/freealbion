/*
 *  Albion Remake
 *  Copyright (C) 2007 - 2011 Florian Ziesche
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

#ifndef __AR_MAP_EVENTS_H__
#define __AR_MAP_EVENTS_H__

#include "ar_global.h"
#include "map_defines.h"
#include "xld.h"
#include "events.h"

class map_events {
public:
	map_events();
	~map_events();
	
	struct map_event_info {
		bool global;
		unsigned int xpos;
		unsigned int ypos;
		unsigned int trigger;
		unsigned int event_num;
		events::event* event_obj;
	};
	
	void load(const xld::xld_object* object, const size_t& data_offset, const size2& map_size);
	void unload();

	const size_t get_event_count() const;
	const size_t get_event_info_count() const;
	events::event* get_event(const size_t& num) const;
	map_event_info* get_event_info(const size_t& num) const;
	
	const size_t& get_end_offset() const;
	
protected:
	vector<map_event_info*> event_info;
	vector<events::event*> events;
	
	size_t end_offset;
	
};

#endif
