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

#ifndef __AR_GLOBAL_HPP__
#define __AR_GLOBAL_HPP__

#include <floor/core/cpp_headers.hpp>
#include <floor/core/logger.hpp>
#include <floor/math/vector_lib.hpp>
#if defined(FLOOR_IOS)
#include <floor/core/gl_support.hpp>
#endif
#include "gfx/palette.hpp"
#include "events/clock.hpp"
#include "xld.hpp"

#define APPLICATION_NAME "Albion Remake PR2"

#if defined(__WINDOWS__)
#pragma warning(disable: 4244)
#pragma warning(disable: 4267)
#endif

class engine;
class gfx;
class texman;
class event;
class shader;
class scene;
class camera;
class ext;
class gui;
class font_manager;

extern texman* t;
extern event* eevt;
extern shader* s;
extern scene* sce;
extern camera* cam;
extern ext* exts;
extern gui* ui;
extern font_manager* fm;

extern pal* palettes;
extern ar_clock* clck;

class bin_graphics;
extern bin_graphics* bin_gfx;

class albion_script;
extern albion_script* script;

class albion_ui;
extern albion_ui* aui;

#endif
