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

#include "map_tiles.h"

map_tiles::map_tiles() : a2estatic(::e, ::s, ::sce) {
}

map_tiles::~map_tiles() {
}

const string map_tiles::select_shader(const DRAW_MODE& draw_mode) const {
	if(draw_mode == DRAW_MODE::GEOMETRY_PASS || draw_mode == DRAW_MODE::GEOMETRY_ALPHA_PASS) return "AR_IR_GBUFFER_MAP_TILES";
	else if(draw_mode == DRAW_MODE::MATERIAL_PASS || draw_mode == DRAW_MODE::MATERIAL_ALPHA_PASS) return "AR_IR_MP_MAP_TILES";
	return "";
}

void map_tiles::pre_draw_geometry(gl3shader& shd, size_t& attr_array_mask, size_t& texture_mask) {
	pre_draw_material(shd, attr_array_mask, texture_mask);
	attr_array_mask |= VA_TEXTURE_COORD | VA_NORMAL;
	texture_mask |= a2ematerial::TT_DIFFUSE;
}

void map_tiles::pre_draw_material(gl3shader& shd, size_t& attr_array_mask, size_t& texture_mask) {
	attr_array_mask &= ~VA_NORMAL;
}
