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

#ifndef __AR_BIN_GRAPHICS_H__
#define __AR_BIN_GRAPHICS_H__

#include "ar_global.h"
#include "xld.h"
#include "palette.h"
#include "gfxconv.h"
#include "scaling.h"
#include "conf.h"
#include "albion_texture.h"

class bin_graphics {
public:
	bin_graphics();
	~bin_graphics();
	
	enum BIN_GRAPHIC_TYPE {
		CURSOR,
		CURSOR_3D_UP,
		CURSOR_3D_DOWN,
		CURSOR_3D_LEFT,
		CURSOR_3D_RIGHT,
		CURSOR_3D_TURN_LEFT_90,
		CURSOR_3D_TURN_RIGHT_90,
		CURSOR_3D_TURN_LEFT_180,
		CURSOR_3D_TURN_RIGHT_180,
		CURSOR_2D_UP,
		CURSOR_2D_DOWN,
		CURSOR_2D_LEFT,
		CURSOR_2D_RIGHT,
		CURSOR_2D_UP_LEFT,
		CURSOR_2D_UP_RIGHT,
		CURSOR_2D_DOWN_RIGHT,
		CURSOR_2D_DOWN_LEFT,
		CURSOR_SELECTED,
		CURSOR_CD_LOAD,
		CURSOR_WAIT,
		CURSOR_MOUSE_CLICK,
		CURSOR_SMALL,
		CURSOR_CROSS_SELECTED,
		CURSOR_CROSS_UNSELECTED,
		CURSOR_MEMORY_LOAD,
		CURSOR_UP_LEFT,
		CURSOR_UP_RIGHT,
		UI_BACKGROUND,
		UI_BACKGROUND_STRIPED,
		UI_BACKGROUND_LINES,
		UI_WINDOW_TOP_LEFT,
		UI_WINDOW_TOP_RIGHT,
		UI_WINDOW_BOTTOM_LEFT,
		UI_WINDOW_BOTTOM_RIGHT,
		UI_EXIT_BUTTON_1,
		UI_EXIT_BUTTON_2,
		UI_EXIT_BUTTON_3,
		UI_OFFENSIVE_VALUE,
		UI_DEFENSIVE_VALUE,
		UI_GOLD,
		UI_FOOD,
		UI_NA,
		UI_BROKEN,
		UI_SPELL_ADVANCE,
		COMBAT_MOVE,
		COMBAT_ATTACK_MELEE,
		COMBAT_ATTACK_RANGE,
		COMBAT_RETREAT,
		COMBAT_MAGIC,
		COMBAT_MAGIC_ITEM,
		MONSTER_EYE_OFF,
		MONSTER_EYE_ON,
		CLOCK,
		CLOCK_NUM_0,
		CLOCK_NUM_1,
		CLOCK_NUM_2,
		CLOCK_NUM_3,
		CLOCK_NUM_4,
		CLOCK_NUM_5,
		CLOCK_NUM_6,
		CLOCK_NUM_7,
		CLOCK_NUM_8,
		CLOCK_NUM_9,
		COMPASS_DE,
		COMPASS_EN,
		COMPASS_FR,
		COMPASS_DOT_0,
		COMPASS_DOT_1,
		COMPASS_DOT_2,
		COMPASS_DOT_3,
		COMPASS_DOT_4,
		COMPASS_DOT_5,
		COMPASS_DOT_6,
		COMPASS_DOT_7,
		SELECT,
		LOCK,
		PRODUCER,
		CHAR_EFFECT_1,
		CHAR_EFFECT_2,
		CHAR_EFFECT_3,
		ARROW_TURN_LEFT_90,
		ARROW_TURN_RIGHT_90,
		ARROW_TURN_LEFT_180,
		ARROW_TURN_RIGHT_180,
		ARROW_LOOK_UP,
		ARROW_LOOK_DOWN,
	};
	
	a2e_texture get_bin_graphic(const BIN_GRAPHIC_TYPE type) const;
	
protected:
	struct bin_gfx_mapping {
		BIN_GRAPHIC_TYPE type;
		size_t offset;
		size_t width;
		size_t height;
		TEXTURE_FILTERING filtering;
		bool scale_nearest;
	};
	
	vector<a2e_texture> bin_textures;
	
};

#endif
