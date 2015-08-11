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

#include "events/clock.hpp"
#include "conf.hpp"
#include <SDL2/SDL.h>

/*! clock constructor
 */
ar_clock::ar_clock() : ms_per_tick(400), last_tick(SDL_GetTicks()),
ticks(conf::get<size_t>("map.hour") * AR_TICKS_PER_HOUR) {
	// 2d map: 395-416.6ms per tick -> use 400ms
	// fast: 20ms
}

/*! clock destructor
 */
ar_clock::~ar_clock() {
}

void ar_clock::run() {
	const size_t cur_ticks = SDL_GetTicks();
	if(cur_ticks - last_tick >= ms_per_tick) {
		last_tick = cur_ticks;
		ticks++;
		
		CLOCK_CB_TYPE tick_type = CCBT_TICK;
		
		// check for hour and day
		if(ticks % AR_TICKS_PER_HOUR == 0) {
			tick_type = CCBT_HOUR;
		}
		if(ticks == AR_TICKS_PER_DAY) {
			ticks = 0; // start again from 0
			tick_type = CCBT_DAY;
		}
		
		// and call all callbacks (note: upper_bound gives us an iterator to the first elem of the next ccbt type => end iter)
		multimap<CLOCK_CB_TYPE, clock_callback*>::const_iterator cb_iter = callbacks.begin(), end_iter = callbacks.upper_bound(tick_type);
		for(; cb_iter != end_iter; cb_iter++) {
			(*cb_iter->second)(ticks);
		}
	}
}

void ar_clock::set_ms_per_tick(const size_t& ms_per_tick_) {
	ar_clock::ms_per_tick = ms_per_tick_;
	ticks = 0;
	last_tick = SDL_GetTicks();
}

size_t ar_clock::get_ms_per_tick() const {
	return ms_per_tick;
}

void ar_clock::add_tick_callback(const CLOCK_CB_TYPE& cb_type, clock_callback& cb) {
	callbacks.insert(make_pair(cb_type, &cb));
}

void ar_clock::delete_tick_callback(const clock_callback& cb) {
	for(multimap<CLOCK_CB_TYPE, clock_callback*>::iterator cb_iter = callbacks.begin(); cb_iter != callbacks.end(); cb_iter++) {
		if(cb_iter->second == &cb) {
			callbacks.erase(cb_iter);
			return;
		}
	}
}

void ar_clock::set_ticks(const size_t& ticks_) {
	ticks = ticks_;
}

size_t ar_clock::get_ticks() const {
	return ticks;
}
