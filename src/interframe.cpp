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
#include "System.h"
#include <stdlib.h>
#include <memory.h>

/*
 * Thanks to Kawaks' Mr. K for the code

   Incorporated into vba by Anthony Di Franco
*/

static u8 *frm1;
static u8 *frm2;
static u8 *frm3;

extern int RGB_LOW_BITS_MASK;

static void Init()
{
  frm1 = (u8 *)calloc(322*242,4);
  // 1 frame ago
  frm2 = (u8 *)calloc(322*242,4);
  // 2 frames ago
  frm3 = (u8 *)calloc(322*242,4);
  // 3 frames ago
}

void SmartIB(u8 *srcPtr, u32 srcPitch, int width, int height)
{
  if(frm1 == NULL) {
    Init();
  }

  u16 colorMask = ~RGB_LOW_BITS_MASK;
  
  u16 *src0 = (u16 *)srcPtr;
  u16 *src1 = (u16 *)frm1;
  u16 *src2 = (u16 *)frm2;
  u16 *src3 = (u16 *)frm3;

  int sPitch = srcPitch >> 1;

  int pos = 0;
  for (int j = 0; j < height;  j++)
    for (int i = 0; i < sPitch; i++) {
      u16 color = src0[pos];
      src0[pos] =
        (src1[pos] != src2[pos]) &&
        (src3[pos] != color) &&
        ((color == src2[pos]) || (src1[pos] == src3[pos]))
        ? (((color & colorMask) >> 1) + ((src1[pos] & colorMask) >> 1)) :
        color;
      src3[pos] = color; /* oldest buffer now holds newest frame */
      pos++;
    }
  
  /* Swap buffers around */
  u8 *temp = frm1;
  frm1 = frm3;
  frm3 = frm2;
  frm2 = temp;
}

void SmartIB32(u8 *srcPtr, u32 srcPitch, int width, int height)
{
  if(frm1 == NULL) {
    Init();
  }
  u32 *src0 = (u32 *)srcPtr;
  u32 *src1 = (u32 *)frm1;
  u32 *src2 = (u32 *)frm2;
  u32 *src3 = (u32 *)frm3;

  u32 colorMask = 0xfefefe;

  int sPitch = srcPitch >> 2;
  int pos = 0;

  for (int j = 0; j < height;  j++)
    for (int i = 0; i < sPitch; i++) {
      u32 color = src0[pos];
      src0[pos] =
        (src1[pos] != src2[pos]) &&
        (src3[pos] != color) &&
        ((color == src2[pos]) || (src1[pos] == src3[pos]))
        ? (((color & colorMask) >> 1) + ((src1[pos] & colorMask) >> 1)) :
        color;
      src3[pos] = color; /* oldest buffer now holds newest frame */
      pos++;
    }
  
  /* Swap buffers around */
  u8 *temp = frm1;
  frm1 = frm3;
  frm3 = frm2;
  frm2 = temp;
}

void MotionBlurIB(u8 *srcPtr, u32 srcPitch, int width, int height)
{
  if(frm1 == NULL) {
    Init();
  }

  u16 colorMask = ~RGB_LOW_BITS_MASK;
  
  u16 *src0 = (u16 *)srcPtr;
  u16 *src1 = (u16 *)frm1;

  int sPitch = srcPitch >> 1;

  int pos = 0;
  for (int j = 0; j < height;  j++)
    for (int i = 0; i < sPitch; i++) {
      u16 color = src0[pos];
      src0[pos] =
        (((color & colorMask) >> 1) + ((src1[pos] & colorMask) >> 1));
      src1[pos] = color; /* oldest buffer now holds newest frame */
      pos++;
    }
}

void MotionBlurIB32(u8 *srcPtr, u32 srcPitch, int width, int height)
{
  if(frm1 == NULL) {
    Init();
  }
  u32 *src0 = (u32 *)srcPtr;
  u32 *src1 = (u32 *)frm1;

  u32 colorMask = 0xfefefe;

  int sPitch = srcPitch >> 2;
  int pos = 0;

  for (int j = 0; j < height;  j++)
    for (int i = 0; i < sPitch; i++) {
      u32 color = src0[pos];
      src0[pos] = (((color & colorMask) >> 1) +
                   ((src1[pos] & colorMask) >> 1));
      src1[pos] = color; /* oldest buffer now holds newest frame */
      pos++;
    }
}

