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

#include "conf.hpp"
#include "scaling.hpp"
#include <engine.hpp>
#include <rendering/texture_object.hpp>
#include <core/xml.hpp>

//
unordered_map<string, pair<conf::CONF_TYPE, void*>> conf::settings;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// define add, get and set functions for all valid conf types

//// get
#if !defined(DEBUG)
// non-debug version
#define CONF_DEFINE_GET_FUNCS(type, enum_type) \
template <> void conf::get<type>(const string& name, type& dst) { dst = ((setting<type>*)(settings[name].second))->get(); } \
template <> const type& conf::get<type>(const string& name) { return ((setting<type>*)(settings[name].second))->get(); }

#else
// debug version with run-time type checking
#define CONF_DEFINE_GET_FUNCS(type, enum_type) \
template <> void conf::get<type>(const string& name, type& dst) {													\
	if(((setting<type>*)(settings[name].second))->type_name != typeid(type).name()) {								\
		a2e_error("invalid type for conf setting \"%s\" - used: %s, expected: %s!",									\
				  name, typeid(type).name(), ((setting<type>*)(settings[name].second))->type_name);					\
	}																												\
	dst = ((setting<type>*)(settings[name].second))->get();															\
}																													\
\
template <> const type& conf::get<type>(const string& name) {														\
	if(((setting<type>*)(settings[name].second))->type_name != typeid(type).name()) {								\
		a2e_error("invalid type for conf setting \"%s\" - used: %s, expected: %s!",									\
				  name, typeid(type).name(), ((setting<type>*)(settings[name].second))->type_name);					\
	}																												\
	return ((setting<type>*)(settings[name].second))->get();														\
}

#endif

AR_CONF_TYPES(CONF_DEFINE_GET_FUNCS)

//// set
#if !defined(DEBUG)
// non-debug version
#define CONF_DEFINE_SET_FUNCS(type, enum_type) \
template <> void conf::set<type>(const string& name, const type& value) {											\
	((setting<type>*)(settings[name].second))->set(value);															\
}
#else
// debug version with run-time type checking
#define CONF_DEFINE_SET_FUNCS(type, enum_type) \
template <> void conf::set<type>(const string& name, const type& value) {											\
	if(((setting<type>*)(settings[name].second))->type_name != typeid(type).name()) {								\
		a2e_error("invalid type for conf setting \"%s\" - used: %s, expected: %s!",									\
				  name, typeid(type).name(), ((setting<type>*)(settings[name].second))->type_name);					\
	}																												\
	((setting<type>*)(settings[name].second))->set(value);															\
}
#endif

AR_CONF_TYPES(CONF_DEFINE_SET_FUNCS)

//// add
#define CONF_DEFINE_ADD_FUNCS(type, enum_type) \
template <> bool conf::add<type>(const string& name, const type& value,												\
								 decltype(setting<type>::call_on_set) call_on_set) {								\
	/* check if a setting with such a name already exists */														\
	if(settings.count(name) > 0) {																					\
		a2e_error("a setting with the name \"%s\" already exists!", name);											\
		return false;																								\
	}																												\
																													\
	/* create setting */																							\
	settings.insert(make_pair(name, make_pair(enum_type, (void*)setting<type>::create(value, call_on_set))));		\
	return true;																									\
}																													\
template <> bool conf::add<type>(const string& name, const type& value) {											\
	return conf::add<type>(name, value, [](const type& val a2e_unused){});														\
}
AR_CONF_TYPES(CONF_DEFINE_ADD_FUNCS)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 

scaling::SCALE_TYPE get_scale_type(const string& str);
scaling::SCALE_TYPE get_scale_type(const string& str) {
	if(str == "1x") return scaling::ST_NEAREST_1X;
	else if(str == "2x") return scaling::ST_NEAREST_2X;
	else if(str == "4x") return scaling::ST_NEAREST_4X;
	else if(str == "hq2x") return scaling::ST_HQ2X;
	else if(str == "hq4x") return scaling::ST_HQ4X;
	return scaling::ST_NEAREST_1X;
}

void conf::init() {
	const xml::xml_doc& config_doc = e->get_config_doc();
	
	// add misc conf settings
	conf::add<bool>("map.collision", config_doc.get<bool>("config.albion.map.collision", true));
	conf::add<bool>("map.draw_overlay", true);
	conf::add<bool>("map.draw_underlay", true);
	conf::add<float>("map.scale", 2.0f); // original scale: 3.0f
	conf::add<scaling::SCALE_TYPE>("map.2d.scale_type",
								   get_scale_type(config_doc.get<string>("config.albion.map.scale_2d",
																		 "hq4x")));
	conf::add<scaling::SCALE_TYPE>("map.3d.scale_type",
								   get_scale_type(config_doc.get<string>("config.albion.map.scale_3d",
																		 "hq4x")));
	conf::add<bool>("map.3d.object_lights", config_doc.get<bool>("config.albion.map.object_lights", true));
	conf::add<size_t>("map.hour", config_doc.get<size_t>("config.albion.map.hour", 8) % 24);
	
	conf::add<bool>("ui.display", true);
	
	// for misc debugging purposes:
	conf::add<bool>("debug.ui", false);
	conf::add<size_t>("debug.texture", 0); // actual GLuint
	conf::add<bool>("debug.show_texture", false);
	conf::add<bool>("debug.fps", false);
	conf::add<bool>("debug.osx", false);
	conf::add<bool>("debug.timer", false);
	conf::add<bool>("debug.player_pos", false);
	conf::add<bool>("debug.draw_events", false);
	conf::add<size_t>("debug.npcgfx", config_doc.get<size_t>("config.albion.debug.player_gfx", 200));
	conf::add<bool>("debug.free_cam", config_doc.get<bool>("config.albion.debug.free_cam", false));
}
