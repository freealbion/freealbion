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

#include "events.h"

/*! map_events constructor
 */
events::events() {
}

/*! map_events destructor
 */
events::~events() {
}

events::event* events::dbg_print_event_info(events::event* evt_obj, unsigned int num) {
	if(evt_obj == NULL) return NULL;
	
	cout << "\t" << event_type_to_str(evt_obj->type) << (evt_obj->assigned ? "" : " (unassigned)") << ":";
	string separator = " ";
	for(unsigned int i = 0; i < 7; i++) {
		cout << separator << evt_obj->info[i];
		separator = ", ";
	}
	cout << endl;
	separator = " ";
	if(evt_obj->type == events::ETY_MAP_EXIT) {
		cout << "\t" << event_type_to_str(evt_obj->type) << (evt_obj->assigned ? "" : " (unassigned)") << " (#" << num << ")" << ": ";
		for(unsigned int i = 0; i < 7; i++) {
			cout << separator << evt_obj->info[i];
			separator = ", ";
		}
		cout << endl;
	}
	
	return evt_obj->next_event;
}

void events::dbg_print_query_event_info(events::event* evt_obj, unsigned int depth, unsigned int num) {
	while(evt_obj != NULL) {
		if(evt_obj->type == events::ETY_MAP_EXIT) {
			cout << "\t";
			for(unsigned int i = 0; i < depth; i++) cout << "\t";
			
			cout << event_type_to_str(evt_obj->type) << " (#" << num << ")" << ": ";
			
			for(unsigned int i = 0; i < 7; i++) {
				cout << evt_obj->info[i] << ", ";
			}
			cout << endl;
		}
		if(evt_obj->type == events::ETY_QUERY) {
			//dbg_print_query_event_info(get_event(evt_obj->info[6]), depth+1, evt_obj->info[6]);
		}
		
		//event = get_event(event->info[6]);
		evt_obj = evt_obj->next_event;
	}
}

string events::event_type_to_str(const EVENT_TYPE& type) {
	static const string type_strs[] = {
		"Script",
		"Map exit",
		"Door",
		"Chest",
		"Text",
		"Spinner",
		"Trap",
		"Change used item",
		"Datachange",
		"Change icon",
		"Encounter",
		"Place action",
		"Query",
		"Modify",
		"Action",
		"Signal",
		"Clone automap",
		"Sound",
		"Start dialogue",
		"Create transport",
		"Execute",
		"Remove party member",
		"End dialogue",
		"Wipe",
		"Play animation",
		"Offset",
		"Pause",
		"Simple chest",
		"Ask surrender",
		"Do script" };
	return type_strs[type];
}

string events::event_trigger_to_str(const size_t& trigger) {
	static const string trigger_strs[] = {
		"Normal",
		"Examine",
		"Touch",
		"Speak",
		"Use item",
		"Map init",
		"Every step",
		"Every hour",
		"Every day",
		"Default",
		"Action",
		"NPC",
		"Take" };
	string ostr = "";
	unsigned int n = 1;
	for(unsigned int i = 0; i < 13; i++) {
		if(n & trigger) {
			ostr += trigger_strs[i] + ", ";
		}
		n <<= 1;
	}
	return ostr;
}

