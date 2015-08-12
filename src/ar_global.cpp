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

#include "ar_global.hpp"
#include "ui/bin_graphics.hpp"
#include <a2e.hpp>

texman* t = nullptr;
event* eevt = nullptr;
shader* s = nullptr;
scene* sce = nullptr;
camera* cam = nullptr;
pal* palettes = nullptr;
ext* exts = nullptr;
gui* ui = nullptr;
font_manager* fm = nullptr;
ar_clock* clck = nullptr;
bin_graphics* bin_gfx = nullptr;
albion_script* script = nullptr;
albion_ui* aui = nullptr;
