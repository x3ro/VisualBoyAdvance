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
#ifndef GBA_CHEATS_H
#define GBA_CHEATS_H

struct CheatSearch {
  u8 wRAM[0x40000];
  u8 iRAM[0x8000];
  //  u8 fRAM[0x10000];
  //  u8 eRAM[0x200];

  u32 wBITS[0x40000 >> 3];
  u32 iBITS[0x8000 >> 3];
  //  u32 fBITS[0x10000 >> 3];
  //  u32 eBITS[0x200 >> 3];
};

struct CheatsData {
  int code;
  int size;
  int status;
  bool enabled;
  u32 address;
  u32 value;
  u32 oldValue;
  char codestring[20];
  char desc[32];
};

enum {
  GBA_EQUAL,
  GBA_NOT_EQUAL,
  GBA_LESS_THAN,
  GBA_LESS_EQUAL,
  GBA_GREATER_THAN,
  GBA_GREATER_EQUAL
};

enum {
  SIZE_8,
  SIZE_16,
  SIZE_32
};

#define COMPARE(comp,a,b) \
 ((comp) == GBA_EQUAL ? (a) == (b) : \
  (comp) == GBA_NOT_EQUAL ? (a) != (b) : \
  (comp) == GBA_LESS_THAN ? (a) < (b) : \
  (comp) == GBA_LESS_EQUAL ? (a) <= (b) : \
  (comp) == GBA_GREATER_THAN ? (a) > (b) : \
  (a) >= (b))

#define UNSIGNED_DATA(s,m,o) \
 ((s) == SIZE_8 ? (u8)(*((m)+(o))) : \
  (s) == SIZE_16 ? (u16)(*((u16*)((m)+(o)))) : \
  (u32)(*((u32*)((m)+(o)))))

#define SIGNED_DATA(s,m,o) \
 ((s) == SIZE_8 ? (s8)(*((m)+(o))) : \
  (s) == SIZE_16 ? (s16)(*((s16*)((m)+(o)))) : \
  (s32)(*((s32*)((m)+(o)))))

#define BIT_CLEAR(a,v) \
 (a)[(v) >> 5] &= ~(1 << ((v) & 31))

#define BIT_SET(a,v) \
 (a)[(v) >> 5] |= 1 << ((v) & 31)

#define TEST_BIT(a,v) \
 ((a)[(v) >> 5] & (1 << ((v) & 31)))

extern void cheatsReset();
extern void cheatsSearchChange(int, int, bool);
extern void cheatsSearchValue(int, int, bool, u32);
extern void cheatsUpdateValues();
extern int  cheatsGetCount(int);

extern void cheatsAdd(char *,char *,u32,u32,int,int);
extern void cheatsAddCheatCode(char *code, char *desc);
extern void cheatsAddGSACode(char *code, char *desc, bool v3);
extern void cheatsAddCBACode(char *code, char *desc);
extern bool cheatsImportGSACodeFile(char *name, int game);
extern void cheatsDelete(int number, bool restore);
extern void cheatsDeleteAll(bool restore);
extern void cheatsEnable(int number);
extern void cheatsDisable(int number);
extern void cheatsSaveGame(gzFile file);
extern void cheatsReadGame(gzFile file);
extern void cheatsSaveCheatList(char *file);
extern bool cheatsLoadCheatList(char *file);
extern void cheatsWriteMemory(u32 *, u32, u32);
extern void cheatsWriteHalfWord(u16 *, u16, u16);
extern void cheatsWriteByte(u8 *, u8);
extern int cheatsCheckKeys(u32,u32);
extern int cheatsNumber;
extern CheatsData cheatsList[100];

extern CheatSearch cheatSearch;

#endif // GBA_CHEATS_H
