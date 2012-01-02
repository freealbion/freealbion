/*
 *  Albion Remake
 *  Copyright (C) 2007 - 2012 Florian Ziesche
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

#ifndef __AR_EVENTS_H__
#define __AR_EVENTS_H__

#include "ar_global.h"
#include "map_defines.h"
#include "xld.h"

class events {
public:
	events();
	~events();
	
	enum EVENT_TRIGGER {
		ETR_NORMAL = 0x01,
		ETR_EXAMINE = 0x02,
		ETR_TOUCH = 0x04,
		ETR_SPEAK = 0x08,
		ETR_USE_ITEM = 0x10,
		ETR_MAP_INIT = 0x20,
		ETR_EVERY_STEP = 0x40,
		ETR_EVERY_HOUR = 0x80,
		ETR_EVERY_DAY = 0x100,
		ETR_DEFAULT = 0x200,
		ETR_ACTION = 0x400,
		ETR_NPC = 0x800,
		ETR_TAKE = 0x1000
	};
	
	enum EVENT_TYPE {
		ETY_SCRIPT,
		ETY_MAP_EXIT,
		ETY_DOOR,
		ETY_CHEST,
		ETY_TEXT,
		ETY_SPINNER,
		ETY_TRAP,
		ETY_CHANGE_USED_ITEM,
		ETY_DATACHANGE,
		ETY_CHANGE_ICON,
		ETY_ENCOUNTER,
		ETY_PLACE_ACTION,
		ETY_QUERY,
		ETY_MODIFY,
		ETY_ACTION,
		ETY_SIGNAL,
		ETY_CLONE_AUTOMAP,
		ETY_SOUND,
		ETY_START_DIALOGUE,
		ETY_CREATE_TRANSPORT,
		ETY_EXECUTE,
		ETY_REMOVE_PARTY_MEMBER,
		ETY_END_DIALOGUE,
		ETY_WIPE,
		ETY_PLAY_ANIMATION,
		ETY_OFFSET,
		ETY_PAUSE,
		ETY_SIMPLE_CHEST,
		ETY_ASK_SURRENDER,
		ETY_DO_SCRIPT
	};
	
	// event structs
	struct event {
		EVENT_TYPE type;
		bool assigned;
		unsigned int info[7];
		unsigned int next_event_num;
		event* next_event;
	};
	struct query_event : event {
		event* next_query_event; // info[6]
	};
	struct map_exit_event : event {
		unsigned int map_x; // info[0]
		unsigned int map_y; // info[1]
		MOVE_DIRECTION direction; // info[2]
		unsigned int next_map; // info[5]
	};
	
	static event* create_event(const EVENT_TYPE& type);
	
	static void assign_type_data(vector<events::event*>& evts);
	
	static string event_type_to_str(const EVENT_TYPE& type);
	static string event_trigger_to_str(const size_t& trigger);
	
	static event* dbg_print_event_info(event* evt_obj, unsigned int num, unsigned int depth);
	static void dbg_print_query_event_info(event* evt_obj, unsigned int num, set<events::event*>& handled_queries, unsigned int depth);
	
protected:
	
};

#endif
