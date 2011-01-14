/*
 *  Albion Remake
 *  Copyright (C) 2007 - 2010 Florian Ziesche
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
 
#ifndef __AR_NPCGFX_H__
#define __AR_NPCGFX_H__

#include "ar_global.h"
#include "conf.h"
#include "map_defines.h"
#include "xld.h"
#include "palette.h"
#include "gfxconv.h"
#include "scaling.h"
#include "albion_texture.h"

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
	void draw_npc(const size_t& npc_num, const size_t& frame, const float2& screen_position, const float2& position, const float depth_overwrite = -1.0f);
	
protected:
	const pal* palettes;
	size_t cur_palette;

	map<size_t, a2e_texture> npc_graphics;
	
	xld* npcgfx_xlds[4];

	void load_npcgfx(const size_t& npc_num);
	
};

#endif
