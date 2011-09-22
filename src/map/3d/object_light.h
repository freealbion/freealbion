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

#ifndef __AR_OBJECT_LIGHT_H__
#define __AR_OBJECT_LIGHT_H__

#include <a2e.h>
#include "ar_global.h"

enum class object_light_type {
	STREET_LAMP,
	GLOWING_LAMP,
	TORCH,
	FIRE,
	BLUE_FIRE,
	GREEN_FIRE,
	PILLAR,
	FIREFLY,
	FEAR,
	SERVICE_BOT,
	STORM,
	ARGIM,
	GLOWING_GRABBER,
	LIVING_WALL,
	// TODO: more
};

// more voodoo, less typing ;)
#define __OBJECT_LIGHT_TYPE_LIST(F) \
F(STREET_LAMP) \
F(GLOWING_LAMP) \
F(TORCH) \
F(FIRE) \
F(BLUE_FIRE) \
F(GREEN_FIRE) \
F(PILLAR) \
F(FIREFLY) \
F(FEAR) \
F(SERVICE_BOT) \
F(STORM) \
F(ARGIM) \
F(GLOWING_GRABBER) \
F(LIVING_WALL)

class npc;
class object_light_base {
public:
	object_light_base(const float3& position_, const float3& color_, const float& radius_) : position(position_), color(color_), radius(radius_), step(rand()%10000), direction(1.0f), l(new light(e, position_.x, position_.y, position_.z)), n(NULL) {
		l->set_color(color);
		l->set_radius(radius);
		sce->add_light(l);
		
		original.position.set(position);
		original.color.set(color);
		original.radius = radius;
	}
	virtual ~object_light_base() {
		if(l != NULL) {
			sce->delete_light(l);
			delete l;
		}
	}
	
	static object_light_base* create(const object_light_type& type, const float3& position);
	
	virtual void animate(const size_t& time) = 0;
	virtual void handle();
	
	virtual void set_position(const float3& position_) {
		position = position_;
		original.position = position_;
		if(l != NULL) {
			l->set_position(position_);
		}
	}
	virtual void set_enabled(const bool& state) {
		if(l != NULL) {
			l->set_enabled(state);
		}
	}
	virtual void track(npc* n_);
	
protected:
	float3 position;
	float3 color;
	float radius;
	unsigned int step;
	float direction;
	light* l;
	npc* n;
	
	struct {
		float3 position;
		float3 color;
		float radius;
	} original;
	
};

template <object_light_type T = object_light_type::STREET_LAMP> class object_light : public object_light_base {
public:
	object_light(const float3& position_);
	virtual void animate(const size_t& time);
};

#define __OBJECT_LIGHT_SPECS_AND_EXTERN_TEMPLATE(type) \
template <> object_light<object_light_type::type>::object_light(const float3& position_); \
template <> void object_light<object_light_type::type>::animate(const size_t& time); \
extern template class object_light<object_light_type::type>;

__OBJECT_LIGHT_TYPE_LIST(__OBJECT_LIGHT_SPECS_AND_EXTERN_TEMPLATE)

#endif
