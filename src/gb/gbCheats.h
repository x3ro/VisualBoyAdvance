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
#ifndef __VBA_GB_GBCHEATS_H
#define __VBA_GB_GBCHEATS_H

#include "../System.h"

struct gbXxCheat {
  char cheatDesc[100];
  char cheatCode[20];
};

struct gbCheat {
  char cheatCode[20];
  char cheatDesc[32];
  u16 address;
  int code;
  u8 compare;
  u8 value;
  bool enabled;
};

extern void gbCheatsSaveGame(gzFile);
extern void gbCheatsReadGame(gzFile, int);
extern void gbCheatsSaveCheatList(const char *);
extern bool gbCheatsLoadCheatList(const char *);
extern bool gbCheatReadGSCodeFile(const char *);

extern void gbAddGsCheat(const char *, const char*);
extern void gbAddGgCheat(const char *, const char*);
extern void gbCheatRemove(int);
extern void gbCheatRemoveAll();
extern void gbCheatEnable(int);
extern void gbCheatDisable(int);
extern u8 gbCheatRead(u16);

extern int gbCheatNumber;
extern gbCheat gbCheatList[100];
extern bool gbCheatMap[0x10000];
#endif

