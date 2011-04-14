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

#include "conf.h"
#include "scaling.h"

//
map<string, void*> conf::settings;

// valid conf types
#define CONF_TYPES(F) \
F(string) \
F(bool) \
F(size_t) \
F(ssize_t) \
F(float) \
F(float3) \
F(a2e_texture) \
F(scaling::SCALE_TYPE)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// define add, get and set functions for all valid conf types

//// get
#ifndef DEBUG
// non-debug version
#define CONF_DEFINE_GET_FUNCS(type) \
template <> void conf::get<type>(const string& name, type& dst) { dst = ((setting<type>*)settings[name])->get(); }	\
template <> const type& conf::get<type>(const string& name) { return ((setting<type>*)settings[name])->get(); }

#else
// debug version with run-time type checking
#define CONF_DEFINE_GET_FUNCS(type) \
template <> void conf::get<type>(const string& name, type& dst) {													\
	if(strcmp(((setting<type>*)settings[name])->type_name, typeid(type).name()) != 0) {								\
		cout << "ERROR: get(): invalid type for conf setting \"" << name << "\" - used: " << typeid(type).name()	\
		<< ", expected: " << ((setting<type>*)settings[name])->type_name << "!" << endl;							\
	}																												\
	dst = ((setting<type>*)settings[name])->get();																	\
}																													\
\
template <> const type& conf::get<type>(const string& name) {														\
	if(strcmp(((setting<type>*)settings[name])->type_name, typeid(type).name()) != 0) {								\
		cout << "ERROR: get(): invalid type for conf setting \"" << name << "\" - used: " << typeid(type).name()	\
		<< ", expected: " << ((setting<type>*)settings[name])->type_name << "!" << endl;							\
	}																												\
	return ((setting<type>*)settings[name])->get();																	\
}

#endif

CONF_TYPES(CONF_DEFINE_GET_FUNCS)

//// set
#ifndef DEBUG
// non-debug version
#define CONF_DEFINE_SET_FUNCS(type) \
template <> void conf::set<type>(const string& name, const type& value) {											\
	((setting<type>*)settings[name])->set(value);																	\
}
#else
// debug version with run-time type checking
#define CONF_DEFINE_SET_FUNCS(type) \
template <> void conf::set<type>(const string& name, const type& value) {											\
	if(strcmp(((setting<type>*)settings[name])->type_name, typeid(type).name()) != 0) {								\
		cout << "ERROR: set(): invalid type for conf setting \"" << name << "\" - used: " << typeid(type).name()	\
		<< ", expected: " << ((setting<type>*)settings[name])->type_name << "!" << endl;							\
	}																												\
	((setting<type>*)settings[name])->set(value);																	\
}
#endif

CONF_TYPES(CONF_DEFINE_SET_FUNCS)

//// add
#define CONF_DEFINE_ADD_FUNCS(type)																					\
template <> bool conf::add<type>(const string& name, const type& value) {											\
	/* check if a setting with such a name already exists */														\
	if(settings.count(name) > 0) {																					\
		cout << __func__ << ": a setting with the name \"" << name << "\" already exists!" << endl;					\
		return false;																								\
	}																												\
																													\
	/* create setting */																							\
	settings[name] = setting<type>::create(value);																	\
	return true;																									\
}
CONF_TYPES(CONF_DEFINE_ADD_FUNCS)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 

void conf::init() {
	// add misc conf settings
	conf::add<bool>("map.collision", true);
	conf::add<bool>("map.draw_overlay", true);
	conf::add<bool>("map.draw_underlay", true);
	conf::add<float>("map.scale", 2.0f); // original scale: 3.0f
	conf::add<scaling::SCALE_TYPE>("map.2d.scale_type", scaling::ST_HQ4X);
	//conf::add<scaling::SCALE_TYPE>("map.3d.scale_type", scaling::ST_HQ4X);
	conf::add<scaling::SCALE_TYPE>("map.3d.scale_type", scaling::ST_NEAREST_1X);
	
	conf::add<bool>("ui.display", true);
	
	conf::add<bool>("debug.player_pos", false);
	conf::add<bool>("debug.draw_events", false);
	conf::add<bool>("debug.display_debug_texture", false);
	conf::add<size_t>("debug.npcgfx", 200);
	conf::add<bool>("debug.free_cam", true);
}
