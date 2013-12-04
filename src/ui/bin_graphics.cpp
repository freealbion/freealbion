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

#include "bin_graphics.hpp"
#include "map_defines.hpp"
#include <core/file_io.hpp>
#include <rendering/texman.hpp>

static const string type_strings[] = {
	"CURSOR",
	"CURSOR_3D_UP",
	"CURSOR_3D_DOWN",
	"CURSOR_3D_LEFT",
	"CURSOR_3D_RIGHT",
	"CURSOR_3D_TURN_LEFT_90",
	"CURSOR_3D_TURN_RIGHT_90",
	"CURSOR_3D_TURN_LEFT_180",
	"CURSOR_3D_TURN_RIGHT_180",
	"CURSOR_2D_UP",
	"CURSOR_2D_DOWN",
	"CURSOR_2D_LEFT",
	"CURSOR_2D_RIGHT",
	"CURSOR_2D_UP_LEFT",
	"CURSOR_2D_UP_RIGHT",
	"CURSOR_2D_DOWN_RIGHT",
	"CURSOR_2D_DOWN_LEFT",
	"CURSOR_SELECTED",
	"CURSOR_CD_LOAD",
	"CURSOR_WAIT",
	"CURSOR_MOUSE_CLICK",
	"CURSOR_SMALL",
	"CURSOR_CROSS_SELECTED",
	"CURSOR_CROSS_UNSELECTED",
	"CURSOR_MEMORY_LOAD",
	"CURSOR_UP_LEFT",
	"CURSOR_UP_RIGHT",
	"UI_BACKGROUND",
	"UI_BACKGROUND_STRIPED",
	"UI_BACKGROUND_LINES",
	"UI_WINDOW_TOP_LEFT",
	"UI_WINDOW_TOP_RIGHT",
	"UI_WINDOW_BOTTOM_LEFT",
	"UI_WINDOW_BOTTOM_RIGHT",
	"UI_EXIT_BUTTON_1",
	"UI_EXIT_BUTTON_2",
	"UI_EXIT_BUTTON_3",
	"UI_OFFENSIVE_VALUE",
	"UI_DEFENSIVE_VALUE",
	"UI_GOLD",
	"UI_FOOD",
	"UI_NA",
	"UI_BROKEN",
	"UI_SPELL_ADVANCE",
	"COMBAT_MOVE",
	"COMBAT_ATTACK_MELEE",
	"COMBAT_ATTACK_RANGE",
	"COMBAT_RETREAT",
	"COMBAT_MAGIC",
	"COMBAT_MAGIC_ITEM",
	"MONSTER_EYE_OFF",
	"MONSTER_EYE_ON",
	"CLOCK",
	"CLOCK_NUM_0",
	"CLOCK_NUM_1",
	"CLOCK_NUM_2",
	"CLOCK_NUM_3",
	"CLOCK_NUM_4",
	"CLOCK_NUM_5",
	"CLOCK_NUM_6",
	"CLOCK_NUM_7",
	"CLOCK_NUM_8",
	"CLOCK_NUM_9",
	"COMPASS_DE",
	"COMPASS_EN",
	"COMPASS_FR",
	"COMPASS_DOT_0",
	"COMPASS_DOT_1",
	"COMPASS_DOT_2",
	"COMPASS_DOT_3",
	"COMPASS_DOT_4",
	"COMPASS_DOT_5",
	"COMPASS_DOT_6",
	"COMPASS_DOT_7",
	"SELECT",
	"LOCK",
	"PRODUCER",
	"CHAR_EFFECT_1",
	"CHAR_EFFECT_2",
	"CHAR_EFFECT_3",
	"ARROW_TURN_LEFT_90",
	"ARROW_TURN_RIGHT_90",
	"ARROW_TURN_LEFT_180",
	"ARROW_TURN_RIGHT_180",
	"ARROW_LOOK_UP",
	"ARROW_LOOK_DOWN",
};

bin_graphics::bin_graphics() {
	static const bin_gfx_mapping graphics[] = {
		{ CURSOR, 0xFBE58, 14, 14, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_3D_UP, 0xFBF1C, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_3D_DOWN, 0xFC01C, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_3D_LEFT, 0xFC11C, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_3D_RIGHT, 0xFC21C, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_3D_TURN_LEFT_90, 0xFC31C, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_3D_TURN_RIGHT_90, 0xFC41C, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_3D_TURN_LEFT_180, 0xFC51C, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_3D_TURN_RIGHT_180, 0xFC61C, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_2D_UP, 0xFC71C, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_2D_DOWN, 0xFC81C, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_2D_LEFT, 0xFC91C, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_2D_RIGHT, 0xFCA1C, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_2D_UP_LEFT, 0xFCB1C, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_2D_UP_RIGHT, 0xFCC1C, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_2D_DOWN_RIGHT, 0xFCD1C, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_2D_DOWN_LEFT, 0xFCE1C, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_SELECTED, 0xFCF1C, 14, 13, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_CD_LOAD, 0xFCFD0, 24, 14, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_WAIT, 0xFD120, 16, 19, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_MOUSE_CLICK, 0xFD250, 18, 25, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_SMALL, 0xFD412, 8, 8, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_CROSS_SELECTED, 0xFD452, 20, 19, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_CROSS_UNSELECTED, 0xFD5CE, 22, 21, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_MEMORY_LOAD, 0xFD79C, 28, 21, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_UP_LEFT, 0xFD9EA, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ CURSOR_UP_RIGHT, 0xFDAEC, 16, 14, TEXTURE_FILTERING::LINEAR, false },
		// TODO: unknown data from 0xFDBCC - 0xFDD10 (2*162px?)
		{ UI_BACKGROUND, 0xFDD10, 32, 64, TEXTURE_FILTERING::LINEAR, false },
		{ UI_BACKGROUND_STRIPED, 0xFE510, 16, 12, TEXTURE_FILTERING::LINEAR, false },
		{ UI_BACKGROUND_LINES, 0xFE5D0, 16, 12, TEXTURE_FILTERING::LINEAR, false },
		{ UI_WINDOW_TOP_LEFT, 0xFE690, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ UI_WINDOW_TOP_RIGHT, 0xFE790, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ UI_WINDOW_BOTTOM_LEFT, 0xFE890, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ UI_WINDOW_BOTTOM_RIGHT, 0xFE990, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ UI_EXIT_BUTTON_1, 0xFEA90, 56, 16, TEXTURE_FILTERING::LINEAR, false },
		{ UI_EXIT_BUTTON_2, 0xFEE10, 56, 16, TEXTURE_FILTERING::LINEAR, false },
		{ UI_EXIT_BUTTON_3, 0xFF190, 56, 16, TEXTURE_FILTERING::LINEAR, false },
		{ UI_OFFENSIVE_VALUE, 0xFF510, 8, 8, TEXTURE_FILTERING::LINEAR, false },
		{ UI_DEFENSIVE_VALUE, 0xFF550, 6, 8, TEXTURE_FILTERING::LINEAR, false },
		{ UI_GOLD, 0xFF580, 12, 10, TEXTURE_FILTERING::LINEAR, false },
		{ UI_FOOD, 0xFF5F8, 20, 10, TEXTURE_FILTERING::LINEAR, false },
		{ UI_NA, 0xFF6C0, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ UI_BROKEN, 0xFF7C0, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ UI_SPELL_ADVANCE, 0xFF8C0, 50, 8, TEXTURE_FILTERING::LINEAR, false },
		{ COMBAT_MOVE, 0xFFA50, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ COMBAT_ATTACK_MELEE, 0xFFB50, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ COMBAT_ATTACK_RANGE, 0xFFC50, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ COMBAT_RETREAT, 0xFFD50, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ COMBAT_MAGIC, 0xFFE50, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ COMBAT_MAGIC_ITEM, 0xFFF50, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ MONSTER_EYE_OFF, 0x100050, 32, 27, TEXTURE_FILTERING::LINEAR, false },
		{ MONSTER_EYE_ON, 0x1003B0, 32, 27, TEXTURE_FILTERING::LINEAR, false },
		{ CLOCK, 0x100710, 32, 25, TEXTURE_FILTERING::LINEAR, false },
		{ CLOCK_NUM_0, 0x100A30, 6, 7, TEXTURE_FILTERING::POINT, true },
		{ CLOCK_NUM_1, 0x100A5A, 6, 7, TEXTURE_FILTERING::POINT, true },
		{ CLOCK_NUM_2, 0x100A84, 6, 7, TEXTURE_FILTERING::POINT, true },
		{ CLOCK_NUM_3, 0x100AAE, 6, 7, TEXTURE_FILTERING::POINT, true },
		{ CLOCK_NUM_4, 0x100AD8, 6, 7, TEXTURE_FILTERING::POINT, true },
		{ CLOCK_NUM_5, 0x100B02, 6, 7, TEXTURE_FILTERING::POINT, true },
		{ CLOCK_NUM_6, 0x100B2C, 6, 7, TEXTURE_FILTERING::POINT, true },
		{ CLOCK_NUM_7, 0x100B56, 6, 7, TEXTURE_FILTERING::POINT, true },
		{ CLOCK_NUM_8, 0x100B80, 6, 7, TEXTURE_FILTERING::POINT, true },
		{ CLOCK_NUM_9, 0x100BAA, 6, 7, TEXTURE_FILTERING::POINT, true },
		{ COMPASS_DE, 0x100BD4, 30, 29, TEXTURE_FILTERING::LINEAR, false },
		{ COMPASS_EN, 0x100F3A, 30, 29, TEXTURE_FILTERING::LINEAR, false },
		{ COMPASS_FR, 0x1012A0, 30, 29, TEXTURE_FILTERING::LINEAR, false },
		{ COMPASS_DOT_0, 0x101606, 6, 6, TEXTURE_FILTERING::POINT, false },
		{ COMPASS_DOT_1, 0x10162A, 6, 6, TEXTURE_FILTERING::POINT, false },
		{ COMPASS_DOT_2, 0x10164E, 6, 6, TEXTURE_FILTERING::POINT, false },
		{ COMPASS_DOT_3, 0x101672, 6, 6, TEXTURE_FILTERING::POINT, false },
		{ COMPASS_DOT_4, 0x101696, 6, 6, TEXTURE_FILTERING::POINT, false },
		{ COMPASS_DOT_5, 0x1016BA, 6, 6, TEXTURE_FILTERING::POINT, false },
		{ COMPASS_DOT_6, 0x1016DE, 6, 6, TEXTURE_FILTERING::POINT, false },
		{ COMPASS_DOT_7, 0x101702, 6, 6, TEXTURE_FILTERING::POINT, false },
		{ SELECT, 0x101726, 18, 18, TEXTURE_FILTERING::LINEAR, false },
		{ LOCK, 0x10186A, 22, 22, TEXTURE_FILTERING::LINEAR, false },
		{ PRODUCER, 0x101A4E, 34, 48, TEXTURE_FILTERING::LINEAR, false },
		{ CHAR_EFFECT_1, 0x1020AE, 32, 32, TEXTURE_FILTERING::LINEAR, false },
		{ CHAR_EFFECT_2, 0x1024AE, 32, 32, TEXTURE_FILTERING::LINEAR, false },
		{ CHAR_EFFECT_3, 0x1028AD, 14, 13, TEXTURE_FILTERING::LINEAR, false },
		{ ARROW_TURN_LEFT_90, 0x102963, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ ARROW_TURN_RIGHT_90, 0x102A63, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ ARROW_TURN_LEFT_180, 0x102B63, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ ARROW_TURN_RIGHT_180, 0x102C63, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ ARROW_LOOK_UP, 0x102D63, 16, 16, TEXTURE_FILTERING::LINEAR, false },
		{ ARROW_LOOK_DOWN, 0x102E63, 16, 16, TEXTURE_FILTERING::LINEAR, false },
	};
	
	if(!fio->open(xld::make_xld_path("MAIN.EXE").c_str(), file_io::OPEN_TYPE::READ_BINARY)) {
		log_error("couldn't open MAIN.EXE");
	}
	else {
		unsigned char* albion_binary = new unsigned char[fio->get_filesize()];
		fio->get_block((char*)albion_binary, fio->get_filesize());
		fio->close();
		
		const scaling::SCALE_TYPE scale_type = conf::get<scaling::SCALE_TYPE>("map.2d.scale_type");
		const size_t scale_factor = scaling::get_scale_factor(scale_type);
		for(const auto& graphic : graphics) {
			size2 texture_size(graphic.width * scale_factor, graphic.height * scale_factor);
			unsigned int* tex_surface = new unsigned int[texture_size.x * texture_size.y*4];
			unsigned int* data_32bpp = new unsigned int[graphic.width * graphic.height];
			
			scaling::SCALE_TYPE scale = scale_type;
			if(graphic.scale_nearest) {
				scale = (scale_factor == 4 ? scaling::ST_NEAREST_4X :
						 (scale_factor == 2 ? scaling::ST_NEAREST_2X : scaling::ST_NEAREST_1X));
			}
			
			gfxconv::convert_8to32(&albion_binary[graphic.offset], data_32bpp,
								   graphic.width, graphic.height,
								   18, 0); // pal 18!
			scaling::scale(scale, data_32bpp, size2(graphic.width, graphic.height), tex_surface);
			
			
			bin_textures.emplace_back(t->add_texture(tex_surface, (unsigned int)texture_size.x, (unsigned int)texture_size.y, GL_RGBA8, GL_RGBA, graphic.filtering, 0, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_UNSIGNED_BYTE));
			
			delete [] tex_surface;
			delete [] data_32bpp;
		}
		
		delete [] albion_binary;
	}
}

bin_graphics::~bin_graphics() {
	bin_textures.clear();
}

a2e_texture bin_graphics::get_bin_graphic(const BIN_GRAPHIC_TYPE type) const {
	return bin_textures.at(type);
}
