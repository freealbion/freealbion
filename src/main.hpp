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
 
#ifndef __AR_MAIN_HPP__
#define __AR_MAIN_HPP__

#include <a2e.hpp>
#include "ar_global.hpp"
#include "conf.hpp"

#include "albion_ui.hpp"
#include "xld.hpp"
#include "palette.hpp"
#include "tileset.hpp"
#include "npcgfx.hpp"
#include "player2d.hpp"
#include "scaling.hpp"
#include "transtb.hpp"
#include "clock.hpp"
#include "bin_graphics.hpp"
#include "ar_debug.hpp"
#include "map_handler.hpp"

// prototypes
bool key_handler(EVENT_TYPE type, shared_ptr<event_object> obj);
bool mouse_handler(EVENT_TYPE type, shared_ptr<event_object> obj);
bool quit_handler(EVENT_TYPE type, shared_ptr<event_object> obj);
bool window_handler(EVENT_TYPE type, shared_ptr<event_object> obj);

#endif
