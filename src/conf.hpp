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

#ifndef __AR_CONF_HPP__
#define __AR_CONF_HPP__

#include <core/cpp_headers.hpp>

// valid conf types
#define AR_CONF_TYPES(F) \
F(string, conf::CONF_TYPE::STRING) \
F(bool, conf::CONF_TYPE::BOOL) \
F(float, conf::CONF_TYPE::FLOAT) \
F(float2, conf::CONF_TYPE::FLOAT2) \
F(float3, conf::CONF_TYPE::FLOAT3) \
F(float4, conf::CONF_TYPE::FLOAT4) \
F(size_t, conf::CONF_TYPE::SIZE_T) \
F(size2, conf::CONF_TYPE::SIZE2) \
F(size3, conf::CONF_TYPE::SIZE3) \
F(size4, conf::CONF_TYPE::SIZE4) \
F(ssize_t, conf::CONF_TYPE::SSIZE_T) \
F(ssize2, conf::CONF_TYPE::SSIZE2) \
F(ssize3, conf::CONF_TYPE::SSIZE3) \
F(ssize4, conf::CONF_TYPE::SSIZE4) \
F(scaling::SCALE_TYPE, conf::CONF_TYPE::SCALE_TYPE) \
F(a2e_texture, conf::CONF_TYPE::A2E_TEXTURE)

#define AR_CONF_TYPE_TO_STRING_CHECK(type_, enum_type_) (type == enum_type_) ? #type_ :

class conf {
public:
	enum class CONF_TYPE : unsigned int {
		STRING,
		BOOL,
		FLOAT,
		FLOAT2,
		FLOAT3,
		FLOAT4,
		SIZE_T,
		SIZE2,
		SIZE3,
		SIZE4,
		SSIZE_T,
		SSIZE2,
		SSIZE3,
		SSIZE4,
		SCALE_TYPE,
		A2E_TEXTURE,
	};
	static constexpr const char* conf_type_to_string(const CONF_TYPE type) {
		// magic aka abusing macros ;)
		return AR_CONF_TYPES(AR_CONF_TYPE_TO_STRING_CHECK) "unknown";
	}
	
protected:
	template <typename T> struct setting {
	public:
		T value;
		std::function<void(const T&)> call_on_set;
#if defined(DEBUG)
		const string type_name;
#endif
		
		static void* create(const T& val, decltype(call_on_set) func) {
			setting<T>* s_ = new setting<T> {
				val,
				func,
#if defined(DEBUG)
				typeid(T).name(),
#endif
			};
			return s_;
		}
		
		const T& get() {
			return value;
		}
		
		void set(const T& val) {
			value = val;
			call_on_set(val);
		}
	};
	static unordered_map<string, pair<CONF_TYPE, void*>> settings;
	
public:
	// don't allow construction or copying of a conf object
	conf(const conf& c) = delete;
	conf& operator=(const conf& c) = delete;
	
	//
	template <typename T> static bool add(const string& name, const T& value);
	template <typename T> static bool add(const string& name, const T& value, decltype(setting<T>::call_on_set) call_on_set);
	template <typename T> static void get(const string& name, T& dst);
	template <typename T> static const T& get(const string& name);
	template <typename T> static void set(const string& name, const T& value);
	
	static void clear() {
		for(const auto& st : settings) {
			free(st.second.second);
		}
	}
	
	static void init();
	
	static const decltype(settings)& get_settings() {
		return settings;
	}
	
};

#endif
