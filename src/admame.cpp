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

#ifdef MMX
extern "C" bool cpu_mmx;

/* Suggested in "Intel Optimization" for Pentium II */
#define ASM_JUMP_ALIGN ".p2align 4\n"

static void internal_scale2x_16_def(u16 *dst0, u16* dst1, const u16* src0, const u16* src1, const u16* src2, unsigned count) {
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

static __inline__ void internal_scale2x_16_mmx_single(u16* dst, const u16* src0, const u16* src1, const u16* src2, unsigned count) {
	/* always do the first and last run */
	count -= 2*4;

#ifdef __GNUC__        
	__asm__ __volatile__(
/* first run */
		/* set the current, current_pre, current_next registers */
		"pxor %%mm0,%%mm0\n" /* use a fake black out of screen */
		"movq 0(%1),%%mm7\n"
		"movq 8(%1),%%mm1\n"
		"psrlq $48,%%mm0\n"
		"psllq $48,%%mm1\n"
		"movq %%mm7,%%mm2\n"
		"movq %%mm7,%%mm3\n"
		"psllq $16,%%mm2\n"
		"psrlq $16,%%mm3\n"
		"por %%mm2,%%mm0\n"
		"por %%mm3,%%mm1\n"

		/* current_upper */
		"movq (%0),%%mm6\n"

		/* compute the upper-left pixel for dst0 on %%mm2 */
		/* compute the upper-right pixel for dst0 on %%mm4 */
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"movq %%mm0,%%mm3\n"
		"movq %%mm1,%%mm5\n"
		"pcmpeqw %%mm6,%%mm2\n"
		"pcmpeqw %%mm6,%%mm4\n"
		"pcmpeqw (%2),%%mm3\n"
		"pcmpeqw (%2),%%mm5\n"
		"pandn %%mm2,%%mm3\n"
		"pandn %%mm4,%%mm5\n"
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"pcmpeqw %%mm1,%%mm2\n"
		"pcmpeqw %%mm0,%%mm4\n"
		"pandn %%mm3,%%mm2\n"
		"pandn %%mm5,%%mm4\n"
		"movq %%mm2,%%mm3\n"
		"movq %%mm4,%%mm5\n"
		"pand %%mm6,%%mm2\n"
		"pand %%mm6,%%mm4\n"
		"pandn %%mm7,%%mm3\n"
		"pandn %%mm7,%%mm5\n"
		"por %%mm3,%%mm2\n"
		"por %%mm5,%%mm4\n"

		/* set *dst0 */
		"movq %%mm2,%%mm3\n"
		"punpcklwd %%mm4,%%mm2\n"
		"punpckhwd %%mm4,%%mm3\n"
		"movq %%mm2,(%3)\n"
		"movq %%mm3,8(%3)\n"

		/* next */
		"addl $8,%0\n"
		"addl $8,%1\n"
		"addl $8,%2\n"
		"addl $16,%3\n"

/* central runs */
		"shrl $2,%4\n"
		"jz 1f\n"
		ASM_JUMP_ALIGN
		"0:\n"

		/* set the current, current_pre, current_next registers */
		"movq -8(%1),%%mm0\n"
		"movq (%1),%%mm7\n"
		"movq 8(%1),%%mm1\n"
		"psrlq $48,%%mm0\n"
		"psllq $48,%%mm1\n"
		"movq %%mm7,%%mm2\n"
		"movq %%mm7,%%mm3\n"
		"psllq $16,%%mm2\n"
		"psrlq $16,%%mm3\n"
		"por %%mm2,%%mm0\n"
		"por %%mm3,%%mm1\n"

		/* current_upper */
		"movq (%0),%%mm6\n"

		/* compute the upper-left pixel for dst0 on %%mm2 */
		/* compute the upper-right pixel for dst0 on %%mm4 */
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"movq %%mm0,%%mm3\n"
		"movq %%mm1,%%mm5\n"
		"pcmpeqw %%mm6,%%mm2\n"
		"pcmpeqw %%mm6,%%mm4\n"
		"pcmpeqw (%2),%%mm3\n"
		"pcmpeqw (%2),%%mm5\n"
		"pandn %%mm2,%%mm3\n"
		"pandn %%mm4,%%mm5\n"
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"pcmpeqw %%mm1,%%mm2\n"
		"pcmpeqw %%mm0,%%mm4\n"
		"pandn %%mm3,%%mm2\n"
		"pandn %%mm5,%%mm4\n"
		"movq %%mm2,%%mm3\n"
		"movq %%mm4,%%mm5\n"
		"pand %%mm6,%%mm2\n"
		"pand %%mm6,%%mm4\n"
		"pandn %%mm7,%%mm3\n"
		"pandn %%mm7,%%mm5\n"
		"por %%mm3,%%mm2\n"
		"por %%mm5,%%mm4\n"

		/* set *dst0 */
		"movq %%mm2,%%mm3\n"
		"punpcklwd %%mm4,%%mm2\n"
		"punpckhwd %%mm4,%%mm3\n"
		"movq %%mm2,(%3)\n"
		"movq %%mm3,8(%3)\n"

		/* next */
		"addl $8,%0\n"
		"addl $8,%1\n"
		"addl $8,%2\n"
		"addl $16,%3\n"

		"decl %4\n"
		"jnz 0b\n"
		"1:\n"

/* final run */
		/* set the current, current_pre, current_next registers */
		"movq -8(%1),%%mm0\n"
		"movq (%1),%%mm7\n"
		"pxor %%mm1,%%mm1\n" /* use a fake black out of screen */
		"psrlq $48,%%mm0\n"
		"psllq $48,%%mm1\n"
		"movq %%mm7,%%mm2\n"
		"movq %%mm7,%%mm3\n"
		"psllq $16,%%mm2\n"
		"psrlq $16,%%mm3\n"
		"por %%mm2,%%mm0\n"
		"por %%mm3,%%mm1\n"

		/* current_upper */
		"movq (%0),%%mm6\n"

		/* compute the upper-left pixel for dst0 on %%mm2 */
		/* compute the upper-right pixel for dst0 on %%mm4 */
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"movq %%mm0,%%mm3\n"
		"movq %%mm1,%%mm5\n"
		"pcmpeqw %%mm6,%%mm2\n"
		"pcmpeqw %%mm6,%%mm4\n"
		"pcmpeqw (%2),%%mm3\n"
		"pcmpeqw (%2),%%mm5\n"
		"pandn %%mm2,%%mm3\n"
		"pandn %%mm4,%%mm5\n"
		"movq %%mm0,%%mm2\n"
		"movq %%mm1,%%mm4\n"
		"pcmpeqw %%mm1,%%mm2\n"
		"pcmpeqw %%mm0,%%mm4\n"
		"pandn %%mm3,%%mm2\n"
		"pandn %%mm5,%%mm4\n"
		"movq %%mm2,%%mm3\n"
		"movq %%mm4,%%mm5\n"
		"pand %%mm6,%%mm2\n"
		"pand %%mm6,%%mm4\n"
		"pandn %%mm7,%%mm3\n"
		"pandn %%mm7,%%mm5\n"
		"por %%mm3,%%mm2\n"
		"por %%mm5,%%mm4\n"

		/* set *dst0 */
		"movq %%mm2,%%mm3\n"
		"punpcklwd %%mm4,%%mm2\n"
		"punpckhwd %%mm4,%%mm3\n"
		"movq %%mm2,(%3)\n"
		"movq %%mm3,8(%3)\n"

		: "+r" (src0), "+r" (src1), "+r" (src2), "+r" (dst), "+r" (count)
		:
		: "cc"
	);
#endif
}

static void internal_scale2x_16_mmx(u16* dst0, u16* dst1, const u16* src0, const u16* src1, const u16* src2, unsigned count) {
  //	assert( count >= 2*4 );
	internal_scale2x_16_mmx_single(dst0, src0, src1, src2, count);
	internal_scale2x_16_mmx_single(dst1, src2, src1, src0, count);
}
#endif

void AdMame2x(u8 *srcPtr, u32 srcPitch, u8 *deltaPtr,
              u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u16 *dst0 = (u16 *)dstPtr;
  u16 *dst1 = dst0 + (dstPitch/2);
  
  u16 *src0 = (u16 *)srcPtr;
  u16 *src1 = src0 + (srcPitch/2);
  u16 *src2 = src1 + (srcPitch/2);
#ifdef MMX
  if(cpu_mmx) {
    internal_scale2x_16_mmx(dst0, dst1, src0, src0, src1, width);
  
    int count = height;

    count -= 2;
    while(count) {
      dst0 += dstPitch;
      dst1 += dstPitch;
      internal_scale2x_16_mmx(dst0, dst1, src0, src1, src2, width);
      src0 = src1;
      src1 = src2;
      src2 += srcPitch/2;
      --count;
    }
    dst0 += dstPitch;
    dst1 += dstPitch;
    internal_scale2x_16_mmx(dst0, dst1, src0, src1, src1, width);
  } else {
#endif
    internal_scale2x_16_def(dst0, dst1, src0, src0, src1, width);
  
    int count = height;

    count -= 2;
    while(count) {
      dst0 += dstPitch;
      dst1 += dstPitch;
      internal_scale2x_16_def(dst0, dst1, src0, src1, src2, width);
      src0 = src1;
      src1 = src2;
      src2 += srcPitch/2;
      --count;
    }
    dst0 += dstPitch;
    dst1 += dstPitch;
    internal_scale2x_16_def(dst0, dst1, src0, src1, src1, width);
#ifdef MMX
  }
#endif
}
