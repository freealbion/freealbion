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

#ifndef __AR_GLOBAL_H__
#define __AR_GLOBAL_H__

#include <a2e.h>
#include "palette.h"
using namespace std;

#define APPLICATION_NAME "Albion Remake"

#ifdef __WINDOWS__
#pragma warning(disable: 4244)
#pragma warning(disable: 4267)
#endif

extern engine* e;
extern core* c;
extern file_io* fio;
extern gfx* egfx;
extern gui* egui;
extern a2eui* eui;
extern texman* t;
extern event* evt;
extern gui_style* gs;
extern shader* s;
extern opencl* ocl;
extern scene* sce;
extern camera* cam;
extern pal* palettes;
extern ext* exts;

#endif
