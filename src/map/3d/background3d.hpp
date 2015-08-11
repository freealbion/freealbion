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

#ifndef __AR_BACKGROUND3D_HPP__
#define __AR_BACKGROUND3D_HPP__

#include "ar_global.hpp"
#include "conf.hpp"
#include "map/map_defines.hpp"
#include "gfx/albion_texture.hpp"

#include <scene/model/a2estatic.hpp>

class background3d : public a2estatic {
public:
	background3d();
	virtual ~background3d();
	
	virtual void draw(const DRAW_MODE draw_mode);

	void load(const size_t& bg_num, const size_t& palette);
	void unload();
	a2e_texture& get_bg_texture();
	
	void set_light_color(const float3& color);

protected:
	lazy_xld bg3d_xld;

	a2e_texture bg_texture;
	ssize_t cur_bg_num;
	ssize_t cur_bg_palette;
	
	GLuint vbo_fs_triangle;
	GLuint vbo_fs_coords;
	
	float3 light_color;

};

#endif
