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
 
#ifndef __AR_AUDIO_HANDLER_HPP__
#define __AR_AUDIO_HANDLER_HPP__

#include "ar_global.hpp"

class audio_handler {
public:
	static void init();
	static void destroy();
	
	static void play_sample(const size_t& index);
	static void play_wavelib_sample(const size_t& wavelib_index, const size_t& sample_idx);
	
protected:
	static array<lazy_xld, 3> sample_xlds;
	static lazy_xld wavelib_xld;
	
	struct wavelib_entry {
		const uint32_t _unknown_0;
		const uint32_t index;
		const uint32_t _unknown_1;
		const uint32_t offset;
		const uint32_t length;
		const uint32_t _unknown_2;
		const uint32_t _unknown_3;
		const uint32_t frequency;
	};
	static constexpr size_t wavelib_max_entries = 512u;
	static constexpr size_t wavelib_entry_size = sizeof(wavelib_entry);
	static_assert(sizeof(wavelib_entry) == 0x20, "invalid wavelib_entry size!");
	
	// static class
	audio_handler() = delete;
	~audio_handler() = delete;
	audio_handler& operator=(const audio_handler&) = delete;

};

#endif
