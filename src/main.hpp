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
 
#ifndef __AR_MAIN_HPP__
#define __AR_MAIN_HPP__

#include <a2e.hpp>
#include "ar_global.hpp"
#include "conf.hpp"

#include "ui/albion_ui.hpp"
#include "gfx/palette.hpp"
#include "map/2d/tileset.hpp"
#include "map/2d/npcgfx.hpp"
#include "map/2d/player2d.hpp"
#include "gfx/scaling.hpp"
#include "gfx/transtb.hpp"
#include "events/clock.hpp"
#include "ui/bin_graphics.hpp"
#include "ar_debug.hpp"
#include "map/map_handler.hpp"
#include "script/albion_script.hpp"
#include "audio/audio_handler.hpp"
#include <floor/math/vector_lib.hpp>

// prototypes
bool key_handler(EVENT_TYPE type, shared_ptr<event_object> obj);
bool mouse_handler(EVENT_TYPE type, shared_ptr<event_object> obj);
bool quit_handler(EVENT_TYPE type, shared_ptr<event_object> obj);
bool window_handler(EVENT_TYPE type, shared_ptr<event_object> obj);

#endif
