// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2005-2006 Forgotten and the VBA development team

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or(at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdarg.h>
#include <string.h>

#include "GBA.h"
#include "GBAinline.h"
#include "Globals.h"
#include "Gfx.h"
#include "EEprom.h"
#include "Flash.h"
#include "Sound.h"
#include "Sram.h"
#include "bios.h"
#include "unzip.h"
#include "Cheats.h"
#include "NLS.h"
#include "elf.h"
#include "Util.h"
#include "Port.h"
#include "agbprint.h"
#include "romcheck.h"
#ifdef PROFILING
#include "prof/prof.h"
#endif

#define UPDATE_REG(address, value)\
  {\
    WRITE16LE(((u16 *)&ioMem[address]),value);\
  }\

#define ARM_PREFETCH \
  {\
    cpuPrefetch[0] = CPUReadMemoryQuick(armNextPC);\
    cpuPrefetch[1] = CPUReadMemoryQuick(armNextPC+4);\
  }

#define THUMB_PREFETCH \
  {\
    cpuPrefetch[0] = CPUReadHalfWordQuick(armNextPC);\
    cpuPrefetch[1] = CPUReadHalfWordQuick(armNextPC+2);\
  }

#define ARM_PREFETCH_NEXT \
  cpuPrefetch[1] = CPUReadMemoryQuick(armNextPC+4);

#define THUMB_PREFETCH_NEXT\
  cpuPrefetch[1] = CPUReadHalfWordQuick(armNextPC+2);


extern int emulating;
int SWITicks = 0;
int IRQTicks = 0;

u32 mastercode = 0;
int layerEnableDelay = 0;
bool busPrefetch = false;
bool busPrefetchEnable = false;
u32 busPrefetchCount = 0;
int cpuDmaTicksToUpdate = 0;
int cpuDmaCount = 0;
bool cpuDmaHack = false;
u32 cpuDmaLast = 0;
int dummyAddress = 0;

bool cpuBreakLoop = false;
int cpuNextEvent = 0;

int gbaSaveType = 0; // used to remember the save type on reset
bool intState = false;
bool stopState = false;
bool holdState = false;
int holdType = 0;
bool cpuSramEnabled = true;
bool cpuFlashEnabled = true;
bool cpuEEPROMEnabled = true;
bool cpuEEPROMSensorEnabled = false;

u32 cpuPrefetch[2];

int cpuTotalTicks = 0;
#ifdef PROFILING
int profilingTicks = 0;
int profilingTicksReload = 0;
static profile_segment *profilSegment = NULL;
#endif

#ifdef BKPT_SUPPORT
u8 freezeWorkRAM[0x40000];
u8 freezeInternalRAM[0x8000];
u8 freezeVRAM[0x18000];
u8 freezePRAM[0x400];
u8 freezeOAM[0x400];
bool debugger_last;
#endif

int lcdTicks = (DEF_USEBIOS && !DEF_SKIPBIOS) ? 1008 : 208;
u8 timerOnOffDelay = 0;
u16 timer0Value = 0;
bool timer0On = false;
int timer0Ticks = 0;
int timer0Reload = 0;
int timer0ClockReload  = 0;
u16 timer1Value = 0;
bool timer1On = false;
int timer1Ticks = 0;
int timer1Reload = 0;
int timer1ClockReload  = 0;
u16 timer2Value = 0;
bool timer2On = false;
int timer2Ticks = 0;
int timer2Reload = 0;
int timer2ClockReload  = 0;
u16 timer3Value = 0;
bool timer3On = false;
int timer3Ticks = 0;
int timer3Reload = 0;
int timer3ClockReload  = 0;
u32 dma0Source = 0;
u32 dma0Dest = 0;
u32 dma1Source = 0;
u32 dma1Dest = 0;
u32 dma2Source = 0;
u32 dma2Dest = 0;
u32 dma3Source = 0;
u32 dma3Dest = 0;
void (*cpuSaveGameFunc)(u32,u8) = flashSaveDecide;
void (*renderLine)() = mode0RenderLine;
bool fxOn = false;
bool windowOn = false;
int frameCount = 0;
char buffer[1024];
FILE *out = NULL;
u32 lastTime = 0;
int count = 0;

int capture = 0;
int capturePrevious = 0;
int captureNumber = 0;

static const int TIMER_TICKS[4] = {
  0,
  6,
  8,
  10
};

static const u32  objTilesAddress [3] = {0x010000, 0x014000, 0x014000};
static const u8 gamepakRamWaitState[4] = { 4, 3, 2, 8 };
static const u8 gamepakWaitState[4] =  { 4, 3, 2, 8 };
static const u8 gamepakWaitState0[2] = { 2, 1 };
static const u8 gamepakWaitState1[2] = { 4, 1 };
static const u8 gamepakWaitState2[2] = { 8, 1 };
static const bool isInRom [16] =
  { false, false, false, false, false, false, false, false,
    true, true, true, true, true, true, false, false };

static u8 memoryWait[16] =
  { 0, 0, 2, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 0 };
static u8 memoryWait32[16] =
  { 0, 0, 5, 0, 0, 1, 1, 0, 7, 7, 9, 9, 13, 13, 4, 0 };
static u8 memoryWaitSeq[16] =
  { 0, 0, 2, 0, 0, 0, 0, 0, 2, 2, 4, 4, 8, 8, 4, 0 };
static u8 memoryWaitSeq32[16] =
  { 0, 0, 5, 0, 0, 1, 1, 0, 5, 5, 9, 9, 17, 17, 4, 0 };

// The videoMemoryWait constants are used to add some waitstates
// if the opcode access video memory data outside of vblank/hblank
// It seems to happen on only one ticks for each pixel.
// Not used for now (too problematic with current code).
//const u8 videoMemoryWait[16] =
//  {0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0};


u8 biosProtected[4];

extern u32 myROM[];
extern u32 myROM_size;

static variable_desc saveGameStruct[] = {
  { &DISPCNT  , sizeof(u16) },
  { &DISPSTAT , sizeof(u16) },
  { &VCOUNT   , sizeof(u16) },
  { &BG0CNT   , sizeof(u16) },
  { &BG1CNT   , sizeof(u16) },
  { &BG2CNT   , sizeof(u16) },
  { &BG3CNT   , sizeof(u16) },
  { &BG0HOFS  , sizeof(u16) },
  { &BG0VOFS  , sizeof(u16) },
  { &BG1HOFS  , sizeof(u16) },
  { &BG1VOFS  , sizeof(u16) },
  { &BG2HOFS  , sizeof(u16) },
  { &BG2VOFS  , sizeof(u16) },
  { &BG3HOFS  , sizeof(u16) },
  { &BG3VOFS  , sizeof(u16) },
  { &BG2PA    , sizeof(u16) },
  { &BG2PB    , sizeof(u16) },
  { &BG2PC    , sizeof(u16) },
  { &BG2PD    , sizeof(u16) },
  { &BG2X_L   , sizeof(u16) },
  { &BG2X_H   , sizeof(u16) },
  { &BG2Y_L   , sizeof(u16) },
  { &BG2Y_H   , sizeof(u16) },
  { &BG3PA    , sizeof(u16) },
  { &BG3PB    , sizeof(u16) },
  { &BG3PC    , sizeof(u16) },
  { &BG3PD    , sizeof(u16) },
  { &BG3X_L   , sizeof(u16) },
  { &BG3X_H   , sizeof(u16) },
  { &BG3Y_L   , sizeof(u16) },
  { &BG3Y_H   , sizeof(u16) },
  { &WIN0H    , sizeof(u16) },
  { &WIN1H    , sizeof(u16) },
  { &WIN0V    , sizeof(u16) },
  { &WIN1V    , sizeof(u16) },
  { &WININ    , sizeof(u16) },
  { &WINOUT   , sizeof(u16) },
  { &MOSAIC   , sizeof(u16) },
  { &BLDMOD   , sizeof(u16) },
  { &COLEV    , sizeof(u16) },
  { &COLY     , sizeof(u16) },
  { &DM0SAD_L , sizeof(u16) },
  { &DM0SAD_H , sizeof(u16) },
  { &DM0DAD_L , sizeof(u16) },
  { &DM0DAD_H , sizeof(u16) },
  { &DM0CNT_L , sizeof(u16) },
  { &DM0CNT_H , sizeof(u16) },
  { &DM1SAD_L , sizeof(u16) },
  { &DM1SAD_H , sizeof(u16) },
  { &DM1DAD_L , sizeof(u16) },
  { &DM1DAD_H , sizeof(u16) },
  { &DM1CNT_L , sizeof(u16) },
  { &DM1CNT_H , sizeof(u16) },
  { &DM2SAD_L , sizeof(u16) },
  { &DM2SAD_H , sizeof(u16) },
  { &DM2DAD_L , sizeof(u16) },
  { &DM2DAD_H , sizeof(u16) },
  { &DM2CNT_L , sizeof(u16) },
  { &DM2CNT_H , sizeof(u16) },
  { &DM3SAD_L , sizeof(u16) },
  { &DM3SAD_H , sizeof(u16) },
  { &DM3DAD_L , sizeof(u16) },
  { &DM3DAD_H , sizeof(u16) },
  { &DM3CNT_L , sizeof(u16) },
  { &DM3CNT_H , sizeof(u16) },
  { &TM0D     , sizeof(u16) },
  { &TM0CNT   , sizeof(u16) },
  { &TM1D     , sizeof(u16) },
  { &TM1CNT   , sizeof(u16) },
  { &TM2D     , sizeof(u16) },
  { &TM2CNT   , sizeof(u16) },
  { &TM3D     , sizeof(u16) },
  { &TM3CNT   , sizeof(u16) },
  { &P1       , sizeof(u16) },
  { &IE       , sizeof(u16) },
  { &IF       , sizeof(u16) },
  { &IME      , sizeof(u16) },
  { &holdState, sizeof(bool) },
  { &holdType, sizeof(int) },
  { &lcdTicks, sizeof(int) },
  { &timer0On , sizeof(bool) },
  { &timer0Ticks , sizeof(int) },
  { &timer0Reload , sizeof(int) },
  { &timer0ClockReload  , sizeof(int) },
  { &timer1On , sizeof(bool) },
  { &timer1Ticks , sizeof(int) },
  { &timer1Reload , sizeof(int) },
  { &timer1ClockReload  , sizeof(int) },
  { &timer2On , sizeof(bool) },
  { &timer2Ticks , sizeof(int) },
  { &timer2Reload , sizeof(int) },
  { &timer2ClockReload  , sizeof(int) },
  { &timer3On , sizeof(bool) },
  { &timer3Ticks , sizeof(int) },
  { &timer3Reload , sizeof(int) },
  { &timer3ClockReload  , sizeof(int) },
  { &dma0Source , sizeof(u32) },
  { &dma0Dest , sizeof(u32) },
  { &dma1Source , sizeof(u32) },
  { &dma1Dest , sizeof(u32) },
  { &dma2Source , sizeof(u32) },
  { &dma2Dest , sizeof(u32) },
  { &dma3Source , sizeof(u32) },
  { &dma3Dest , sizeof(u32) },
  { &fxOn, sizeof(bool) },
  { &windowOn, sizeof(bool) },
  { &N_FLAG , sizeof(bool) },
  { &C_FLAG , sizeof(bool) },
  { &Z_FLAG , sizeof(bool) },
  { &V_FLAG , sizeof(bool) },
  { &armState , sizeof(bool) },
  { &armIrqEnable , sizeof(bool) },
  { &armNextPC , sizeof(u32) },
  { &armMode , sizeof(int) },
  { &saveType , sizeof(int) },
  { NULL, 0 }
};

static int romSize = 0x2000000;

#ifdef PROFILING
void cpuProfil(profile_segment *seg)
{
    profilSegment = seg;
}

void cpuEnableProfiling(int hz)
{
  if(hz == 0)
    hz = 100;
  profilingTicks = profilingTicksReload = 16777216 / hz;
  profSetHertz(hz);
}
#endif


// Waitstates when accessing data
static inline int dataTicksAccess16(u32 address) // DATA 8/16bits NON SEQ
{
  int addr = (address>>24)&15;
  int value =  memoryWait[addr];

  if ((addr>=0x08) || (addr < 0x02))
  {
    busPrefetchCount=0;
    busPrefetch=false;
  }
  else if (busPrefetch)
  {
    int waitState = value;
    if (!waitState)
      waitState = 1;
    busPrefetchCount = ((busPrefetchCount+1)<<waitState) - 1;
  }

  return value;
}

static inline int dataTicksAccess32(u32 address) // DATA 32bits NON SEQ
{
  int addr = (address>>24)&15;
  int value = memoryWait32[addr];

  if ((addr>=0x08) || (addr < 0x02))
  {
    busPrefetchCount=0;
    busPrefetch=false;
  }
  else if (busPrefetch)
  {
    int waitState = value;
    if (!waitState)
      waitState = 1;
    busPrefetchCount = ((busPrefetchCount+1)<<waitState) - 1;
  }

  return value;
}

static inline int dataTicksAccessSeq16(u32 address)// DATA 8/16bits SEQ
{
  int addr = (address>>24)&15;
  int value = memoryWaitSeq[addr];

  if ((addr>=0x08) || (addr < 0x02))
  {
    busPrefetchCount=0;
    busPrefetch=false;
  }
  else if (busPrefetch)
  {
    int waitState = value;
    if (!waitState)
      waitState = 1;
    busPrefetchCount = ((busPrefetchCount+1)<<waitState) - 1;
  }

  return value;
}

static inline int dataTicksAccessSeq32(u32 address)// DATA 32bits SEQ
{
  int addr = (address>>24)&15;
  int value =  memoryWaitSeq32[addr];

  if ((addr>=0x08) || (addr < 0x02))
  {
    busPrefetchCount=0;
    busPrefetch=false;
  }
  else if (busPrefetch)
  {
    int waitState = value;
    if (!waitState)
      waitState = 1;
    busPrefetchCount = ((busPrefetchCount+1)<<waitState) - 1;
  }

  return value;
}


// Waitstates when executing opcode
static inline int codeTicksAccess16(u32 address) // THUMB NON SEQ
{
  int addr = (address>>24)&15;

  if ((addr>=0x08) && (addr<=0x0D))
  {
    if (busPrefetchCount&0x1)
    {
      if (busPrefetchCount&0x2)
      {
        busPrefetchCount = ((busPrefetchCount&0xFF)>>2) | (busPrefetchCount&0xFFFFFF00);
        return 0;
      }
      busPrefetchCount = ((busPrefetchCount&0xFF)>>1) | (busPrefetchCount&0xFFFFFF00);
      return memoryWaitSeq[addr]-1;
    }
    else
    {
      busPrefetchCount=0;
      return memoryWait[addr];
    }
  }
  else
  {
    busPrefetchCount = 0;
    return memoryWait[addr];
  }
}

static inline int codeTicksAccess32(u32 address) // ARM NON SEQ
{
  int addr = (address>>24)&15;

  if ((addr>=0x08) && (addr<=0x0D))
  {
    if (busPrefetchCount&0x1)
    {
      if (busPrefetchCount&0x2)
      {
        busPrefetchCount = ((busPrefetchCount&0xFF)>>2) | (busPrefetchCount&0xFFFFFF00);
        return 0;
      }
      busPrefetchCount = ((busPrefetchCount&0xFF)>>1) | (busPrefetchCount&0xFFFFFF00);
      return memoryWaitSeq[addr] - 1;
    }
    else
    {
      busPrefetchCount = 0;
      return memoryWait32[addr];
    }
  }
  else
  {
    busPrefetchCount = 0;
    return memoryWait32[addr];
  }
}

static inline int codeTicksAccessSeq16(u32 address) // THUMB SEQ
{
  int addr = (address>>24)&15;

  if ((addr>=0x08) && (addr<=0x0D))
  {
    if (busPrefetchCount&0x1)
    {
      busPrefetchCount = ((busPrefetchCount&0xFF)>>1) | (busPrefetchCount&0xFFFFFF00);
      return 0;
    }
    else
    if (busPrefetchCount>0xFF)
    {
      busPrefetchCount=0;
      return memoryWait[addr];
    }
    else
      return memoryWaitSeq[addr];
  }
  else
  {
    busPrefetchCount = 0;
    return memoryWaitSeq[addr];
  }
}

static inline int codeTicksAccessSeq32(u32 address) // ARM SEQ
{
  int addr = (address>>24)&15;

  if ((addr>=0x08) && (addr<=0x0D))
  {
    if (busPrefetchCount&0x1)
    {
      if (busPrefetchCount&0x2)
      {
        busPrefetchCount = ((busPrefetchCount&0xFF)>>2) | (busPrefetchCount&0xFFFFFF00);
        return 0;
      }
      busPrefetchCount = ((busPrefetchCount&0xFF)>>1) | (busPrefetchCount&0xFFFFFF00);
      return memoryWaitSeq[addr];
    }
    else
    if (busPrefetchCount>0xFF)
    {
      busPrefetchCount=0;
      return memoryWait32[addr];
    }
    else
      return memoryWaitSeq32[addr];
  }
  else
  {
    return memoryWaitSeq32[addr];
  }
}


static inline int CPUUpdateTicks()
{
  int cpuLoopTicks = lcdTicks;
  
  if(soundTicks < cpuLoopTicks)
    cpuLoopTicks = soundTicks;
  
  if(timer0On && (timer0Ticks < cpuLoopTicks)) {
    cpuLoopTicks = timer0Ticks;
  }
  if(timer1On && !(TM1CNT & 4) && (timer1Ticks < cpuLoopTicks)) {
    cpuLoopTicks = timer1Ticks;
  }
  if(timer2On && !(TM2CNT & 4) && (timer2Ticks < cpuLoopTicks)) {
    cpuLoopTicks = timer2Ticks;
  }
  if(timer3On && !(TM3CNT & 4) && (timer3Ticks < cpuLoopTicks)) {
    cpuLoopTicks = timer3Ticks;
  }
#ifdef PROFILING
  if(profilingTicksReload != 0) {
    if(profilingTicks < cpuLoopTicks) {
      cpuLoopTicks = profilingTicks;
    }
  }
#endif

  if (SWITicks) {
    if (SWITicks < cpuLoopTicks)
        cpuLoopTicks = SWITicks;
  }

  if (IRQTicks) {
    if (IRQTicks < cpuLoopTicks)
        cpuLoopTicks = IRQTicks;
  }

  return cpuLoopTicks;
}

void CPUUpdateWindow0()
{
  int x00 = WIN0H>>8;
  int x01 = WIN0H & 255;

  if(x00 <= x01) {
    for(int i = 0; i < 240; i++) {
      gfxInWin0[i] = (i >= x00 && i < x01);
    }
  } else {
    for(int i = 0; i < 240; i++) {
      gfxInWin0[i] = (i >= x00 || i < x01);
    }
  }
}

void CPUUpdateWindow1()
{
  int x00 = WIN1H>>8;
  int x01 = WIN1H & 255;

  if(x00 <= x01) {
    for(int i = 0; i < 240; i++) {
      gfxInWin1[i] = (i >= x00 && i < x01);
    }
  } else {
    for(int i = 0; i < 240; i++) {
      gfxInWin1[i] = (i >= x00 || i < x01);
    }
  }
}

extern u32 line0[240];
extern u32 line1[240];
extern u32 line2[240];
extern u32 line3[240];
u32 *cleared_line;

static void init_cleared_line(void) {
  if(cleared_line) return;
  cleared_line = malloc(240*4);
  for(int i = 0; i < 240; i++)
     cleared_line[i] = 0x80000000;
}

static void clear_line(u32 *line) {
  memcpy(line, cleared_line, 240*4);
}

void CPUUpdateRenderBuffers(bool force)
{
  if(!(layerEnable & 0x0100) || force) {
    clear_line(line0);
  }
  if(!(layerEnable & 0x0200) || force) {
    clear_line(line1);
  }
  if(!(layerEnable & 0x0400) || force) {
    clear_line(line2);
  }
  if(!(layerEnable & 0x0800) || force) {
    clear_line(line3);
  }
}

static bool CPUWriteStateGz(gzFile gzFile)
{
  utilWriteInt(gzFile, SAVE_GAME_VERSION);

  utilGzWrite(gzFile, &rom[0xa0], 16);

  utilWriteInt(gzFile, useBios);
  
  utilGzWrite(gzFile, &reg[0], sizeof(reg));

  utilWriteData(gzFile, saveGameStruct);

  // new to version 0.7.1
  utilWriteInt(gzFile, stopState);
  // new to version 0.8
  utilWriteInt(gzFile, IRQTicks);

  utilGzWrite(gzFile, internalRAM, 0x8000);
  utilGzWrite(gzFile, paletteRAM, 0x400);
  utilGzWrite(gzFile, workRAM, 0x40000);
  utilGzWrite(gzFile, vram, 0x20000);
  utilGzWrite(gzFile, oam, 0x400);
  utilGzWrite(gzFile, pix, 4*241*162);
  utilGzWrite(gzFile, ioMem, 0x400);

  eepromSaveGame(gzFile);
  flashSaveGame(gzFile);
  soundSaveGame(gzFile);

  cheatsSaveGame(gzFile);

  // version 1.5
  rtcSaveGame(gzFile);
  
  return true;
}

bool CPUWriteState(const char *file)
{
  gzFile gzFile = utilGzOpen(file, "wb");

  if(gzFile == NULL) {
    systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"), file);
    return false;
  }
  
  bool res = CPUWriteStateGz(gzFile);

  utilGzClose(gzFile);
  
  return res;
}

bool CPUWriteMemState(char *memory, int available)
{
  gzFile gzFile = utilMemGzOpen(memory, available, "w");

  if(gzFile == NULL) {
    return false;
  }

  bool res = CPUWriteStateGz(gzFile);

  long pos = utilGzMemTell(gzFile)+8;

  if(pos >= (available))
    res = false;

  utilGzClose(gzFile);

  return res;
}

static bool CPUReadStateGz(gzFile gzFile)
{
  int version = utilReadInt(gzFile);

  if(version > SAVE_GAME_VERSION || version < SAVE_GAME_VERSION_1) {
    systemMessage(MSG_UNSUPPORTED_VBA_SGM,
                  N_("Unsupported VisualBoyAdvance save game version %d"),
                  version);
    return false;
  }
  
  u8 romname[17];

  utilGzRead(gzFile, romname, 16);

  if(memcmp(&rom[0xa0], romname, 16) != 0) {
    romname[16]=0;
    for(int i = 0; i < 16; i++)
      if(romname[i] < 32)
        romname[i] = 32;
    systemMessage(MSG_CANNOT_LOAD_SGM, N_("Cannot load save game for %s"), romname);
    return false;
  }

  bool ub = utilReadInt(gzFile) ? true : false;

  if(ub != useBios) {
    if(useBios)
      systemMessage(MSG_SAVE_GAME_NOT_USING_BIOS,
                    N_("Save game is not using the BIOS files"));
    else
      systemMessage(MSG_SAVE_GAME_USING_BIOS,
                    N_("Save game is using the BIOS file"));
    return false;
  }

  utilGzRead(gzFile, &reg[0], sizeof(reg));

  utilReadData(gzFile, saveGameStruct);

  if(version < SAVE_GAME_VERSION_3)
    stopState = false;
  else
    stopState = utilReadInt(gzFile) ? true : false;

  if(version < SAVE_GAME_VERSION_4)
  {
    IRQTicks = 0;
    intState = false;
  }
  else
  {
    IRQTicks = utilReadInt(gzFile);
    if (IRQTicks>0)
      intState = true;
    else
    {
      intState = false;
      IRQTicks = 0;
    }
  }
  
  utilGzRead(gzFile, internalRAM, 0x8000);
  utilGzRead(gzFile, paletteRAM, 0x400);
  utilGzRead(gzFile, workRAM, 0x40000);
  utilGzRead(gzFile, vram, 0x20000);
  utilGzRead(gzFile, oam, 0x400);
  if(version < SAVE_GAME_VERSION_6)
    utilGzRead(gzFile, pix, 4*240*160);
  else
    utilGzRead(gzFile, pix, 4*241*162);
  utilGzRead(gzFile, ioMem, 0x400);

  eepromReadGame(gzFile, version);
  flashReadGame(gzFile, version);
  soundReadGame(gzFile, version);
  
  if(version > SAVE_GAME_VERSION_1) {
    cheatsReadGame(gzFile, version);
  }
  if(version > SAVE_GAME_VERSION_6) {
    rtcReadGame(gzFile);
  }

  if(version <= SAVE_GAME_VERSION_7) {
    u32 temp;
#define SWAP(a,b,c) \
    temp = (a);\
    (a) = (b)<<16|(c);\
    (b) = (temp) >> 16;\
    (c) = (temp) & 0xFFFF;
    
    SWAP(dma0Source, DM0SAD_H, DM0SAD_L);
    SWAP(dma0Dest,   DM0DAD_H, DM0DAD_L);
    SWAP(dma1Source, DM1SAD_H, DM1SAD_L);
    SWAP(dma1Dest,   DM1DAD_H, DM1DAD_L);
    SWAP(dma2Source, DM2SAD_H, DM2SAD_L);
    SWAP(dma2Dest,   DM2DAD_H, DM2DAD_L);
    SWAP(dma3Source, DM3SAD_H, DM3SAD_L);
    SWAP(dma3Dest,   DM3DAD_H, DM3DAD_L);
  }

  if(version <= SAVE_GAME_VERSION_8) {
    timer0ClockReload = TIMER_TICKS[TM0CNT & 3]; 
    timer1ClockReload = TIMER_TICKS[TM1CNT & 3];
    timer2ClockReload = TIMER_TICKS[TM2CNT & 3];
    timer3ClockReload = TIMER_TICKS[TM3CNT & 3];

    timer0Ticks = ((0x10000 - TM0D) << timer0ClockReload) - timer0Ticks;
    timer1Ticks = ((0x10000 - TM1D) << timer1ClockReload) - timer1Ticks;
    timer2Ticks = ((0x10000 - TM2D) << timer2ClockReload) - timer2Ticks;
    timer3Ticks = ((0x10000 - TM3D) << timer3ClockReload) - timer3Ticks;
  }

  // set pointers!
  layerEnable = layerSettings & DISPCNT;
  
  CPUUpdateRender();
  CPUUpdateRenderBuffers(true);
  CPUUpdateWindow0();
  CPUUpdateWindow1();
  gbaSaveType = 0;
  switch(saveType) {
  case 0:
    cpuSaveGameFunc = flashSaveDecide;
    break;
  case 1:
    cpuSaveGameFunc = sramWrite;
    gbaSaveType = 1;
    break;
  case 2:
    cpuSaveGameFunc = flashWrite;
    gbaSaveType = 2;
    break;
  case 3:
     break;
  case 5:
    gbaSaveType = 5;
    break;
  default:
    systemMessage(MSG_UNSUPPORTED_SAVE_TYPE,
                  N_("Unsupported save type %d"), saveType);
    break;
  }
  if(eepromInUse)
    gbaSaveType = 3;

  systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
  if(armState) {
    ARM_PREFETCH;
  } else {
    THUMB_PREFETCH;
  }

  CPUUpdateRegister(0x204, CPUReadHalfWordQuick(0x4000204));
  
  return true;  
}

bool CPUReadMemState(char *memory, int available)
{
  gzFile gzFile = utilMemGzOpen(memory, available, "r");

  bool res = CPUReadStateGz(gzFile);

  utilGzClose(gzFile);

  return res;
}

bool CPUReadState(const char * file)
{
  gzFile gzFile = utilGzOpen(file, "rb");

  if(gzFile == NULL)
    return false;
  
  bool res = CPUReadStateGz(gzFile);

  utilGzClose(gzFile);

  return res;
}

bool CPUExportEepromFile(const char *fileName)
{
  if(eepromInUse) {
    FILE *file = fopen(fileName, "wb");
    
    if(!file) {
      systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"),
                    fileName);
      return false;
    }

    for(int i = 0; i < eepromSize;) {
      for(int j = 0; j < 8; j++) {
        if(fwrite(&eepromData[i+7-j], 1, 1, file) != 1) {
          fclose(file);
          return false;
        }
      }
      i += 8;
    }
    fclose(file);
  }
  return true;
}

bool CPUWriteBatteryFile(const char *fileName)
{
  if(gbaSaveType == 0) {
    if(eepromInUse)
      gbaSaveType = 3;
    else switch(saveType) {
    case 1:
      gbaSaveType = 1;
      break;
    case 2:
      gbaSaveType = 2;
      break;
    }
  }
  
  if((gbaSaveType) && (gbaSaveType!=5)) {
    FILE *file = fopen(fileName, "wb");
    
    if(!file) {
      systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"),
                    fileName);
      return false;
    }
    
    // only save if Flash/Sram in use or EEprom in use
    if(gbaSaveType != 3) {
      if(gbaSaveType == 2) {
        if(fwrite(flashSaveMemory, 1, flashSize, file) != (size_t)flashSize) {
          fclose(file);
          return false;
        }
      } else {
        if(fwrite(flashSaveMemory, 1, 0x10000, file) != 0x10000) {
          fclose(file);
          return false;
        }
      }
    } else {
      if(fwrite(eepromData, 1, eepromSize, file) != (size_t)eepromSize) {
        fclose(file);
        return false;
      }
    }
    fclose(file);
  }
  return true;
}

bool CPUReadGSASnapshot(const char *fileName)
{
  int i;
  FILE *file = fopen(fileName, "rb");
    
  if(!file) {
    systemMessage(MSG_CANNOT_OPEN_FILE, N_("Cannot open file %s"), fileName);
    return false;
  }
  
  // check file size to know what we should read
  fseek(file, 0, SEEK_END);

  // long size = ftell(file);
  fseek(file, 0x0, SEEK_SET);
  fread(&i, 1, 4, file);
  fseek(file, i, SEEK_CUR); // Skip SharkPortSave
  fseek(file, 4, SEEK_CUR); // skip some sort of flag
  fread(&i, 1, 4, file); // name length
  fseek(file, i, SEEK_CUR); // skip name
  fread(&i, 1, 4, file); // desc length
  fseek(file, i, SEEK_CUR); // skip desc
  fread(&i, 1, 4, file); // notes length
  fseek(file, i, SEEK_CUR); // skip notes
  int saveSize;
  fread(&saveSize, 1, 4, file); // read length
  saveSize -= 0x1c; // remove header size
  char buffer[17];
  char buffer2[17];
  fread(buffer, 1, 16, file);
  buffer[16] = 0;
  for(i = 0; i < 16; i++)
    if(buffer[i] < 32)
      buffer[i] = 32;
  memcpy(buffer2, &rom[0xa0], 16);
  buffer2[16] = 0;
  for(i = 0; i < 16; i++)
    if(buffer2[i] < 32)
      buffer2[i] = 32;  
  if(memcmp(buffer, buffer2, 16)) {
    systemMessage(MSG_CANNOT_IMPORT_SNAPSHOT_FOR,
                  N_("Cannot import snapshot for %s. Current game is %s"),
                  buffer,
                  buffer2);
    fclose(file);
    return false;
  }
  fseek(file, 12, SEEK_CUR); // skip some flags
  if(saveSize >= 65536) {
    if(fread(flashSaveMemory, 1, saveSize, file) != (size_t)saveSize) {
      fclose(file);
      return false;
    }
  } else {
    systemMessage(MSG_UNSUPPORTED_SNAPSHOT_FILE,
                  N_("Unsupported snapshot file %s"),
                  fileName);
    fclose(file);
    return false;
  }
  fclose(file);
  CPUReset();
  return true;
}

bool CPUWriteGSASnapshot(const char *fileName, 
                         const char *title, 
                         const char *desc, 
                         const char *notes)
{
  FILE *file = fopen(fileName, "wb");
    
  if(!file) {
    systemMessage(MSG_CANNOT_OPEN_FILE, N_("Cannot open file %s"), fileName);
    return false;
  }

  u8 buffer[17];

  utilPutDword(buffer, 0x0d); // SharkPortSave length
  fwrite(buffer, 1, 4, file);
  fwrite("SharkPortSave", 1, 0x0d, file);
  utilPutDword(buffer, 0x000f0000);
  fwrite(buffer, 1, 4, file); // save type 0x000f0000 = GBA save
  utilPutDword(buffer, (u32)strlen(title));
  fwrite(buffer, 1, 4, file); // title length
  fwrite(title, 1, strlen(title), file);
  utilPutDword(buffer, (u32)strlen(desc));
  fwrite(buffer, 1, 4, file); // desc length
  fwrite(desc, 1, strlen(desc), file);
  utilPutDword(buffer, (u32)strlen(notes));
  fwrite(buffer, 1, 4, file); // notes length
  fwrite(notes, 1, strlen(notes), file);
  int saveSize = 0x10000;
  if(gbaSaveType == 2)
    saveSize = flashSize;
  int totalSize = saveSize + 0x1c;

  utilPutDword(buffer, totalSize); // length of remainder of save - CRC
  fwrite(buffer, 1, 4, file);

  char temp[0x2001c];
  memset(temp, 0, 28);
  memcpy(temp, &rom[0xa0], 16); // copy internal name
  temp[0x10] = rom[0xbe]; // reserved area (old checksum)
  temp[0x11] = rom[0xbf]; // reserved area (old checksum)
  temp[0x12] = rom[0xbd]; // complement check
  temp[0x13] = rom[0xb0]; // maker
  temp[0x14] = 1; // 1 save ?
  memcpy(&temp[0x1c], flashSaveMemory, saveSize); // copy save
  fwrite(temp, 1, totalSize, file); // write save + header
  u32 crc = 0;
  
  for(int i = 0; i < totalSize; i++) {
    crc += ((u32)temp[i] << (crc % 0x18));
  }
  
  utilPutDword(buffer, crc);
  fwrite(buffer, 1, 4, file); // CRC?
  
  fclose(file);
  return true;
}

bool CPUImportEepromFile(const char *fileName)
{
  FILE *file = fopen(fileName, "rb");
    
  if(!file)
    return false;
  
  // check file size to know what we should read
  fseek(file, 0, SEEK_END);

  long size = ftell(file);
  fseek(file, 0, SEEK_SET);
  if(size == 512 || size == 0x2000) {
    if(fread(eepromData, 1, size, file) != (size_t)size) {
      fclose(file);
      return false;
    }
    for(int i = 0; i < size;) {
      for(int j = 0; j < 4; ++j) {
        u8 tmp = eepromData[i];
        eepromData[i] = eepromData[7-i];
        eepromData[7-i] = tmp;
        i++;
      }
      i += 4;
    }
  } else
    return false;
  fclose(file);
  return true;
}

bool CPUReadBatteryFile(const char *fileName)
{
  FILE *file = fopen(fileName, "rb");
    
  if(!file)
    return false;
  
  // check file size to know what we should read
  fseek(file, 0, SEEK_END);

  long size = ftell(file);
  fseek(file, 0, SEEK_SET);
  systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

  if(size == 512 || size == 0x2000) {
    if(fread(eepromData, 1, size, file) != (size_t)size) {
      fclose(file);
      return false;
    }
  } else {
    if(size == 0x20000) {
      if(fread(flashSaveMemory, 1, 0x20000, file) != 0x20000) {
        fclose(file);
        return false;
      }
      flashSetSize(0x20000);
    } else {
      if(fread(flashSaveMemory, 1, 0x10000, file) != 0x10000) {
        fclose(file);
        return false;
      }
      flashSetSize(0x10000);
    }
  }
  fclose(file);
  return true;
}

bool CPUWritePNGFile(const char *fileName)
{
  return utilWritePNGFile(fileName, 240, 160, pix);
}

bool CPUWriteBMPFile(const char *fileName)
{
  return utilWriteBMPFile(fileName, 240, 160, pix);
}

static bool CPUIsGBABios(const char * file)
{
  return utilFileMatchFilter(file, "\004.gba\0.agb\0.bin\0.bios");
}

static bool CPUIsELF(const char *file)
{
  return utilFileMatchFilter(file, "\001.elf");
}

#define FREEANDNULL(X) do { if(X) free(X); X = 0; } while(0)

void CPUCleanUp()
{
#ifdef PROFILING
  if(profilingTicksReload) {
    profCleanup();
  }
#endif
  FREEANDNULL(rom);
  FREEANDNULL(vram);
  FREEANDNULL(paletteRAM);
  FREEANDNULL(internalRAM);
  FREEANDNULL(workRAM);
  FREEANDNULL(bios);
  FREEANDNULL(pix);
  FREEANDNULL(oam);
  FREEANDNULL(ioMem);

  elfCleanUp();
  systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
  emulating = 0;
}

static int try_alloc(u8 **dest, size_t m, size_t n, const char* id)
{
  if(!(*dest = calloc(m, n))) {
    systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"), id);
    return 0;
  }
  return 1;
}

int CPULoadRom(const char *szFile)
{
  init_cleared_line();

  romSize = 0x2000000;
  if(rom != NULL) {
    CPUCleanUp();
  }

  systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
  if(!try_alloc(&rom, 1, 0x2000000, "ROM")) return 0;
  if(!try_alloc(&workRAM, 1, 0x40000, "WRAM")) return 0;

  u8 *whereToLoad = rom;
  if(cpuIsMultiBoot)
    whereToLoad = workRAM;

  if(CPUIsELF(szFile)) {
    FILE *f = fopen(szFile, "rb");
    if(!f) {
      systemMessage(MSG_ERROR_OPENING_IMAGE, N_("Error opening image %s"),
                    szFile);
      FREEANDNULL(rom);
      FREEANDNULL(workRAM);
      return 0;
    }
    bool res = elfRead(szFile, &romSize, f);
    if(!res || romSize == 0) {
      FREEANDNULL(rom);
      FREEANDNULL(workRAM);
      elfCleanUp();
      return 0;
    }
  } else if(!utilLoad(szFile,
                      utilIsGBAImage,
                      whereToLoad,
                      &romSize)) {
    FREEANDNULL(rom);
    FREEANDNULL(workRAM);
    return 0;
  }

  u16 *temp = (u16 *)(rom+((romSize+1)&~1));
  int i;
  for(i = (romSize+1)&~1; i < 0x2000000; i+=2) {
    WRITE16LE(temp, (i >> 1) & 0xFFFF);
    temp++;
  }

  if(!try_alloc(&bios, 1, 0x4000, "BIOS")) goto oom;
  if(!try_alloc(&internalRAM, 1, 0x8000, "IRAM")) goto oom;
  if(!try_alloc(&paletteRAM, 1, 0x400, "PRAM")) goto oom;
  if(!try_alloc(&vram, 1, 0x20000, "VRAM")) goto oom;
  if(!try_alloc(&oam, 1, 0x400, "OAM")) goto oom;
  if(!try_alloc(&pix, 4, 241 * 162, "PIX")) goto oom;
  if(!try_alloc(&ioMem, 1, 0x400, "IO")) goto oom;

  flashInit();
  eepromInit();
  if(flashSize != 0x20000 && is_mbit_rom(rom+0xa0+12))
    flashSetSize(0x20000);

  CPUUpdateRenderBuffers(true);

  return romSize;

oom:;
  CPUCleanUp();
  return 0;
}

void doMirroring (bool b)
{
  u32 mirroredRomSize = (((romSize)>>20) & 0x3F)<<20;
  u32 mirroredRomAddress = romSize;
  if ((mirroredRomSize <=0x800000) && (b))
  {
    mirroredRomAddress = mirroredRomSize;
    if (mirroredRomSize==0)
        mirroredRomSize=0x100000;
    while (mirroredRomAddress<0x01000000)
    {
      memcpy ((u16 *)(rom+mirroredRomAddress), (u16 *)(rom), mirroredRomSize);
      mirroredRomAddress+=mirroredRomSize;
    }
  }
}

void CPUUpdateRender()
{
  switch(DISPCNT & 7) {
  case 0:
    if((!fxOn && !windowOn && !(layerEnable & 0x8000)) ||
       cpuDisableSfx)
      renderLine = mode0RenderLine;
    else if(fxOn && !windowOn && !(layerEnable & 0x8000))
      renderLine = mode0RenderLineNoWindow;
    else 
      renderLine = mode0RenderLineAll;
    break;
  case 1:
    if((!fxOn && !windowOn && !(layerEnable & 0x8000)) ||
       cpuDisableSfx)
      renderLine = mode1RenderLine;
    else if(fxOn && !windowOn && !(layerEnable & 0x8000))
      renderLine = mode1RenderLineNoWindow;
    else
      renderLine = mode1RenderLineAll;
    break;
  case 2:
    if((!fxOn && !windowOn && !(layerEnable & 0x8000)) ||
       cpuDisableSfx)
      renderLine = mode2RenderLine;
    else if(fxOn && !windowOn && !(layerEnable & 0x8000))
      renderLine = mode2RenderLineNoWindow;
    else
      renderLine = mode2RenderLineAll;
    break;
  case 3:
    if((!fxOn && !windowOn && !(layerEnable & 0x8000)) ||
       cpuDisableSfx)
      renderLine = mode3RenderLine;
    else if(fxOn && !windowOn && !(layerEnable & 0x8000))
      renderLine = mode3RenderLineNoWindow;
    else
      renderLine = mode3RenderLineAll;
    break;
  case 4:
    if((!fxOn && !windowOn && !(layerEnable & 0x8000)) ||
       cpuDisableSfx)
      renderLine = mode4RenderLine;
    else if(fxOn && !windowOn && !(layerEnable & 0x8000))
      renderLine = mode4RenderLineNoWindow;
    else
      renderLine = mode4RenderLineAll;
    break;
  case 5:
    if((!fxOn && !windowOn && !(layerEnable & 0x8000)) ||
       cpuDisableSfx)
      renderLine = mode5RenderLine;
    else if(fxOn && !windowOn && !(layerEnable & 0x8000))
      renderLine = mode5RenderLineNoWindow;
    else
      renderLine = mode5RenderLineAll;
  default:
    break;
  }
}

void CPUUpdateCPSR()
{
  u32 CPSR = reg[16].I & 0x40;
  if(N_FLAG)
    CPSR |= 0x80000000;
  if(Z_FLAG)
    CPSR |= 0x40000000;
  if(C_FLAG)
    CPSR |= 0x20000000;
  if(V_FLAG)
    CPSR |= 0x10000000;
  if(!armState)
    CPSR |= 0x00000020;
  if(!armIrqEnable)
    CPSR |= 0x80;
  CPSR |= (armMode & 0x1F);
  reg[16].I = CPSR;
}

void CPUUpdateFlagsB(bool breakLoop)
{
  u32 CPSR = reg[16].I;
  
  N_FLAG = (CPSR & 0x80000000) ? true: false;
  Z_FLAG = (CPSR & 0x40000000) ? true: false;
  C_FLAG = (CPSR & 0x20000000) ? true: false;
  V_FLAG = (CPSR & 0x10000000) ? true: false;
  armState = (CPSR & 0x20) ? false : true;
  armIrqEnable = (CPSR & 0x80) ? false : true;
  if(breakLoop) {
      if (armIrqEnable && (IF & IE) && (IME & 1))
        cpuNextEvent = cpuTotalTicks;
  }
}

void CPUUpdateFlags(void)
{
  CPUUpdateFlagsB(true);
}

#ifdef WORDS_BIGENDIAN
static void CPUSwap(volatile u32 *a, volatile u32 *b)
{
  volatile u32 c = *b;
  *b = *a;
  *a = c;
}
#else
static void CPUSwap(u32 *a, u32 *b)
{
  u32 c = *b;
  *b = *a;
  *a = c;
}
#endif

void CPUSwitchMode3(int mode, bool saveState, bool breakLoop)
{
  //  if(armMode == mode)
  //    return;
  
  CPUUpdateCPSR();

  switch(armMode) {
  case 0x10:
  case 0x1F:
    reg[R13_USR].I = reg[13].I;
    reg[R14_USR].I = reg[14].I;
    reg[17].I = reg[16].I;
    break;
  case 0x11:
    CPUSwap(&reg[R8_FIQ].I, &reg[8].I);
    CPUSwap(&reg[R9_FIQ].I, &reg[9].I);
    CPUSwap(&reg[R10_FIQ].I, &reg[10].I);
    CPUSwap(&reg[R11_FIQ].I, &reg[11].I);
    CPUSwap(&reg[R12_FIQ].I, &reg[12].I);
    reg[R13_FIQ].I = reg[13].I;
    reg[R14_FIQ].I = reg[14].I;
    reg[SPSR_FIQ].I = reg[17].I;
    break;
  case 0x12:
    reg[R13_IRQ].I  = reg[13].I;
    reg[R14_IRQ].I  = reg[14].I;
    reg[SPSR_IRQ].I =  reg[17].I;
    break;
  case 0x13:
    reg[R13_SVC].I  = reg[13].I;
    reg[R14_SVC].I  = reg[14].I;
    reg[SPSR_SVC].I =  reg[17].I;
    break;
  case 0x17:
    reg[R13_ABT].I  = reg[13].I;
    reg[R14_ABT].I  = reg[14].I;
    reg[SPSR_ABT].I =  reg[17].I;
    break;
  case 0x1b:
    reg[R13_UND].I  = reg[13].I;
    reg[R14_UND].I  = reg[14].I;
    reg[SPSR_UND].I =  reg[17].I;
    break;
  }

  u32 CPSR = reg[16].I;
  u32 SPSR = reg[17].I;
  
  switch(mode) {
  case 0x10:
  case 0x1F:
    reg[13].I = reg[R13_USR].I;
    reg[14].I = reg[R14_USR].I;
    reg[16].I = SPSR;
    break;
  case 0x11:
    CPUSwap(&reg[8].I, &reg[R8_FIQ].I);
    CPUSwap(&reg[9].I, &reg[R9_FIQ].I);
    CPUSwap(&reg[10].I, &reg[R10_FIQ].I);
    CPUSwap(&reg[11].I, &reg[R11_FIQ].I);
    CPUSwap(&reg[12].I, &reg[R12_FIQ].I);
    reg[13].I = reg[R13_FIQ].I;
    reg[14].I = reg[R14_FIQ].I;
    if(saveState)
      reg[17].I = CPSR;
    else
      reg[17].I = reg[SPSR_FIQ].I;
    break;
  case 0x12:
    reg[13].I = reg[R13_IRQ].I;
    reg[14].I = reg[R14_IRQ].I;
    reg[16].I = SPSR;
    if(saveState)
      reg[17].I = CPSR;
    else
      reg[17].I = reg[SPSR_IRQ].I;
    break;
  case 0x13:
    reg[13].I = reg[R13_SVC].I;
    reg[14].I = reg[R14_SVC].I;
    reg[16].I = SPSR;
    if(saveState)
      reg[17].I = CPSR;
    else
      reg[17].I = reg[SPSR_SVC].I;
    break;
  case 0x17:
    reg[13].I = reg[R13_ABT].I;
    reg[14].I = reg[R14_ABT].I;
    reg[16].I = SPSR;
    if(saveState)
      reg[17].I = CPSR;
    else
      reg[17].I = reg[SPSR_ABT].I;
    break;    
  case 0x1b:
    reg[13].I = reg[R13_UND].I;
    reg[14].I = reg[R14_UND].I;
    reg[16].I = SPSR;
    if(saveState)
      reg[17].I = CPSR;
    else
      reg[17].I = reg[SPSR_UND].I;
    break;    
  default:
    systemMessage(MSG_UNSUPPORTED_ARM_MODE, N_("Unsupported ARM mode %02x"), mode);
    break;
  }
  armMode = mode;
  CPUUpdateFlagsB(breakLoop);
  CPUUpdateCPSR();
}

void CPUSwitchMode(int mode, bool saveState)
{
  CPUSwitchMode3(mode, saveState, true);
}

void CPUUndefinedException()
{
  u32 PC = reg[15].I;
  bool savedArmState = armState;
  CPUSwitchMode3(0x1b, true, false);
  reg[14].I = PC - (savedArmState ? 4 : 2);
  reg[15].I = 0x04;
  armState = true;
  armIrqEnable = false;
  armNextPC = 0x04;
  ARM_PREFETCH;
  reg[15].I += 4;  
}

void CPUSoftwareInterrupt(void)
{
  u32 PC = reg[15].I;
  bool savedArmState = armState;
  CPUSwitchMode3(0x13, true, false);
  reg[14].I = PC - (savedArmState ? 4 : 2);
  reg[15].I = 0x08;
  armState = true;
  armIrqEnable = false;
  armNextPC = 0x08;
  ARM_PREFETCH;
  reg[15].I += 4;
}

void CPUSoftwareInterrupt1(int comment)
{
  static bool disableMessage = false;
  if(armState) comment >>= 16;
#ifdef BKPT_SUPPORT
  if(comment == 0xff) {
    extern void (*dbgOutput)(char *, u32);
    dbgOutput(NULL, reg[0].I);
    return;
  }
#endif
#ifdef PROFILING
  if(comment == 0xfe) {
    profStartup(reg[0].I, reg[1].I);
    return;
  }
  if(comment == 0xfd) {
    profControl(reg[0].I);
    return;
  }
  if(comment == 0xfc) {
    profCleanup();
    return;
  }
  if(comment == 0xfb) {
    profCount();
    return;
  }
#endif
  if(comment == 0xfa) {
    agbPrintFlush();
    return;
  }
#ifdef SDL
  if(comment == 0xf9) {
    emulating = 0;
    cpuNextEvent = cpuTotalTicks;
    cpuBreakLoop = true;
    return;
  }
#endif
  if(useBios) {
#ifdef DEV_VERSION
    if(systemVerbose & VERBOSE_SWI) {
      log("SWI: %08x at %08x (0x%08x,0x%08x,0x%08x,VCOUNT = %2d)\n", comment,
          armState ? armNextPC - 4: armNextPC -2,
          reg[0].I,
          reg[1].I,
          reg[2].I,
          VCOUNT);
    }
#endif
    CPUSoftwareInterrupt();
    return;
  }
  // This would be correct, but it causes problems if uncommented
  //  else {
  //    biosProtected = 0xe3a02004;
  //  }
     
  switch(comment) {
  case 0x00:
    BIOS_SoftReset();
    ARM_PREFETCH;
    break;
  case 0x01:
    BIOS_RegisterRamReset();
    break;
  case 0x02:
#ifdef DEV_VERSION
    if(systemVerbose & VERBOSE_SWI) {
      log("Halt: (VCOUNT = %2d)\n",
          VCOUNT);      
    }
#endif    
    holdState = true;
    holdType = -1;
    cpuNextEvent = cpuTotalTicks;
    break;
  case 0x03:
#ifdef DEV_VERSION
    if(systemVerbose & VERBOSE_SWI) {
      log("Stop: (VCOUNT = %2d)\n",
          VCOUNT);      
    }
#endif    
    holdState = true;
    holdType = -1;
    stopState = true;
    cpuNextEvent = cpuTotalTicks;
    break;
  case 0x04:
#ifdef DEV_VERSION
    if(systemVerbose & VERBOSE_SWI) {
      log("IntrWait: 0x%08x,0x%08x (VCOUNT = %2d)\n",
          reg[0].I,
          reg[1].I,
          VCOUNT);      
    }
#endif
    CPUSoftwareInterrupt();
    break;    
  case 0x05:
#ifdef DEV_VERSION
    if(systemVerbose & VERBOSE_SWI) {
      log("VBlankIntrWait: (VCOUNT = %2d)\n", 
          VCOUNT);      
    }
#endif
    CPUSoftwareInterrupt();
    break;
  case 0x06:
    CPUSoftwareInterrupt();
    break;
  case 0x07:
    CPUSoftwareInterrupt();
    break;
  case 0x08:
    BIOS_Sqrt();
    break;
  case 0x09:
    BIOS_ArcTan();
    break;
  case 0x0A:
    BIOS_ArcTan2();
    break;
  case 0x0B:
    {
      int len = (reg[2].I & 0x1FFFFF) >>1;
      if (!(((reg[0].I & 0xe000000) == 0) ||
         ((reg[0].I + len) & 0xe000000) == 0))
      {
        if ((reg[2].I >> 24) & 1)
        {
          if ((reg[2].I >> 26) & 1) 
          SWITicks = (7 + memoryWait32[(reg[1].I>>24) & 0xF]) * (len>>1);
          else
          SWITicks = (8 + memoryWait[(reg[1].I>>24) & 0xF]) * (len);
        }
        else
        {
          if ((reg[2].I >> 26) & 1) 
          SWITicks = (10 + memoryWait32[(reg[0].I>>24) & 0xF] +
              memoryWait32[(reg[1].I>>24) & 0xF]) * (len>>1);
          else
          SWITicks = (11 + memoryWait[(reg[0].I>>24) & 0xF] +
              memoryWait[(reg[1].I>>24) & 0xF]) * len;
        }
      }
    }
    BIOS_CpuSet();
    break;
  case 0x0C:
    {
      int len = (reg[2].I & 0x1FFFFF) >>5;
      if (!(((reg[0].I & 0xe000000) == 0) ||
         ((reg[0].I + len) & 0xe000000) == 0))
      {
        if ((reg[2].I >> 24) & 1)
          SWITicks = (6 + memoryWait32[(reg[1].I>>24) & 0xF] +
              7 * (memoryWaitSeq32[(reg[1].I>>24) & 0xF] + 1)) * len;
        else
          SWITicks = (9 + memoryWait32[(reg[0].I>>24) & 0xF] +
              memoryWait32[(reg[1].I>>24) & 0xF] + 
              7 * (memoryWaitSeq32[(reg[0].I>>24) & 0xF] +
              memoryWaitSeq32[(reg[1].I>>24) & 0xF] + 2)) * len;
      }
    }
    BIOS_CpuFastSet();
    break;
  case 0x0D:
    BIOS_GetBiosChecksum();
    break;
  case 0x0E:
    BIOS_BgAffineSet();
    break;
  case 0x0F:
    BIOS_ObjAffineSet();
    break;
  case 0x10:
    {
      int len = CPUReadHalfWord(reg[2].I);
      if (!(((reg[0].I & 0xe000000) == 0) ||
         ((reg[0].I + len) & 0xe000000) == 0))
        SWITicks = (32 + memoryWait[(reg[0].I>>24) & 0xF]) * len;
    }
    BIOS_BitUnPack();
    break;
  case 0x11:
    {
      u32 len = CPUReadMemory(reg[0].I) >> 8;
      if(!(((reg[0].I & 0xe000000) == 0) ||
          ((reg[0].I + (len & 0x1fffff)) & 0xe000000) == 0))
        SWITicks = (9 + memoryWait[(reg[1].I>>24) & 0xF]) * len;
    }
    BIOS_LZ77UnCompWram();
    break;
  case 0x12:
    {
      u32 len = CPUReadMemory(reg[0].I) >> 8;
      if(!(((reg[0].I & 0xe000000) == 0) ||
          ((reg[0].I + (len & 0x1fffff)) & 0xe000000) == 0))
        SWITicks = (19 + memoryWait[(reg[1].I>>24) & 0xF]) * len;
    }
    BIOS_LZ77UnCompVram();
    break;
  case 0x13:
    {
      u32 len = CPUReadMemory(reg[0].I) >> 8;
      if(!(((reg[0].I & 0xe000000) == 0) ||
          ((reg[0].I + (len & 0x1fffff)) & 0xe000000) == 0))
        SWITicks = (29 + (memoryWait[(reg[0].I>>24) & 0xF]<<1)) * len;
    }
    BIOS_HuffUnComp();
    break;
  case 0x14:
    {
      u32 len = CPUReadMemory(reg[0].I) >> 8;
      if(!(((reg[0].I & 0xe000000) == 0) ||
          ((reg[0].I + (len & 0x1fffff)) & 0xe000000) == 0))
        SWITicks = (11 + memoryWait[(reg[0].I>>24) & 0xF] +
          memoryWait[(reg[1].I>>24) & 0xF]) * len;
    }
    BIOS_RLUnCompWram();
    break;
  case 0x15:
    {
      u32 len = CPUReadMemory(reg[0].I) >> 9;
      if(!(((reg[0].I & 0xe000000) == 0) ||
          ((reg[0].I + (len & 0x1fffff)) & 0xe000000) == 0))
        SWITicks = (34 + (memoryWait[(reg[0].I>>24) & 0xF] << 1) +
          memoryWait[(reg[1].I>>24) & 0xF]) * len;
    }
    BIOS_RLUnCompVram();
    break;
  case 0x16:
    {
      u32 len = CPUReadMemory(reg[0].I) >> 8;
      if(!(((reg[0].I & 0xe000000) == 0) ||
          ((reg[0].I + (len & 0x1fffff)) & 0xe000000) == 0))
        SWITicks = (13 + memoryWait[(reg[0].I>>24) & 0xF] +
          memoryWait[(reg[1].I>>24) & 0xF]) * len;
    }
    BIOS_Diff8bitUnFilterWram();
    break;
  case 0x17:
    {
      u32 len = CPUReadMemory(reg[0].I) >> 9;
      if(!(((reg[0].I & 0xe000000) == 0) ||
          ((reg[0].I + (len & 0x1fffff)) & 0xe000000) == 0))
        SWITicks = (39 + (memoryWait[(reg[0].I>>24) & 0xF]<<1) +
          memoryWait[(reg[1].I>>24) & 0xF]) * len;
    }
    BIOS_Diff8bitUnFilterVram();
    break;
  case 0x18:
    {
      u32 len = CPUReadMemory(reg[0].I) >> 9;
      if(!(((reg[0].I & 0xe000000) == 0) ||
          ((reg[0].I + (len & 0x1fffff)) & 0xe000000) == 0))
        SWITicks = (13 + memoryWait[(reg[0].I>>24) & 0xF] +
          memoryWait[(reg[1].I>>24) & 0xF]) * len;
    }
    BIOS_Diff16bitUnFilter();
    break;
  case 0x19:
#ifdef DEV_VERSION
    if(systemVerbose & VERBOSE_SWI) {
      log("SoundBiasSet: 0x%08x (VCOUNT = %2d)\n",
          reg[0].I,
          VCOUNT);      
    }
#endif    
    if(reg[0].I)
      systemSoundPause();
    else
      systemSoundResume();
    break;
  case 0x1F:
    BIOS_MidiKey2Freq();
    break;
  case 0x2A:
    BIOS_SndDriverJmpTableCopy();
    // let it go, because we don't really emulate this function
  default:
#ifdef DEV_VERSION
    if(systemVerbose & VERBOSE_SWI) {
      log("SWI: %08x at %08x (0x%08x,0x%08x,0x%08x,VCOUNT = %2d)\n", comment,
          armState ? armNextPC - 4: armNextPC -2,
          reg[0].I,
          reg[1].I,
          reg[2].I,
          VCOUNT);
    }
#endif
    
    if(!disableMessage) {
      systemMessage(MSG_UNSUPPORTED_BIOS_FUNCTION,
                    N_("Unsupported BIOS function %02x called from %08x. A BIOS file is needed in order to get correct behaviour."),
                    comment,
                    armMode ? armNextPC - 4: armNextPC - 2);
      disableMessage = true;
    }
    break;
  }
}

void CPUCompareVCOUNT()
{
  if(VCOUNT == (DISPSTAT >> 8)) {
    DISPSTAT |= 4;
    UPDATE_REG(0x04, DISPSTAT);

    if(DISPSTAT & 0x20) {
      IF |= 4;
      UPDATE_REG(0x202, IF);
    }
  } else {
    DISPSTAT &= 0xFFFB;
    UPDATE_REG(0x4, DISPSTAT);
  }
  if (layerEnableDelay>0)
  {
      layerEnableDelay--;
      if (layerEnableDelay==1)
          layerEnable = layerSettings & DISPCNT;
  }

}

void doDMA(u32 *s, u32 *d, u32 si, u32 di, u32 c, int transfer32)
{
  int sm = *s >> 24;
  int dm = *d >> 24;
  int sw = 0;
  int dw = 0;
  int sc = c;

  cpuDmaCount = c;
  // This is done to get the correct waitstates.
  if (sm>15)
      sm=15;
  if (dm>15)
      dm=15;
  
  //if ((sm>=0x05) && (sm<=0x07) || (dm>=0x05) && (dm <=0x07))
  //    blank = (((DISPSTAT | ((DISPSTAT>>1)&1))==1) ?  true : false);

  if(transfer32) {
    *s &= 0xFFFFFFFC;
    if(*s < 0x02000000 && (reg[15].I >> 24)) {
      while(c != 0) {
        CPUWriteMemory(*d, 0);
        *d += di;
        c--;
      }
    } else {
      while(c != 0) {
        cpuDmaLast = CPUReadMemory(*s);
        CPUWriteMemory(*d, cpuDmaLast);
        *d += di;
        *s += si;
        c--;
      }
    }
  } else {
    *s &= 0xFFFFFFFE;
    si = (int)si >> 1;
    di = (int)di >> 1;
    if(*s < 0x02000000 && (reg[15].I >> 24)) {
      while(c != 0) {
        CPUWriteHalfWord(*d, 0);
        *d += di;
        c--;
      }
    } else {
      while(c != 0) {
        cpuDmaLast = CPUReadHalfWord(*s);
        CPUWriteHalfWord(*d, cpuDmaLast);
        cpuDmaLast |= (cpuDmaLast<<16);
        *d += di;
        *s += si;
        c--;
      }
    }
  }

  cpuDmaCount = 0;
  
  int totalTicks = 0;

  if(transfer32) {
      sw =1+memoryWaitSeq32[sm & 15];
      dw =1+memoryWaitSeq32[dm & 15];
      totalTicks = (sw+dw)*(sc-1) + 6 + memoryWait32[sm & 15] +
          memoryWaitSeq32[dm & 15];
  }
  else
  {
     sw = 1+memoryWaitSeq[sm & 15];
     dw = 1+memoryWaitSeq[dm & 15];
      totalTicks = (sw+dw)*(sc-1) + 6 + memoryWait[sm & 15] +
          memoryWaitSeq[dm & 15];
  }

  cpuDmaTicksToUpdate += totalTicks;

}

void CPUCheckDMA(int reason, int dmamask)
{
  // DMA 0
  if((DM0CNT_H & 0x8000) && (dmamask & 1)) {
    if(((DM0CNT_H >> 12) & 3) == reason) {
      u32 sourceIncrement = 4;
      u32 destIncrement = 4;
      switch((DM0CNT_H >> 7) & 3) {
      case 0:
        break;
      case 1:
        sourceIncrement = (u32)-4;
        break;
      case 2:
        sourceIncrement = 0;
        break;
      }
      switch((DM0CNT_H >> 5) & 3) {
      case 0:
        break;
      case 1:
        destIncrement = (u32)-4;
        break;
      case 2:
        destIncrement = 0;
        break;
      }      
#ifdef DEV_VERSION
      if(systemVerbose & VERBOSE_DMA0) {
        int count = (DM0CNT_L ? DM0CNT_L : 0x4000) << 1;
        if(DM0CNT_H & 0x0400)
          count <<= 1;
        log("DMA0: s=%08x d=%08x c=%04x count=%08x\n", dma0Source, dma0Dest, 
            DM0CNT_H,
            count);
      }
#endif
      doDMA(&dma0Source, &dma0Dest, sourceIncrement, destIncrement,
            DM0CNT_L ? DM0CNT_L : 0x4000,
            DM0CNT_H & 0x0400);
      cpuDmaHack = true;

      if(DM0CNT_H & 0x4000) {
        IF |= 0x0100;
        UPDATE_REG(0x202, IF);
        cpuNextEvent = cpuTotalTicks;
      }
      
      if(((DM0CNT_H >> 5) & 3) == 3) {
        dma0Dest = DM0DAD_L | (DM0DAD_H << 16);
      }
      
      if(!(DM0CNT_H & 0x0200) || (reason == 0)) {
        DM0CNT_H &= 0x7FFF;
        UPDATE_REG(0xBA, DM0CNT_H);
      }
    }
  }
  
  // DMA 1
  if((DM1CNT_H & 0x8000) && (dmamask & 2)) {
    if(((DM1CNT_H >> 12) & 3) == reason) {
      u32 sourceIncrement = 4;
      u32 destIncrement = 4;
      switch((DM1CNT_H >> 7) & 3) {
      case 0:
        break;
      case 1:
        sourceIncrement = (u32)-4;
        break;
      case 2:
        sourceIncrement = 0;
        break;
      }
      switch((DM1CNT_H >> 5) & 3) {
      case 0:
        break;
      case 1:
        destIncrement = (u32)-4;
        break;
      case 2:
        destIncrement = 0;
        break;
      }      
      if(reason == 3) {
#ifdef DEV_VERSION
        if(systemVerbose & VERBOSE_DMA1) {
          log("DMA1: s=%08x d=%08x c=%04x count=%08x\n", dma1Source, dma1Dest,
              DM1CNT_H,
              16);
        }
#endif  
        doDMA(&dma1Source, &dma1Dest, sourceIncrement, 0, 4,
              0x0400);
      } else {
#ifdef DEV_VERSION
        if(systemVerbose & VERBOSE_DMA1) {
          int count = (DM1CNT_L ? DM1CNT_L : 0x4000) << 1;
          if(DM1CNT_H & 0x0400)
            count <<= 1;
          log("DMA1: s=%08x d=%08x c=%04x count=%08x\n", dma1Source, dma1Dest,
              DM1CNT_H,
              count);
        }
#endif          
        doDMA(&dma1Source, &dma1Dest, sourceIncrement, destIncrement,
              DM1CNT_L ? DM1CNT_L : 0x4000,
              DM1CNT_H & 0x0400);
      }
      cpuDmaHack = true;

      if(DM1CNT_H & 0x4000) {
        IF |= 0x0200;
        UPDATE_REG(0x202, IF);
        cpuNextEvent = cpuTotalTicks;
      }
      
      if(((DM1CNT_H >> 5) & 3) == 3) {
        dma1Dest = DM1DAD_L | (DM1DAD_H << 16);
      }
      
      if(!(DM1CNT_H & 0x0200) || (reason == 0)) {
        DM1CNT_H &= 0x7FFF;
        UPDATE_REG(0xC6, DM1CNT_H);
      }
    }
  }
  
  // DMA 2
  if((DM2CNT_H & 0x8000) && (dmamask & 4)) {
    if(((DM2CNT_H >> 12) & 3) == reason) {
      u32 sourceIncrement = 4;
      u32 destIncrement = 4;
      switch((DM2CNT_H >> 7) & 3) {
      case 0:
        break;
      case 1:
        sourceIncrement = (u32)-4;
        break;
      case 2:
        sourceIncrement = 0;
        break;
      }
      switch((DM2CNT_H >> 5) & 3) {
      case 0:
        break;
      case 1:
        destIncrement = (u32)-4;
        break;
      case 2:
        destIncrement = 0;
        break;
      }      
      if(reason == 3) {
#ifdef DEV_VERSION
        if(systemVerbose & VERBOSE_DMA2) {
          int count = (4) << 2;
          log("DMA2: s=%08x d=%08x c=%04x count=%08x\n", dma2Source, dma2Dest,
              DM2CNT_H,
              count);
        }
#endif                  
        doDMA(&dma2Source, &dma2Dest, sourceIncrement, 0, 4,
              0x0400);
      } else {
#ifdef DEV_VERSION
        if(systemVerbose & VERBOSE_DMA2) {
          int count = (DM2CNT_L ? DM2CNT_L : 0x4000) << 1;
          if(DM2CNT_H & 0x0400)
            count <<= 1;
          log("DMA2: s=%08x d=%08x c=%04x count=%08x\n", dma2Source, dma2Dest,
              DM2CNT_H,
              count);
        }
#endif                  
        doDMA(&dma2Source, &dma2Dest, sourceIncrement, destIncrement,
              DM2CNT_L ? DM2CNT_L : 0x4000,
              DM2CNT_H & 0x0400);
      }
      cpuDmaHack = true;

      if(DM2CNT_H & 0x4000) {
        IF |= 0x0400;
        UPDATE_REG(0x202, IF);
        cpuNextEvent = cpuTotalTicks;
      }

      if(((DM2CNT_H >> 5) & 3) == 3) {
        dma2Dest = DM2DAD_L | (DM2DAD_H << 16);
      }
      
      if(!(DM2CNT_H & 0x0200) || (reason == 0)) {
        DM2CNT_H &= 0x7FFF;
        UPDATE_REG(0xD2, DM2CNT_H);
      }
    }
  }

  // DMA 3
  if((DM3CNT_H & 0x8000) && (dmamask & 8)) {
    if(((DM3CNT_H >> 12) & 3) == reason) {
      u32 sourceIncrement = 4;
      u32 destIncrement = 4;
      switch((DM3CNT_H >> 7) & 3) {
      case 0:
        break;
      case 1:
        sourceIncrement = (u32)-4;
        break;
      case 2:
        sourceIncrement = 0;
        break;
      }
      switch((DM3CNT_H >> 5) & 3) {
      case 0:
        break;
      case 1:
        destIncrement = (u32)-4;
        break;
      case 2:
        destIncrement = 0;
        break;
      }      
#ifdef DEV_VERSION
      if(systemVerbose & VERBOSE_DMA3) {
        int count = (DM3CNT_L ? DM3CNT_L : 0x10000) << 1;
        if(DM3CNT_H & 0x0400)
          count <<= 1;
        log("DMA3: s=%08x d=%08x c=%04x count=%08x\n", dma3Source, dma3Dest,
            DM3CNT_H,
            count);
      }
#endif                
      doDMA(&dma3Source, &dma3Dest, sourceIncrement, destIncrement,
            DM3CNT_L ? DM3CNT_L : 0x10000,
            DM3CNT_H & 0x0400);
      if(DM3CNT_H & 0x4000) {
        IF |= 0x0800;
        UPDATE_REG(0x202, IF);
        cpuNextEvent = cpuTotalTicks;
      }

      if(((DM3CNT_H >> 5) & 3) == 3) {
        dma3Dest = DM3DAD_L | (DM3DAD_H << 16);
      }
      
      if(!(DM3CNT_H & 0x0200) || (reason == 0)) {
        DM3CNT_H &= 0x7FFF;
        UPDATE_REG(0xDE, DM3CNT_H);
      }
    }
  }
}

void CPUUpdateRegister(u32 address, u16 value)
{
  switch(address) {
  case 0x00:
    {
      if ((value & 7) >5)
          DISPCNT = (value &7);
      bool change = ((DISPCNT ^ value) & 0x80) ? true : false;
      bool changeBG = ((DISPCNT ^ value) & 0x0F00) ? true : false;
      u16 changeBGon = (((~DISPCNT) & value) & 0x0F00);
      DISPCNT = (value & 0xFFF7);
      UPDATE_REG(0x00, DISPCNT);

      if (changeBGon)
      {
         layerEnableDelay=4;
         layerEnable = layerSettings & value & (~changeBGon);
      }
       else
         layerEnable = layerSettings & value;
      //      CPUUpdateTicks();

      windowOn = (layerEnable & 0x6000) ? true : false;
      if(change && !((value & 0x80))) {
        if(!(DISPSTAT & 1)) {
          lcdTicks = 1008;
          //      VCOUNT = 0;
          //      UPDATE_REG(0x06, VCOUNT);
          DISPSTAT &= 0xFFFC;
          UPDATE_REG(0x04, DISPSTAT);
          CPUCompareVCOUNT();
        }
        //        (*renderLine)();
      }
      CPUUpdateRender();
      // we only care about changes in BG0-BG3
      if(changeBG)
        CPUUpdateRenderBuffers(false);
    }
    break;
  case 0x04:
    DISPSTAT = (value & 0xFF38) | (DISPSTAT & 7);
    UPDATE_REG(0x04, DISPSTAT);
    break;
  case 0x06:
    // not writable
    break;
  case 0x08:
    BG0CNT = (value & 0xDFCF);
    UPDATE_REG(0x08, BG0CNT);
    break;
  case 0x0A:
    BG1CNT = (value & 0xDFCF);
    UPDATE_REG(0x0A, BG1CNT);
    break;
  case 0x0C:
    BG2CNT = (value & 0xFFCF);
    UPDATE_REG(0x0C, BG2CNT);
    break;
  case 0x0E:
    BG3CNT = (value & 0xFFCF);
    UPDATE_REG(0x0E, BG3CNT);
    break;
  case 0x10:
    BG0HOFS = value & 511;
    UPDATE_REG(0x10, BG0HOFS);
    break;
  case 0x12:
    BG0VOFS = value & 511;
    UPDATE_REG(0x12, BG0VOFS);
    break;
  case 0x14:
    BG1HOFS = value & 511;
    UPDATE_REG(0x14, BG1HOFS);
    break;
  case 0x16:
    BG1VOFS = value & 511;
    UPDATE_REG(0x16, BG1VOFS);
    break;      
  case 0x18:
    BG2HOFS = value & 511;
    UPDATE_REG(0x18, BG2HOFS);
    break;
  case 0x1A:
    BG2VOFS = value & 511;
    UPDATE_REG(0x1A, BG2VOFS);
    break;
  case 0x1C:
    BG3HOFS = value & 511;
    UPDATE_REG(0x1C, BG3HOFS);
    break;
  case 0x1E:
    BG3VOFS = value & 511;
    UPDATE_REG(0x1E, BG3VOFS);
    break;      
  case 0x20:
    BG2PA = value;
    UPDATE_REG(0x20, BG2PA);
    break;
  case 0x22:
    BG2PB = value;
    UPDATE_REG(0x22, BG2PB);
    break;
  case 0x24:
    BG2PC = value;
    UPDATE_REG(0x24, BG2PC);
    break;
  case 0x26:
    BG2PD = value;
    UPDATE_REG(0x26, BG2PD);
    break;
  case 0x28:
    BG2X_L = value;
    UPDATE_REG(0x28, BG2X_L);
    gfxBG2Changed |= 1;
    break;
  case 0x2A:
    BG2X_H = (value & 0xFFF);
    UPDATE_REG(0x2A, BG2X_H);
    gfxBG2Changed |= 1;    
    break;
  case 0x2C:
    BG2Y_L = value;
    UPDATE_REG(0x2C, BG2Y_L);
    gfxBG2Changed |= 2;    
    break;
  case 0x2E:
    BG2Y_H = value & 0xFFF;
    UPDATE_REG(0x2E, BG2Y_H);
    gfxBG2Changed |= 2;    
    break;
  case 0x30:
    BG3PA = value;
    UPDATE_REG(0x30, BG3PA);
    break;
  case 0x32:
    BG3PB = value;
    UPDATE_REG(0x32, BG3PB);
    break;
  case 0x34:
    BG3PC = value;
    UPDATE_REG(0x34, BG3PC);
    break;
  case 0x36:
    BG3PD = value;
    UPDATE_REG(0x36, BG3PD);
    break;
  case 0x38:
    BG3X_L = value;
    UPDATE_REG(0x38, BG3X_L);
    gfxBG3Changed |= 1;
    break;
  case 0x3A:
    BG3X_H = value & 0xFFF;
    UPDATE_REG(0x3A, BG3X_H);
    gfxBG3Changed |= 1;    
    break;
  case 0x3C:
    BG3Y_L = value;
    UPDATE_REG(0x3C, BG3Y_L);
    gfxBG3Changed |= 2;    
    break;
  case 0x3E:
    BG3Y_H = value & 0xFFF;
    UPDATE_REG(0x3E, BG3Y_H);
    gfxBG3Changed |= 2;    
    break;
  case 0x40:
    WIN0H = value;
    UPDATE_REG(0x40, WIN0H);
    CPUUpdateWindow0();
    break;
  case 0x42:
    WIN1H = value;
    UPDATE_REG(0x42, WIN1H);
    CPUUpdateWindow1();    
    break;      
  case 0x44:
    WIN0V = value;
    UPDATE_REG(0x44, WIN0V);
    break;
  case 0x46:
    WIN1V = value;
    UPDATE_REG(0x46, WIN1V);
    break;
  case 0x48:
    WININ = value & 0x3F3F;
    UPDATE_REG(0x48, WININ);
    break;
  case 0x4A:
    WINOUT = value & 0x3F3F;
    UPDATE_REG(0x4A, WINOUT);
    break;
  case 0x4C:
    MOSAIC = value;
    UPDATE_REG(0x4C, MOSAIC);
    break;
  case 0x50:
    BLDMOD = value & 0x3FFF;
    UPDATE_REG(0x50, BLDMOD);
    fxOn = ((BLDMOD>>6)&3) != 0;
    CPUUpdateRender();
    break;
  case 0x52:
    COLEV = value & 0x1F1F;
    UPDATE_REG(0x52, COLEV);
    break;
  case 0x54:
    COLY = value & 0x1F;
    UPDATE_REG(0x54, COLY);
    break;
  case 0x60:
  case 0x62:
  case 0x64:
  case 0x68:
  case 0x6c:
  case 0x70:
  case 0x72:
  case 0x74:
  case 0x78:
  case 0x7c:
  case 0x80:
  case 0x84:
    soundEvent8(address&0xFF, (u8)(value & 0xFF));
    soundEvent8((address&0xFF)+1, (u8)(value>>8));
    break;
  case 0x82:
  case 0x88:
  case 0xa0:
  case 0xa2:
  case 0xa4:
  case 0xa6:
  case 0x90:
  case 0x92:
  case 0x94:
  case 0x96:
  case 0x98:
  case 0x9a:
  case 0x9c:
  case 0x9e:    
    soundEvent16(address&0xFF, value);
    break;
  case 0xB0:
    DM0SAD_L = value;
    UPDATE_REG(0xB0, DM0SAD_L);
    break;
  case 0xB2:
    DM0SAD_H = value & 0x07FF;
    UPDATE_REG(0xB2, DM0SAD_H);
    break;
  case 0xB4:
    DM0DAD_L = value;
    UPDATE_REG(0xB4, DM0DAD_L);
    break;
  case 0xB6:
    DM0DAD_H = value & 0x07FF;
    UPDATE_REG(0xB6, DM0DAD_H);
    break;
  case 0xB8:
    DM0CNT_L = value & 0x3FFF;
    UPDATE_REG(0xB8, 0);
    break;
  case 0xBA:
    {
      bool start = ((DM0CNT_H ^ value) & 0x8000) ? true : false;
      value &= 0xF7E0;

      DM0CNT_H = value;
      UPDATE_REG(0xBA, DM0CNT_H);    
    
      if(start && (value & 0x8000)) {
        dma0Source = DM0SAD_L | (DM0SAD_H << 16);
        dma0Dest = DM0DAD_L | (DM0DAD_H << 16);
        CPUCheckDMA(0, 1);
      }
    }
    break;      
  case 0xBC:
    DM1SAD_L = value;
    UPDATE_REG(0xBC, DM1SAD_L);
    break;
  case 0xBE:
    DM1SAD_H = value & 0x0FFF;
    UPDATE_REG(0xBE, DM1SAD_H);
    break;
  case 0xC0:
    DM1DAD_L = value;
    UPDATE_REG(0xC0, DM1DAD_L);
    break;
  case 0xC2:
    DM1DAD_H = value & 0x07FF;
    UPDATE_REG(0xC2, DM1DAD_H);
    break;
  case 0xC4:
    DM1CNT_L = value & 0x3FFF;
    UPDATE_REG(0xC4, 0);
    break;
  case 0xC6:
    {
      bool start = ((DM1CNT_H ^ value) & 0x8000) ? true : false;
      value &= 0xF7E0;
      
      DM1CNT_H = value;
      UPDATE_REG(0xC6, DM1CNT_H);
      
      if(start && (value & 0x8000)) {
        dma1Source = DM1SAD_L | (DM1SAD_H << 16);
        dma1Dest = DM1DAD_L | (DM1DAD_H << 16);
        CPUCheckDMA(0, 2);
      }
    }
    break;
  case 0xC8:
    DM2SAD_L = value;
    UPDATE_REG(0xC8, DM2SAD_L);
    break;
  case 0xCA:
    DM2SAD_H = value & 0x0FFF;
    UPDATE_REG(0xCA, DM2SAD_H);
    break;
  case 0xCC:
    DM2DAD_L = value;
    UPDATE_REG(0xCC, DM2DAD_L);
    break;
  case 0xCE:
    DM2DAD_H = value & 0x07FF;
    UPDATE_REG(0xCE, DM2DAD_H);
    break;
  case 0xD0:
    DM2CNT_L = value & 0x3FFF;
    UPDATE_REG(0xD0, 0);
    break;
  case 0xD2:
    {
      bool start = ((DM2CNT_H ^ value) & 0x8000) ? true : false;
      
      value &= 0xF7E0;
      
      DM2CNT_H = value;
      UPDATE_REG(0xD2, DM2CNT_H);
      
      if(start && (value & 0x8000)) {
        dma2Source = DM2SAD_L | (DM2SAD_H << 16);
        dma2Dest = DM2DAD_L | (DM2DAD_H << 16);

        CPUCheckDMA(0, 4);
      }            
    }
    break;
  case 0xD4:
    DM3SAD_L = value;
    UPDATE_REG(0xD4, DM3SAD_L);
    break;
  case 0xD6:
    DM3SAD_H = value & 0x0FFF;
    UPDATE_REG(0xD6, DM3SAD_H);
    break;
  case 0xD8:
    DM3DAD_L = value;
    UPDATE_REG(0xD8, DM3DAD_L);
    break;
  case 0xDA:
    DM3DAD_H = value & 0x0FFF;
    UPDATE_REG(0xDA, DM3DAD_H);
    break;
  case 0xDC:
    DM3CNT_L = value;
    UPDATE_REG(0xDC, 0);
    break;
  case 0xDE:
    {
      bool start = ((DM3CNT_H ^ value) & 0x8000) ? true : false;

      value &= 0xFFE0;

      DM3CNT_H = value;
      UPDATE_REG(0xDE, DM3CNT_H);
    
      if(start && (value & 0x8000)) {
        dma3Source = DM3SAD_L | (DM3SAD_H << 16);
        dma3Dest = DM3DAD_L | (DM3DAD_H << 16);
        CPUCheckDMA(0,8);
      }
    }
    break;
 case 0x100:
    timer0Reload = value;
    break;
  case 0x102:
    timer0Value = value;
    timerOnOffDelay|=1;
    cpuNextEvent = cpuTotalTicks;
    break;
  case 0x104:
    timer1Reload = value;
    break;
  case 0x106:
    timer1Value = value;
    timerOnOffDelay|=2;
    cpuNextEvent = cpuTotalTicks;
    break;
  case 0x108:
    timer2Reload = value;
    break;
  case 0x10A:
    timer2Value = value;
    timerOnOffDelay|=4;
    cpuNextEvent = cpuTotalTicks;
    break;
  case 0x10C:
    timer3Reload = value;
    break;
  case 0x10E:
    timer3Value = value;
    timerOnOffDelay|=8;
    cpuNextEvent = cpuTotalTicks;
    break;
  case 0x128:
    if(value & 0x80) {
      value &= 0xff7f;
      if(value & 1 && (value & 0x4000)) {
        UPDATE_REG(0x12a, 0xFF);
        IF |= 0x80;
        UPDATE_REG(0x202, IF);
        value &= 0x7f7f;
      }
    }
    UPDATE_REG(0x128, value);
    break;
  case 0x130:
    P1 |= (value & 0x3FF);
    UPDATE_REG(0x130, P1);
    break;
  case 0x132:
    UPDATE_REG(0x132, value & 0xC3FF);
    break;
  case 0x200:
    IE = value & 0x3FFF;
    UPDATE_REG(0x200, IE);
    if ((IME & 1) && (IF & IE) && armIrqEnable)
      cpuNextEvent = cpuTotalTicks;
    break;
  case 0x202:
    IF ^= (value & IF);
    UPDATE_REG(0x202, IF);
    break;
  case 0x204:
    {
      memoryWait[0x0e] = memoryWaitSeq[0x0e] = gamepakRamWaitState[value & 3];
      
      if(!speedHack) {
        memoryWait[0x08] = memoryWait[0x09] = gamepakWaitState[(value >> 2) & 3];
        memoryWaitSeq[0x08] = memoryWaitSeq[0x09] =
          gamepakWaitState0[(value >> 4) & 1];
        
        memoryWait[0x0a] = memoryWait[0x0b] = gamepakWaitState[(value >> 5) & 3];
        memoryWaitSeq[0x0a] = memoryWaitSeq[0x0b] =
          gamepakWaitState1[(value >> 7) & 1];
        
        memoryWait[0x0c] = memoryWait[0x0d] = gamepakWaitState[(value >> 8) & 3];
        memoryWaitSeq[0x0c] = memoryWaitSeq[0x0d] =
          gamepakWaitState2[(value >> 10) & 1];
      } else {
        memoryWait[0x08] = memoryWait[0x09] = 3;
        memoryWaitSeq[0x08] = memoryWaitSeq[0x09] = 1;
        
        memoryWait[0x0a] = memoryWait[0x0b] = 3;
        memoryWaitSeq[0x0a] = memoryWaitSeq[0x0b] = 1;
        
        memoryWait[0x0c] = memoryWait[0x0d] = 3;
        memoryWaitSeq[0x0c] = memoryWaitSeq[0x0d] = 1;
      }
         
      for(int i = 8; i < 15; i++) {
        memoryWait32[i] = memoryWait[i] + memoryWaitSeq[i] + 1;
        memoryWaitSeq32[i] = memoryWaitSeq[i]*2 + 1;
      }

      if((value & 0x4000) == 0x4000) {
        busPrefetchEnable = true;
        busPrefetch = false;
        busPrefetchCount = 0;
      } else {
        busPrefetchEnable = false;
        busPrefetch = false;
        busPrefetchCount = 0;
      }
      UPDATE_REG(0x204, value & 0x7FFF);

    }
    break;
  case 0x208:
    IME = value & 1;
    UPDATE_REG(0x208, IME);
    if ((IME & 1) && (IF & IE) && armIrqEnable)
      cpuNextEvent = cpuTotalTicks;
    break;
  case 0x300:
    if(value != 0)
      value &= 0xFFFE;
    UPDATE_REG(0x300, value);
    break;
  default:
    UPDATE_REG(address&0x3FE, value);
    break;
  }
}

void applyTimer ()
{
  if (timerOnOffDelay & 1)
  {
    timer0ClockReload = TIMER_TICKS[timer0Value & 3];        
    if(!timer0On && (timer0Value & 0x80)) {
      // reload the counter
      TM0D = timer0Reload;      
      timer0Ticks = (0x10000 - TM0D) << timer0ClockReload;
      UPDATE_REG(0x100, TM0D);
    }
    timer0On = timer0Value & 0x80 ? true : false;
    TM0CNT = timer0Value & 0xC7;
    UPDATE_REG(0x102, TM0CNT);
    //    CPUUpdateTicks();
  }
  if (timerOnOffDelay & 2)
  {
    timer1ClockReload = TIMER_TICKS[timer1Value & 3];        
    if(!timer1On && (timer1Value & 0x80)) {
      // reload the counter
      TM1D = timer1Reload;      
      timer1Ticks = (0x10000 - TM1D) << timer1ClockReload;
      UPDATE_REG(0x104, TM1D);
    }
    timer1On = timer1Value & 0x80 ? true : false;
    TM1CNT = timer1Value & 0xC7;
    UPDATE_REG(0x106, TM1CNT);
  }
  if (timerOnOffDelay & 4)
  {
    timer2ClockReload = TIMER_TICKS[timer2Value & 3];        
    if(!timer2On && (timer2Value & 0x80)) {
      // reload the counter
      TM2D = timer2Reload;      
      timer2Ticks = (0x10000 - TM2D) << timer2ClockReload;
      UPDATE_REG(0x108, TM2D);
    }
    timer2On = timer2Value & 0x80 ? true : false;
    TM2CNT = timer2Value & 0xC7;
    UPDATE_REG(0x10A, TM2CNT);
  }
  if (timerOnOffDelay & 8)
  {
    timer3ClockReload = TIMER_TICKS[timer3Value & 3];        
    if(!timer3On && (timer3Value & 0x80)) {
      // reload the counter
      TM3D = timer3Reload;      
      timer3Ticks = (0x10000 - TM3D) << timer3ClockReload;
      UPDATE_REG(0x10C, TM3D);
    }
    timer3On = timer3Value & 0x80 ? true : false;
    TM3CNT = timer3Value & 0xC7;
    UPDATE_REG(0x10E, TM3CNT);
  }
  cpuNextEvent = CPUUpdateTicks();
  timerOnOffDelay = 0;
}

void CPUWriteHalfWord(u32 address, u16 value)
{
#ifdef DEV_VERSION
  if(address & 1) {
    if(systemVerbose & VERBOSE_UNALIGNED_MEMORY) {
      log("Unaligned halfword write: %04x to %08x from %08x\n",
          value,
          address,
          armMode ? armNextPC - 4 : armNextPC - 2);
    }
  }
#endif
  
  switch(address >> 24) {
  case 2:
#ifdef BKPT_SUPPORT
    if(*((u16 *)&freezeWorkRAM[address & 0x3FFFE]))
      cheatsWriteHalfWord(address & 0x203FFFE,
                          value);
    else
#endif
      WRITE16LE(((u16 *)&workRAM[address & 0x3FFFE]),value);
    break;
  case 3:
#ifdef BKPT_SUPPORT
    if(*((u16 *)&freezeInternalRAM[address & 0x7ffe]))
      cheatsWriteHalfWord(address & 0x3007ffe,
                          value);
    else
#endif
      WRITE16LE(((u16 *)&internalRAM[address & 0x7ffe]), value);
    break;    
  case 4:
    if(address < 0x4000400)
      CPUUpdateRegister(address & 0x3fe, value);
    else goto unwritable;
    break;
  case 5:
#ifdef BKPT_SUPPORT
    if(*((u16 *)&freezePRAM[address & 0x03fe]))
      cheatsWriteHalfWord(address & 0x70003fe,
                          value);
    else
#endif
    WRITE16LE(((u16 *)&paletteRAM[address & 0x3fe]), value);
    break;
  case 6:
    address = (address & 0x1fffe);
    if (((DISPCNT & 7) >2) && ((address & 0x1C000) == 0x18000))
        return;
    if ((address & 0x18000) == 0x18000)
      address &= 0x17fff;
#ifdef BKPT_SUPPORT
    if(*((u16 *)&freezeVRAM[address]))
      cheatsWriteHalfWord(address + 0x06000000,
                          value);
    else
#endif
    WRITE16LE(((u16 *)&vram[address]), value); 
    break;
  case 7:
#ifdef BKPT_SUPPORT
    if(*((u16 *)&freezeOAM[address & 0x03fe]))
      cheatsWriteHalfWord(address & 0x70003fe,
                          value);
    else
#endif
    WRITE16LE(((u16 *)&oam[address & 0x3fe]), value);
    break;
  case 8:
  case 9:
    if(address == 0x80000c4 || address == 0x80000c6 || address == 0x80000c8) {
      if(!rtcWrite(address, value))
        goto unwritable;
    } else if(!agbPrintWrite(address, value)) goto unwritable;
    break;
  case 13:
    if(cpuEEPROMEnabled) {
      eepromWrite(address, (u8)value);
      break;
    }
    goto unwritable;
  case 14:
    if(!eepromInUse | cpuSramEnabled | cpuFlashEnabled) {
      (*cpuSaveGameFunc)(address, (u8)value);
      break;
    }
    goto unwritable;
  default:
  unwritable:
#ifdef DEV_VERSION
    if(systemVerbose & VERBOSE_ILLEGAL_WRITE) {
      log("Illegal halfword write: %04x to %08x from %08x\n",
          value,
          address,
          armMode ? armNextPC - 4 : armNextPC - 2);
    }
#endif
    break;
  }
}

void CPUWriteByte(u32 address, u8 b)
{
  switch(address >> 24) {
  case 2:
#ifdef BKPT_SUPPORT
      if(freezeWorkRAM[address & 0x3FFFF])
        cheatsWriteByte(address & 0x203FFFF, b);
      else
#endif  
        workRAM[address & 0x3FFFF] = b;
    break;
  case 3:
#ifdef BKPT_SUPPORT
    if(freezeInternalRAM[address & 0x7fff])
      cheatsWriteByte(address & 0x3007fff, b);
    else
#endif
      internalRAM[address & 0x7fff] = b;
    break;
  case 4:
    if(address < 0x4000400) {
      switch(address & 0x3FF) {
      case 0x301:
	if(b == 0x80)
	  stopState = true;
	holdState = 1;
	holdType = -1;
  cpuNextEvent = cpuTotalTicks;
	break;
      case 0x60:
      case 0x61:
      case 0x62:
      case 0x63:
      case 0x64:
      case 0x65:
      case 0x68:
      case 0x69:
      case 0x6c:
      case 0x6d:
      case 0x70:
      case 0x71:
      case 0x72:
      case 0x73:
      case 0x74:
      case 0x75:
      case 0x78:
      case 0x79:
      case 0x7c:
      case 0x7d:
      case 0x80:
      case 0x81:
      case 0x84:
      case 0x85:
      case 0x90:
      case 0x91:
      case 0x92:
      case 0x93:
      case 0x94:
      case 0x95:
      case 0x96:
      case 0x97:
      case 0x98:
      case 0x99:
      case 0x9a:
      case 0x9b:
      case 0x9c:
      case 0x9d:
      case 0x9e:
      case 0x9f:      
	soundEvent8(address&0xFF, b);
	break;
      default:
	if(address & 1)
	  CPUUpdateRegister(address & 0x3fe,
			    ((READ16LE(((u16 *)&ioMem[address & 0x3fe])))
			     & 0x00FF) |
			    b<<8);
	else
	  CPUUpdateRegister(address & 0x3fe,
			    ((READ16LE(((u16 *)&ioMem[address & 0x3fe])) & 0xFF00) | b));
      }
      break;
    } else goto unwritable;
    break;
  case 5:
    // no need to switch
    *((u16 *)&paletteRAM[address & 0x3FE]) = (b << 8) | b;
    break;
  case 6:
    address = (address & 0x1fffe);
    if (((DISPCNT & 7) >2) && ((address & 0x1C000) == 0x18000))
        return;
    if ((address & 0x18000) == 0x18000)
      address &= 0x17fff;

    // no need to switch 
    // byte writes to OBJ VRAM are ignored
    if ((address) < objTilesAddress[((DISPCNT&7)+1)>>2])
    {
#ifdef BKPT_SUPPORT
      if(freezeVRAM[address])
        cheatsWriteByte(address + 0x06000000, b);
      else
#endif  
            *((u16 *)&vram[address]) = (b << 8) | b;
    }
    break;
  case 7:
    // no need to switch
    // byte writes to OAM are ignored
    //    *((u16 *)&oam[address & 0x3FE]) = (b << 8) | b;
    break;    
  case 13:
    if(cpuEEPROMEnabled) {
      eepromWrite(address, b);
      break;
    }
    goto unwritable;
  case 14:
      if (!(saveType == 5) && (!eepromInUse | cpuSramEnabled | cpuFlashEnabled)) {

    //if(!cpuEEPROMEnabled && (cpuSramEnabled | cpuFlashEnabled)) { 

        (*cpuSaveGameFunc)(address, b);
      break;
    }
    // default
  default:
  unwritable:
#ifdef DEV_VERSION
    if(systemVerbose & VERBOSE_ILLEGAL_WRITE) {
      log("Illegal byte write: %02x to %08x from %08x\n",
          b,
          address,
          armMode ? armNextPC - 4 : armNextPC -2 );
    }
#endif
    break;
  }
}

u8 cpuBitsSet[256];
u8 cpuLowestBitSet[256];

void CPUInit(const char *biosFileName, bool useBiosFile)
{
  gbaSaveType = 0;
  eepromInUse = 0;
  saveType = 0;
  useBios = false;
  
  if(useBiosFile) {
    int size = 0x4000;
    if(utilLoad(biosFileName,
                CPUIsGBABios,
                bios,
                &size)) {
      if(size == 0x4000)
        useBios = true;
      else
        systemMessage(MSG_INVALID_BIOS_FILE_SIZE, N_("Invalid BIOS file size"));
    }
  }
  
  if(!useBios) {
    memcpy(bios, myROM, myROM_size);
  }

  int i = 0;

  biosProtected[0] = 0x00;
  biosProtected[1] = 0xf0;
  biosProtected[2] = 0x29;
  biosProtected[3] = 0xe1;

  init_cleared_line();

  for(i = 0; i < 256; i++) {
    int count = 0;
    int j;
    for(j = 0; j < 8; j++)
      if(i & (1 << j))
        count++;
    cpuBitsSet[i] = count;
    
    for(j = 0; j < 8; j++)
      if(i & (1 << j))
        break;
    cpuLowestBitSet[i] = j;
  }

  for(i = 0; i < 0x400; i++)
    ioReadable[i] = true;
  for(i = 0x10; i < 0x48; i++)
    ioReadable[i] = false;
  for(i = 0x4c; i < 0x50; i++)
    ioReadable[i] = false;
  for(i = 0x54; i < 0x60; i++)
    ioReadable[i] = false;
  for(i = 0x8c; i < 0x90; i++)
    ioReadable[i] = false;
  for(i = 0xa0; i < 0xb8; i++)
    ioReadable[i] = false;
  for(i = 0xbc; i < 0xc4; i++)
    ioReadable[i] = false;
  for(i = 0xc8; i < 0xd0; i++)
    ioReadable[i] = false;
  for(i = 0xd4; i < 0xdc; i++)
    ioReadable[i] = false;
  for(i = 0xe0; i < 0x100; i++)
    ioReadable[i] = false;
  for(i = 0x110; i < 0x120; i++)
    ioReadable[i] = false;
  for(i = 0x12c; i < 0x130; i++)
    ioReadable[i] = false;
  for(i = 0x138; i < 0x140; i++)
    ioReadable[i] = false;
  for(i = 0x144; i < 0x150; i++)
    ioReadable[i] = false;
  for(i = 0x15c; i < 0x200; i++)
    ioReadable[i] = false;
  for(i = 0x20c; i < 0x300; i++)
    ioReadable[i] = false;
  for(i = 0x304; i < 0x400; i++)
    ioReadable[i] = false;

  if(romSize < 0x1fe2000) {
    *((u16 *)&rom[0x1fe209c]) = 0xdffa; // SWI 0xFA
    *((u16 *)&rom[0x1fe209e]) = 0x4770; // BX LR
  } else {
    agbPrintEnable(false);
  }
}

void CPUReset()
{
  if(gbaSaveType == 0) {
    if(eepromInUse)
      gbaSaveType = 3;
    else
      switch(saveType) {
      case 1:
        gbaSaveType = 1;
        break;
      case 2:
        gbaSaveType = 2;
        break;
      }
  }
  rtcReset();
  // clean registers
  memset(&reg[0], 0, sizeof(reg));
  // clean OAM
  memset(oam, 0, 0x400);
  // clean palette
  memset(paletteRAM, 0, 0x400);
  // clean picture
  memset(pix, 0, 4*160*240);
  // clean vram
  memset(vram, 0, 0x20000);
  // clean io memory
  memset(ioMem, 0, 0x400);

  DISPCNT  = 0x0080;
  DISPSTAT = 0x0000;
  VCOUNT   = (useBios && !skipBios) ? 0 :0x007E;
  BG0CNT   = 0x0000;
  BG1CNT   = 0x0000;
  BG2CNT   = 0x0000;
  BG3CNT   = 0x0000;
  BG0HOFS  = 0x0000;
  BG0VOFS  = 0x0000;
  BG1HOFS  = 0x0000;
  BG1VOFS  = 0x0000;
  BG2HOFS  = 0x0000;
  BG2VOFS  = 0x0000;
  BG3HOFS  = 0x0000;
  BG3VOFS  = 0x0000;
  BG2PA    = 0x0100;
  BG2PB    = 0x0000;
  BG2PC    = 0x0000;
  BG2PD    = 0x0100;
  BG2X_L   = 0x0000;
  BG2X_H   = 0x0000;
  BG2Y_L   = 0x0000;
  BG2Y_H   = 0x0000;
  BG3PA    = 0x0100;
  BG3PB    = 0x0000;
  BG3PC    = 0x0000;
  BG3PD    = 0x0100;
  BG3X_L   = 0x0000;
  BG3X_H   = 0x0000;
  BG3Y_L   = 0x0000;
  BG3Y_H   = 0x0000;
  WIN0H    = 0x0000;
  WIN1H    = 0x0000;
  WIN0V    = 0x0000;
  WIN1V    = 0x0000;
  WININ    = 0x0000;
  WINOUT   = 0x0000;
  MOSAIC   = 0x0000;
  BLDMOD   = 0x0000;
  COLEV    = 0x0000;
  COLY     = 0x0000;
  DM0SAD_L = 0x0000;
  DM0SAD_H = 0x0000;
  DM0DAD_L = 0x0000;
  DM0DAD_H = 0x0000;
  DM0CNT_L = 0x0000;
  DM0CNT_H = 0x0000;
  DM1SAD_L = 0x0000;
  DM1SAD_H = 0x0000;
  DM1DAD_L = 0x0000;
  DM1DAD_H = 0x0000;
  DM1CNT_L = 0x0000;
  DM1CNT_H = 0x0000;
  DM2SAD_L = 0x0000;
  DM2SAD_H = 0x0000;
  DM2DAD_L = 0x0000;
  DM2DAD_H = 0x0000;
  DM2CNT_L = 0x0000;
  DM2CNT_H = 0x0000;
  DM3SAD_L = 0x0000;
  DM3SAD_H = 0x0000;
  DM3DAD_L = 0x0000;
  DM3DAD_H = 0x0000;
  DM3CNT_L = 0x0000;
  DM3CNT_H = 0x0000;
  TM0D     = 0x0000;
  TM0CNT   = 0x0000;
  TM1D     = 0x0000;
  TM1CNT   = 0x0000;
  TM2D     = 0x0000;
  TM2CNT   = 0x0000;
  TM3D     = 0x0000;
  TM3CNT   = 0x0000;
  P1       = 0x03FF;
  IE       = 0x0000;
  IF       = 0x0000;
  IME      = 0x0000;

  armMode = 0x1F;
  
  if(cpuIsMultiBoot) {
    reg[13].I = 0x03007F00;
    reg[15].I = 0x02000000;
    reg[16].I = 0x00000000;
    reg[R13_IRQ].I = 0x03007FA0;
    reg[R13_SVC].I = 0x03007FE0;
    armIrqEnable = true;
  } else {
    if(useBios && !skipBios) {
      reg[15].I = 0x00000000;
      armMode = 0x13;
      armIrqEnable = false;  
    } else {
      reg[13].I = 0x03007F00;
      reg[15].I = 0x08000000;
      reg[16].I = 0x00000000;
      reg[R13_IRQ].I = 0x03007FA0;
      reg[R13_SVC].I = 0x03007FE0;
      armIrqEnable = true;      
    }    
  }
  armState = true;
  C_FLAG = V_FLAG = N_FLAG = Z_FLAG = false;
  UPDATE_REG(0x00, DISPCNT);
  UPDATE_REG(0x06, VCOUNT);
  UPDATE_REG(0x20, BG2PA);
  UPDATE_REG(0x26, BG2PD);
  UPDATE_REG(0x30, BG3PA);
  UPDATE_REG(0x36, BG3PD);
  UPDATE_REG(0x130, P1);
  UPDATE_REG(0x88, 0x200);

  // disable FIQ
  reg[16].I |= 0x40;
  
  CPUUpdateCPSR();
  
  armNextPC = reg[15].I;
  reg[15].I += 4;

  // reset internal state
  holdState = false;
  holdType = 0;
  
  biosProtected[0] = 0x00;
  biosProtected[1] = 0xf0;
  biosProtected[2] = 0x29;
  biosProtected[3] = 0xe1;
  
  lcdTicks = (useBios && !skipBios) ? 1008 : 208;
  timer0On = false;
  timer0Ticks = 0;
  timer0Reload = 0;
  timer0ClockReload  = 0;
  timer1On = false;
  timer1Ticks = 0;
  timer1Reload = 0;
  timer1ClockReload  = 0;
  timer2On = false;
  timer2Ticks = 0;
  timer2Reload = 0;
  timer2ClockReload  = 0;
  timer3On = false;
  timer3Ticks = 0;
  timer3Reload = 0;
  timer3ClockReload  = 0;
  dma0Source = 0;
  dma0Dest = 0;
  dma1Source = 0;
  dma1Dest = 0;
  dma2Source = 0;
  dma2Dest = 0;
  dma3Source = 0;
  dma3Dest = 0;
  cpuSaveGameFunc = flashSaveDecide;
  renderLine = mode0RenderLine;
  fxOn = false;
  windowOn = false;
  frameCount = 0;
  saveType = 0;
  layerEnable = DISPCNT & layerSettings;

  CPUUpdateRenderBuffers(true);
  
  for(int i = 0; i < 256; i++) {
    map[i].address = (u8 *)&dummyAddress;
    map[i].mask = 0;
  }

  map[0].address = bios;
  map[0].mask = 0x3FFF;
  map[2].address = workRAM;
  map[2].mask = 0x3FFFF;
  map[3].address = internalRAM;
  map[3].mask = 0x7FFF;
  map[4].address = ioMem;
  map[4].mask = 0x3FF;
  map[5].address = paletteRAM;
  map[5].mask = 0x3FF;
  map[6].address = vram;
  map[6].mask = 0x1FFFF;
  map[7].address = oam;
  map[7].mask = 0x3FF;
  map[8].address = rom;
  map[8].mask = 0x1FFFFFF;
  map[9].address = rom;
  map[9].mask = 0x1FFFFFF;  
  map[10].address = rom;
  map[10].mask = 0x1FFFFFF;
  map[12].address = rom;
  map[12].mask = 0x1FFFFFF;
  map[14].address = flashSaveMemory;
  map[14].mask = 0xFFFF;

  eepromReset();
  flashReset();
  
  soundReset();

  CPUUpdateWindow0();
  CPUUpdateWindow1();

  // make sure registers are correctly initialized if not using BIOS
  if(!useBios) {
    if(cpuIsMultiBoot)
      BIOS_RegisterRamResetU(0xfe);
    else
      BIOS_RegisterRamResetU(0xff);
  } else {
    if(cpuIsMultiBoot)
      BIOS_RegisterRamResetU(0xfe);
  }

  switch(cpuSaveType) {
  case 0: // automatic
    cpuSramEnabled = true;
    cpuFlashEnabled = true;
    cpuEEPROMEnabled = true;
    cpuEEPROMSensorEnabled = false;
    saveType = gbaSaveType = 0;
    break;
  case 1: // EEPROM
    cpuSramEnabled = false;
    cpuFlashEnabled = false;
    cpuEEPROMEnabled = true;
    cpuEEPROMSensorEnabled = false;
    saveType = gbaSaveType = 3;
    // EEPROM usage is automatically detected
    break;
  case 2: // SRAM
    cpuSramEnabled = true;
    cpuFlashEnabled = false;
    cpuEEPROMEnabled = false;
    cpuEEPROMSensorEnabled = false;
    cpuSaveGameFunc = sramDelayedWrite; // to insure we detect the write
    saveType = gbaSaveType = 1;
    break;
  case 3: // FLASH
    cpuSramEnabled = false;
    cpuFlashEnabled = true;
    cpuEEPROMEnabled = false;
    cpuEEPROMSensorEnabled = false;
    cpuSaveGameFunc = flashDelayedWrite; // to insure we detect the write
    saveType = gbaSaveType = 2;
    break;
  case 4: // EEPROM+Sensor
    cpuSramEnabled = false;
    cpuFlashEnabled = false;
    cpuEEPROMEnabled = true;
    cpuEEPROMSensorEnabled = true;
    // EEPROM usage is automatically detected
    saveType = gbaSaveType = 3;
    break;
  case 5: // NONE
    cpuSramEnabled = false;
    cpuFlashEnabled = false;
    cpuEEPROMEnabled = false;
    cpuEEPROMSensorEnabled = false;
    // no save at all
    saveType = gbaSaveType = 5;
    break;
  } 

  ARM_PREFETCH;

  systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

  cpuDmaHack = false;

  lastTime = systemGetClock();

  SWITicks = 0;
}

void CPUInterrupt()
{
  u32 PC = reg[15].I;
  bool savedState = armState;
  CPUSwitchMode3(0x12, true, false);
  reg[14].I = PC;
  if(!savedState)
    reg[14].I += 2;
  reg[15].I = 0x18;
  armState = true;
  armIrqEnable = false;

  armNextPC = reg[15].I;
  reg[15].I += 4;
  ARM_PREFETCH;

  //  if(!holdState)
  biosProtected[0] = 0x02;
  biosProtected[1] = 0xc0;
  biosProtected[2] = 0x5e;
  biosProtected[3] = 0xe5;
}

#ifdef SDL
void log(const char *defaultMsg, ...)
{
  char buffer[2048];
  va_list valist;
  
  va_start(valist, defaultMsg);
  vsprintf(buffer, defaultMsg, valist);

  if(out == NULL) {
    out = fopen("trace.log","w");
  }

  fputs(buffer, out);
  
  va_end(valist);
}
#else
extern void winlog(const char *, ...);
#endif

void CPULoop(int ticks)
{  
  int clockTicks;
  int timerOverflow = 0;
  // variable used by the CPU core
  cpuTotalTicks = 0;
  cpuBreakLoop = false;
  cpuNextEvent = CPUUpdateTicks();
  if(cpuNextEvent > ticks)
    cpuNextEvent = ticks;


  for(;;) {
#ifndef FINAL_VERSION
    if(systemDebug) {
      if(systemDebug >= 10 && !holdState) {
        CPUUpdateCPSR();
#ifdef BKPT_SUPPORT
		if (debugger_last)
		{
		sprintf(buffer, "R00=%08x R01=%08x R02=%08x R03=%08x R04=%08x R05=%08x R06=%08x R07=%08x R08=%08x R09=%08x R10=%08x R11=%08x R12=%08x R13=%08x R14=%08x R15=%08x R16=%08x R17=%08x\n",
                 oldreg[0], oldreg[1], oldreg[2], oldreg[3], oldreg[4], oldreg[5],
                 oldreg[6], oldreg[7], oldreg[8], oldreg[9], oldreg[10], oldreg[11],
                 oldreg[12], oldreg[13], oldreg[14], oldreg[15], oldreg[16],
                 oldreg[17]);
		}
#endif
        sprintf(buffer, "R00=%08x R01=%08x R02=%08x R03=%08x R04=%08x R05=%08x R06=%08x R07=%08x R08=%08x R09=%08x R10=%08x R11=%08x R12=%08x R13=%08x R14=%08x R15=%08x R16=%08x R17=%08x\n",
                 reg[0].I, reg[1].I, reg[2].I, reg[3].I, reg[4].I, reg[5].I,
                 reg[6].I, reg[7].I, reg[8].I, reg[9].I, reg[10].I, reg[11].I,
                 reg[12].I, reg[13].I, reg[14].I, reg[15].I, reg[16].I,
                 reg[17].I);
#ifdef SDL
        log(buffer);
#else
        winlog(buffer);
#endif
      } else if(!holdState) {
        sprintf(buffer, "PC=%08x\n", armNextPC);
#ifdef SDL
        log(buffer);
#else
        winlog(buffer);
#endif
      }
    }
#endif /* FINAL_VERSION */

    if(!holdState && !SWITicks) {

      // Emulates the Cheat System (m) code
      if((cheatsEnabled) && (mastercode) && (mastercode == armNextPC))
      {
        u32 joy = 0;
        if(systemReadJoypads())
          joy = systemReadJoypad(-1);
        u32 ext = (joy >> 10);
        cpuTotalTicks += cheatsCheckKeys(P1^0x3FF, ext);
      }

      if ((armNextPC & 0x0803FFFF) == 0x08020000)
        busPrefetchCount=0x100;

      if(armState) {
#include "arm-new.h"
      } else {
#include "thumb.h"
      }
    } else
      clockTicks = CPUUpdateTicks();

    cpuTotalTicks += clockTicks;


    if(cpuTotalTicks >= cpuNextEvent) {
      int remainingTicks = cpuTotalTicks - cpuNextEvent;

      if (SWITicks)
      {
        SWITicks-=clockTicks;
        if (SWITicks<0)
          SWITicks = 0;
      }

      clockTicks = cpuNextEvent;
      cpuTotalTicks = 0;
      cpuDmaHack = false;
    
    updateLoop:

      if (IRQTicks)
      {
          IRQTicks -= clockTicks;
        if (IRQTicks<0)
          IRQTicks = 0;
      }

      lcdTicks -= clockTicks;

      
      if(lcdTicks <= 0) {
        if(DISPSTAT & 1) { // V-BLANK
          // if in V-Blank mode, keep computing...
          if(DISPSTAT & 2) {
            lcdTicks += 1008;
            VCOUNT++;
            UPDATE_REG(0x06, VCOUNT);
            DISPSTAT &= 0xFFFD;
            UPDATE_REG(0x04, DISPSTAT);
            CPUCompareVCOUNT();
          } else {
            lcdTicks += 224;
            DISPSTAT |= 2;
            UPDATE_REG(0x04, DISPSTAT);
            if(DISPSTAT & 16) {
              IF |= 2;
              UPDATE_REG(0x202, IF);
            }
          }
          
          if(VCOUNT >= 228) { //Reaching last line
            DISPSTAT &= 0xFFFC;
            UPDATE_REG(0x04, DISPSTAT);
            VCOUNT = 0;
            UPDATE_REG(0x06, VCOUNT);
            CPUCompareVCOUNT();
          }
        } else {
          int framesToSkip = systemFrameSkip;
          if(speedup)
            framesToSkip = 9; // try 6 FPS during speedup
          
          if(DISPSTAT & 2) {
            // if in H-Blank, leave it and move to drawing mode
            VCOUNT++;
            UPDATE_REG(0x06, VCOUNT);

            lcdTicks += 1008;
            DISPSTAT &= 0xFFFD;
            if(VCOUNT == 160) {
              count++;
              systemFrame();
              
              if((count % 10) == 0) {
                system10Frames(60);
              }
              if(count == 60) {
                u32 time = systemGetClock();
                if(time != lastTime) {
                  u32 t = 100000/(time - lastTime);
                  systemShowSpeed(t);
                } else
                  systemShowSpeed(0);
                lastTime = time;
                count = 0;
              }
              u32 joy = 0;
              // update joystick information
              if(systemReadJoypads())
                // read default joystick
                joy = systemReadJoypad(-1);
              P1 = 0x03FF ^ (joy & 0x3FF);
              if(cpuEEPROMSensorEnabled)
                systemUpdateMotionSensor();              
              UPDATE_REG(0x130, P1);
              u16 P1CNT = READ16LE(((u16 *)&ioMem[0x132]));
              // this seems wrong, but there are cases where the game
              // can enter the stop state without requesting an IRQ from
              // the joypad.
              if((P1CNT & 0x4000) || stopState) {
                u16 p1 = (0x3FF ^ P1) & 0x3FF;
                if(P1CNT & 0x8000) {
                  if(p1 == (P1CNT & 0x3FF)) {
                    IF |= 0x1000;
                    UPDATE_REG(0x202, IF);
                  }
                } else {
                  if(p1 & P1CNT) {
                    IF |= 0x1000;
                    UPDATE_REG(0x202, IF);
                  }
                }
              }
              
              u32 ext = (joy >> 10);
              // If no (m) code is enabled, apply the cheats at each LCDline
              if((cheatsEnabled) && (mastercode==0))
                remainingTicks += cheatsCheckKeys(P1^0x3FF, ext);
              speedup = (ext & 1) ? true : false;
              capture = (ext & 2) ? true : false;
              
              if(capture && !capturePrevious) {
                captureNumber++;
                systemScreenCapture(captureNumber);
              }
              capturePrevious = capture;

              DISPSTAT |= 1;
              DISPSTAT &= 0xFFFD;
              UPDATE_REG(0x04, DISPSTAT);
              if(DISPSTAT & 0x0008) {
                IF |= 1;
                UPDATE_REG(0x202, IF);
              }
              CPUCheckDMA(1, 0x0f);
              if(frameCount >= framesToSkip) {
                systemDrawScreen();
                frameCount = 0;
              } else 
                frameCount++;
              if(systemPauseOnFrame())
                ticks = 0;
            }
            
            UPDATE_REG(0x04, DISPSTAT);
            CPUCompareVCOUNT();

          } else {
            if(frameCount >= framesToSkip)
            {
              (*renderLine)();
              switch(systemColorDepth) {
                case 16:
                {
                  u16 *dest = (u16 *)pix + 242 * (VCOUNT+1);
                  for(ulong x = 0; x < 240;)
                    *dest++ = systemColorMap16[lineMix[x++]&0xFFFF];
                  // for filters that read past the screen
                  *dest++ = 0;
                }
                break;
                case 24:
                {
                  u8 *dest = (u8 *)pix + 240 * VCOUNT * 3;
                  for(ulong x = 0; x < 240;) {
                    *((u32 *)dest) = systemColorMap32[lineMix[x++] & 0xFFFF];
                    dest += 3;
                  }
                }
                break;
                case 32:
                {
                  u32 *dest = (u32 *)pix + 241 * (VCOUNT+1);
                  for(ulong x = 0; x < 240; )
                    *dest++ = systemColorMap32[lineMix[x++] & 0xFFFF];
                }
                break;
              }
            }
            // entering H-Blank
            DISPSTAT |= 2;
            UPDATE_REG(0x04, DISPSTAT);
            lcdTicks += 224;
            CPUCheckDMA(2, 0x0f);
            if(DISPSTAT & 16) {
              IF |= 2;
              UPDATE_REG(0x202, IF);
            }
          }
        }       
      }

      if(!stopState) {
        if(timer0On) {
          timer0Ticks -= clockTicks;
          if(timer0Ticks <= 0) {
            timer0Ticks += (0x10000 - timer0Reload) << timer0ClockReload;
            timerOverflow |= 1;
            soundTimerOverflow(0);
            if(TM0CNT & 0x40) {
              IF |= 0x08;
              UPDATE_REG(0x202, IF);
            }
          }
          TM0D = 0xFFFF - (timer0Ticks >> timer0ClockReload);
          UPDATE_REG(0x100, TM0D);            
        }
        
        if(timer1On) {
          if(TM1CNT & 4) {
            if(timerOverflow & 1) {
              TM1D++;
              if(TM1D == 0) {
                TM1D += timer1Reload;
                timerOverflow |= 2;
                soundTimerOverflow(1);
                if(TM1CNT & 0x40) {
                  IF |= 0x10;
                  UPDATE_REG(0x202, IF);
                }
              }
              UPDATE_REG(0x104, TM1D);
            }
          } else {
            timer1Ticks -= clockTicks;
            if(timer1Ticks <= 0) {
              timer1Ticks += (0x10000 - timer1Reload) << timer1ClockReload;
              timerOverflow |= 2;           
              soundTimerOverflow(1);
              if(TM1CNT & 0x40) {
                IF |= 0x10;
                UPDATE_REG(0x202, IF);
              }
            }
            TM1D = 0xFFFF - (timer1Ticks >> timer1ClockReload);
            UPDATE_REG(0x104, TM1D); 
          }
        }
        
        if(timer2On) {
          if(TM2CNT & 4) {
            if(timerOverflow & 2) {
              TM2D++;
              if(TM2D == 0) {
                TM2D += timer2Reload;
                timerOverflow |= 4;
                if(TM2CNT & 0x40) {
                  IF |= 0x20;
                  UPDATE_REG(0x202, IF);
                }
              }
              UPDATE_REG(0x108, TM2D);
            }
          } else {
            timer2Ticks -= clockTicks;
            if(timer2Ticks <= 0) {
              timer2Ticks += (0x10000 - timer2Reload) << timer2ClockReload;
              timerOverflow |= 4;           
              if(TM2CNT & 0x40) {
                IF |= 0x20;
                UPDATE_REG(0x202, IF);
              }
            }
            TM2D = 0xFFFF - (timer2Ticks >> timer2ClockReload);
            UPDATE_REG(0x108, TM2D); 
          }
        }
        
        if(timer3On) {
          if(TM3CNT & 4) {
            if(timerOverflow & 4) {
              TM3D++;
              if(TM3D == 0) {
                TM3D += timer3Reload;
                if(TM3CNT & 0x40) {
                  IF |= 0x40;
                  UPDATE_REG(0x202, IF);
                }
              }
              UPDATE_REG(0x10C, TM3D);
            }
          } else {
              timer3Ticks -= clockTicks;
            if(timer3Ticks <= 0) {
              timer3Ticks += (0x10000 - timer3Reload) << timer3ClockReload;         
              if(TM3CNT & 0x40) {
                IF |= 0x40;
                UPDATE_REG(0x202, IF);
              }
            }
            TM3D = 0xFFFF - (timer3Ticks >> timer3ClockReload);
            UPDATE_REG(0x10C, TM3D); 
          }
        }
      }

      timerOverflow = 0;

      // we shouldn't be doing sound in stop state, but we loose synchronization
      // if sound is disabled, so in stop state, soundTick will just produce
      // mute sound
      soundTicks -= clockTicks;
      if(soundTicks <= 0) {
        soundTick();
        soundTicks += SOUND_CLOCK_TICKS;
      }

#ifdef PROFILING
      profilingTicks -= clockTicks;
      if(profilingTicks <= 0) {
        profilingTicks += profilingTicksReload;
        if(profilSegment) {
	  profile_segment *seg = profilSegment;
	  do {
	    u16 *b = (u16 *)seg->sbuf;
	    int pc = ((reg[15].I - seg->s_lowpc) * seg->s_scale)/0x10000;
	    if(pc >= 0 && pc < seg->ssiz) {
            b[pc]++;
	      break;
          }

	    seg = seg->next;
	  } while(seg);
        }
      }
#endif

      ticks -= clockTicks;

      cpuNextEvent = CPUUpdateTicks();
      
      if(cpuDmaTicksToUpdate > 0) {
        if(cpuDmaTicksToUpdate > cpuNextEvent)
          clockTicks = cpuNextEvent;
        else
          clockTicks = cpuDmaTicksToUpdate;
        cpuDmaTicksToUpdate -= clockTicks;
        if(cpuDmaTicksToUpdate < 0)
          cpuDmaTicksToUpdate = 0;
        cpuDmaHack = true;
        goto updateLoop;
      }

      if(IF && (IME & 1) && armIrqEnable) {
        int res = IF & IE;
        if(stopState)
          res &= 0x3080;
        if(res) {
          if (intState)
          {
            if (!IRQTicks)
            {
              CPUInterrupt();
              intState = false;
              holdState = false;
              stopState = false;
              holdType = 0;
            }
          }
          else
          {
            if (!holdState)
            {
              intState = true;
              IRQTicks=7;
              if (cpuNextEvent> IRQTicks)
                cpuNextEvent = IRQTicks;
            }
            else
            {
              CPUInterrupt();
              holdState = false;
              stopState = false;
              holdType = 0;
            }
          }

          // Stops the SWI Ticks emulation if an IRQ is executed
          //(to avoid problems with nested IRQ/SWI)
          if (SWITicks)
            SWITicks = 0;
        }
      }

      if(remainingTicks > 0) {
        if(remainingTicks > cpuNextEvent)
          clockTicks = cpuNextEvent;
        else
          clockTicks = remainingTicks;
        remainingTicks -= clockTicks;
        if(remainingTicks < 0)
          remainingTicks = 0;
        goto updateLoop;
      }

      if (timerOnOffDelay)
          applyTimer();

      if(cpuNextEvent > ticks)
        cpuNextEvent = ticks;

      if(ticks <= 0 || cpuBreakLoop)
        break;

    }
  }
}



struct EmulatedSystem GBASystem = {
  // emuMain
  CPULoop,
  // emuReset
  CPUReset,
  // emuCleanUp
  CPUCleanUp,
  // emuReadBattery
  CPUReadBatteryFile,
  // emuWriteBattery
  CPUWriteBatteryFile,
  // emuReadState
  CPUReadState,
  // emuWriteState 
  CPUWriteState,
  // emuReadMemState
  CPUReadMemState,
  // emuWriteMemState
  CPUWriteMemState,
  // emuWritePNG
  CPUWritePNGFile,
  // emuWriteBMP
  CPUWriteBMPFile,
  // emuUpdateCPSR
  CPUUpdateCPSR,
  // emuHasDebugger
  true,
  // emuCount
#ifdef FINAL_VERSION
  250000
#else
  5000
#endif
};
