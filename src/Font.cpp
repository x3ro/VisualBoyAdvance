/*
 * VisualBoyAdvanced - Nintendo Gameboy/GameboyAdvance (TM) emulator
 * Copyrigh(c) 1999-2002 Forgotten (vb@emuhq.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 * Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
 *
 * (c) Copyright 1996 - 2001 Gary Henderson (gary@daniver.demon.co.uk) and
 *                           Jerremy Koot (jkoot@snes9x.com)
 *
 * Super FX C emulator code 
 * (c) Copyright 1997 - 1999 Ivar (Ivar@snes9x.com) and
 *                           Gary Henderson.
 * Super FX assembler emulator code (c) Copyright 1998 zsKnight and _Demo_.
 *
 * DSP1 emulator code (c) Copyright 1998 Ivar, _Demo_ and Gary Henderson.
 * C4 asm and some C emulation code (c) Copyright 2000 zsKnight and _Demo_.
 * C4 C code (c) Copyright 2001 Gary Henderson (gary@daniver.demon.co.uk).
 *
 * DOS port code contains the works of other authors. See headers in
 * individual files.
 *
 * Snes9x homepage: www.snes9x.com
 *
 * Permission to use, copy, modify and distribute Snes9x in both binary and
 * source form, for non-commercial purposes, is hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event shall the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Snes9x is freeware for PERSONAL USE only. Commercial users should
 * seek permission of the copyright holders first. Commercial use includes
 * charging money for Snes9x or software derived from Snes9x.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so everyone can benefit from the modifications
 * in future versions.
 *
 * Super NES and Super Nintendo Entertainment System are trademarks of
 * Nintendo Co., Limited and its subsidiary companies.
 */

#include "System.h"

static char *font[] = {
"           .      . .                    .                ..       .      .                                                     ",
"          .#.    .#.#.    . .     ...   .#. .     .      .##.     .#.    .#.     . .       .                                .   ",
"          .#.    .#.#.   .#.#.   .###.  .#..#.   .#.     .#.     .#.      .#.   .#.#.     .#.                              .#.  ",
"          .#.    .#.#.  .#####. .#.#.    ..#.   .#.#.   .#.      .#.      .#.    .#.     ..#..           ....             .#.   ",
"          .#.     . .    .#.#.   .###.   .#..    .#.     .       .#.      .#.   .###.   .#####.   ..    .####.    ..     .#.    ",
"           .            .#####.   .#.#. .#..#.  .#.#.            .#.      .#.    .#.     ..#..   .##.    ....    .##.   .#.     ",
"          .#.            .#.#.   .###.   . .#.   .#.#.            .#.    .#.    .#.#.     .#.    .#.             .##.    .      ",
"           .              . .     ...       .     . .              .      .      . .       .    .#.               ..            ",
"                                                                                                 .                              ",
"  .       .       ..     ....      .     ....     ..     ....     ..      ..                                              .     ",
" .#.     .#.     .##.   .####.    .#.   .####.   .##.   .####.   .##.    .##.     ..      ..       .             .       .#.    ",
".#.#.   .##.    .#..#.   ...#.   .##.   .#...   .#..     ...#.  .#..#.  .#..#.   .##.    .##.     .#.    ....   .#.     .#.#.   ",
".#.#.    .#.     . .#.   .##.   .#.#.   .###.   .###.     .#.    .##.   .#..#.   .##.    .##.    .#.    .####.   .#.     ..#.   ",
".#.#.    .#.      .#.    ...#.  .####.   ...#.  .#..#.    .#.   .#..#.   .###.    ..      ..    .#.      ....     .#.    .#.    ",
".#.#.    .#.     .#..   .#..#.   ..#.   .#..#.  .#..#.   .#.    .#..#.    ..#.   .##.    .##.    .#.    .####.   .#.      .     ",
" .#.    .###.   .####.   .##.     .#.    .##.    .##.    .#.     .##.    .##.    .##.    .#.      .#.    ....   .#.      .#.    ",
"  .      ...     ....     ..       .      ..      ..      .       ..      ..      ..    .#.        .             .        .     ",
"                                                                                         .                                      ",
"  ..      ..     ...      ..     ...     ....    ....     ..     .  .    ...        .    .  .    .       .   .   .   .    ..    ",
" .##.    .##.   .###.    .##.   .###.   .####.  .####.   .##.   .#..#.  .###.      .#.  .#..#.  .#.     .#. .#. .#. .#.  .##.   ",
".#..#.  .#..#.  .#..#.  .#..#.  .#..#.  .#...   .#...   .#..#.  .#..#.   .#.       .#.  .#.#.   .#.     .##.##. .##..#. .#..#.  ",
".#.##.  .#..#.  .###.   .#. .   .#..#.  .###.   .###.   .#...   .####.   .#.       .#.  .##.    .#.     .#.#.#. .#.#.#. .#..#.  ",
".#.##.  .####.  .#..#.  .#. .   .#..#.  .#..    .#..    .#.##.  .#..#.   .#.     . .#.  .##.    .#.     .#...#. .#.#.#. .#..#.  ",
".#...   .#..#.  .#..#.  .#..#.  .#..#.  .#...   .#.     .#..#.  .#..#.   .#.    .#..#.  .#.#.   .#...   .#. .#. .#..##. .#..#.  ",
" .##.   .#..#.  .###.    .##.   .###.   .####.  .#.      .###.  .#..#.  .###.    .##.   .#..#.  .####.  .#. .#. .#. .#.  .##.   ",
"  ..     .  .    ...      ..     ...     ....    .        ...    .  .    ...      ..     .  .    ....    .   .   .   .    ..    ",
"                                                                                                                                ",
" ...      ..     ...      ..     ...     .   .   .   .   .   .   .  .    . .     ....    ...             ...      .             ",
".###.    .##.   .###.    .##.   .###.   .#. .#. .#. .#. .#. .#. .#..#.  .#.#.   .####.  .###.    .      .###.    .#.            ",
".#..#.  .#..#.  .#..#.  .#..#.   .#.    .#. .#. .#. .#. .#...#. .#..#.  .#.#.    ...#.  .#..    .#.      ..#.   .#.#.           ",
".#..#.  .#..#.  .#..#.   .#..    .#.    .#. .#. .#. .#. .#.#.#.  .##.   .#.#.     .#.   .#.      .#.      .#.    . .            ",
".###.   .#..#.  .###.    ..#.    .#.    .#. .#. .#. .#. .#.#.#. .#..#.   .#.     .#.    .#.       .#.     .#.                   ",
".#..    .##.#.  .#.#.   .#..#.   .#.    .#...#.  .#.#.  .##.##. .#..#.   .#.    .#...   .#..       .#.   ..#.            ....   ",
".#.      .##.   .#..#.   .##.    .#.     .###.    .#.   .#. .#. .#..#.   .#.    .####.  .###.       .   .###.           .####.  ",
" .        ..#.   .  .     ..      .       ...      .     .   .   .  .     .      ....    ...             ...             ....   ",
"            .                                                                                                                   ",
" ..              .                  .              .             .        .        .     .       ..                             ",
".##.            .#.                .#.            .#.           .#.      .#.      .#.   .#.     .##.                            ",
" .#.      ...   .#..      ..      ..#.    ..     .#.#.    ...   .#..     ..        .    .#..     .#.     .. ..   ...      ..    ",
"  .#.    .###.  .###.    .##.    .###.   .##.    .#..    .###.  .###.   .##.      .#.   .#.#.    .#.    .##.##. .###.    .##.   ",
"   .    .#..#.  .#..#.  .#..    .#..#.  .#.##.  .###.   .#..#.  .#..#.   .#.      .#.   .##.     .#.    .#.#.#. .#..#.  .#..#.  ",
"        .#.##.  .#..#.  .#..    .#..#.  .##..    .#.     .##.   .#..#.   .#.     ..#.   .#.#.    .#.    .#...#. .#..#.  .#..#.  ",
"         .#.#.  .###.    .##.    .###.   .##.    .#.    .#...   .#..#.  .###.   .#.#.   .#..#.  .###.   .#. .#. .#..#.   .##.   ",
"          . .    ...      ..      ...     ..      .      .###.   .  .    ...     .#.     .  .    ...     .   .   .  .     ..    ",
"                                                          ...                     .                                             ",
"                                  .                                                        .      .      .        . .           ",
"                                 .#.                                                      .#.    .#.    .#.      .#.#.          ",
" ...      ...    ...      ...    .#.     .  .    . .     .   .   .  .    .  .    ....    .#.     .#.     .#.    .#.#.           ",
".###.    .###.  .###.    .###.  .###.   .#..#.  .#.#.   .#...#. .#..#.  .#..#.  .####.  .##.     .#.     .##.    . .            ",
".#..#.  .#..#.  .#..#.  .##..    .#.    .#..#.  .#.#.   .#.#.#.  .##.   .#..#.   ..#.    .#.     .#.     .#.                    ",
".#..#.  .#..#.  .#. .    ..##.   .#..   .#..#.  .#.#.   .#.#.#.  .##.    .#.#.   .#..    .#.     .#.     .#.                    ",
".###.    .###.  .#.     .###.     .##.   .###.   .#.     .#.#.  .#..#.    .#.   .####.    .#.    .#.    .#.                     ",
".#..      ..#.   .       ...       ..     ...     .       . .    .  .    .#.     ....      .      .      .                      ",
" .          .                                                             .                                                     ",
};

static int font_width = 8;
static int font_height = 9;

extern int RGB_LOW_BITS_MASK;

void fontDisplayChar(u8 *screen, int pitch, u8 c, bool trans)
{
  int line = (((c & 0x7f) - 32) >> 4) * font_height;
  int offset = (((c & 0x7f) - 32) & 15) * font_width;
  switch(systemColorDepth) {
  case 16:
    {
      u16 mask = ~RGB_LOW_BITS_MASK;
      int h, w;
      u16 *s = (u16 *)screen;
      for (h = 0; h < font_height; h++, line++) {
        for (w = 0; w < font_width; w++, s++) {
          u8 p = font [line][offset + w];

          if(trans) {
            if (p == '#')
              *s = ((0xf) << systemRedShift) +
                ((*s & mask) >>1);
            else if (p == '.')
              *s = ((*s & mask)>>1);
          } else {
            if (p == '#')
              *s = (0x1f) << systemRedShift;
            else if (p == '.')
              *s = 0x0000;
          }
        }
        screen += pitch;
        s = (u16 *)screen;
      }
    }
    break;
  case 24:
    {
      int h, w;
      u8 *s = (u8 *)screen;
      for (h = 0; h < font_height; h++, line++) {
        for (w = 0; w < font_width; w++, s+=3) {
          u8 p = font [line][offset + w];

          if(trans) {
            if (p == '#') {
              u32 color = (0x1f) << systemRedShift;
              *s = ((color & 255)>>1)+(*s>>1);
              *(s+1) = (((color >> 8) & 255)>>1)+(*(s+1)>>1);
              *(s+2) = (((color >> 16) & 255)>>1)+(*(s+2)>>1);
            } else if (p == '.') {
              *s = *s>>1;
              *(s+1) = *(s+1)>>1;
              *(s+2) = *(s+2)>>1;
            }
          } else {
            if (p == '#') {
              u32 color = (0x1f) << systemRedShift;
              *s = (color & 255);
              *(s+1) = (color >> 8) & 255;
              *(s+2) = (color >> 16) & 255;
            } else if (p == '.') {
              *s = 0;
              *(s+1) = 0;
              *(s+2) = 0;
            }
          }
        }
        screen += pitch;
        s = (u8 *)screen;
      }
    }
    break;    
  case 32:
    {
      int h, w;
      u32 mask = 0xfefefe;
      u32 *s = (u32 *)screen;
      for (h = 0; h < font_height; h++, line++) {
        for (w = 0; w < font_width; w++, s++) {
          u8 p = font [line][offset + w];

          if(trans) {
            if (p == '#')
              *s = ((0xf) << systemRedShift) + ((*s & mask)>>1);
            else if (p == '.')
              *s = (*s & mask) >> 1;
          } else {
            if (p == '#')
              *s = (0x1f) << systemRedShift;
            else if (p == '.')
              *s = 0x00000000;
          }
        }
        screen += pitch;
        s = (u32 *)screen;
      }
    }
    break;    
  }
}

void fontDisplayString(u8 *screen, int pitch, int x, int y, char *string)
{
  screen += y*pitch;
  int inc = 2;
  switch(systemColorDepth) {
  case 24:
    inc = 3;
    break;
  case 32:
    inc = 4;
    break;
  }
  screen += x*inc;

  while(*string) {
    fontDisplayChar(screen, pitch, *string++, false);
    screen += inc*font_width;
  }
}

void fontDisplayStringTransp(u8 *screen, int pitch, int x, int y, char *string)
{
  screen += y*pitch;
  int inc = 2;
  switch(systemColorDepth) {
  case 24:
    inc = 3;
    break;
  case 32:
    inc = 4;
    break;
  }
  screen += x*inc;

  while(*string) {
    fontDisplayChar(screen, pitch, *string++, true);
    screen += inc*font_width;
  }
}
