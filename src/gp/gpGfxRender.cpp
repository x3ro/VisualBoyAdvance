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
#ifdef GP_EMULATION
#include <memory.h>
#include <stdio.h>
#include "../System.h"
#include "gpIO.h"

extern u8 *gpIo;
extern u8 *pix;
extern u32 GPReadByte(u32);
extern u32 GPReadHalfWord(u32);

void gpGfxRender8bpp()
{
  u32 *palette = (u32 *)&gpIo[0xa00400];
  u32 addr = (LCDSADDR1->lcdbank << 22) + (LCDSADDR1->lcdbaseu << 1);
  u16 *dest = (u16 *)pix;
  //dest += 321;
  dest += (321*240);
  //u32 addr = start;

  int add = LCDCON3->hozval + 1; //LCDSADDR3->pagewidth * 2;
  for(int i = 0; i < 240; i++) {
    u32 a = addr;
    for(int j = 0; j < 320; j++) {
      u8 p = GPReadByte(a);
      a += add;
      u32 color = palette[p];
      *dest++ = systemColorMap16[color];
    }
    dest++;
    dest -= 321*2;
    addr++;
  }
}

void gpGfxRender16bpp()
{
  u32 addr = (LCDSADDR1->lcdbank << 22) + (LCDSADDR1->lcdbaseu << 1);
  u16 *dest = (u16 *)pix;

  //dest += 321;
  dest += (321*240);
  //dest += (320 * 240) - 1;
  //u32 addr = start;

  int add = (LCDCON3->hozval + 1) << 1;
  for(int i = 0; i < 240; i++) {
    u32 a = addr;
    for(int j = 0; j < 320; j++) {
      u16 p = GPReadHalfWord(a);
      a += add; //change back to 'add'
      *dest++ = systemColorMap16[p];
    }
    dest++;
    dest -= 321*2;
    addr+= 2;
  }
}

void gpGfxRender()
{
  if(!(LCDCON1->envid)) {
    memset(pix, 0, 4*320*240);
    return;
  }

  int mode = (LCDCON1->bppmode);
  
  switch(mode) {
  case 11:
    gpGfxRender8bpp();
    break;
  case 12:
    gpGfxRender16bpp();
    break;
  default:
    printf("Unknown lcd mode %d\n", LCDCON1->bppmode);
    break;
  }
}
#endif
