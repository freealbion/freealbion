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

#include "map_events.h"

/*! map_events constructor
 */
map_events::map_events() {
	end_offset = 0;
}

/*! map_events destructor
 */
map_events::~map_events() {
	unload();
}

void map_events::load(const xld::xld_object* object, const size_t& data_offset, const size2& map_size) {
	unload();
	
	const unsigned char* data = object->data;
	const size_t length = object->length;
	if(data_offset >= length) {
		a2e_error("offset is larger than file size!");
		return;
	}
	
	// get events
	size_t offset = data_offset;
	const size_t global_event_count = AR_GET_USINT(data, offset);
	offset += 2;
	if(global_event_count == 0xFFFF) {
		a2e_error("invalid global event count!");
		return; // sth is wrong, break
	}
	for(size_t i = 0; i < global_event_count; i++) {
		event_info.push_back(new map_event_info());
		event_info.back()->global = true;
		event_info.back()->xpos = AR_GET_USINT(data, offset);
		offset += 2;
		event_info.back()->ypos = 0;
		event_info.back()->trigger = AR_GET_USINT(data, offset);
		offset += 2;
		//if(event_info.back()->trigger > 0x1FFF) cout << "unknown event trigger!" << endl;
		event_info.back()->event_num = AR_GET_USINT(data, offset);
		offset += 2;
		event_info.back()->event_obj = NULL;
		if(offset >= length) break;
	}

	// for each (y) line
	const size_t y_event_count = map_size.y;
	if(y_event_count == 0xFFFF) {
		a2e_error("invalid event count (y map size)!");
		return; // sth is wrong, break
	}
	for(size_t i = 0; i < y_event_count; i++) {
		const size_t line_event_count = AR_GET_USINT(data, offset);
		offset += 2;
		if(offset >= length) break;
		if(line_event_count == 0xFFFF) {
			a2e_error("invalid row event count!");
			return; // sth is wrong, break
		}
		for(size_t j = 0; j < line_event_count; j++) {
			event_info.push_back(new map_event_info());
			event_info.back()->global = false;
			// pos starts @(0, 0)
			event_info.back()->xpos = AR_GET_USINT(data, offset)-1;
			offset += 2;
			event_info.back()->ypos = (unsigned int)i;
			event_info.back()->trigger = AR_GET_USINT(data, offset);
			offset += 2;
			//if(event_info.back()->trigger > 0x1FFF) cout << "unknown event trigger!" << endl;
			event_info.back()->event_num = AR_GET_USINT(data, offset);
			offset += 2;
			event_info.back()->event_obj = NULL;
		}
	}
	
	cout << "----------------------" << endl;
	cout << "-- Map Events:" << endl;
	cout << "event info count: " << event_info.size() << endl;
	
	// get event data
	if(offset < length) {
		const size_t event_count = AR_GET_USINT(data, offset);
		cout << "event count: " << event_count << endl;
		if(event_count == 0xFFFF) {
			a2e_error("invalid event count!");
			return; // sth is wrong, break
		}
		
		offset += 2;
		if(offset >= length) {
			a2e_error("invalid event data!");
			return; // sth is wrong, break
		}
		
		for(size_t i = 0; i < event_count; i++) {
			events::EVENT_TYPE type = (events::EVENT_TYPE)(data[offset] & 0xFF);
			mevents.push_back(events::create_event(type));
			events::event* cur_event = mevents.back();
			
			// initial data
			cur_event->assigned = false;
			cur_event->type = type; offset++;
			cur_event->info[0] = data[offset] & 0xFF; offset++;
			cur_event->info[1] = data[offset] & 0xFF; offset++;
			cur_event->info[2] = data[offset] & 0xFF; offset++;
			cur_event->info[3] = data[offset] & 0xFF; offset++;
			cur_event->info[4] = data[offset] & 0xFF; offset++;
			cur_event->info[5] = AR_GET_USINT(data, offset); offset += 2;
			cur_event->info[6] = AR_GET_USINT(data, offset); offset += 2;
			cur_event->next_event_num = AR_GET_USINT(data, offset); offset += 2;
			cur_event->next_event = NULL;
			
			if(offset >= length) break;
		}
	}
	else a2e_error("offset is larger than file size!");
	
	// assign pointers
	for(vector<map_event_info*>::iterator ei_iter = event_info.begin(); ei_iter != event_info.end(); ei_iter++) {
		(*ei_iter)->event_obj = get_event((*ei_iter)->event_num);
		if((*ei_iter)->event_obj != NULL) {
			(*ei_iter)->event_obj->assigned = true;
		}
	}
	for(vector<events::event*>::iterator e_iter = mevents.begin(); e_iter != mevents.end(); e_iter++) {
		if((*e_iter)->next_event_num != 0xFFFF) {
			(*e_iter)->next_event = get_event((*e_iter)->next_event_num);
		}
	}
	
	// assign event type dependent data
	events::assign_type_data(mevents);
	
	// handle assignment
	set<events::event*> handled_events;
	for(vector<events::event*>::iterator e_iter = mevents.begin(); e_iter != mevents.end(); e_iter++) {
		if((*e_iter)->assigned && (*e_iter)->next_event != NULL && handled_events.count(*e_iter) == 0) {
			handled_events.insert(*e_iter);
			
			deque<events::event*> evt_list;
			evt_list.push_back((*e_iter)->next_event);
			if((*e_iter)->type == events::ETY_QUERY) {
				events::event* next_event = ((events::query_event*)(*e_iter))->next_query_event;
				if(next_event != NULL) {
					evt_list.push_back(next_event);
				}
			}
			
			while(!evt_list.empty()) {
				events::event* cur_evt = evt_list[0];
				cur_evt->assigned = true;
				
				if(cur_evt->next_event != NULL && handled_events.count(cur_evt->next_event) == 0) {
					evt_list.push_back(cur_evt->next_event);
				}
				if(cur_evt->type == events::ETY_QUERY) {
					events::event* next_event = ((events::query_event*)cur_evt)->next_query_event;
					if(next_event != NULL && handled_events.count(next_event) == 0) {
						evt_list.push_back(next_event);
					}
				}
				
				handled_events.insert(cur_evt);
				evt_list.pop_front();
			}
		}
	}
	handled_events.clear();
	
	cout << "offset: " << offset << endl;
	end_offset = offset;
	
	// debug output
	/*size_t ix = 0;
	for(vector<map_event_info*>::iterator ei_iter = event_info.begin(); ei_iter != event_info.end(); ei_iter++) {
		cout << "#" << ix << " (Global: " << (*ei_iter)->global << ")" << endl;
		cout << "Pos: (" << (*ei_iter)->xpos << ", " << (*ei_iter)->ypos << ")" << endl;
		cout << "Trigger: " << events::event_trigger_to_str((*ei_iter)->trigger) << endl;
		cout << "Data:" << endl;
		events::event* evt_obj = (*ei_iter)->event_obj;
		unsigned int num = (*ei_iter)->event_num;
		while(evt_obj != NULL) {
			events::event* last_event = evt_obj;
			evt_obj = events::dbg_print_event_info(evt_obj, num, 0);
			num = last_event->next_event_num;
			if(last_event->type == events::ETY_QUERY) {
				set<events::event*> handled_queries;
				handled_queries.insert(last_event);
				events::dbg_print_query_event_info(((events::query_event*)last_event)->next_query_event, last_event->info[6], handled_queries, 1);
			}
		}
		ix++;
		cout << "----------------------" << endl;
	}
	
	cout << "unassigned events:" << endl;
	for(vector<events::event*>::iterator e_iter = mevents.begin(); e_iter != mevents.end(); e_iter++) {
		if(!(*e_iter)->assigned) {
			events::dbg_print_event_info(*e_iter, 0xFFFFFFFF, 0);
			cout << "----------------------" << endl;
		}
	}*/
}

void map_events::unload() {
	event_info.clear();
	mevents.clear();
	end_offset = 0;
}

size_t map_events::get_event_count() const {
	return mevents.size();
}

size_t map_events::get_event_info_count() const {
	return event_info.size();
}

events::event* map_events::get_event(const size_t& num) const {
	if(num >= mevents.size()) return NULL;
	return mevents[num];
}

map_events::map_event_info* map_events::get_event_info(const size_t& num) const {
	if(num >= event_info.size()) return NULL;
	return event_info[num];
}

const size_t& map_events::get_end_offset() const {
	return end_offset;
}

const events::map_exit_event* map_events::get_map_exit_event(const size2& position) const {
	for(auto evt = event_info.begin(); evt != event_info.end(); evt++) {
		if((*evt)->xpos == position.x && (*evt)->ypos == position.y && (*evt)->event_obj != NULL) {
			deque<events::event*> evt_list;
			evt_list.push_back((*evt)->event_obj);
			while(!evt_list.empty()) {
				events::event* cur_evt = evt_list[0];
				
				// ignore invalid (=0) map numbers (these are used for closed shops, etc.)
				if(cur_evt->type == events::ETY_MAP_EXIT && ((events::map_exit_event*)cur_evt)->next_map >= 100) {
					return (events::map_exit_event*)cur_evt;
				}
				else if(cur_evt->type == events::ETY_QUERY) {
					events::event* next_event = ((events::query_event*)cur_evt)->next_query_event;
					if(next_event != NULL) {
						evt_list.push_back(next_event);
					}
				}
				
				if(cur_evt->next_event != NULL) evt_list.push_back(cur_evt->next_event);
				
				evt_list.pop_front();
			}
		}
	}
	return NULL;
}
