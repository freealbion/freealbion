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

#ifndef __AR_MAP_TILES_HPP__
#define __AR_MAP_TILES_HPP__

#include "ar_global.hpp"
#include "conf.hpp"
#include "map/map_defines.hpp"

#include <scene/model/a2estatic.hpp>

class map_tiles : public a2estatic {
public:
	map_tiles();
	virtual ~map_tiles();
	
protected:
	
	virtual const string select_shader(const DRAW_MODE& draw_mode) const;
	virtual void pre_draw_geometry(gl_shader& shd, VERTEX_ATTRIBUTE& attr_array_mask, a2ematerial::TEXTURE_TYPE& texture_mask);
	virtual void pre_draw_material(gl_shader& shd, VERTEX_ATTRIBUTE& attr_array_mask, a2ematerial::TEXTURE_TYPE& texture_mask);
	
};

#endif
