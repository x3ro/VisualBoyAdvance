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

extern int RGB_LOW_BITS_MASK;

void Scanlines (u8 *srcPtr, u32 srcPitch, u8 *,
                u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u8 *nextLine, *finish;
  
  nextLine = dstPtr + dstPitch;
  
  do {
    u32 *bP = (u32 *) srcPtr;
    u32 *dP = (u32 *) dstPtr;
    u32 *nL = (u32 *) nextLine;
    u32 currentPixel;
    u32 nextPixel;
    
    finish = (u8 *) bP + ((width+2) << 1);
    nextPixel = *bP++;
    
    do {
      currentPixel = nextPixel;
      nextPixel = *bP++;
      u32 colorA, colorB;
      
#ifdef WORDS_BIGENDIAN
      colorA = currentPixel >> 16;
      colorB = currentPixel & 0xffff;
#else
      colorA = currentPixel & 0xffff;
      colorB = (currentPixel & 0xffff0000) >> 16;
#endif

      *(dP) = colorA | colorA<<16;
      *(nL) = 0;

#ifdef WORDS_BIGENDIAN
      colorA = nextPixel >> 16;
#else
      colorA = nextPixel & 0xffff;
#endif

      *(dP + 1) = colorB | (colorB << 16);
      *(nL + 1) = 0;
      
      dP += 2;
      nL += 2;
    } while ((u8 *) bP < finish);
    
    srcPtr += srcPitch;
    dstPtr += dstPitch * 2;
    nextLine += dstPitch * 2;
  }
  while (--height);
}

void Scanlines32(u8 *srcPtr, u32 srcPitch, u8 * /* deltaPtr */,
                 u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u8 *nextLine, *finish;
  
  nextLine = dstPtr + dstPitch;
  
  do {
    u32 *bP = (u32 *) srcPtr;
    u32 *dP = (u32 *) dstPtr;
    u32 *nL = (u32 *) nextLine;
    u32 currentPixel;
    u32 nextPixel;
    
    finish = (u8 *) bP + ((width+1) << 2);
    nextPixel = *bP++;
    
    do {
      currentPixel = nextPixel;
      nextPixel = *bP++;
      
      u32 colorA, colorB;
        
      colorA = currentPixel;
      colorB = nextPixel;

      *(dP) = colorA;
      *(dP+1) = colorA;
      *(nL) = 0;
      *(nL+1) = 0;

      nextPixel = *bP++;
      colorA = nextPixel;

      *(dP + 2) = colorB;
      *(dP + 3) = colorB;
      *(nL+2) = 0;      
      *(nL+3) = 0;
      
      dP += 4;
      nL += 4;
    } while ((u8 *) bP < finish);
    
    srcPtr += srcPitch;
    dstPtr += dstPitch*2;
    nextLine += dstPitch*2;
  }
  while (--height);
}
