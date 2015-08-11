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

#include "map/3d/map_tiles.hpp"

map_tiles::map_tiles() : a2estatic(::s, ::sce) {
}

map_tiles::~map_tiles() {
}

const string map_tiles::select_shader(const DRAW_MODE& draw_mode) const {
	if(draw_mode == DRAW_MODE::GEOMETRY_PASS || draw_mode == DRAW_MODE::GEOMETRY_ALPHA_PASS) return "AR_IR_GBUFFER_MAP_TILES";
	else if(draw_mode == DRAW_MODE::MATERIAL_PASS || draw_mode == DRAW_MODE::MATERIAL_ALPHA_PASS) return "AR_IR_MP_MAP_TILES";
	return "";
}

void map_tiles::pre_draw_geometry(gl_shader& shd, VERTEX_ATTRIBUTE& attr_array_mask, a2ematerial::TEXTURE_TYPE& texture_mask) {
	pre_draw_material(shd, attr_array_mask, texture_mask);
	attr_array_mask |= a2emodel::VERTEX_ATTRIBUTE::TEXTURE_COORD | a2emodel::VERTEX_ATTRIBUTE::NORMAL;
	texture_mask |= a2ematerial::TEXTURE_TYPE::DIFFUSE;
}

void map_tiles::pre_draw_material(gl_shader& shd floor_unused, VERTEX_ATTRIBUTE& attr_array_mask, a2ematerial::TEXTURE_TYPE& texture_mask floor_unused) {
	attr_array_mask &= (a2emodel::VERTEX_ATTRIBUTE)(~(unsigned int)a2emodel::VERTEX_ATTRIBUTE::NORMAL);
}
