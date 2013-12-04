/*
 * Copyright (C) 2003 Maxim Stepin ( maxst@hiend3d.com )
 *
 * Copyright (C) 2010 Cameron Zemek ( grom@zeminvaders.net)
 *
 * Copyright (C) 2010 Florian Ziesche (added alpha/transparency awareness and interpolation)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __AR_HQX_HPP__
#define __AR_HQX_HPP__

#include "scaling.hpp"

#define MASK_2 0x00FF00
#define MASK_13 0xFF00FF
#define MALPHA(x) ((x & 0xFF000000) >> 24)
#define TOALPHA(x) ((x > 0xFF ? 0xFF : x) << 24)

#define Ymask 0x00FF0000
#define Umask 0x0000FF00
#define Vmask 0x000000FF
#define trY   0x00300000
#define trU   0x00000700
#define trV   0x00000006

/* Test if there is difference in color */
inline int scaling::Diff(unsigned int w1, unsigned int w2)
{
    // Mask against RGB_MASK to discard the alpha channel
    unsigned int YUV1 = RGBtoYUV[w1 & 0x00FFFFFF];
    unsigned int YUV2 = RGBtoYUV[w2 & 0x00FFFFFF];
    return ( ( labs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
            ( labs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
            ( labs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) );
}

/* Interpolate functions */

static inline void Interp1(unsigned int * pc, unsigned int c1, unsigned int c2)
{
    //*pc = (c1*3+c2) >> 2;
    if (c1 == c2) {
        *pc = c1;
        return;
    }
    *pc = ((((c1 & MASK_2) * 3 + (c2 & MASK_2)) >> 2) & MASK_2) +
	((((c1 & MASK_13) * 3 + (c2 & MASK_13)) >> 2) & MASK_13);
	*pc |= TOALPHA((MALPHA(c1)*3 + MALPHA(c2)) >> 2);
}

static inline void Interp2(unsigned int * pc, unsigned int c1, unsigned int c2, unsigned int c3)
{
    //*pc = (c1*2+c2+c3) >> 2;
    *pc = ((((c1 & MASK_2) * 2 + (c2 & MASK_2) + (c3 & MASK_2)) >> 2) & MASK_2) +
	((((c1 & MASK_13) * 2 + (c2 & MASK_13) + (c3 & MASK_13)) >> 2) & MASK_13);
	*pc |= TOALPHA((MALPHA(c1)*2 + MALPHA(c2) + MALPHA(c3)) >> 2);
}

static inline void Interp3(unsigned int * pc, unsigned int c1, unsigned int c2)
{
    //*pc = (c1*7+c2)/8;
    if (c1 == c2) {
        *pc = c1;
        return;
    }
    *pc = ((((c1 & MASK_2) * 7 + (c2 & MASK_2)) >> 3) & MASK_2) +
	((((c1 & MASK_13) * 7 + (c2 & MASK_13)) >> 3) & MASK_13);
	*pc |= TOALPHA((MALPHA(c1)*7 + MALPHA(c2)) / 8);
}

static inline void Interp4(unsigned int * pc, unsigned int c1, unsigned int c2, unsigned int c3)
{
    //*pc = (c1*2+(c2+c3)*7)/16;
    *pc = ((((c1 & MASK_2) * 2 + (c2 & MASK_2) * 7 + (c3 & MASK_2) * 7) >> 4) & MASK_2) +
	((((c1 & MASK_13) * 2 + (c2 & MASK_13) * 7 + (c3 & MASK_13) * 7) >> 4) & MASK_13);
	*pc |= TOALPHA((MALPHA(c1)*2 + (MALPHA(c2) + MALPHA(c3)) * 7) / 16);
}

static inline void Interp5(unsigned int * pc, unsigned int c1, unsigned int c2)
{
    //*pc = (c1+c2) >> 1;
    if (c1 == c2) {
        *pc = c1;
        return;
    }
    *pc = ((((c1 & MASK_2) + (c2 & MASK_2)) >> 1) & MASK_2) +
	((((c1 & MASK_13) + (c2 & MASK_13)) >> 1) & MASK_13);
	*pc |= TOALPHA((MALPHA(c1) + MALPHA(c2)) >> 1);
}

static inline void Interp6(unsigned int * pc, unsigned int c1, unsigned int c2, unsigned int c3)
{
    //*pc = (c1*5+c2*2+c3)/8;
    *pc = ((((c1 & MASK_2) * 5 + (c2 & MASK_2) * 2 + (c3 & MASK_2)) >> 3) & MASK_2) +
	((((c1 & MASK_13) * 5 + (c2 & MASK_13) * 2 + (c3 & MASK_13)) >> 3) & MASK_13);
	*pc |= TOALPHA((MALPHA(c1)*5 + MALPHA(c2)*2 + MALPHA(c3)) / 8);
}

static inline void Interp7(unsigned int * pc, unsigned int c1, unsigned int c2, unsigned int c3)
{
    //*pc = (c1*6+c2+c3)/8;
    *pc = ((((c1 & MASK_2) * 6 + (c2 & MASK_2) + (c3 & MASK_2)) >> 3) & MASK_2) +
	((((c1 & MASK_13) * 6 + (c2 & MASK_13) + (c3 & MASK_13)) >> 3) & MASK_13);
	*pc |= TOALPHA((MALPHA(c1)*6 + MALPHA(c2) + MALPHA(c3)) / 8);
}

static inline void Interp8(unsigned int * pc, unsigned int c1, unsigned int c2)
{
    //*pc = (c1*5+c2*3)/8;
    if (c1 == c2) {
        *pc = c1;
        return;
    }
    *pc = ((((c1 & MASK_2) * 5 + (c2 & MASK_2) * 3) >> 3) & MASK_2) +
	((((c1 & MASK_13) * 5 + (c2 & MASK_13) * 3) >> 3) & MASK_13);
	*pc |= TOALPHA((MALPHA(c1)*5 + MALPHA(c2)*3) / 8);
}

static inline void Interp9(unsigned int * pc, unsigned int c1, unsigned int c2, unsigned int c3)
{
    //*pc = (c1*2+(c2+c3)*3)/8;
    *pc = ((((c1 & MASK_2) * 2 + (c2 & MASK_2) * 3 + (c3 & MASK_2) * 3) >> 3) & MASK_2) +
	((((c1 & MASK_13) * 2 + (c2 & MASK_13) * 3 + (c3 & MASK_13) * 3) >> 3) & MASK_13);
	*pc |= TOALPHA((MALPHA(c1)*2 + (MALPHA(c2) + MALPHA(c3))*3) / 8);
}

static inline void Interp10(unsigned int * pc, unsigned int c1, unsigned int c2, unsigned int c3)
{
    //*pc = (c1*14+c2+c3)/16;
    *pc = ((((c1 & MASK_2) * 14 + (c2 & MASK_2) + (c3 & MASK_2)) >> 4) & MASK_2) +
	((((c1 & MASK_13) * 14 + (c2 & MASK_13) + (c3 & MASK_13)) >> 4) & MASK_13);
	*pc |= TOALPHA((MALPHA(c1)*14 + MALPHA(c2) + MALPHA(c3)) / 16);
}

#endif
