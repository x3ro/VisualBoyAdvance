// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2004 Forgotten and the VBA development team

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

#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "GBA.h"
#include "GBAinline.h"
#include "Cheats.h"
#include "Globals.h"
#include "NLS.h"
#include "Util.h"

/**
 * Gameshark code types:
 *
 * NNNNNNNN 001DC0DE - ID code for the game (game 4 character name) from ROM
 * DEADFACE XXXXXXXX - changes decryption seeds
 * 0AAAAAAA 000000YY - 8-bit constant write
 * 1AAAAAAA 0000YYYY - 16-bit constant write
 * 2AAAAAAA YYYYYYYY - 32-bit constant write
 * 3AAAAAAA YYYYYYYY - ??
 * 6AAAAAAA 0000YYYY - 16-bit ROM Patch (address >> 1)
 * 6AAAAAAA 1000YYYY - 16-bit ROM Patch ? (address >> 1)
 * 6AAAAAAA 2000YYYY - 16-bit ROM Patch ? (address >> 1)
 * 8A1AAAAA 000000YY - 8-bit button write
 * 8A2AAAAA 0000YYYY - 16-bit button write
 * 8A3AAAAA YYYYYYYY - 32-bit button write
 * 80F00000 0000YYYY - button slow motion
 * DAAAAAAA 0000YYYY - if address contains 16-bit value enable next code
 * FAAAAAAA 0000YYYY - Master code function
 *
 * CodeBreaker codes types:
 *
 * 0000AAAA 000Y - Game CRC (Y are flags: 8 - CRC, 2 - DI)
 * 1AAAAAAA YYYY - Master Code function (store address at ((YYYY << 0x16)
 *                 + 0x08000100))
 * 2AAAAAAA YYYY - 16-bit or
 * 3AAAAAAA YYYY - 8-bit constant write
 * 4AAAAAAA YYYY - Slide code
 * XXXXCCCC IIII   (C is count and I is address increment, X is value incr.)
 * 5AAAAAAA CCCC - Super code (Write bytes to address, 2*CCCC is count)
 * BBBBBBBB BBBB 
 * 6AAAAAAA YYYY - 16-bit and
 * 7AAAAAAA YYYY - if address contains 16-bit value enable next code
 * 8AAAAAAA YYYY - 16-bit constant write
 * 9AAAAAAA YYYY - change decryption (when first code only?)
 * AAAAAAAA YYYY - if address does not contain 16-bit value enable next code
 * BAAAAAAA YYYY - if 16-bit > YYYY
 * CAAAAAAA YYYY - if 16-bit < YYYY
 * D00000X0 YYYY - if button keys ... enable next code
 * EAAAAAAA YYYY - increase value stored in address
 * FAAAAAAA YYYY - if 16-bit AND YYYY != 0 then enable next code
 **/

#define UNKNOWN_CODE            -1
#define INT_8_BIT_WRITE         0
#define INT_16_BIT_WRITE        1
#define INT_32_BIT_WRITE        2
#define GSA_16_BIT_ROM_PATCH    3
#define GSA_8_BIT_GS_WRITE      4
#define GSA_16_BIT_GS_WRITE     5
#define GSA_32_BIT_GS_WRITE     6
#define CBA_IF_KEYS_PRESSED     7
#define CBA_IF_TRUE             8
#define CBA_SLIDE_CODE          9
#define CBA_IF_FALSE            10
#define CBA_AND                 11
#define GSA_8_BIT_GS_WRITE2     12
#define GSA_16_BIT_GS_WRITE2    13
#define GSA_32_BIT_GS_WRITE2    14
#define GSA_16_BIT_ROM_PATCH2C  15
#define GSA_8_BIT_SLIDE         16
#define GSA_16_BIT_SLIDE        17
#define GSA_32_BIT_SLIDE        18
#define GSA_8_BIT_IF_TRUE       19
#define GSA_32_BIT_IF_TRUE      20
#define GSA_8_BIT_IF_FALSE      21
#define GSA_32_BIT_IF_FALSE     22
#define GSA_8_BIT_FILL          23
#define GSA_16_BIT_FILL         24
#define GSA_8_BIT_IF_TRUE2      25
#define GSA_16_BIT_IF_TRUE2     26
#define GSA_32_BIT_IF_TRUE2     27
#define GSA_8_BIT_IF_FALSE2     28
#define GSA_16_BIT_IF_FALSE2    29
#define GSA_32_BIT_IF_FALSE2    30
#define GSA_SLOWDOWN            31
#define CBA_ADD                 32
#define CBA_OR                  33
#define CBA_LT                  34
#define CBA_GT                  35 
#define CBA_SUPER               36
#define GSA_8_BIT_POINTER       37
#define GSA_16_BIT_POINTER      38
#define GSA_32_BIT_POINTER      39
#define GSA_8_BIT_ADD           40
#define GSA_16_BIT_ADD          41
#define GSA_32_BIT_ADD          42
#define GSA_8_BIT_IF_LOWER_U    43
#define GSA_16_BIT_IF_LOWER_U   44
#define GSA_32_BIT_IF_LOWER_U   45
#define GSA_8_BIT_IF_HIGHER_U   46
#define GSA_16_BIT_IF_HIGHER_U  47
#define GSA_32_BIT_IF_HIGHER_U  48
#define GSA_8_BIT_IF_AND        49
#define GSA_16_BIT_IF_AND       50
#define GSA_32_BIT_IF_AND       51
#define GSA_8_BIT_IF_LOWER_U2   52
#define GSA_16_BIT_IF_LOWER_U2  53
#define GSA_32_BIT_IF_LOWER_U2  54
#define GSA_8_BIT_IF_HIGHER_U2  55
#define GSA_16_BIT_IF_HIGHER_U2 56
#define GSA_32_BIT_IF_HIGHER_U2 57
#define GSA_8_BIT_IF_AND2       58
#define GSA_16_BIT_IF_AND2      59
#define GSA_32_BIT_IF_AND2      60
#define GSA_ALWAYS              61
#define GSA_ALWAYS2             62
#define GSA_8_BIT_IF_LOWER_S    63
#define GSA_16_BIT_IF_LOWER_S   64
#define GSA_32_BIT_IF_LOWER_S   65
#define GSA_8_BIT_IF_HIGHER_S   66
#define GSA_16_BIT_IF_HIGHER_S  67
#define GSA_32_BIT_IF_HIGHER_S  68
#define GSA_8_BIT_IF_LOWER_S2   69
#define GSA_16_BIT_IF_LOWER_S2  70
#define GSA_32_BIT_IF_LOWER_S2  71
#define GSA_8_BIT_IF_HIGHER_S2  72
#define GSA_16_BIT_IF_HIGHER_S2 73
#define GSA_32_BIT_IF_HIGHER_S2 74
#define GSA_16_BIT_WRITE_IOREGS 75
#define GSA_32_BIT_WRITE_IOREGS 76
#define GSA_CODES_ON            77
#define GSA_8_BIT_IF_TRUE3      78
#define GSA_16_BIT_IF_TRUE3     79
#define GSA_32_BIT_IF_TRUE3     80
#define GSA_8_BIT_IF_FALSE3     81
#define GSA_16_BIT_IF_FALSE3    82
#define GSA_32_BIT_IF_FALSE3    83
#define GSA_8_BIT_IF_LOWER_S3   84
#define GSA_16_BIT_IF_LOWER_S3  85
#define GSA_32_BIT_IF_LOWER_S3  86
#define GSA_8_BIT_IF_HIGHER_S3  87
#define GSA_16_BIT_IF_HIGHER_S3 88
#define GSA_32_BIT_IF_HIGHER_S3 89
#define GSA_8_BIT_IF_LOWER_U3   90
#define GSA_16_BIT_IF_LOWER_U3  91
#define GSA_32_BIT_IF_LOWER_U3  92
#define GSA_8_BIT_IF_HIGHER_U3  93
#define GSA_16_BIT_IF_HIGHER_U3 94
#define GSA_32_BIT_IF_HIGHER_U3 95
#define GSA_8_BIT_IF_AND3       96
#define GSA_16_BIT_IF_AND3      97
#define GSA_32_BIT_IF_AND3      98
#define GSA_ALWAYS3             99
#define GSA_16_BIT_ROM_PATCH2D  100
#define GSA_16_BIT_ROM_PATCH2E  101
#define GSA_16_BIT_ROM_PATCH2F  102



CheatsData cheatsList[100];
int cheatsNumber = 0;
u32 rompatch2addr [4];
u16 rompatch2val [4];
u16 rompatch2oldval [4];

u8 cheatsCBASeedBuffer[0x30];
u32 cheatsCBASeed[4];
u32 cheatsCBATemporaryValue = 0;
u16 cheatsCBATable[256];
bool cheatsCBATableGenerated = false;
u16 super = 0;

u8 cheatsCBACurrentSeed[12] = {
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

#define CHEAT_IS_HEX(a) ( ((a)>='A' && (a) <='F') || ((a) >='0' && (a) <= '9'))

#define CHEAT_PATCH_ROM_16BIT(a,v) \
  WRITE16LE(((u16 *)&rom[(a) & 0x1ffffff]), v); 

static bool isMultilineWithData(int i)
{
  // we consider it a multiline code if it has more than one line of data
  // otherwise, it can still be considered a single code
  if(i < cheatsNumber && i >= 0)
    switch(cheatsList[i].size) {
    case INT_8_BIT_WRITE:
    case INT_16_BIT_WRITE:
    case INT_32_BIT_WRITE:
    case GSA_16_BIT_ROM_PATCH:
    case GSA_8_BIT_GS_WRITE:
    case GSA_16_BIT_GS_WRITE:
    case GSA_32_BIT_GS_WRITE:
    case CBA_AND:
    case CBA_IF_KEYS_PRESSED:
    case CBA_IF_TRUE:
    case CBA_IF_FALSE:
    case GSA_8_BIT_IF_TRUE:
    case GSA_32_BIT_IF_TRUE:
    case GSA_8_BIT_IF_FALSE:
    case GSA_32_BIT_IF_FALSE:
    case GSA_8_BIT_FILL:
    case GSA_16_BIT_FILL:
    case GSA_8_BIT_IF_TRUE2:
    case GSA_16_BIT_IF_TRUE2:
    case GSA_32_BIT_IF_TRUE2:
    case GSA_8_BIT_IF_FALSE2:
    case GSA_16_BIT_IF_FALSE2:
    case GSA_32_BIT_IF_FALSE2:
    case GSA_SLOWDOWN:
    case CBA_ADD:
    case CBA_OR:
    case CBA_LT:
    case CBA_GT:
    case GSA_8_BIT_POINTER:
    case GSA_16_BIT_POINTER:
    case GSA_32_BIT_POINTER:
    case GSA_8_BIT_ADD:
    case GSA_16_BIT_ADD:
    case GSA_32_BIT_ADD:
    case GSA_8_BIT_IF_LOWER_U:
    case GSA_16_BIT_IF_LOWER_U:
    case GSA_32_BIT_IF_LOWER_U:
    case GSA_8_BIT_IF_HIGHER_U:
    case GSA_16_BIT_IF_HIGHER_U:
    case GSA_32_BIT_IF_HIGHER_U:
    case GSA_8_BIT_IF_AND:
    case GSA_16_BIT_IF_AND:
    case GSA_32_BIT_IF_AND:
    case GSA_8_BIT_IF_LOWER_U2:
    case GSA_16_BIT_IF_LOWER_U2:
    case GSA_32_BIT_IF_LOWER_U2:
    case GSA_8_BIT_IF_HIGHER_U2:
    case GSA_16_BIT_IF_HIGHER_U2:
    case GSA_32_BIT_IF_HIGHER_U2:
    case GSA_8_BIT_IF_AND2:
    case GSA_16_BIT_IF_AND2:
    case GSA_32_BIT_IF_AND2:
    case GSA_ALWAYS:
    case GSA_ALWAYS2:
    case GSA_8_BIT_IF_LOWER_S:
    case GSA_16_BIT_IF_LOWER_S:
    case GSA_32_BIT_IF_LOWER_S:
    case GSA_8_BIT_IF_HIGHER_S:
    case GSA_16_BIT_IF_HIGHER_S:
    case GSA_32_BIT_IF_HIGHER_S:
    case GSA_8_BIT_IF_LOWER_S2:
    case GSA_16_BIT_IF_LOWER_S2:
    case GSA_32_BIT_IF_LOWER_S2:
    case GSA_8_BIT_IF_HIGHER_S2:
    case GSA_16_BIT_IF_HIGHER_S2:
    case GSA_32_BIT_IF_HIGHER_S2:
    case GSA_16_BIT_WRITE_IOREGS:
    case GSA_32_BIT_WRITE_IOREGS:
    case GSA_CODES_ON:
    case GSA_8_BIT_IF_TRUE3:
    case GSA_16_BIT_IF_TRUE3:
    case GSA_32_BIT_IF_TRUE3:
    case GSA_8_BIT_IF_FALSE3:
    case GSA_16_BIT_IF_FALSE3:
    case GSA_32_BIT_IF_FALSE3:
    case GSA_8_BIT_IF_LOWER_S3:
    case GSA_16_BIT_IF_LOWER_S3:
    case GSA_32_BIT_IF_LOWER_S3:
    case GSA_8_BIT_IF_HIGHER_S3:
    case GSA_16_BIT_IF_HIGHER_S3:
    case GSA_32_BIT_IF_HIGHER_S3:
    case GSA_8_BIT_IF_LOWER_U3:
    case GSA_16_BIT_IF_LOWER_U3:
    case GSA_32_BIT_IF_LOWER_U3:
    case GSA_8_BIT_IF_HIGHER_U3:
    case GSA_16_BIT_IF_HIGHER_U3:
    case GSA_32_BIT_IF_HIGHER_U3:
    case GSA_8_BIT_IF_AND3:
    case GSA_16_BIT_IF_AND3:
    case GSA_32_BIT_IF_AND3:
    case GSA_ALWAYS3:
      return false;
      // the codes below have two lines of data
    case CBA_SLIDE_CODE:
    case GSA_8_BIT_GS_WRITE2:
    case GSA_16_BIT_GS_WRITE2:
    case GSA_32_BIT_GS_WRITE2:
    case GSA_16_BIT_ROM_PATCH2C:
    case GSA_16_BIT_ROM_PATCH2D:
    case GSA_16_BIT_ROM_PATCH2E:
    case GSA_16_BIT_ROM_PATCH2F:
    case GSA_8_BIT_SLIDE:
    case GSA_16_BIT_SLIDE:
    case GSA_32_BIT_SLIDE:
    case CBA_SUPER:
      return true;
    }
  return false;
}

static int getCodeLength(int num)
{
  if(num >= cheatsNumber || num < 0)
    return 1;

  // this is for all the codes that are true multiline
  switch(cheatsList[num].size) {
  case INT_8_BIT_WRITE:
  case INT_16_BIT_WRITE:
  case INT_32_BIT_WRITE:
  case GSA_16_BIT_ROM_PATCH:
  case GSA_8_BIT_GS_WRITE:
  case GSA_16_BIT_GS_WRITE:
  case GSA_32_BIT_GS_WRITE:
  case CBA_AND:
  case GSA_8_BIT_FILL:
  case GSA_16_BIT_FILL:
  case GSA_SLOWDOWN:
  case CBA_ADD:
  case CBA_OR:
  case GSA_8_BIT_POINTER:
  case GSA_16_BIT_POINTER:
  case GSA_32_BIT_POINTER:
  case GSA_8_BIT_ADD:
  case GSA_16_BIT_ADD:
  case GSA_32_BIT_ADD:
  case GSA_CODES_ON:
  case GSA_8_BIT_IF_TRUE3:
  case GSA_16_BIT_IF_TRUE3:
  case GSA_32_BIT_IF_TRUE3:
  case GSA_8_BIT_IF_FALSE3:
  case GSA_16_BIT_IF_FALSE3:
  case GSA_32_BIT_IF_FALSE3:
  case GSA_8_BIT_IF_LOWER_S3:
  case GSA_16_BIT_IF_LOWER_S3:
  case GSA_32_BIT_IF_LOWER_S3:
  case GSA_8_BIT_IF_HIGHER_S3:
  case GSA_16_BIT_IF_HIGHER_S3:
  case GSA_32_BIT_IF_HIGHER_S3:
  case GSA_8_BIT_IF_LOWER_U3:
  case GSA_16_BIT_IF_LOWER_U3:
  case GSA_32_BIT_IF_LOWER_U3:
  case GSA_8_BIT_IF_HIGHER_U3:
  case GSA_16_BIT_IF_HIGHER_U3:
  case GSA_32_BIT_IF_HIGHER_U3:
  case GSA_8_BIT_IF_AND3:
  case GSA_16_BIT_IF_AND3:
  case GSA_32_BIT_IF_AND3:
  case GSA_ALWAYS3:
    return 1;
  case CBA_IF_KEYS_PRESSED:
  case CBA_IF_TRUE:
  case CBA_IF_FALSE:
  case CBA_SLIDE_CODE:
  case GSA_8_BIT_GS_WRITE2:
  case GSA_16_BIT_GS_WRITE2:
  case GSA_32_BIT_GS_WRITE2:
  case GSA_16_BIT_ROM_PATCH2C:
  case GSA_16_BIT_ROM_PATCH2D:
  case GSA_16_BIT_ROM_PATCH2E:
  case GSA_16_BIT_ROM_PATCH2F:
  case GSA_8_BIT_SLIDE:
  case GSA_16_BIT_SLIDE:
  case GSA_32_BIT_SLIDE:
  case GSA_8_BIT_IF_TRUE:
  case GSA_32_BIT_IF_TRUE:
  case GSA_8_BIT_IF_FALSE:
  case GSA_32_BIT_IF_FALSE:
  case CBA_LT:
  case CBA_GT:
  case GSA_8_BIT_IF_LOWER_U:
  case GSA_16_BIT_IF_LOWER_U:
  case GSA_32_BIT_IF_LOWER_U:
  case GSA_8_BIT_IF_HIGHER_U:
  case GSA_16_BIT_IF_HIGHER_U:
  case GSA_32_BIT_IF_HIGHER_U:
  case GSA_8_BIT_IF_AND:
  case GSA_16_BIT_IF_AND:
  case GSA_32_BIT_IF_AND:
  case GSA_ALWAYS:
  case GSA_8_BIT_IF_LOWER_S:
  case GSA_16_BIT_IF_LOWER_S:
  case GSA_32_BIT_IF_LOWER_S:
  case GSA_8_BIT_IF_HIGHER_S:
  case GSA_16_BIT_IF_HIGHER_S:
  case GSA_32_BIT_IF_HIGHER_S:
  case GSA_16_BIT_WRITE_IOREGS:
  case GSA_32_BIT_WRITE_IOREGS:
    return 2;
  case GSA_8_BIT_IF_TRUE2:
  case GSA_16_BIT_IF_TRUE2:
  case GSA_32_BIT_IF_TRUE2:
  case GSA_8_BIT_IF_FALSE2:
  case GSA_16_BIT_IF_FALSE2:
  case GSA_32_BIT_IF_FALSE2:
  case GSA_8_BIT_IF_LOWER_U2:
  case GSA_16_BIT_IF_LOWER_U2:
  case GSA_32_BIT_IF_LOWER_U2:
  case GSA_8_BIT_IF_HIGHER_U2:
  case GSA_16_BIT_IF_HIGHER_U2:
  case GSA_32_BIT_IF_HIGHER_U2:
  case GSA_8_BIT_IF_AND2:
  case GSA_16_BIT_IF_AND2:
  case GSA_32_BIT_IF_AND2:
  case GSA_ALWAYS2:
  case GSA_8_BIT_IF_LOWER_S2:
  case GSA_16_BIT_IF_LOWER_S2:
  case GSA_32_BIT_IF_LOWER_S2:
  case GSA_8_BIT_IF_HIGHER_S2:
  case GSA_16_BIT_IF_HIGHER_S2:
  case GSA_32_BIT_IF_HIGHER_S2:
    return 3;
  case CBA_SUPER:
    return ((((cheatsList[num].value-1) & 0xFFFF)/3) + 1);
  }
  return 1;
}

int cheatsCheckKeys(u32 keys, u32 extended)
{
  bool onoff = true;
  int ticks = 0;
  int i;
  for (i = 0; i<4; i++)
    if (rompatch2addr [i] != 0) {
      CHEAT_PATCH_ROM_16BIT(rompatch2addr [i],rompatch2oldval [i]);
      rompatch2addr [i] = 0;
    }

  for (i = 0; i < cheatsNumber; i++) {
    if(!cheatsList[i].enabled) {
      // make sure we skip other lines in this code
      i += getCodeLength(i)-1;
      continue;
    }
    switch(cheatsList[i].size) {
    case GSA_CODES_ON:
      onoff = true;
      break;
    case GSA_SLOWDOWN:
      // check if button was pressed and released, if so toggle our state
      if((cheatsList[i].status & 4) && !(extended & 4))
        cheatsList[i].status ^= 1;
      if(extended & 4)
        cheatsList[i].status |= 4;
      else
        cheatsList[i].status &= ~4;
      
      if(cheatsList[i].status & 1)
        ticks += 2*256*((cheatsList[i].value >> 8) & 255);
      break;
    case GSA_8_BIT_SLIDE:
      i++;
      if(i < cheatsNumber) {
        u32 addr = cheatsList[i-1].value;
        u8 value = cheatsList[i].address;
        int vinc = (cheatsList[i].value >> 24) & 255;
        int count = (cheatsList[i].value >> 16) & 255;
        int ainc = (cheatsList[i].value & 0xffff);
        while(count > 0) {
          CPUWriteByte(addr, value);
          value += vinc;
          addr += ainc;
          count--;
        }
      }
      break;
    case GSA_16_BIT_SLIDE:
      i++;
      if(i < cheatsNumber) {
        u32 addr = cheatsList[i-1].value;
        u16 value = cheatsList[i].address;
        int vinc = (cheatsList[i].value >> 24) & 255;
        int count = (cheatsList[i].value >> 16) & 255;
        int ainc = (cheatsList[i].value & 0xffff)*2;
        while(count > 0) {
          CPUWriteHalfWord(addr, value);
          value += vinc;
          addr += ainc;
          count--;
        }
      }
      break;
    case GSA_32_BIT_SLIDE:
      i++;
      if(i < cheatsNumber) {
        u32 addr = cheatsList[i-1].value;
        u32 value = cheatsList[i].address;
        int vinc = (cheatsList[i].value >> 24) & 255;
        int count = (cheatsList[i].value >> 16) & 255;
        int ainc = (cheatsList[i].value & 0xffff)*4;
        while(count > 0) {
          CPUWriteMemory(addr, value);
          value += vinc;
          addr += ainc;
          count--;
        }
      }
      break;
    case GSA_8_BIT_GS_WRITE2:
      i++;
      if(i < cheatsNumber) {
        if(extended & 4) {
          CPUWriteByte(cheatsList[i-1].value, cheatsList[i].address);
        }
      }
      break;
    case GSA_16_BIT_GS_WRITE2:
      i++;
      if(i < cheatsNumber) {
        if(extended & 4) {
          CPUWriteHalfWord(cheatsList[i-1].value, cheatsList[i].address);
        }
      }
      break;
    case GSA_32_BIT_GS_WRITE2:
      i++;
      if(i < cheatsNumber) {
        if(extended & 4) {
          CPUWriteMemory(cheatsList[i-1].value, cheatsList[i].address);
        }
      }
      break;
    case GSA_16_BIT_ROM_PATCH2C:
      i++;
      if((i < cheatsNumber) && (cheatsList[i].status & 1) == 0) {
		  rompatch2addr [0] = ((cheatsList[i-1].value & 0x00FFFFFF) << 1) + 0x8000000;
		  rompatch2oldval [0] = CPUReadHalfWord(rompatch2addr [0]);
		  rompatch2val [0] = cheatsList[i].address & 0xFFFF;
      }
      break;
    case GSA_16_BIT_ROM_PATCH2D:
      i++;
      if((i < cheatsNumber) && (cheatsList[i].status & 1) == 0) {
		  rompatch2addr [1] = ((cheatsList[i-1].value & 0x00FFFFFF) << 1) + 0x8000000;
		  rompatch2oldval [1] = CPUReadHalfWord(rompatch2addr [1]);
		  rompatch2val [1] = cheatsList[i].address & 0xFFFF;
      }
      break;
    case GSA_16_BIT_ROM_PATCH2E:
      i++;
      if((i < cheatsNumber) && (cheatsList[i].status & 1) == 0) {
		  rompatch2addr [2] = ((cheatsList[i-1].value & 0x00FFFFFF) << 1) + 0x8000000;
		  rompatch2oldval [2] = CPUReadHalfWord(rompatch2addr [2]);
		  rompatch2val [2] = cheatsList[i].address & 0xFFFF;
      }
      break;
    case GSA_16_BIT_ROM_PATCH2F:
      i++;
      if((i < cheatsNumber) && (cheatsList[i].status & 1) == 0) {
		  rompatch2addr [3] = ((cheatsList[i-1].value & 0x00FFFFFF) << 1) + 0x8000000;
		  rompatch2oldval [3] = CPUReadHalfWord(rompatch2addr [3]);
		  rompatch2val [3] = cheatsList[i].address & 0xFFFF;
      }
      break;
    }
    if (onoff) {
      switch(cheatsList[i].size) {
      case INT_8_BIT_WRITE:
        CPUWriteByte(cheatsList[i].address, cheatsList[i].value);
        break;
      case INT_16_BIT_WRITE:
        CPUWriteHalfWord(cheatsList[i].address, cheatsList[i].value);
        break;
      case INT_32_BIT_WRITE:
        CPUWriteMemory(cheatsList[i].address, cheatsList[i].value);
        break;
      case GSA_16_BIT_ROM_PATCH:
        if((cheatsList[i].status & 1) == 0) {
          if(CPUReadHalfWord(cheatsList[i].address) != cheatsList[i].value) {
            cheatsList[i].oldValue = CPUReadHalfWord(cheatsList[i].address);
            cheatsList[i].status |= 1;
            CHEAT_PATCH_ROM_16BIT(cheatsList[i].address, cheatsList[i].value);
          }
        }
        break;
      case GSA_8_BIT_GS_WRITE:
        if(extended & 4) {
          CPUWriteByte(cheatsList[i].address, cheatsList[i].value);
        }
        break;
      case GSA_16_BIT_GS_WRITE:
        if(extended & 4) {
          CPUWriteHalfWord(cheatsList[i].address, cheatsList[i].value);   
        }
        break;
      case GSA_32_BIT_GS_WRITE:
        if(extended & 4) {
          CPUWriteMemory(cheatsList[i].address, cheatsList[i].value);     
        }
        break;
      case CBA_IF_KEYS_PRESSED:
        {
          u16 value = cheatsList[i].value;
          u32 addr = cheatsList[i].address;
          if((addr & 0xF0) == 0x20) {
            if((keys & value) == 0) {
              i++;
			}
		  } else if((addr & 0xF0) == 0x10) {
            if((keys & value) == value) {
              i++;
			}
		  } else if((addr & 0xF0) == 0x00) {
            if(((~keys) & 0x3FF) == value) {
              i++;
			}
		  }
		}
        break;
      case CBA_IF_TRUE:
        if(CPUReadHalfWord(cheatsList[i].address) != cheatsList[i].value) {
          i++;
        }
        break;
      case CBA_SLIDE_CODE:
		{
          u32 address = cheatsList[i].address;
          u16 value = cheatsList[i].value;
          i++;
          if(i < cheatsNumber) {
            int count = ((cheatsList[i].address - 1) & 0xFFFF);
            u16 vinc = (cheatsList[i].address >> 16) & 0xFFFF;
            int inc = cheatsList[i].value;
            for(int x = 0; x <= count ; x++) {
              CPUWriteHalfWord(address, value);
              address += inc;
              value += vinc;
			}
		  }
		}
        break;
      case CBA_IF_FALSE:
        if(CPUReadHalfWord(cheatsList[i].address) == cheatsList[i].value){
          i++;
        }
      break;
      case CBA_AND:
        CPUWriteHalfWord(cheatsList[i].address,
                         CPUReadHalfWord(cheatsList[i].address) &
                         cheatsList[i].value);
        break;
      case GSA_8_BIT_IF_TRUE:
        if(CPUReadByte(cheatsList[i].address) != cheatsList[i].value) {
          i++;
        }
        break;
      case GSA_32_BIT_IF_TRUE:
        if(CPUReadMemory(cheatsList[i].address) != cheatsList[i].value) {
          i++;
        }
        break;
      case GSA_8_BIT_IF_FALSE:
        if(CPUReadByte(cheatsList[i].address) == cheatsList[i].value) {
          i++;
        }
        break;
      case GSA_32_BIT_IF_FALSE:
        if(CPUReadMemory(cheatsList[i].address) == cheatsList[i].value) {
          i++;
        }
        break;
      case GSA_8_BIT_FILL:
		{
          u32 addr = cheatsList[i].address;
          u8 v = cheatsList[i].value & 0xff;
          u32 end = addr + (cheatsList[i].value >> 8);
          do {
            CPUWriteByte(addr, v);
            addr++;
		  } while (addr <= end);
		}
        break;
      case GSA_16_BIT_FILL:
		{
          u32 addr = cheatsList[i].address;
          u16 v = cheatsList[i].value & 0xffff;
          u32 end = addr + ((cheatsList[i].value >> 16) << 1);
          do {
            CPUWriteHalfWord(addr, v);
            addr+=2;
		  } while (addr <= end);
		}
        break;
      case GSA_8_BIT_IF_TRUE2:
        if(CPUReadByte(cheatsList[i].address) != cheatsList[i].value) {
          i+=2;
        }
        break;
      case GSA_16_BIT_IF_TRUE2:
        if(CPUReadHalfWord(cheatsList[i].address) != cheatsList[i].value) {
          i+=2;
        }
        break;
      case GSA_32_BIT_IF_TRUE2:
        if(CPUReadMemory(cheatsList[i].address) != cheatsList[i].value) {
          i+=2;
        }
        break;
      case GSA_8_BIT_IF_FALSE2:
        if(CPUReadByte(cheatsList[i].address) == cheatsList[i].value) {
          i+=2;
        }
        break;
      case GSA_16_BIT_IF_FALSE2:
        if(CPUReadHalfWord(cheatsList[i].address) == cheatsList[i].value) {
          i+=2;
        }
        break;
      case GSA_32_BIT_IF_FALSE2:
        if(CPUReadMemory(cheatsList[i].address) == cheatsList[i].value) {
          i+=2;
        }
        break;
      case CBA_ADD:
        if ((cheatsList[i].address & 1) == 0) { 
          CPUWriteHalfWord(cheatsList[i].address, 
                           CPUReadHalfWord(cheatsList[i].address) +
                           cheatsList[i].value);
        } else {
          CPUWriteMemory(cheatsList[i].address & 0x0FFFFFFE, 
                           CPUReadMemory(cheatsList[i].address & 0x0FFFFFFE) +
                           cheatsList[i].value);
        }
        break;
      case CBA_OR:
        CPUWriteHalfWord(cheatsList[i].address,
                         CPUReadHalfWord(cheatsList[i].address) |
                         cheatsList[i].value);
        break;
      case CBA_GT:
        if (!(CPUReadHalfWord(cheatsList[i].address) > cheatsList[i].value)){
          i++;
        }
        break;
      case CBA_LT:
        if (!(CPUReadHalfWord(cheatsList[i].address) < cheatsList[i].value)){
          i++;
        }
        break;
      case CBA_SUPER:
		{
          int count = 2*((cheatsList[i].value -1) & 0xFFFF);
          u32 address = cheatsList[i].address;
          for(int x = 0; x <= count; x++) {
            u8 b;
            int res = x % 6;
		    if (res==0)
		 	  i++;
            if(res < 4)
              b = (cheatsList[i].address >> (24-8*res)) & 0xFF;
            else
              b = (cheatsList[i].value >> (8 - 8*(res-4))) & 0xFF;
            CPUWriteByte(address, b);
            address++;
		  }
		}
        break;
      case GSA_8_BIT_POINTER :
        if ((CPUReadMemory(cheatsList[i].address)>=0x02000000) && (CPUReadMemory(cheatsList[i].address)<0x02040000) ||
            (CPUReadMemory(cheatsList[i].address)>=0x03000000) && (CPUReadMemory(cheatsList[i].address)<0x03008000))
        {
          CPUWriteByte(CPUReadMemory(cheatsList[i].address)+((cheatsList[i].value & 0xFFFFFF00) >> 8),
                       cheatsList[i].value & 0xFF);
        }
        break;
      case GSA_16_BIT_POINTER :
        if ((CPUReadMemory(cheatsList[i].address)>=0x02000000) && (CPUReadMemory(cheatsList[i].address)<0x02040000) ||
            (CPUReadMemory(cheatsList[i].address)>=0x03000000) && (CPUReadMemory(cheatsList[i].address)<0x03008000))
        {
          CPUWriteHalfWord(CPUReadMemory(cheatsList[i].address)+((cheatsList[i].value & 0xFFFF0000) >> 15),
                       cheatsList[i].value & 0xFFFF);
        }
        break;
      case GSA_32_BIT_POINTER :
        if ((CPUReadMemory(cheatsList[i].address)>=0x02000000) && (CPUReadMemory(cheatsList[i].address)<0x02040000) ||
            (CPUReadMemory(cheatsList[i].address)>=0x03000000) && (CPUReadMemory(cheatsList[i].address)<0x03008000))
        {
          CPUWriteMemory(CPUReadMemory(cheatsList[i].address),
                       cheatsList[i].value);
        }
        break;
      case GSA_8_BIT_ADD :
        CPUWriteByte(cheatsList[i].address,
                    (cheatsList[i].value & 0xFF) + CPUReadMemory(cheatsList[i].address) & 0xFF);
        break;
      case GSA_16_BIT_ADD :
        CPUWriteHalfWord(cheatsList[i].address,
                        (cheatsList[i].value & 0xFFFF) + CPUReadMemory(cheatsList[i].address) & 0xFFFF);
        break;
      case GSA_32_BIT_ADD :
        CPUWriteMemory(cheatsList[i].address ,
                       cheatsList[i].value + CPUReadMemory(cheatsList[i].address) & 0xFFFFFFFF);
        break;
      case GSA_8_BIT_IF_LOWER_U:
        if (!(CPUReadByte(cheatsList[i].address) < (cheatsList[i].value & 0xFF))) {
          i++;
        }
        break;
      case GSA_16_BIT_IF_LOWER_U:
        if (!(CPUReadHalfWord(cheatsList[i].address) < (cheatsList[i].value & 0xFFFF))) {
          i++;
        }
        break;
      case GSA_32_BIT_IF_LOWER_U:
        if (!(CPUReadMemory(cheatsList[i].address) < cheatsList[i].value)) {
          i++;
        }
        break;
      case GSA_8_BIT_IF_HIGHER_U:
        if (!(CPUReadByte(cheatsList[i].address) > (cheatsList[i].value & 0xFF))) {
          i++;
        }
        break;
      case GSA_16_BIT_IF_HIGHER_U:
        if (!(CPUReadHalfWord(cheatsList[i].address) > (cheatsList[i].value & 0xFFFF))) {
          i++;
        }
        break;
      case GSA_32_BIT_IF_HIGHER_U:
        if (!(CPUReadMemory(cheatsList[i].address) > cheatsList[i].value)) {
          i++;
        }
        break;
      case GSA_8_BIT_IF_AND:
        if (!(CPUReadByte(cheatsList[i].address) & (cheatsList[i].value & 0xFF))) {
          i++;
        }
        break;
      case GSA_16_BIT_IF_AND:
        if (!(CPUReadHalfWord(cheatsList[i].address) & (cheatsList[i].value & 0xFFFF))) {
          i++;
        }
        break;
      case GSA_32_BIT_IF_AND:
        if (!(CPUReadMemory(cheatsList[i].address) & cheatsList[i].value)) {
          i++;
        }
        break;
      case GSA_8_BIT_IF_LOWER_U2:
        if (!(CPUReadByte(cheatsList[i].address) < (cheatsList[i].value & 0xFF))) {
          i+=2;
        }
        break;
      case GSA_16_BIT_IF_LOWER_U2:
        if (!(CPUReadHalfWord(cheatsList[i].address) < (cheatsList[i].value & 0xFFFF))) {
          i+=2;
        }
        break;
      case GSA_32_BIT_IF_LOWER_U2:
        if (!(CPUReadMemory(cheatsList[i].address) < cheatsList[i].value)) {
          i+=2;
        }
        break;
      case GSA_8_BIT_IF_HIGHER_U2:
        if (!(CPUReadByte(cheatsList[i].address) > (cheatsList[i].value & 0xFF))) {
          i+=2;
        }
        break;
      case GSA_16_BIT_IF_HIGHER_U2:
        if (!(CPUReadHalfWord(cheatsList[i].address) > (cheatsList[i].value & 0xFFFF))) {
          i+=2;
        }
        break;
      case GSA_32_BIT_IF_HIGHER_U2:
        if (!(CPUReadMemory(cheatsList[i].address) > cheatsList[i].value)) {
          i+=2;
        }
        break;
      case GSA_8_BIT_IF_AND2:
        if (!(CPUReadByte(cheatsList[i].address) & (cheatsList[i].value & 0xFF))) {
          i+=2;
        }
        break;
      case GSA_16_BIT_IF_AND2:
        if (!(CPUReadHalfWord(cheatsList[i].address) & (cheatsList[i].value & 0xFFFF))) {
          i+=2;
        }
        break;
      case GSA_32_BIT_IF_AND2:
        if (!(CPUReadMemory(cheatsList[i].address) & cheatsList[i].value)) {
          i+=2;
        }
        break;
      case GSA_ALWAYS:
        i++;
        break;
      case GSA_ALWAYS2:
        i+=2;
        break;
      case GSA_8_BIT_IF_LOWER_S:
        if (!((s8)CPUReadByte(cheatsList[i].address) < ((s8)cheatsList[i].value & 0xFF))) {
          i++;
        }
        break;
      case GSA_16_BIT_IF_LOWER_S:
        if (!((s16)CPUReadHalfWord(cheatsList[i].address) < ((s16)cheatsList[i].value & 0xFFFF))) {
          i++;
        }
        break;
      case GSA_32_BIT_IF_LOWER_S:
        if (!((s32)CPUReadMemory(cheatsList[i].address) < (s32)cheatsList[i].value)) {
          i++;
        }
        break;
      case GSA_8_BIT_IF_HIGHER_S:
        if (!((s8)CPUReadByte(cheatsList[i].address) > ((s8)cheatsList[i].value & 0xFF))) {
          i++;
        }
        break;
      case GSA_16_BIT_IF_HIGHER_S:
        if (!((s16)CPUReadHalfWord(cheatsList[i].address) > ((s16)cheatsList[i].value & 0xFFFF))) {
          i++;
        }
        break;
      case GSA_32_BIT_IF_HIGHER_S:
        if (!((s32)CPUReadMemory(cheatsList[i].address) > (s32)cheatsList[i].value)) {
          i++;
        }
        break;
      case GSA_8_BIT_IF_LOWER_S2:
        if (!((s8)CPUReadByte(cheatsList[i].address) < ((s8)cheatsList[i].value & 0xFF))) {
          i+=2;
        }
        break;
      case GSA_16_BIT_IF_LOWER_S2:
        if (!((s16)CPUReadHalfWord(cheatsList[i].address) < ((s16)cheatsList[i].value & 0xFFFF))) {
          i+=2;
        }
        break;
      case GSA_32_BIT_IF_LOWER_S2:
        if (!((s32)CPUReadMemory(cheatsList[i].address) < (s32)cheatsList[i].value)) {
          i+=2;
        }
        break;
      case GSA_8_BIT_IF_HIGHER_S2:
        if (!((s8)CPUReadByte(cheatsList[i].address) > ((s8)cheatsList[i].value & 0xFF))) {
          i+=2;
        }
        break;
      case GSA_16_BIT_IF_HIGHER_S2:
        if (!((s16)CPUReadHalfWord(cheatsList[i].address) > ((s16)cheatsList[i].value & 0xFFFF))) {
          i+=2;
        }
        break;
      case GSA_32_BIT_IF_HIGHER_S2:
        if (!((s32)CPUReadMemory(cheatsList[i].address) > (s32)cheatsList[i].value)) {
          i+=2;
        }
        break;
      case GSA_16_BIT_WRITE_IOREGS:
        if ((cheatsList[i].address <= 0x3FF) && (cheatsList[i].address != 0x6) &&
            (cheatsList[i].address != 0x130)) 
          ioMem[cheatsList[i].address & 0x3FE]=cheatsList[i].value & 0xFFFF;
        break;
      case GSA_32_BIT_WRITE_IOREGS:
        if (cheatsList[i].address<=0x3FF)  
        {
          if (((cheatsList[i].address & 0x3FC) != 0x6) && ((cheatsList[i].address & 0x3FC) != 0x130))
            ioMem[cheatsList[i].address & 0x3FC]= (cheatsList[i].value & 0xFFFF);
          if ((((cheatsList[i].address & 0x3FC)+2) != 0x6) && ((cheatsList[i].address & 0x3FC) +2) != 0x130)
            ioMem[(cheatsList[i].address & 0x3FC) + 2 ]= ((cheatsList[i].value>>16 ) & 0xFFFF);
        }
        break;
      case GSA_8_BIT_IF_TRUE3:
        if(CPUReadByte(cheatsList[i].address) != cheatsList[i].value) {
          onoff=false;
        }
        break;
      case GSA_16_BIT_IF_TRUE3:
        if(CPUReadHalfWord(cheatsList[i].address) != cheatsList[i].value) {
          onoff=false;
        }
        break;
      case GSA_32_BIT_IF_TRUE3:
        if(CPUReadMemory(cheatsList[i].address) != cheatsList[i].value) {
          onoff=false;
        }
        break;
      case GSA_8_BIT_IF_FALSE3:
        if(CPUReadByte(cheatsList[i].address) == cheatsList[i].value) {
          onoff=false;
        }
        break;
      case GSA_16_BIT_IF_FALSE3:
        if(CPUReadHalfWord(cheatsList[i].address) == cheatsList[i].value) {
          onoff=false;
        }
        break;
      case GSA_32_BIT_IF_FALSE3:
        if(CPUReadMemory(cheatsList[i].address) == cheatsList[i].value) {
          onoff=false;
        }
        break;
      case GSA_8_BIT_IF_LOWER_S3:
        if (!((s8)CPUReadByte(cheatsList[i].address) < ((s8)cheatsList[i].value & 0xFF))) {
          onoff=false;
        }
        break;
      case GSA_16_BIT_IF_LOWER_S3:
        if (!((s16)CPUReadHalfWord(cheatsList[i].address) < ((s16)cheatsList[i].value & 0xFFFF))) {
          onoff=false;
        }
        break;
      case GSA_32_BIT_IF_LOWER_S3:
        if (!((s32)CPUReadMemory(cheatsList[i].address) < (s32)cheatsList[i].value)) {
          onoff=false;
        }
        break;
      case GSA_8_BIT_IF_HIGHER_S3:
        if (!((s8)CPUReadByte(cheatsList[i].address) > ((s8)cheatsList[i].value & 0xFF))) {
          onoff=false;
        }
        break;
      case GSA_16_BIT_IF_HIGHER_S3:
        if (!((s16)CPUReadHalfWord(cheatsList[i].address) > ((s16)cheatsList[i].value & 0xFFFF))) {
          onoff=false;
        }
        break;
      case GSA_32_BIT_IF_HIGHER_S3:
        if (!((s32)CPUReadMemory(cheatsList[i].address) > (s32)cheatsList[i].value)) {
          onoff=false;
        }
        break;
      case GSA_8_BIT_IF_LOWER_U3:
        if (!(CPUReadByte(cheatsList[i].address) < (cheatsList[i].value & 0xFF))) {
          onoff=false;
        }
        break;
      case GSA_16_BIT_IF_LOWER_U3:
        if (!(CPUReadHalfWord(cheatsList[i].address) < (cheatsList[i].value & 0xFFFF))) {
          onoff=false;
        }
        break;
      case GSA_32_BIT_IF_LOWER_U3:
        if (!(CPUReadMemory(cheatsList[i].address) < cheatsList[i].value)) {
          onoff=false;
        }
        break;
      case GSA_8_BIT_IF_HIGHER_U3:
        if (!(CPUReadByte(cheatsList[i].address) > (cheatsList[i].value & 0xFF))) {
          onoff=false;
        }
        break;
      case GSA_16_BIT_IF_HIGHER_U3:
        if (!(CPUReadHalfWord(cheatsList[i].address) > (cheatsList[i].value & 0xFFFF))) {
          onoff=false;
        }
        break;
      case GSA_32_BIT_IF_HIGHER_U3:
        if (!(CPUReadMemory(cheatsList[i].address) > cheatsList[i].value)) {
          onoff=false;
        }
        break;
      case GSA_8_BIT_IF_AND3:
        if (!(CPUReadByte(cheatsList[i].address) & (cheatsList[i].value & 0xFF))) {
          onoff=false;
        }
        break;
      case GSA_16_BIT_IF_AND3:
        if (!(CPUReadHalfWord(cheatsList[i].address) & (cheatsList[i].value & 0xFFFF))) {
          onoff=false;
        }
        break;
      case GSA_32_BIT_IF_AND3:
        if (!(CPUReadMemory(cheatsList[i].address) & cheatsList[i].value)) {
          onoff=false;
        }
        break;
      case GSA_ALWAYS3:
        if (!(CPUReadMemory(cheatsList[i].address) & cheatsList[i].value)) {
          onoff=false;
        }
        break;
      }
    }
  }
  for (i = 0; i<4; i++)
    if (rompatch2addr [i] != 0)
      CHEAT_PATCH_ROM_16BIT(rompatch2addr [i],rompatch2val [i]);
  return ticks;
}

void cheatsAdd(const char *codeStr,
               const char *desc,
               u32 address,
               u32 value,
               int code,
               int size)
{
  if(cheatsNumber < 100) {
    int x = cheatsNumber;
    cheatsList[x].code = code;
    cheatsList[x].size = size;
    cheatsList[x].address = address;
    cheatsList[x].value = value;
    strcpy(cheatsList[x].codestring, codeStr);
    strcpy(cheatsList[x].desc, desc);
    cheatsList[x].enabled = true;
    cheatsList[x].status = 0;

    // we only store the old value for this simple codes. ROM patching
    // is taken care when it actually patches the ROM
    switch(cheatsList[x].size) {
    case INT_8_BIT_WRITE:
      cheatsList[x].oldValue = CPUReadByte(address);
      break;
    case INT_16_BIT_WRITE:
      cheatsList[x].oldValue = CPUReadHalfWord(address);
      break;
    case INT_32_BIT_WRITE:
      cheatsList[x].oldValue = CPUReadMemory(address);
      break;
    }
    cheatsNumber++;
  }
}

void cheatsDelete(int number, bool restore)
{
  if(number < cheatsNumber && number >= 0) {
    int x = number;

    if(restore) {
      switch(cheatsList[x].size) {
      case INT_8_BIT_WRITE:
        CPUWriteByte(cheatsList[x].address, (u8)cheatsList[x].oldValue);
        break;
      case INT_16_BIT_WRITE:
        CPUWriteHalfWord(cheatsList[x].address, (u16)cheatsList[x].oldValue);
        break;
      case INT_32_BIT_WRITE:
        CPUWriteMemory(cheatsList[x].address, cheatsList[x].oldValue);
        break;
      case GSA_16_BIT_ROM_PATCH:
        if(cheatsList[x].status & 1) {
          cheatsList[x].status &= ~1;
          CHEAT_PATCH_ROM_16BIT(cheatsList[x].address,
                                cheatsList[x].oldValue);  
        }
        break;
      case GSA_16_BIT_ROM_PATCH2C:
	  case GSA_16_BIT_ROM_PATCH2D:
	  case GSA_16_BIT_ROM_PATCH2E:
	  case GSA_16_BIT_ROM_PATCH2F:
        if(cheatsList[x].status & 1) {
          cheatsList[x].status &= ~1;
        }
        break;
      }
    }
    if((x+1) <  cheatsNumber) {
      memcpy(&cheatsList[x], &cheatsList[x+1], sizeof(CheatsData)*
             (cheatsNumber-x-1));
    }
    cheatsNumber--;
  }
}

void cheatsDeleteAll(bool restore)
{
  for(int i = cheatsNumber-1; i >= 0; i--) {
    cheatsDelete(i, restore);
  }
}

void cheatsEnable(int i)
{
  if(i >= 0 && i < cheatsNumber) {
    cheatsList[i].enabled = true;
  }
}

void cheatsDisable(int i)
{
  if(i >= 0 && i < cheatsNumber) {
    switch(cheatsList[i].size) {
    case GSA_16_BIT_ROM_PATCH:
      if(cheatsList[i].status & 1) {
        cheatsList[i].status &= ~1;
        CHEAT_PATCH_ROM_16BIT(cheatsList[i].address,
                              cheatsList[i].oldValue);
      }
      break;
    case GSA_16_BIT_ROM_PATCH2C:
    case GSA_16_BIT_ROM_PATCH2D:
    case GSA_16_BIT_ROM_PATCH2E:
    case GSA_16_BIT_ROM_PATCH2F:
      if(cheatsList[i].status & 1) {
        cheatsList[i].status &= ~1;
      }
      break;
    }
    cheatsList[i].enabled = false;
  }
}

bool cheatsVerifyCheatCode(const char *code, const char *desc)
{
  int len = strlen(code);
  if(len != 11 && len != 13 && len != 17) {
    systemMessage(MSG_INVALID_CHEAT_CODE, N_("Invalid cheat code '%s'"), code);
    return false;
  }

  if(code[8] != ':') {
    systemMessage(MSG_INVALID_CHEAT_CODE, N_("Invalid cheat code '%s'"), code);
    return false;    
  }

  int i;
  for(i = 0; i < 8; i++) {
    if(!CHEAT_IS_HEX(code[i])) {
      // wrong cheat
      systemMessage(MSG_INVALID_CHEAT_CODE,
                    N_("Invalid cheat code '%s'"), code);
      return false;
    }
  }
  for(i = 9; i < len; i++) {
    if(!CHEAT_IS_HEX(code[i])) {
      // wrong cheat
      systemMessage(MSG_INVALID_CHEAT_CODE,
                    N_("Invalid cheat code '%s'"), code);
      return false;
    }
  }
  
  u32 address = 0;
  u32 value = 0;
  
  char buffer[10];
  strncpy(buffer, code, 8);
  buffer[8] = 0;
  sscanf(buffer, "%x", &address);

  switch(address >> 24) {
  case 2:
  case 3:
    break;
  default:
    systemMessage(MSG_INVALID_CHEAT_CODE_ADDRESS,
                  N_("Invalid cheat code address: %08x"),
                  address);
    return false;
  }
  
  strncpy(buffer, &code[9], 8);  
  sscanf(buffer, "%x", &value);
  int type = 0;
  if(len == 13)
    type = 1;
  if(len == 17)
    type = 2;
  cheatsAdd(code, desc, address, value, type, type);
  return true;
}

void cheatsAddCheatCode(const char *code, const char *desc)
{
  cheatsVerifyCheatCode(code, desc);
}

void cheatsDecryptGSACode(u32& address, u32& value, bool v3) 
{
  u32 rollingseed = 0xC6EF3720;
  u32 seeds_v1[] =  { 0x09F4FBBD, 0x9681884A, 0x352027E9, 0xF3DEE5A7 }; 
  u32 seeds_v3[] = { 0x7AA9648F, 0x7FAE6994, 0xC0EFAAD5, 0x42712C57 };
  u32 *seeds = v3 ? seeds_v3 : seeds_v1;
  
  int bitsleft = 32;
  while (bitsleft > 0) {
    value -= ((((address << 4) + seeds[2]) ^ (address + rollingseed)) ^
              ((address >> 5) + seeds[3]));
    address -= ((((value << 4) + seeds[0]) ^ (value + rollingseed)) ^
                ((value >> 5) + seeds[1]));
    rollingseed -= 0x9E3779B9;
    bitsleft--;
  }
}

void cheatsAddGSACode(const char *code, const char *desc, bool v3)
{
  if(strlen(code) != 16) {
    // wrong cheat
    systemMessage(MSG_INVALID_GSA_CODE,
                  N_("Invalid GSA code. Format is XXXXXXXXYYYYYYYY"));
    return;
  }
  
  int i;
  for(i = 0; i < 16; i++) {
    if(!CHEAT_IS_HEX(code[i])) {
      // wrong cheat
      systemMessage(MSG_INVALID_GSA_CODE,
                    N_("Invalid GSA code. Format is XXXXXXXXYYYYYYYY"));
      return;
    }
  }
  
  char buffer[10];
  strncpy(buffer, code, 8);
  buffer[8] = 0;
  u32 address;
  sscanf(buffer, "%x", &address);
  strncpy(buffer, &code[8], 8);
  buffer[8] = 0;
  u32 value;
  sscanf(buffer, "%x", &value);

  cheatsDecryptGSACode(address, value, v3);

  if(value == 0x1DC0DE) {
    u32 gamecode = READ32LE(((u32 *)&rom[0xac]));
    if(gamecode != address) {
      char buffer[5];
      *((u32 *)buffer) = address;
      buffer[4] = 0;
      char buffer2[5];
      *((u32 *)buffer2) = READ32LE(((u32 *)&rom[0xac]));
      buffer2[4] = 0;
      systemMessage(MSG_GBA_CODE_WARNING, N_("Warning: cheats are for game %s. Current game is %s.\nCodes may not work correctly."),
                    buffer, buffer2);
    }
    cheatsAdd(code, desc, address & 0x0FFFFFFF, value, v3 ? 257 : 256, 
              UNKNOWN_CODE);
    return;
  }
  if(isMultilineWithData(cheatsNumber-1)) {
    cheatsAdd(code, desc, address, value, v3 ? 257 : 256, UNKNOWN_CODE);
    return;
  }
  if(v3) {
    int type = ((address >> 25) & 127) | ((address >> 17) & 0x80);
    u32 addr = (address & 0x00F00000) << 4 | (address & 0x0003FFFF);
    switch(type) {
    case 0x00:
      if(address == 0) {
        type = (value >> 25) & 127;
        addr = (value & 0x00F00000) << 4 | (value & 0x0003FFFF);
        switch(type) {
        case 0x04:
          cheatsAdd(code, desc, 0, value & 0x00FFFFFF, 257, GSA_SLOWDOWN);
          break;
        case 0x08:
          cheatsAdd(code, desc, 0, addr, 257, GSA_8_BIT_GS_WRITE2);
          break;
        case 0x09:
          cheatsAdd(code, desc, 0, addr, 257, GSA_16_BIT_GS_WRITE2);
          break;
        case 0x0a:
          cheatsAdd(code, desc, 0, addr, 257, GSA_32_BIT_GS_WRITE2);
          break;
        case 0x0c:
          cheatsAdd(code, desc, 0, value & 0x00FFFFFF, 257, GSA_16_BIT_ROM_PATCH2C);
          break;
        case 0x0d:
          cheatsAdd(code, desc, 0, value & 0x00FFFFFF, 257, GSA_16_BIT_ROM_PATCH2D);
          break;
        case 0x0e:
          cheatsAdd(code, desc, 0, value & 0x00FFFFFF, 257, GSA_16_BIT_ROM_PATCH2E);
          break;
        case 0x0f:
          cheatsAdd(code, desc, 0, value & 0x00FFFFFF, 257, GSA_16_BIT_ROM_PATCH2F);
          break;
        case 0x20:
          cheatsAdd(code, desc, 0, addr, 257, GSA_CODES_ON);
          break;
        case 0x40:
          cheatsAdd(code, desc, 0, addr, 257, GSA_8_BIT_SLIDE);
          break;
        case 0x41:
          cheatsAdd(code, desc, 0, addr, 257, GSA_16_BIT_SLIDE);
          break;
        case 0x42:
          cheatsAdd(code, desc, 0, addr, 257, GSA_32_BIT_SLIDE);
          break;
        default:
          cheatsAdd(code, desc, address, value, 257, UNKNOWN_CODE);
          break;
        }
      } else
        cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_FILL);
      break;
    case 0x01:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_FILL);
      break;
    case 0x02:
      cheatsAdd(code, desc, addr, value, 257, INT_32_BIT_WRITE);
      break;
    case 0x04:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_TRUE);
      break;
    case 0x05:
      cheatsAdd(code, desc, addr, value, 257, CBA_IF_TRUE);
      break;
    case 0x06:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_TRUE);
      break;
    case 0x07:
      cheatsAdd(code, desc, addr, value, 257, GSA_ALWAYS);
      break;
    case 0x08:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_FALSE);
      break;
    case 0x09:
      cheatsAdd(code, desc, addr, value, 257, CBA_IF_FALSE);
      break;
    case 0x0a:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_FALSE);
      break;
    case 0xc:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_LOWER_S);
      break;
    case 0xd:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_IF_LOWER_S);
      break;
    case 0xe:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_LOWER_S);
      break;
    case 0x10:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_HIGHER_S);
      break;
    case 0x11:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_IF_HIGHER_S);
      break;
    case 0x12:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_HIGHER_S);
      break;
    case 0x14:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_LOWER_U);
      break;
    case 0x15:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_IF_LOWER_U);
      break;
    case 0x16:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_LOWER_U);
      break;
    case 0x18:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_HIGHER_U);
      break;
    case 0x19:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_IF_HIGHER_U);
      break;
    case 0x1A:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_HIGHER_U);
      break;
    case 0x1C:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_AND);
      break;
    case 0x1D:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_IF_AND);
      break;
    case 0x1E:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_AND);
      break;
    case 0x20:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_POINTER);
      break;
    case 0x21:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_POINTER);
      break;
    case 0x22:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_POINTER);
      break;
    case 0x24:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_TRUE2);
      break;
    case 0x25:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_IF_TRUE2);
      break;
    case 0x26:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_TRUE2);
      break;
    case 0x27:
      cheatsAdd(code, desc, addr, value, 257, GSA_ALWAYS2);
      break;
    case 0x28:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_FALSE2);
      break;
    case 0x29:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_IF_FALSE2);
      break;
    case 0x2a:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_FALSE2);
      break;
    case 0x2c:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_LOWER_S2);
      break;
    case 0x2d:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_IF_LOWER_S2);
      break;
    case 0x2e:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_LOWER_S2);
      break;
    case 0x30:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_HIGHER_S2);
      break;
    case 0x31:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_IF_HIGHER_S2);
      break;
    case 0x32:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_HIGHER_S2);
      break;
    case 0x34:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_LOWER_U2);
      break;
    case 0x35:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_IF_LOWER_U2);
      break;
    case 0x36:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_LOWER_U2);
      break;
    case 0x38:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_HIGHER_U2);
      break;
    case 0x39:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_IF_HIGHER_U2);
      break;
    case 0x3A:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_HIGHER_U2);
      break;
    case 0x3C:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_AND2);
      break;
    case 0x3D:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_IF_AND2);
      break;
    case 0x3E:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_AND2);
      break;
    case 0x40:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_ADD);
      break;
    case 0x41:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_ADD);
      break;
    case 0x42:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_ADD);
      break;
    case 0x44:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_TRUE3);
      break;
    case 0x45:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_IF_TRUE3);
      break;
    case 0x46:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_TRUE3);
      break;
	case 0x47:
      cheatsAdd(code, desc, addr, value, 257, GSA_ALWAYS3);
      break;
    case 0x48:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_FALSE3);
      break;
    case 0x49:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_IF_FALSE3);
      break;
    case 0x4a:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_FALSE3);
      break;
    case 0x4c:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_LOWER_S3);
      break;
    case 0x4d:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_IF_LOWER_S3);
      break;
    case 0x4e:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_LOWER_S3);
      break;
    case 0x50:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_HIGHER_S3);
      break;
    case 0x51:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_IF_HIGHER_S3);
      break;
    case 0x52:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_HIGHER_S3);
      break;
    case 0x54:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_LOWER_U3);
      break;
    case 0x55:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_IF_LOWER_U3);
      break;
    case 0x56:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_LOWER_U3);
      break;
    case 0x58:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_HIGHER_U3);
      break;
    case 0x59:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_IF_HIGHER_U3);
      break;
    case 0x5a:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_HIGHER_U3);
      break;
    case 0x5c:
      cheatsAdd(code, desc, addr, value, 257, GSA_8_BIT_IF_AND3);
      break;
    case 0x5d:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_IF_AND3);
      break;
    case 0x5e:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_IF_AND3);
      break;
    case 0x63:
      cheatsAdd(code, desc, addr, value, 257, GSA_16_BIT_WRITE_IOREGS);
      break;
    case 0xE3:
      cheatsAdd(code, desc, addr, value, 257, GSA_32_BIT_WRITE_IOREGS);
      break;
    default:
      cheatsAdd(code, desc, address, value, 257, UNKNOWN_CODE);
      break;
    }
  } else {
    int type = (address >> 28) & 15;  
    switch(type) {
    case 0:
    case 1:
    case 2:
      cheatsAdd(code, desc, address & 0x0FFFFFFF, value, 256, type);
      break;
    case 6:
      address <<= 1;
      type = (address >> 28) & 15;
      if(type == 0x0c) {
        cheatsAdd(code, desc, address & 0x0FFFFFFF, value, 256, 
                  GSA_16_BIT_ROM_PATCH);
        break;
      }
      // unsupported code
      cheatsAdd(code, desc, address, value, 256, 
                UNKNOWN_CODE);
      break;
    case 8:
      switch((address >> 20) & 15) {
      case 1:
        cheatsAdd(code, desc, address & 0x0F0FFFFF, value, 256, 
                  GSA_8_BIT_GS_WRITE);
        break;
      case 2:
        cheatsAdd(code, desc, address & 0x0F0FFFFF, value, 256, 
                  GSA_16_BIT_GS_WRITE);
        break;
      case 3:
        cheatsAdd(code, desc, address & 0x0F0FFFFF, value, 256, 
                  GSA_32_BIT_GS_WRITE);
      case 15:
        cheatsAdd(code, desc, 0, value & 0xFF00, 256, GSA_SLOWDOWN);
        break;
      default:
        // unsupported code
        cheatsAdd(code, desc, address, value, 256, 
                  UNKNOWN_CODE);
        break;
      }
      break;
    case 0x0d:
      if(address != 0xDEADFACE) {
        cheatsAdd(code, desc, address & 0x0FFFFFFF, value, 256, 
                  CBA_IF_TRUE);
      } else
        cheatsAdd(code, desc, address, value, 256, 
                  UNKNOWN_CODE);
      break;
    default:
      // unsupported code
      cheatsAdd(code, desc, address, value, 256, 
                UNKNOWN_CODE);
      break;
    }
  }
}

bool cheatsImportGSACodeFile(const char *name, int game, bool v3)
{
  FILE *f = fopen(name, "rb");
  if(!f)
    return false;
  
  int games = 0;
  int len = 0;
  fseek(f, 0x1e, SEEK_CUR);
  fread(&games, 1, 4, f);
  bool found = false;
  int g = 0;
  while(games > 0) {
    if(g == game) {
      found = true;
      break;
    }
    fread(&len, 1, 4, f);
    fseek(f,len,SEEK_CUR);
    int codes = 0;
    fread(&codes, 1, 4, f);
    while(codes > 0) {
      fread(&len, 1, 4, f);
      fseek(f, len, SEEK_CUR);
      fseek(f, 8, SEEK_CUR);
      fread(&len, 1, 4, f);
      fseek(f, len*12, SEEK_CUR);
      codes--;
    }
    games--;
    g++;
  }
  if(found) {
    char desc[256];
    char code[17];
    fread(&len, 1, 4, f);
    fseek(f, len, SEEK_CUR);
    int codes = 0;
    fread(&codes, 1, 4, f);
    while(codes > 0) {
      fread(&len, 1, 4, f);
      fread(desc, 1, len, f);
      desc[len] =0;
      desc[31] = 0;
      fread(&len, 1, 4, f);
      fseek(f, len, SEEK_CUR);
      fseek(f, 4, SEEK_CUR);
      fread(&len, 1, 4, f);
      while(len) {
        fseek(f, 4, SEEK_CUR);
        fread(code, 1, 8, f);
        fseek(f, 4, SEEK_CUR);
        fread(&code[8], 1, 8, f);
        code[16] = 0;
        cheatsAddGSACode(code, desc, v3);
        len -= 2;
      }
      codes--;
    }
  }
  fclose(f);
  return false;
}

void cheatsCBAReverseArray(u8 *array, u8 *dest)
{
  dest[0] = array[3];
  dest[1] = array[2];
  dest[2] = array[1];
  dest[3] = array[0];
  dest[4] = array[5];
  dest[5] = array[4];
}

void chatsCBAScramble(u8 *array, int count, u8 b)
{
  u8 *x = array + (count >> 3);
  u8 *y = array + (b >> 3);
  u32 z = *x & (1 << (count & 7));
  u32 x0 = (*x & (~(1 << (count & 7))));
  if (z != 0)
    z = 1;
  if ((*y & (1 << (b & 7))) != 0)
    x0 |= (1 << (count & 7));
  *x = x0;
  u32 temp = *y & (~(1 << (b & 7)));
  if (z != 0)
    temp |= (1 << (b & 7));
  *y = temp;
}

u32 cheatsCBAGetValue(u8 *array)
{
  return array[0] | array[1]<<8 | array[2] << 16 | array[3]<<24;
}

u16 cheatsCBAGetData(u8 *array)
{
  return array[4] | array[5]<<8;
}

void cheatsCBAArrayToValue(u8 *array, u8 *dest)
{
  dest[0] = array[3];
  dest[1] = array[2];
  dest[2] = array[1];
  dest[3] = array[0];
  dest[4] = array[5];
  dest[5] = array[4];
}

void cheatsCBAParseSeedCode(u32 address, u32 value, u32 *array)
{
  array[0] = 1;
  array[1] = value & 0xFF;
  array[2] = (address >> 0x10) & 0xFF;
  array[3] = (value >> 8) & 0xFF;
  array[4] = (address >> 0x18) & 0x0F;
  array[5] = address & 0xFFFF;
  array[6] = address;
  array[7] = value;
}

u32 cheatsCBAEncWorker()
{
  u32 x = (cheatsCBATemporaryValue * 0x41c64e6d) + 0x3039;
  u32 y = (x * 0x41c64e6d) + 0x3039;
  u32 z = x >> 0x10;
  x = ((y >> 0x10) & 0x7fff) << 0x0f;
  z = (z << 0x1e) | x;
  x = (y * 0x41c64e6d) + 0x3039;
  cheatsCBATemporaryValue = x;
  return z | ((x >> 0x10) & 0x7fff);
}

#define ROR(v, s) \
  (((v) >> (s)) | (((v) & ((1 << (s))-1)) << (32 - (s))))

u32 cheatsCBACalcIndex(u32 x, u32 y)
{
  if(y != 0) {
    if(y == 1)
      x = 0;
    else if(x == y)
      x = 0;
    if(y < 1)
      return x;
    else if(x < y)
      return x;
    u32 x0 = 1;

    while(y < 0x10000000) {
      if(y < x) {
        y = y << 4;
        x0 = x0 << 4;
      } else break;
    }

    while(y < 0x80000000) {
      if(y < x) {
        y = y << 1;
        x0 = x0 << 1;
      } else break;
    }

  loop:
    u32 z = 0;
    if(x >= y)
      x -= y;
    if(x >= (y >> 1)) {
      x -= (y >> 1);
      z |= ROR(x0, 1);
    }
    if(x >= (y >> 2)) {
      x -= (y >> 2);
      z |= ROR(x0, 2);
    }
    if(x >= (y >> 3)) {
      x -= (y >> 3);
      z |= ROR(x0, 3);
    }

    u32 temp = x0;

    if(x != 0) {
      x0 = x0 >> 4;
      if(x0 != 0) {
        y = y >> 4;
        goto loop;
      }
    }

    z = z & 0xe0000000;

    if(z != 0) {
      if((temp & 7) == 0)
        return x;
    } else
      return x;

    if((z & ROR(temp, 3)) != 0)
      x += y >> 3;
    if((z & ROR(temp, 2)) != 0)
      x += y >> 2;
    if((z & ROR(temp, 1)) != 0)
      x += y >> 1;
    return x;
  } else {
  }
  // should not happen in the current code
  return 0;
}

void cheatsCBAUpdateSeedBuffer(u32 a, u8 *buffer, int count)
{
  int i;
  for(i = 0; i < count; i++)
    buffer[i] = i;
  for(i = 0; (u32)i < a; i++) {
    u32 a = cheatsCBACalcIndex(cheatsCBAEncWorker(), count);
    u32 b = cheatsCBACalcIndex(cheatsCBAEncWorker(), count);
    u32 t = buffer[a];
    buffer[a] = buffer[b];
    buffer[b] = t;
  }
}

void cheatsCBAChangeEncryption(u32 *seed)
{
  int i;

  cheatsCBATemporaryValue = (seed[1] ^ 0x1111);
  cheatsCBAUpdateSeedBuffer(0x50, cheatsCBASeedBuffer, 0x30);
  cheatsCBATemporaryValue = 0x4efad1c3;

  for(i = 0; (u32)i < seed[4]; i++) {
    cheatsCBATemporaryValue = cheatsCBAEncWorker();
  }
  cheatsCBASeed[2] = cheatsCBAEncWorker();
  cheatsCBASeed[3] = cheatsCBAEncWorker();

  cheatsCBATemporaryValue = seed[3] ^ 0xf254;

  for(i = 0; (u32)i < seed[3]; i++) {
    cheatsCBATemporaryValue = cheatsCBAEncWorker();
  }

  cheatsCBASeed[0] = cheatsCBAEncWorker();
  cheatsCBASeed[1] = cheatsCBAEncWorker();

  *((u32 *)&cheatsCBACurrentSeed[0]) = seed[6];
  *((u32 *)&cheatsCBACurrentSeed[4]) = seed[7];
  *((u32 *)&cheatsCBACurrentSeed[8]) = 0;
}

u16 cheatsCBAGenValue(u32 x, u32 y, u32 z)
{
  y <<= 0x10;
  z <<= 0x10;
  x <<= 0x18;
  u32 x0 = (int)y >> 0x10;
  z = (int)z >> 0x10;
  x = (int)x >> 0x10;
  for(int i = 0; i < 8; i++) {
    u32 temp = z ^ x;
    if ((int)temp >= 0) {
      temp = z << 0x11;
    }
    else {
      temp = z << 0x01;
      temp ^= x0;
      temp = temp << 0x10;
    }
    z = (int)temp >> 0x10;
    temp = x << 0x11;
    x = (int)temp >> 0x10;
  }
  return z & 0xffff;
}

void cheatsCBAGenTable() {
  for (int i = 0; i < 0x100; i++) {
    cheatsCBATable[i] = cheatsCBAGenValue(i, 0x1021, 0);
  }
  cheatsCBATableGenerated = true;
}

u16 cheatsCBACalcCRC(u8 *rom, int count)
{
  u32 crc = 0xffffffff;
  
  if (count & 3) {
    // 0x08000EAE
  } else {
    count = (count >> 2) - 1;
    if(count != -1) {
      while(count != -1) {
        crc = (((crc << 0x08) ^ cheatsCBATable[(((u32)crc << 0x10) >> 0x18)
                                               ^ *rom++]) << 0x10) >> 0x10;
        crc = (((crc << 0x08) ^ cheatsCBATable[(((u32)crc << 0x10) >> 0x18)
                                               ^ *rom++]) << 0x10) >> 0x10;
        crc = (((crc << 0x08) ^ cheatsCBATable[(((u32)crc << 0x10) >> 0x18)
                                               ^ *rom++]) << 0x10) >> 0x10;
        crc = (((crc << 0x08) ^ cheatsCBATable[(((u32)crc << 0x10) >> 0x18)
                                               ^ *rom++]) << 0x10) >> 0x10;
        count--;
      }
    }
  }
  return crc & 0xffff;
}

void cheatsCBADecrypt(u8 *decrypt)
{
  u8 buffer[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  u8 *array = &buffer[1];

  cheatsCBAReverseArray(decrypt, array);

  for(int count = 0x2f; count >= 0; count--) {
    chatsCBAScramble(array, count, cheatsCBASeedBuffer[count]);
  }
  cheatsCBAArrayToValue(array, decrypt);
  *((u32 *)decrypt) = cheatsCBAGetValue(decrypt) ^
    cheatsCBASeed[0];
  *((u16 *)(decrypt+4)) = (cheatsCBAGetData(decrypt) ^
                           cheatsCBASeed[1]) & 0xffff;

  cheatsCBAReverseArray(decrypt, array);

  u32 cs = cheatsCBAGetValue(cheatsCBACurrentSeed);
  for(int i = 0; i <= 4; i++) {
    array[i] = ((cs >> 8) ^ array[i+1]) ^ array[i] ;
  }

  array[5] = (cs >> 8) ^ array[5];

  for(int j = 5; j >=0; j--) {
    array[j] = (cs ^ array[j-1]) ^ array[j];
  }

  cheatsCBAArrayToValue(array, decrypt);

  *((u32 *)decrypt) = cheatsCBAGetValue(decrypt)
    ^ cheatsCBASeed[2];
  *((u16 *)(decrypt+4)) = (cheatsCBAGetData(decrypt)
                           ^ cheatsCBASeed[3]) & 0xffff;
}

int cheatsCBAGetCount()
{
  int count = 0;
  for(int i = 0; i < cheatsNumber; i++) {
    if(cheatsList[i].code == 512)
      count++;
  }
  return count;
}

bool cheatsCBAShouldDecrypt()
{
  for(int i = 0; i < cheatsNumber; i++) {
    if(cheatsList[i].code == 512) {
      return (cheatsList[i].codestring[0] == '9');
    }
  }
  return false;
}

void cheatsAddCBACode(const char *code, const char *desc)
{
  if(strlen(code) != 13) {
    // wrong cheat
    systemMessage(MSG_INVALID_CBA_CODE,
                  N_("Invalid CBA code. Format is XXXXXXXX YYYY."));
    return;
  }
  
  int i;
  for(i = 0; i < 8; i++) {
    if(!CHEAT_IS_HEX(code[i])) {
      // wrong cheat
      systemMessage(MSG_INVALID_CBA_CODE,
                    N_("Invalid CBA code. Format is XXXXXXXX YYYY."));
      return;
    }
  }

  if(code[8] != ' ') {
    systemMessage(MSG_INVALID_CBA_CODE,
                  N_("Invalid CBA code. Format is XXXXXXXX YYYY."));
    return;    
  }
  
  for(i = 9; i < 13; i++) {
    if(!CHEAT_IS_HEX(code[i])) {
      // wrong cheat
      systemMessage(MSG_INVALID_CBA_CODE,
                    N_("Invalid CBA code. Format is XXXXXXXX YYYY."));
      return;
    }
  }  
  
  char buffer[10];
  strncpy(buffer, code, 8);
  buffer[8] = 0;
  u32 address;
  sscanf(buffer, "%x", &address);
  strncpy(buffer, &code[9], 4);
  buffer[4] = 0;
  u32 value;
  sscanf(buffer, "%x", &value);

  u8 array[8] = {
    address & 255,
    (address >> 8) & 255,
    (address >> 16) & 255,
    (address >> 24) & 255,
    (value & 255),
    (value >> 8) & 255,
    0,
    0
  };

  if(cheatsCBAGetCount() == 0 &&
     (address >> 28) == 9) {
    u32 seed[8];
    cheatsCBAParseSeedCode(address, value, seed);
    cheatsCBAChangeEncryption(seed);
    cheatsAdd(code, desc, address & 0x0FFFFFFF, value, 512, UNKNOWN_CODE);
  } else {
    if(cheatsCBAShouldDecrypt())
      cheatsCBADecrypt(array);

    address = READ32LE(((u32 *)array));
    value = READ16LE(((u16 *)&array[4]));
    
    int type = (address >> 28) & 15;

    if(isMultilineWithData(cheatsNumber-1) || (super>0)) {
      cheatsAdd(code, desc, address, value, 512, UNKNOWN_CODE);
	  if (super>0)
		  super-= 1;
      return;
    }
    
    switch(type) {
    case 0x00:
      {
        if(!cheatsCBATableGenerated)
          cheatsCBAGenTable();
        u32 crc = cheatsCBACalcCRC(rom, 0x10000);
        if(crc != address) {
          systemMessage(MSG_CBA_CODE_WARNING,
                        N_("Warning: Codes seem to be for a different game.\nCodes may not work correctly."));
        }
        cheatsAdd(code, desc, address & 0x0FFFFFFF, value, 512, 
                  UNKNOWN_CODE);
      }
      break;
    case 0x02:
      cheatsAdd(code, desc, address & 0x0FFFFFFE, value, 512, 
                CBA_OR);
      break;
    case 0x03:
      cheatsAdd(code, desc, address & 0x0FFFFFFF, value, 512, 
                INT_8_BIT_WRITE);
      break;
    case 0x04:
      cheatsAdd(code, desc, address & 0x0FFFFFFE, value, 512, 
                CBA_SLIDE_CODE);
      break;
    case 0x05:
		cheatsAdd(code, desc, address & 0x0FFFFFFE, value, 512,
                  CBA_SUPER);
		super = getCodeLength(cheatsNumber-1);
      break;
    case 0x06:
      cheatsAdd(code, desc, address & 0x0FFFFFFE, value, 512, 
                CBA_AND);
      break;
    case 0x07:
      cheatsAdd(code, desc, address & 0x0FFFFFFE, value, 512, 
                CBA_IF_TRUE);
      break;
    case 0x08:
      cheatsAdd(code, desc, address & 0x0FFFFFFE, value, 512, 
                INT_16_BIT_WRITE);
      break;
    case 0x0a:
      cheatsAdd(code, desc, address & 0x0FFFFFFE, value, 512, 
                CBA_IF_FALSE);
      break;
    case 0x0b:
      cheatsAdd(code, desc, address & 0x0FFFFFFE, value, 512, 
                CBA_GT);
      break;
    case 0x0c:
      cheatsAdd(code, desc, address & 0x0FFFFFFE, value, 512, 
                CBA_LT);
      break;
    case 0x0d:
		if ((address & 0xF0)<0x30)
      cheatsAdd(code, desc, address & 0xF0, value, 512, 
                CBA_IF_KEYS_PRESSED);
      break;
    case 0x0e:
      cheatsAdd(code, desc, address & 0x0FFFFFFF, value & 0x8000 ? value | 0xFFFF0000 : value, 512,
                CBA_ADD);
      break;
    case 0x0f:
      cheatsAdd(code, desc, address & 0x0FFFFFFE, value, 512,
                GSA_16_BIT_IF_AND);
      break;
    default:
      // unsupported code
      cheatsAdd(code, desc, address & 0xFFFFFFFF, value, 512, 
                UNKNOWN_CODE);
      break;
    }
  }
}

void cheatsSaveGame(gzFile file)
{
  utilWriteInt(file, cheatsNumber);
  
  utilGzWrite(file, cheatsList, sizeof(cheatsList));
}

void cheatsReadGame(gzFile file)
{
  cheatsNumber = 0;
  
  cheatsNumber = utilReadInt(file);

  utilGzRead(file, cheatsList, sizeof(cheatsList));

  bool firstCodeBreaker = true;
  
  for(int i = 0; i < cheatsNumber; i++) {
    cheatsList[i].status = 0;
    if(!cheatsList[i].codestring[0]) {
      switch(cheatsList[i].size) {
      case 0:
        sprintf(cheatsList[i].codestring, "%08x:%02x", cheatsList[i].address,
                cheatsList[i].value);
        break;
      case 1:
        sprintf(cheatsList[i].codestring, "%08x:%04x", cheatsList[i].address,
                cheatsList[i].value);
        break;
      case 2:
        sprintf(cheatsList[i].codestring, "%08x:%08x", cheatsList[i].address,
                cheatsList[i].value);
        break;
      }
    }

    if(cheatsList[i].enabled) {
      cheatsEnable(i);
    }

    if(cheatsList[i].code == 512 && firstCodeBreaker) {
      firstCodeBreaker = false;
      char buffer[10];
      strncpy(buffer, cheatsList[i].codestring, 8);
      buffer[8] = 0;
      u32 address;
      sscanf(buffer, "%x", &address);
      if((address >> 28) == 9) {
        strncpy(buffer, &cheatsList[i].codestring[9], 4);
        buffer[4] = 0;
        u32 value;
        sscanf(buffer, "%x", &value);
        
        u32 seed[8];
        cheatsCBAParseSeedCode(address, value, seed);
        cheatsCBAChangeEncryption(seed);
      }
    }
  }
}

void cheatsSaveCheatList(const char *file)
{
  if(cheatsNumber == 0)
    return;
  FILE *f = fopen(file, "wb");
  if(f == NULL)
    return;
  int version = 1;
  fwrite(&version, 1, sizeof(version), f);
  int type = 0;
  fwrite(&type, 1, sizeof(type), f);
  fwrite(&cheatsNumber, 1, sizeof(cheatsNumber), f);
  fwrite(cheatsList, 1, sizeof(cheatsList), f);
  fclose(f);
}

bool cheatsLoadCheatList(const char *file)
{
  cheatsNumber = 0;

  int count = 0;

  FILE *f = fopen(file, "rb");

  if(f == NULL)
    return false;

  int version = 0;

  if(fread(&version, 1, sizeof(version), f) != sizeof(version)) {
    fclose(f);
    return false;
  }
     
  if(version != 1) {
    systemMessage(MSG_UNSUPPORTED_CHEAT_LIST_VERSION,
                  N_("Unsupported cheat list version %d"), version);
    fclose(f);
    return false;
  }

  int type = 0;
  if(fread(&type, 1, sizeof(type), f) != sizeof(type)) {
    fclose(f);
    return false;
  }

  if(type != 0) {
    systemMessage(MSG_UNSUPPORTED_CHEAT_LIST_TYPE,
                  N_("Unsupported cheat list type %d"), type);
    fclose(f);
    return false;
  }
  
  if(fread(&count, 1, sizeof(count), f) != sizeof(count)) {
    fclose(f);
    return false;
  }
  
  if(fread(cheatsList, 1, sizeof(cheatsList), f) != sizeof(cheatsList)) {
    fclose(f);
    return false;
  }

  bool firstCodeBreaker = true;
  
  for(int i = 0; i < count; i++) {
    cheatsList[i].status = 0; // remove old status as it is not used
    if(!cheatsList[i].codestring[0]) {
      switch(cheatsList[i].size) {
      case 0:
        sprintf(cheatsList[i].codestring, "%08x:%02x", cheatsList[i].address,
                cheatsList[i].value);
        break;
      case 1:
        sprintf(cheatsList[i].codestring, "%08x:%04x", cheatsList[i].address,
                cheatsList[i].value);
        break;
      case 2:
        sprintf(cheatsList[i].codestring, "%08x:%08x", cheatsList[i].address,
                cheatsList[i].value);
        break;
      }
    }
    
    if(cheatsList[i].code == 512 && firstCodeBreaker) {
      firstCodeBreaker = false;
      char buffer[10];
      strncpy(buffer, cheatsList[i].codestring, 8);
      buffer[8] = 0;
      u32 address;
      sscanf(buffer, "%x", &address);
      if((address >> 28) == 9) {
        strncpy(buffer, &cheatsList[i].codestring[9], 4);
        buffer[4] = 0;
        u32 value;
        sscanf(buffer, "%x", &value);
        
        u32 seed[8];
        cheatsCBAParseSeedCode(address, value, seed);
        cheatsCBAChangeEncryption(seed);
      }    
    }
  }
  cheatsNumber = count;
  fclose(f);
  return true;
}

extern int *extCpuLoopTicks;
extern int *extClockTicks;
extern int *extTicks;
extern int cpuSavedTicks;

extern void debuggerBreakOnWrite(u32 *, u32, u32, int); 

#define CPU_BREAK_LOOP \
  cpuSavedTicks = cpuSavedTicks - *extCpuLoopTicks;\
  *extCpuLoopTicks = *extClockTicks;\
  *extTicks = *extClockTicks;

void cheatsWriteMemory(u32 *address, u32 value, u32 mask)
{
#ifdef BKPT_SUPPORT
#ifdef SDL
  if(cheatsNumber == 0) {
    debuggerBreakOnWrite(address, *address, value, 2);
    CPU_BREAK_LOOP;
    *address = value;
    return;
  }
#endif
#endif
}

void cheatsWriteHalfWord(u16 *address, u16 value, u16 mask)
{
#ifdef BKPT_SUPPORT
#ifdef SDL
  if(cheatsNumber == 0) {
    debuggerBreakOnWrite((u32 *)address, *address, value, 1);
    CPU_BREAK_LOOP;
    *address = value;
    return;
  }
#endif
#endif
}

#if defined BKPT_SUPPORT && defined SDL
void cheatsWriteByte(u8 *address, u8 value)
#else
void cheatsWriteByte(u8 *, u8)
#endif
{
#ifdef BKPT_SUPPORT
#ifdef SDL
  if(cheatsNumber == 0) {
    debuggerBreakOnWrite((u32 *)address, *address, value, 0);
    CPU_BREAK_LOOP;
    *address = value;
    return;
  }
#endif
#endif
}