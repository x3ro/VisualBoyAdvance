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
 * (c) Copyright 1996 - 2001 Gary Henderson (gary.henderson@ntlworld.com) and
 *                           Jerremy Koot (jkoot@snes9x.com)
 *
 * Super FX C emulator code 
 * (c) Copyright 1997 - 1999 Ivar (ivar@snes9x.com) and
 *                           Gary Henderson.
 * Super FX assembler emulator code (c) Copyright 1998 zsKnight and _Demo_.
 *
 * DSP1 emulator code (c) Copyright 1998 Ivar, _Demo_ and Gary Henderson.
 * C4 asm and some C emulation code (c) Copyright 2000 zsKnight and _Demo_.
 * C4 C code (c) Copyright 2001 Gary Henderson (gary.henderson@ntlworld.com).
 *
 * DOS port code contains the works of other authors. See headers in
 * individual files.
 *
 * Snes9x homepage: http://www.snes9x.com
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

extern int RGB_LOW_BITS_MASK;

void TVMode (u8 *srcPtr, u32 srcPitch, u8 *deltaPtr,
             u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u8 *nextLine, *finish;
  u32 colorMask = ~(RGB_LOW_BITS_MASK | (RGB_LOW_BITS_MASK << 16));
  u32 lowPixelMask = RGB_LOW_BITS_MASK;
  
  nextLine = dstPtr + dstPitch;
  
  do {
    u32 *bP = (u32 *) srcPtr;
    u32 *xP = (u32 *) deltaPtr;
    u32 *dP = (u32 *) dstPtr;
    u32 *nL = (u32 *) nextLine;
    u32 currentPixel;
    u32 nextPixel;
    u32 currentDelta;
    u32 nextDelta;
    
    finish = (u8 *) bP + ((width + 2) << 1);
    nextPixel = *bP++;
    nextDelta = *xP++;
    
    do {
      currentPixel = nextPixel;
      currentDelta = nextDelta;
      nextPixel = *bP++;
      nextDelta = *xP++;
      
      if ((nextPixel != nextDelta) || (currentPixel != currentDelta)) {
        u32 colorA, colorB, product, darkened;
        
        *(xP - 2) = currentPixel;
#ifdef WORDS_BIGENDIAN
        colorA = currentPixel >> 16;
        colorB = (currentPixel << 16) >> 16;
#else
        colorA = currentPixel & 0xffff;
        colorB = (currentPixel & 0xffff0000) >> 16;
#endif

        *(dP) = product = colorA |
          ((((colorA & colorMask) >> 1) +
            ((colorB & colorMask) >> 1) +
            (colorA & colorB & lowPixelMask)) << 16);
        darkened = (product = ((product & colorMask) >> 1));
        darkened += (product = ((product & colorMask) >> 1));
        darkened += (product & colorMask) >> 1;
        *(nL) = darkened;

#ifdef WORDS_BIGENDIAN
        colorA = nextPixel >> 16;
#else
        colorA = nextPixel & 0xffff;
#endif

        *(dP + 1) = product = colorB |
          ((((colorA & colorMask) >> 1) +
            ((colorB & colorMask) >> 1) +
            (colorA & colorB & lowPixelMask)) << 16);
        darkened = (product = ((product & colorMask) >> 1));
        darkened += (product = ((product & colorMask) >> 1));
        darkened += (product & colorMask) >> 1;
        *(nL + 1) = darkened;
      }
      
      dP += 2;
      nL += 2;
    } while ((u8 *) bP < finish);
    
    deltaPtr += srcPitch;
    srcPtr += srcPitch;
    dstPtr += dstPitch * 2;
    nextLine += dstPitch * 2;
  }
  while (--height);
}

#define RGB32_LOW_BITS_MASK 0x010101

void TVMode32(u8 *srcPtr, u32 srcPitch, u8 */* deltaPtr */,
              u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u8 *nextLine, *finish;
  u32 colorMask = ~(RGB32_LOW_BITS_MASK);
  u32 lowPixelMask = RGB32_LOW_BITS_MASK;
  
  nextLine = dstPtr + dstPitch;
  
  do {
    u32 *bP = (u32 *) srcPtr;
    //    u32 *xP = (u32 *) deltaPtr;
    u32 *dP = (u32 *) dstPtr;
    u32 *nL = (u32 *) nextLine;
    u32 currentPixel;
    u32 nextPixel;
    //    u32 currentDelta;
    //    u32 nextDelta;
    
    finish = (u8 *) bP + ((width) << 2);
    nextPixel = *bP++;
    //    nextDelta = *xP++;
    
    do {
      currentPixel = nextPixel;
      //      currentDelta = nextDelta;
      nextPixel = *bP++;
      //      nextDelta = *xP++;
      
      u32 colorA, colorB, product, darkened;
        
      //      *(xP - 2) = currentPixel;
      colorA = currentPixel;
      colorB = nextPixel;

      *(dP) = colorA;
      *(dP+1) = product = (((colorA & colorMask) >> 1) +
                           ((colorB & colorMask) >> 1) +
                           (colorA & colorB & lowPixelMask));
      darkened = (product = ((product & colorMask) >> 1));
      darkened += (product = ((product & colorMask) >> 1));
      //      darkened += (product & colorMask) >> 1;
      product = (colorA & colorMask) >> 1;
      product += (product & colorMask) >> 1;
      //      product += (product & colorMask) >> 1;
      *(nL) = product;
      *(nL+1) = darkened;

      nextPixel = *bP++;
      colorA = nextPixel;

      *(dP + 2) = colorB;
      *(dP + 3) = product = 
        (((colorA & colorMask) >> 1) +
         ((colorB & colorMask) >> 1) +
         (colorA & colorB & lowPixelMask));
      darkened = (product = ((product & colorMask) >> 1));
      darkened += (product = ((product & colorMask) >> 1));
      //      darkened += (product & colorMask) >> 1;
      product = (colorB & colorMask) >> 1;
      product += (product & colorMask) >> 1;
      //      product += (product & colorMask) >> 1;
      *(nL+2) = product;      
      *(nL+3) = darkened;
      
      dP += 4;
      nL += 4;
    } while ((u8 *) bP < finish);
    
    //    deltaPtr += srcPitch;
    srcPtr += srcPitch;
    dstPtr += dstPitch*2;
    nextLine += dstPitch*2;
  }
  while (--height);
}
