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
 
#ifndef __AR_NPCGFX_HPP__
#define __AR_NPCGFX_HPP__

#include "ar_global.hpp"
#include "conf.hpp"
#include "map/map_defines.hpp"
#include "gfx/palette.hpp"
#include "gfx/gfxconv.hpp"
#include "gfx/scaling.hpp"
#include "gfx/albion_texture.hpp"

/*! @class npcgfx
 *  @brief npc graphics
 *  @author flo
 *  
 *  loads npc graphics
 */

class npcgfx {
public:
	npcgfx(const pal* palettes);
	~npcgfx();
	
	void clear();
	void set_palette(const size_t& palette_num);

	const a2e_texture& get_npcgfx(const size_t& npc_num);
	void draw_npc(const size_t& npc_num, const size_t& frame,
				  const float2& screen_position, const float2& position);
	
protected:
	const pal* palettes;
	size_t cur_palette;

	map<size_t, a2e_texture> npc_graphics;
	
	array<lazy_xld, 6> npcgfx_xlds;

	void load_npcgfx(const size_t& npc_num);
	
};

#endif
