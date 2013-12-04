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

#include "events.hpp"

/*! events constructor
 */
events::events() {
}

/*! events destructor
 */
events::~events() {
}

events::event* events::dbg_print_event_info(events::event* evt_obj, unsigned int num, unsigned int depth) {
	if(evt_obj == nullptr) return nullptr;
	
	for(unsigned int i = 0; i < (depth+1); i++) {
		cout << "\t";
	}
	cout << event_type_to_str(evt_obj->type) << " #" << num << (evt_obj->assigned ? "" : " (unassigned)") << ":";
	string separator = " ";
	for(unsigned int i = 0; i < 7; i++) {
		cout << separator << evt_obj->info[i];
		separator = ", ";
	}
	cout << endl;
	
	return evt_obj->next_event;
}

void events::dbg_print_query_event_info(events::event* evt_obj, unsigned int num, set<events::event*>& handled_queries, unsigned int depth) {
	while(evt_obj != nullptr) {
		dbg_print_event_info(evt_obj, num, depth);
		if(evt_obj->type == events::ETY_QUERY) {
			if(handled_queries.count(evt_obj) == 0) {
				handled_queries.insert(evt_obj);
				if(((query_event*)evt_obj)->next_query_event != nullptr) {
					dbg_print_query_event_info(((query_event*)evt_obj)->next_query_event, evt_obj->info[6], handled_queries, depth+1);
				}
			}
			else {
				for(unsigned int i = 0; i < (depth+2); i++) {
					cout << "\t";
				}
				cout << "<...>" << endl;
			}
		}
		
		num = evt_obj->next_event_num;
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

events::event* events::create_event(const events::EVENT_TYPE& type) {
	switch(type) {
		case ETY_SCRIPT:
		case ETY_DOOR:
		case ETY_CHEST:
		case ETY_TEXT:
		case ETY_SPINNER:
		case ETY_TRAP:
		case ETY_CHANGE_USED_ITEM:
		case ETY_DATACHANGE:
		case ETY_CHANGE_ICON:
		case ETY_ENCOUNTER:
		case ETY_PLACE_ACTION:
		case ETY_MODIFY:
		case ETY_ACTION:
		case ETY_SIGNAL:
		case ETY_CLONE_AUTOMAP:
		case ETY_SOUND:
		case ETY_START_DIALOGUE:
		case ETY_CREATE_TRANSPORT:
		case ETY_EXECUTE:
		case ETY_REMOVE_PARTY_MEMBER:
		case ETY_END_DIALOGUE:
		case ETY_WIPE:
		case ETY_PLAY_ANIMATION:
		case ETY_OFFSET:
		case ETY_PAUSE:
		case ETY_SIMPLE_CHEST:
		case ETY_ASK_SURRENDER:
		case ETY_DO_SCRIPT:
			return new event();
		case ETY_QUERY:
			return new query_event();
		case ETY_MAP_EXIT:
			return new map_exit_event();
	}
	log_error("unknown event type: %i", type);
	return new event();
}

void events::assign_type_data(vector<events::event*>& evts) {
	// assign event type dependent data
	for(auto& evt : evts) {
		switch(evt->type) {
			case events::ETY_QUERY:
				((events::query_event*)evt)->next_query_event = (evt->info[6] < evts.size() ? evts[evt->info[6]] : nullptr);
				break;
			case events::ETY_MAP_EXIT: {
				events::map_exit_event* mevt = (events::map_exit_event*)evt;
				mevt->map_x = evt->info[0];
				mevt->map_y = evt->info[1];
				switch(evt->info[2]) {
					case 0: mevt->direction = MD_UP; break;
					case 1: mevt->direction = MD_RIGHT; break;
					case 2: mevt->direction = MD_DOWN; break;
					case 3: mevt->direction = MD_LEFT; break;
					default: mevt->direction = MD_NONE; break;
				}
				mevt->next_map = evt->info[5];
			}
			break;
			default:
				break;
		}
	}
}
