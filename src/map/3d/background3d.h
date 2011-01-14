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

#ifndef __AR_BACKGROUND3D_H__
#define __AR_BACKGROUND3D_H__

#include "ar_global.h"
#include "conf.h"
#include "xld.h"
#include "map_defines.h"
#include "albion_texture.h"

class background3d : public a2estatic {
public:
	background3d();
	virtual ~background3d();
	
	virtual void draw(const size_t draw_mode = a2emodel::MDM_NORMAL);

	void load(const size_t& bg_num, const size_t& palette);
	void unload();
	a2e_texture& get_bg_texture();

protected:
	const xld* bg3d_xld;

	a2e_texture bg_texture;
	ssize_t cur_bg_num;

};

#endif
