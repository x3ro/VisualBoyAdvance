/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1999-2002 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * In addition, as a special exception, Andrea Mazzoleni
 * gives permission to link the code of this program with
 * the MAME library (or with modified versions of MAME that use the
 * same license as MAME), and distribute linked combinations including
 * the two.  You must obey the GNU General Public License in all
 * respects for all of the code used other than MAME.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

/*
 * Alternatively at the previous license terms, you are allowed to use this
 * code in your program with these conditions:
 * - the program is not used in commercial activities.
 * - the whole source code of the program is released with the binary.
 */

#include "System.h"

static void internal_scale_2x_16_def(u16 *dst0, u16* dst1, const u16* src0, const u16* src1, const u16* src2, unsigned count) {
        /* first pixel */
        dst0[0] = src1[0];
        dst1[0] = src1[0];
        if (src1[1] == src0[0] && src2[0] != src0[0])
                dst0[1] =src0[0];
        else
                dst0[1] =src1[0];
        if (src1[1] == src2[0] && src0[0] != src2[0])
                dst1[1] =src2[0];
        else
                dst1[1] =src1[0];
        ++src0;
        ++src1;
        ++src2;
        dst0 += 2;
        dst1 += 2;

        /* central pixels */
        count -= 2;
        while (count) {
                if (src1[-1] == src0[0] && src2[0] != src0[0] && src1[1] != src0[0])
                        dst0[0] = src0[0];
                else
                        dst0[0] = src1[0];
                if (src1[1] == src0[0] && src2[0] != src0[0] && src1[-1] != src0[0])
                        dst0[1] =src0[0];
                else
                        dst0[1] =src1[0];

                if (src1[-1] == src2[0] && src0[0] != src2[0] && src1[1] != src2[0])
                        dst1[0] =src2[0];
                else
                        dst1[0] =src1[0];
                if (src1[1] == src2[0] && src0[0] != src2[0] && src1[-1] != src2[0])
                        dst1[1] =src2[0];
                else
                        dst1[1] =src1[0];

                ++src0;
                ++src1;
                ++src2;
                dst0 += 2;
                dst1 += 2;
                --count;
        }

        /* last pixel */
        if (src1[-1] == src0[0] && src2[0] != src0[0])
                dst0[0] =src0[0];
        else
                dst0[0] =src1[0];
        if (src1[-1] == src2[0] && src0[0] != src2[0])
                dst1[0] =src2[0];
        else
                dst1[0] =src1[0];
        dst0[1] =src1[0];
        dst1[1] =src1[0];
}

void AdMame2x(u8 *srcPtr, u32 srcPitch, u8 *deltaPtr,
              u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u16 *dst0 = (u16 *)dstPtr;
  u16 *dst1 = dst0 + (dstPitch/2);
  
  u16 *src0 = (u16 *)srcPtr;
  u16 *src1 = src0 + (srcPitch/2);
  u16 *src2 = src1 + (srcPitch/2);
  internal_scale_2x_16_def(dst0, dst1, src0, src0, src1, width);
  
  int count = height;

  count -= 2;
  while(count) {
    dst0 += dstPitch;
    dst1 += dstPitch;
    internal_scale_2x_16_def(dst0, dst1, src0, src1, src2, width);
    src0 = src1;
    src1 = src2;
    src2 += srcPitch/2;
    --count;
  }
  dst0 += dstPitch;
  dst1 += dstPitch;
  internal_scale_2x_16_def(dst0, dst1, src0, src1, src1, width);    
}
