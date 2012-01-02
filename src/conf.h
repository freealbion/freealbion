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

#ifndef __AR_CONF_H__
#define __AR_CONF_H__

#include "ar_global.h"

class conf {
protected:
	template <typename T> struct setting {
	public:
		T* value;
#if DEBUG
		const char* type_name;
#endif
		
		static void* create(const T& val) {
#ifndef DEBUG
			void* s_ = malloc(sizeof(T*));
#else
			void* s_ = malloc(sizeof(T*) + sizeof(const char*));
			((setting<T>*)s_)->type_name = typeid(T).name();
#endif
			((setting<T>*)s_)->value = new T();
			*((setting<T>*)s_)->value = val;
			return s_;
		}
		
		const T& get() {
			return *value;
		}
		
		void set(const T& val) {
			*value = val;
		}
	};
	static map<string, void*> settings;
	
public:
	
	//
	template <typename T> static bool add(const string& name, const T& value);
	template <typename T> static void get(const string& name, T& dst);
	template <typename T> static const T& get(const string& name);
	template <typename T> static void set(const string& name, const T& value);
	
	static void clear() {
		for(map<string, void*>::iterator siter = settings.begin(); siter != settings.end(); siter++) {
			free(siter->second);
		}
	}
	
	static void init();
	
protected:
	// don't allow construction or copying of a conf object
	conf(const conf& c);
	conf& operator=(const conf& c);
};

#endif
