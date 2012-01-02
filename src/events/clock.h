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

#ifndef __AR_CLOCK_H__
#define __AR_CLOCK_H__

#include <core/functor.h>
#include <map>
using namespace std;

#define AR_TICKS_PER_HOUR 48
#define AR_TICKS_PER_DAY (24*AR_TICKS_PER_HOUR)

typedef functor<void, size_t> clock_callback;

class ar_clock {
public:
	ar_clock();
	~ar_clock();
	
	void run();
	
	void set_ms_per_tick(const size_t& ms_per_tick);
	size_t get_ms_per_tick() const;
	void set_ticks(const size_t& ticks);
	size_t get_ticks() const;
	
	enum CLOCK_CB_TYPE {
		CCBT_TICK,
		CCBT_HOUR,
		CCBT_DAY,
	};
	void add_tick_callback(const CLOCK_CB_TYPE& cb_type, clock_callback& cb);
	void delete_tick_callback(const clock_callback& cb);
	
	//void set_clock_mode(const CLOCK_MODE mode);
	
protected:
	size_t ms_per_tick;
	size_t last_tick;
	size_t ticks;
	
	multimap<CLOCK_CB_TYPE, clock_callback*> callbacks;
	
};

#endif
