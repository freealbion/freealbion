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

#ifndef __AR_MAP_EVENTS_HPP__
#define __AR_MAP_EVENTS_HPP__

#include "ar_global.hpp"
#include "map/map_defines.hpp"
#include "events/events.hpp"

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
	
	void load(const unsigned char* data, const size_t& data_size,
			  const size_t& data_offset, const size2& map_size);
	void unload();

	size_t get_event_count() const;
	size_t get_event_info_count() const;
	events::event* get_event(const size_t& num) const;
	map_event_info* get_event_info(const size_t& num) const;
	
	const size_t& get_end_offset() const;
	
	const events::map_exit_event* get_map_exit_event(const size2& position) const;
	
protected:
	vector<map_event_info*> event_info;
	vector<events::event*> mevents;
	
	size_t end_offset;
	
};

#endif
