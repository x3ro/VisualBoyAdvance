// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2004 Forgotten and the VBA development team

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or(at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include "System.h"

int coeff[32] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16};

u32 line0[240];
u32 line1[240];
u32 line2[240];
u32 line3[240];
u32 lineOBJ[240];
u32 lineOBJWin[240];
u32 lineMix[240];
bool gfxInWin0[240];
bool gfxInWin1[240];
int lineOBJpixleft[128];

int gfxBG2Changed = 0;
int gfxBG3Changed = 0;

int gfxBG2X = 0;
int gfxBG2Y = 0;
int gfxBG2LastX = 0;
int gfxBG2LastY = 0;
int gfxBG3X = 0;
int gfxBG3Y = 0;
int gfxBG3LastX = 0;
int gfxBG3LastY = 0;
int gfxLastVCOUNT = 0;

static u32 *lineMix_cleared;

#include <stdlib.h>
#include <string.h>
void lineMix_clear(void) {
	if(!lineMix_cleared) {
		lineMix_cleared = malloc(240*4);
		for (int i = 0; i < 240; ++i)
			lineMix_cleared[i] = 0x7fff;
	}
	memcpy(lineMix, lineMix_cleared, 240*4);
}
