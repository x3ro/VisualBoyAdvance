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
#ifndef VBA_PORT_H
#define VBA_PORT_H

// swaps a 16-bit value
static inline u16 swap16(u16 v)
{
  return (v<<8)|(v>>8);
}

// swaps a 32-bit value
static inline u32 swap32(u32 v)
{
  return (v<<24)|((v<<8)&0xff0000)|((v>>8)&0xff00)|(v>>24);
}

#ifdef WORDS_BIGENDIAN
#define READ16LE(x) \
  swap16(*((u16 *)(x)))
#define READ32LE(x) \
  swap32(*((u32 *)(x)))
#define WRITE16LE(x,v) \
  *((u16 *)x) = swap16((v))
#define WRITE32LE(x,v) \
  *((u32 *)x) = swap32((v))
#else
#define READ16LE(x) \
  *((u16 *)x)
#define READ32LE(x) \
  *((u32 *)x)
#define WRITE16LE(x,v) \
  *((u16 *)x) = (v)
#define WRITE32LE(x,v) \
  *((u32 *)x) = (v)
#endif

#endif
