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

#include "object_light.hpp"
#include "conf.hpp"
#include "map/map_defines.hpp"
#include "map/npc.hpp"
#include "map/3d/npc3d.hpp"

//// object light base functions
void object_light_base::handle() {
	if(n == nullptr || l == nullptr) return;
	
	npc3d* n3d = (npc3d*)n;
	const float y_size = n3d->get_y_scale()/32.0f;
	float3 offset = n3d->get_offset()/32.0f;
	offset.y *= -1.0f;
	offset.z += y_size;
	
	const float2 interp_pos = n->get_interpolated_pos();
	position.set(interp_pos.x * std_tile_size + offset.x,
				 offset.z,
				 offset.y + (interp_pos.y+1.0f) * std_tile_size);
	original.position = position;
	l->set_position(position);
}

void object_light_base::track(npc* n_) {
	if(n != nullptr) {
		((npc3d*)n)->set_object_light(nullptr);
	}
	n = n_;
	((npc3d*)n)->set_object_light(this);
}

//// object light type specific functions
#define __OBJECT_LIGHT_CASE_CREATE(type) \
case object_light_type::type: return new object_light<object_light_type::type>(position);

object_light_base* object_light_base::create(const object_light_type& type, const float3& position) {
	switch(type) {
		__OBJECT_LIGHT_TYPE_LIST(__OBJECT_LIGHT_CASE_CREATE)
	}
	log_error("unknown object light type: %u!", (unsigned int)type);
	return new object_light<object_light_type::STREET_LAMP>(position);
}

template <> object_light<object_light_type::STREET_LAMP>::object_light(const float3& position_) : object_light_base(position_, float3(0.9f, 0.9f, 0.1f), std_tile_size*5.0f) {
}
template <> void object_light<object_light_type::STREET_LAMP>::animate(const size_t& time floor_unused) {
}

template <> object_light<object_light_type::GLOWING_LAMP>::object_light(const float3& position_) : object_light_base(position_, float3(215.0f, 255.0f, 176.0f)/255.0f, std_tile_size*4.0f) {
}
template <> void object_light<object_light_type::GLOWING_LAMP>::animate(const size_t& time) {
	static const unsigned int step_count = 2000;
	const float2 minmax_radius(std_tile_size*3.0f, std_tile_size*4.0f);
	const float radius_diff = minmax_radius.y - minmax_radius.x;
	
	step += time;
	if(step >= step_count) {
		radius = (direction > 0.0f ? minmax_radius.x : minmax_radius.y);
		step %= step_count;
		direction *= -1.0f;
	}
	
	radius = (float(step) / float(step_count)) * radius_diff;
	if(direction > 0.0f) radius = minmax_radius.y - radius;
	else radius = minmax_radius.x + radius;
	
	l->set_radius(radius);
}

template <> object_light<object_light_type::TORCH>::object_light(const float3& position_) : object_light_base(position_, float3(1.0f), std_tile_size*8.0f) {
}
template <> void object_light<object_light_type::TORCH>::animate(const size_t& time floor_unused) {
	// TODO: flickering/light color dependent on wether it's a torch or a light staff
}

template <> object_light<object_light_type::FIRE>::object_light(const float3& position_) : object_light_base(position_, float3(222, 76, 11)/255.0f, std_tile_size*3.0f) {
	// TODO
}
template <> void object_light<object_light_type::FIRE>::animate(const size_t& time floor_unused) {
	// TODO
}

template <> object_light<object_light_type::BLUE_FIRE>::object_light(const float3& position_) : object_light_base(position_, float3(1.0f), std_tile_size*1.0f) {
	// TODO
}
template <> void object_light<object_light_type::BLUE_FIRE>::animate(const size_t& time floor_unused) {
	// TODO
}

template <> object_light<object_light_type::GREEN_FIRE>::object_light(const float3& position_) : object_light_base(position_, float3(1.0f), std_tile_size*1.0f) {
	// TODO
}
template <> void object_light<object_light_type::GREEN_FIRE>::animate(const size_t& time floor_unused) {
	// TODO
}

template <> object_light<object_light_type::PILLAR>::object_light(const float3& position_) : object_light_base(position_, float3(1.0f), std_tile_size*1.0f) {
	// TODO
}
template <> void object_light<object_light_type::PILLAR>::animate(const size_t& time floor_unused) {
	// TODO
}

template <> object_light<object_light_type::FIREFLY>::object_light(const float3& position_) : object_light_base(position_, float3(0.9f, 0.9f, 0.1f), std_tile_size*2.0f) {
}
template <> void object_light<object_light_type::FIREFLY>::animate(const size_t& time floor_unused) {
	// TODO: glow or not?
}

template <> object_light<object_light_type::FEAR>::object_light(const float3& position_) : object_light_base(position_, float3(1.0f), std_tile_size*1.0f) {
	// TODO
}
template <> void object_light<object_light_type::FEAR>::animate(const size_t& time floor_unused) {
	// TODO
}

template <> object_light<object_light_type::SERVICE_BOT>::object_light(const float3& position_) : object_light_base(position_, float3(1.0f), std_tile_size*1.0f) {
	// TODO
}
template <> void object_light<object_light_type::SERVICE_BOT>::animate(const size_t& time floor_unused) {
	// TODO
}

template <> object_light<object_light_type::STORM>::object_light(const float3& position_) : object_light_base(position_, float3(1.0f), std_tile_size*1.0f) {
	// TODO
}
template <> void object_light<object_light_type::STORM>::animate(const size_t& time floor_unused) {
	// TODO
}

template <> object_light<object_light_type::ARGIM>::object_light(const float3& position_) : object_light_base(position_, float3(81, 135, 102)/255.0f, std_tile_size*8.0f) {
}
template <> void object_light<object_light_type::ARGIM>::animate(const size_t& time floor_unused) {
}

template <> object_light<object_light_type::GLOWING_GRABBER>::object_light(const float3& position_) : object_light_base(position_, float3(0.0f), std_tile_size*1.5f) {
}
template <> void object_light<object_light_type::GLOWING_GRABBER>::animate(const size_t& time) {
	static const unsigned int step_count = 300; // i should put an epilepsy warning somewhere
	static const float3 colors[] = {
		float3(239.0f, 206.0f, 77.0f)/255.0f,
		float3(177.0f, 129.0f, 85.0f)/255.0f,
		float3(173.0f, 73.0f, 1.0f)/255.0f,
	};
	
	step += time;
	if(step >= step_count) {
		step %= step_count;
	}
	l->set_color(colors[step/100]);
}

template <> object_light<object_light_type::LIVING_WALL>::object_light(const float3& position_) : object_light_base(position_, float3(0.9f, 0.9f, 0.1f), std_tile_size*4.0f) {
}
template <> void object_light<object_light_type::LIVING_WALL>::animate(const size_t& time) {
	static const unsigned int step_count = 80;
	static const float deviation = 0.3f;
	static const float step_size = 0.04f;
	
	step += time;
	if(step >= step_count) {
		step %= step_count;
		
		float3 cur_color = l->get_color();
		cur_color.r += step_size * (float(rand()%2) - 0.5f) * 2.0f;
		cur_color.g += step_size * (float(rand()%2) - 0.5f) * 2.0f;
		cur_color.r = const_math::clamp(cur_color.r, original.color.r - deviation, 1.0f);
		cur_color.g = const_math::clamp(cur_color.g, original.color.g - deviation, 1.0f);
		l->set_color(cur_color);
	}
}

#define __OBJECT_LIGHT_TEMPLATE_INSTANTIATION(light_type) template class object_light<object_light_type::light_type>;
__OBJECT_LIGHT_TYPE_LIST(__OBJECT_LIGHT_TEMPLATE_INSTANTIATION)
