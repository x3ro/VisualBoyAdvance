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
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "GBA.h"
#include "GBAinline.h"
#include "Cheats.h"
#include "Globals.h"
#include "NLS.h"
#include <zlib.h>

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
 * 3AAAAAAA YYYY - 8-bit constant write
 * 4AAAAAAA YYYY - Slide code
 * CCCCCCCC IIII   (C is count and I is address increment)
 * 6AAAAAAA YYYY - 16-bit and
 * 7AAAAAAA YYYY - if address contains 16-bit value enable next code
 * 8AAAAAAA YYYY - 16-bit constant write
 * 9AAAAAAA YYYY - change decryption (when first code only?)
 * AAAAAAAA YYYY - if address does not contain 16-bit value enable next code
 * D0000020 YYYY - if button keys equal value enable next code
 */

#define INT_8_BIT_WRITE        0
#define INT_16_BIT_WRITE       1
#define INT_32_BIT_WRITE       2
#define GSA_16_BIT_ROM_PATCH   3
#define GSA_8_BIT_GS_WRITE     4
#define GSA_16_BIT_GS_WRITE    5
#define GSA_32_BIT_GS_WRITE    6
#define CBA_IF_KEYS_PRESSED    7
#define CBA_IF_TRUE            8
#define CBA_SLIDE_CODE         9
#define CBA_IF_FALSE           10
#define CBA_AND                11

CheatSearch cheatSearch;
CheatsData cheatsList[100];
int cheatsNumber = 0;

u8 cheatsCBASeedBuffer[0x30];
u32 cheatsCBASeed[4];
u32 cheatsCBATemporaryValue = 0;
u16 cheatsCBATable[256];
bool cheatsCBATableGenerated = false;

u8 cheatsCBACurrentSeed[12] = {
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

#define CHEAT_IS_HEX(a) ( ((a)>='A' && (a) <='F') || ((a) >='0' && (a) <= '9'))

#define CHEAT_PATCH_ROM_16BIT(a,v) \
  *((u16 *)&rom[(a) & 0x1ffffff]) = (v)

void cheatsCheckKeys(u32 keys, u32 extended)
{
  for(int i = 0; i < cheatsNumber; i++) {
    if(!cheatsList[i].enabled)
      continue;
    switch(cheatsList[i].size) {
    case 0:
      CPUWriteByte(cheatsList[i].address, cheatsList[i].value);
      break;
    case 1:
      CPUWriteHalfWord(cheatsList[i].address, cheatsList[i].value);
      break;
    case 2:
      CPUWriteMemory(cheatsList[i].address, cheatsList[i].value);
      break;
      // case 3: nothing to do for ROM patching
    case 4:
      if(extended & 4) {
        CPUWriteByte(cheatsList[i].address, cheatsList[i].value);
      }
      break;
    case 5:
      if(extended & 4) {
        CPUWriteHalfWord(cheatsList[i].address, cheatsList[i].value);   
      }
      break;
    case 6:
      if(extended & 4) {
        CPUWriteMemory(cheatsList[i].address, cheatsList[i].value);     
      }
      break;
    case 7:
      if(cheatsList[i].address == 0x20) {
        if((keys & cheatsList[i].value) != cheatsList[i].value) {
          i++;
        }
      } else if(cheatsList[i].value != keys) {
        i++;
      }
      break;
    case 8:
      if(CPUReadHalfWord(cheatsList[i].address) != cheatsList[i].value) {
        i++;
      }
      break;
    case 9:
      {
        u32 address = cheatsList[i].address;
        u16 value = cheatsList[i].value;
        i++;
        if(i < cheatsNumber) {
          int count = cheatsList[i].address;
          int inc = cheatsList[i].value;

          for(int x = 0; x < count; x++) {
            CPUWriteHalfWord(address, value);
            address += inc;
          }
        }
      }
      break;
    case 10:
      if(CPUReadHalfWord(cheatsList[i].address) == cheatsList[i].value) {
        i++;
      }
      break;
    case 11:
      CPUWriteHalfWord(cheatsList[i].address,
                       CPUReadHalfWord(cheatsList[i].address) &
                       cheatsList[i].value);
      break;
    }
  }
}

void cheatsAdd(char *codeStr,
               char *desc,
               u32 address,
               u32 value,
               int code,
               int size,
               bool freeze)
{
  if(cheatsNumber < 100) {
    int x = cheatsNumber;
    cheatsList[x].code = code;
    cheatsList[x].size = size;
    cheatsList[x].address = address;
    cheatsList[x].value = value;
    strcpy(cheatsList[x].codestring, codeStr);
    strcpy(cheatsList[x].desc, desc);
    cheatsList[x].status = 2;
    cheatsList[x].enabled = true;

    bool normalCode = true;
    
    if(address == 0) {
      code = 0xFFFF;
      cheatsList[x].enabled = false;
      cheatsList[x].status = 0;
      normalCode = false;
    }

    // don't do anything in case of multi line codes
    switch(code) {
    case 7: // press xxx codes
    case 8: // if equal codes
    case 9: // slide codes
      normalCode = false;
      break;
    }

    if(normalCode) {
      if(freeze)
        cheatsList[x].status |= 1;
      if(address < 0x8000000) {
        cheatsList[x].oldValue = (size == 0 ? CPUReadByte(address) :
                                  size == 1 ? CPUReadHalfWord(address) :
                                  size == 3 ? CPUReadMemory(address) :
                                  0);
      } else {
        cheatsList[x].oldValue = CPUReadHalfWord(address);
        CHEAT_PATCH_ROM_16BIT(address,value);
      }
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
      case 0:
        CPUWriteByte(cheatsList[x].address, (u8)cheatsList[x].oldValue);
        break;
      case 1:
        CPUWriteHalfWord(cheatsList[x].address, (u16)cheatsList[x].oldValue);
        break;
      case 2:
        CPUWriteMemory(cheatsList[x].address, cheatsList[x].oldValue);
        break;
      case 3:
        CHEAT_PATCH_ROM_16BIT(cheatsList[x].address,
                              cheatsList[x].oldValue);  
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
    if(cheatsList[i].status & 1) {
      u32 address = cheatsList[i].address;
      switch(address >> 24) {
      case 8:
      case 9:
        CHEAT_PATCH_ROM_16BIT(address,
                              cheatsList[i].value);
        break;
      }
    }
    cheatsList[i].enabled = true;
    cheatsList[i].status |= 2;
  }
}

void cheatsDisable(int i)
{
  if(i >= 0 && i < cheatsNumber) {
    if(cheatsList[i].status & 1) {
      u32 address = cheatsList[i].address;
      switch(address >> 24) {
      case 8:
      case 9:
        CHEAT_PATCH_ROM_16BIT(address,
                              cheatsList[i].oldValue);
        break;  
      }
    }
    cheatsList[i].enabled = false;
    cheatsList[i].status &= ~2;
  }
}

void cheatsReset()
{
  memcpy(cheatSearch.wRAM, workRAM, 0x40000);
  memcpy(cheatSearch.iRAM, internalRAM, 0x8000);
  memset(cheatSearch.wBITS, 0xFF, 0x40000 >> 3);
  memset(cheatSearch.iBITS, 0xFF, 0x8000 >> 3);
}

void cheatsSearchChange(int compare, int size, bool isSigned)
{
  int inc = 1;

  if(size == SIZE_16)
    inc = 2;
  else if(size == SIZE_32)
    inc = 4;

  if(isSigned) {
    int i;
    for(i = 0; i < 0x40000; i += inc) {
      if(TEST_BIT(cheatSearch.wBITS, i) &&
         COMPARE(compare,
                 SIGNED_DATA(size, workRAM, i),
                 SIGNED_DATA(size, cheatSearch.wRAM, i))) {
      } else {
        BIT_CLEAR(cheatSearch.wBITS, i);
        if(size != SIZE_8)
          BIT_CLEAR(cheatSearch.wBITS, i+1);
        if(size == SIZE_32) {
          BIT_CLEAR(cheatSearch.wBITS, i+2);
          BIT_CLEAR(cheatSearch.wBITS, i+3);
        }
      }
    }

    for(i = 0; i < 0x8000 ; i += inc) {
      if(TEST_BIT(cheatSearch.iBITS, i) &&
         COMPARE(compare,
                 SIGNED_DATA(size, internalRAM, i),
                 SIGNED_DATA(size, cheatSearch.iRAM, i))) {
      } else {
        BIT_CLEAR(cheatSearch.iBITS, i);
        if(size != SIZE_8)
          BIT_CLEAR(cheatSearch.iBITS, i+1);
        if(size == SIZE_32) {
          BIT_CLEAR(cheatSearch.iBITS, i+2);
          BIT_CLEAR(cheatSearch.iBITS, i+3);
        }       
      }
    }    
  } else {
    int i;
    for(i = 0; i < 0x40000; i += inc) {
      if(TEST_BIT(cheatSearch.wBITS, i) &&
         COMPARE(compare,
                 UNSIGNED_DATA(size, workRAM, i),
                 UNSIGNED_DATA(size, cheatSearch.wRAM, i))) {
      } else {
        BIT_CLEAR(cheatSearch.wBITS, i);
        if(size != SIZE_8)
          BIT_CLEAR(cheatSearch.wBITS, i+1);
        if(size == SIZE_32) {
          BIT_CLEAR(cheatSearch.wBITS, i+2);
          BIT_CLEAR(cheatSearch.wBITS, i+3);
        }
      }
    }

    for(i = 0; i <= 0x8000 ; i += inc) {
      if(TEST_BIT(cheatSearch.iBITS, i) &&
         COMPARE(compare,
                 UNSIGNED_DATA(size, internalRAM, i),
                 UNSIGNED_DATA(size, cheatSearch.iRAM, i))) {
      } else {
        BIT_CLEAR(cheatSearch.iBITS, i);
        if(size != SIZE_8)
          BIT_CLEAR(cheatSearch.iBITS, i+1);
        if(size == SIZE_32) {
          BIT_CLEAR(cheatSearch.iBITS, i+2);
          BIT_CLEAR(cheatSearch.iBITS, i+3);
        }       
      }
    }    
  }
}


void cheatsSearchValue(int compare, int size, bool isSigned, u32 value)
{
  int inc = 1;

  if(size == SIZE_16)
    inc = 2;
  else if(size == SIZE_32)
    inc = 4;

  if(isSigned) {
    int i;
    for(i = 0; i < 0x40000 ; i += inc) {
      if(TEST_BIT(cheatSearch.wBITS, i) &&
         COMPARE(compare,
                 SIGNED_DATA(size, workRAM, i),
                 (s32)value)) {
      } else {
        BIT_CLEAR(cheatSearch.wBITS, i);
        if(size != SIZE_8)
          BIT_CLEAR(cheatSearch.wBITS, i+1);
        if(size == SIZE_32) {
          BIT_CLEAR(cheatSearch.wBITS, i+2);
          BIT_CLEAR(cheatSearch.wBITS, i+3);
        }
      }
    }

    for(i = 0; i <= 0x8000 ; i += inc) {
      if(TEST_BIT(cheatSearch.iBITS, i) &&
         COMPARE(compare,
                 SIGNED_DATA(size, internalRAM, i),
                 (s32)value)) {
      } else {
        BIT_CLEAR(cheatSearch.iBITS, i);
        if(size != SIZE_8)
          BIT_CLEAR(cheatSearch.iBITS, i+1);
        if(size == SIZE_32) {
          BIT_CLEAR(cheatSearch.iBITS, i+2);
          BIT_CLEAR(cheatSearch.iBITS, i+3);
        }
      }
    }    
  } else {
    int i;
    for(i = 0; i <= 0x40000 ; i += inc) {
      if(TEST_BIT(cheatSearch.wBITS, i) &&
         COMPARE(compare,
                 UNSIGNED_DATA(size, workRAM, i),
                 value)) {
      } else {
        BIT_CLEAR(cheatSearch.wBITS, i);
        if(size != SIZE_8)
          BIT_CLEAR(cheatSearch.wBITS, i+1);
        if(size == SIZE_32) {
          BIT_CLEAR(cheatSearch.wBITS, i+2);
          BIT_CLEAR(cheatSearch.wBITS, i+3);
        }
      } 
    }

    for(i = 0; i <= 0x8000 ; i += inc) {
      if(TEST_BIT(cheatSearch.iBITS, i) &&
         COMPARE(compare,
                 UNSIGNED_DATA(size, internalRAM, i),
                 value)) {
      } else {
        BIT_CLEAR(cheatSearch.iBITS, i);
        if(size != SIZE_8)
          BIT_CLEAR(cheatSearch.iBITS, i+1);
        if(size == SIZE_32) {
          BIT_CLEAR(cheatSearch.iBITS, i+2);
          BIT_CLEAR(cheatSearch.iBITS, i+3);
        }       
      }
    }    
  }
}

int cheatsGetCount(int size)
{
  int count = 0;
  int i;
  int inc = 1;
  if(size == SIZE_16)
    inc = 2;
  else if(size == SIZE_32)
    inc = 4;

  for(i = 0; i < 0x40000; i += inc) {
    if(TEST_BIT(cheatSearch.wBITS, i))
      count++;
  }

  for(i = 0; i < 0x8000; i += inc) {
    if(TEST_BIT(cheatSearch.iBITS, i))
      count++;
  }
  return count;
}

void cheatsUpdateValues()
{
  int i;
  for(i = 0; i < 0x40000; i++) {
    if(TEST_BIT(cheatSearch.wBITS, i))
      cheatSearch.wRAM[i] = workRAM[i];
  }

  for(i = 0; i < 0x8000; i++) {
    if(TEST_BIT(cheatSearch.iBITS, i))
      cheatSearch.iRAM[i] = internalRAM[i];
  }    
}

void cheatsToUpper(char *s)
{
  while(*s) {
    *s++ = toupper(*s);
  }
}

bool cheatsVerifyCheatCode(char *code, char *desc)
{
  int len = strlen(code);
  if(len != 11 && len != 13 && len != 17) {
    systemMessage(MSG_INVALID_CHEAT_CODE, "Invalid cheat code '%s'", code);
    return false;
  }

  if(code[8] != ':') {
    systemMessage(MSG_INVALID_CHEAT_CODE, "Invalid cheat code '%s'", code);
    return false;    
  }

  cheatsToUpper(code);

  int i;
  for(i = 0; i < 8; i++) {
    if(!CHEAT_IS_HEX(code[i])) {
      // wrong cheat
      systemMessage(MSG_INVALID_CHEAT_CODE,
                    "Invalid cheat code '%s'", code);
      return false;
    }
  }
  for(i = 9; i < len; i++) {
    if(!CHEAT_IS_HEX(code[i])) {
      // wrong cheat
      systemMessage(MSG_INVALID_CHEAT_CODE,
                    "Invalid cheat code '%s'", code);
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
                  "Invalid cheat code address: %08x",
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
  cheatsAdd(code, desc, address, value, type, type, true);
  return true;
}

void cheatsAddCheatCode(char *code, char *desc)
{
  cheatsVerifyCheatCode(code, desc);
}

void cheatsDecryptGSACode(u32& address, u32& value) 
{
  u32 rollingseed = 0xC6EF3720;
  u32 seeds[] =  { 0x09F4FBBD, 0x9681884A, 0x352027E9, 0xF3DEE5A7} ; 
  
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

void cheatsAddGSACode(char *code, char *desc)
{
  if(strlen(code) != 16) {
    // wrong cheat
    systemMessage(MSG_INVALID_GSA_CODE,
                  "Invalid GSA code. Format is XXXXXXXXYYYYYYYY");
    return;
  }
  cheatsToUpper(code);
  
  int i;
  for(i = 0; i < 16; i++) {
    if(!CHEAT_IS_HEX(code[i])) {
      // wrong cheat
      systemMessage(MSG_INVALID_GSA_CODE,
                    "Invalid GSA code. Format is XXXXXXXXYYYYYYYY");
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

  cheatsDecryptGSACode(address, value);
  
  int type = (address >> 28) & 15;

  if(value == 0x1DC0DE) {
    u32 gamecode = *((u32 *)&rom[0xac]);
    if(gamecode != address) {
      char buffer[5];
      *((u32 *)buffer) = address;
      buffer[4] = 0;
      char buffer2[5];
      *((u32 *)buffer2) = *((u32 *)&rom[0xac]);
      buffer2[4] = 0;
      systemMessage(MSG_GBA_CODE_WARNING, "Warning: cheats are for game %s. Current game is %s.\nCodes may not work correctly.",
                    buffer, buffer2);
    }
  }
  
  switch(type) {
  case 0:
  case 1:
  case 2:
    cheatsAdd(code, desc, address & 0x0FFFFFFF, value, 256, type,true);
    break;
  case 6:
    address <<= 1;
    type = (address >> 28) & 15;
    if(type == 0x0c) {
      cheatsAdd(code, desc, address & 0x0FFFFFFF, value, 256, 3, true);
      break;
    }
    // unsupported code
    cheatsAdd(code, desc, 0, 0, 256, 0, false);
    break;
  case 8:
    switch((address >> 20) & 15) {
    case 1:
      cheatsAdd(code, desc, address & 0x0F0FFFFF, value, 256, 4, false);
      break;
    case 2:
      cheatsAdd(code, desc, address & 0x0F0FFFFF, value, 256, 5, false);
      break;
    case 3:
      cheatsAdd(code, desc, address & 0x0F0FFFFF, value, 256, 6, false);
    default:
      // unsupported code
      cheatsAdd(code, desc, 0, 0, 256, 0, false);
      break;
    }
    break;
  case 0x0d:
    if(address != 0xDEADFACE) {
      cheatsAdd(code, desc, address & 0x0FFFFFFF, value, 256, 8, false);
    }
    break;
  default:
    // unsupported code
    cheatsAdd(code, desc, 0, 0, 256, 0, false);
    break;
  }
}

bool cheatsImportGSACodeFile(char *name, int game)
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
        cheatsAddGSACode(code, desc);
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

void cheatsAddCBACode(char *code, char *desc)
{
  if(strlen(code) != 13) {
    // wrong cheat
    systemMessage(MSG_INVALID_CBA_CODE,
                  "Invalid CBA code. Format is XXXXXXXX YYYY.");
    return;
  }
  
  cheatsToUpper(code);
  
  int i;
  for(i = 0; i < 8; i++) {
    if(!CHEAT_IS_HEX(code[i])) {
      // wrong cheat
      systemMessage(MSG_INVALID_CBA_CODE,
                    "Invalid CBA code. Format is XXXXXXXX YYYY.");
      return;
    }
  }

  if(code[8] != ' ') {
    systemMessage(MSG_INVALID_CBA_CODE,
                  "Invalid CBA code. Format is XXXXXXXX YYYY.");
    return;    
  }
  
  for(i = 9; i < 13; i++) {
    if(!CHEAT_IS_HEX(code[i])) {
      // wrong cheat
      systemMessage(MSG_INVALID_CBA_CODE,
                    "Invalid CBA code. Format is XXXXXXXX YYYY.");
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
    cheatsAdd(code, desc, 0, 0, 512, 0, false);
  } else {
    if(cheatsCBAShouldDecrypt())
      cheatsCBADecrypt(array);

    address = *((u32 *)array);
    value = *((u16 *)&array[4]);
    
    int type = (address >> 28) & 15;

    if(cheatsNumber) {
      int n = cheatsNumber - 1;

      switch(cheatsList[n].size) {
      case 9:
        cheatsAdd(code, desc, address, value, 512, 0xffff, false);
        return;
      }
    }
    
    switch(type) {
    case 0x00:
      {
        if(!cheatsCBATableGenerated)
          cheatsCBAGenTable();
        u32 crc = cheatsCBACalcCRC(rom, 0x10000);
        if(crc != address) {
          systemMessage(MSG_CBA_CODE_WARNING,
                        "Warning: Codes seem to be for a different game.\nCodes may not work correctly.");
        }
        cheatsAdd(code, desc, 0, 0, 512, 0, false);
      }
      break;
    case 0x03:
      cheatsAdd(code, desc, address & 0x0FFFFFFF, value, 512, 0, true);
      break;
    case 0x04:
      cheatsAdd(code, desc, address & 0x0FFFFFFF, value, 512, 9, true);
      break;
    case 0x06:
      cheatsAdd(code, desc, address & 0x0FFFFFFF, value, 512, 11, true);
      break;
    case 0x07:
      cheatsAdd(code, desc, address & 0x0FFFFFFF, value, 512, 8, true);
      break;
    case 0x08:
      cheatsAdd(code, desc, address & 0x0FFFFFFF, value, 512, 1, true);
      break;
    case 0x0a:
      cheatsAdd(code, desc, address & 0x0FFFFFFF, value, 512, 10, true);
      break;
    case 0x0d:
      cheatsAdd(code, desc, address & 0x0FFFFFFF, value, 512, 7, true);
      break;
    default:
      // unsupported code
      cheatsAdd(code, desc, 0, 0, 512, 0, false);
      break;
    }
  }
}

void cheatsSaveGame(gzFile file)
{
  CPUWriteInt(file, cheatsNumber);
  
  gzwrite(file, cheatsList, sizeof(cheatsList));
}

void cheatsReadGame(gzFile file)
{
  cheatsNumber = 0;
  
  cheatsNumber = CPUReadInt(file);

  gzread(file, cheatsList, sizeof(cheatsList));

  bool firstCodeBreaker = true;
  
  for(int i = 0; i < cheatsNumber; i++) {
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

void cheatsSaveCheatList(char *file)
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

bool cheatsLoadCheatList(char *file)
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
                  "Unsupported cheat list version %d", version);
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
                  "Unsupported cheat list type %d", type);
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
  u32 finalMask = 0;
  
  if(mask & 0x00000001) {
    finalMask |= 0x000000ff;
  }
  if(mask & 0x00000100) {
    finalMask |= 0x0000ff00;
  }
  if(mask & 0x00010000) {
    finalMask |= 0x00ff0000;
  }
  if(mask & 0x01000000) {
    finalMask |= 0xff000000;
  }

  *address = (*address & finalMask) | (value & (~finalMask));
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

  u16 finalMask = 0;
  
  if(mask & 0x00000001) {
    finalMask |= 0x000000ff;
  }
  if(mask & 0x00000100) {
    finalMask |= 0x0000ff00;
  }

  *address = (*address & finalMask) | (value & (~finalMask));
}

void cheatsWriteByte(u8 *address, u8 value)
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
