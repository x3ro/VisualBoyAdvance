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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "../System.h"
#include "../Cheats.h"
#include "../NLS.h"
#include "../Util.h"

#include "gbCheats.h"
#include "gbGlobals.h"

gbCheat gbCheatList[100];
int gbCheatNumber = 0;
bool gbCheatMap[0x10000];

int gbCheatsSearchCount = 0;
GbCheatsSearchMap gbCheatsSearchMap[3];

#define GBCHEAT_IS_HEX(a) ( ((a)>='A' && (a) <='F') || ((a) >='0' && (a) <= '9'))
#define GBCHEAT_HEX_VALUE(a) ( (a) >= 'A' ? (a) - 'A' + 10 : (a) - '0')

void gbCheatUpdateMap()
{
  memset(gbCheatMap, 0, 0x10000);

  for(int i = 0; i < gbCheatNumber; i++) {
    if(gbCheatList[i].enabled)
      gbCheatMap[gbCheatList[i].address] = true;
  }
}

void gbCheatsSaveGame(gzFile gzFile)
{
  utilWriteInt(gzFile, gbCheatNumber);
  if(gbCheatNumber)
    utilGzWrite(gzFile, &gbCheatList[0], sizeof(gbCheat)*gbCheatNumber);
}

void gbCheatsReadGame(gzFile gzFile, int version)
{
  if(version <= 8) {
    int gbGgOn = utilReadInt(gzFile);

    if(gbGgOn) {
      int n = utilReadInt(gzFile);
      gbXxCheat tmpCheat;
      for(int i = 0; i < n; i++) {
        utilGzRead(gzFile,&tmpCheat, sizeof(gbXxCheat));
        gbAddGgCheat(tmpCheat.cheatCode, tmpCheat.cheatDesc);
      }
    }
  
    int gbGsOn = utilReadInt(gzFile);
    
    if(gbGsOn) {
      int n = utilReadInt(gzFile);
      gbXxCheat tmpCheat;
      for(int i = 0; i < n; i++) {
        utilGzRead(gzFile,&tmpCheat, sizeof(gbXxCheat));
        gbAddGsCheat(tmpCheat.cheatCode, tmpCheat.cheatDesc);
      }
    }
  } else {
    gbCheatNumber = utilReadInt(gzFile);

    if(gbCheatNumber) {
      utilGzRead(gzFile, &gbCheatList[0], sizeof(gbCheat)*gbCheatNumber);
    }
  }

  gbCheatUpdateMap();
}

void gbCheatsSaveCheatList(char *file)
{
  if(gbCheatNumber == 0)
    return;
  FILE *f = fopen(file, "wb");
  if(f == NULL)
    return;
  int version = 1;
  fwrite(&version, 1, sizeof(version), f);
  int type = 1;
  fwrite(&type, 1, sizeof(type), f);
  fwrite(&gbCheatNumber, 1, sizeof(gbCheatNumber), f);
  fwrite(gbCheatList, 1, sizeof(gbCheatList), f);
  fclose(f);
}

bool gbCheatsLoadCheatList(char *file)
{
  gbCheatNumber = 0;

  gbCheatUpdateMap();
  
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

  if(type != 1) {
    systemMessage(MSG_UNSUPPORTED_CHEAT_LIST_TYPE,
                  "Unsupported cheat list type %d", type);
    fclose(f);
    return false;
  }
  
  if(fread(&count, 1, sizeof(count), f) != sizeof(count)) {
    fclose(f);
    return false;
  }
  
  if(fread(gbCheatList, 1, sizeof(gbCheatList), f) != sizeof(gbCheatList)) {
    fclose(f);
    return false;
  }

  gbCheatNumber = count;
  gbCheatUpdateMap();
  
  return true;
}

void gbCheatToUpper(char *s)
{
  while(*s) {
    *s++ = toupper(*s);
  }
}

bool gbVerifyGsCode(char *code)
{
  int len = strlen(code);

  if(len == 0)
    return true;
  
  if(len != 8)
    return false;

  for(int i = 0; i < 8; i++)
    if(!GBCHEAT_IS_HEX(code[i]))
      return false;

  int address = GBCHEAT_HEX_VALUE(code[6]) << 12 |
    GBCHEAT_HEX_VALUE(code[7]) << 8 |
    GBCHEAT_HEX_VALUE(code[4]) << 4 |
    GBCHEAT_HEX_VALUE(code[5]);

  if(address < 0xa000 ||
     address > 0xdfff)
    return false;

  return true;
}

void gbAddGsCheat(char *code, char *desc)
{
  if(gbCheatNumber > 99) {
    systemMessage(MSG_MAXIMUM_NUMBER_OF_CHEATS,
                  "Maximum number of cheats reached.");
    return;
  }

  gbCheatToUpper(code);
  
  if(!gbVerifyGsCode(code)) {
    systemMessage(MSG_INVALID_GAMESHARK_CODE,
                  "Invalid GameShark code: %s", code);
    return;
  }
  
  int i = gbCheatNumber;

  strcpy(gbCheatList[i].cheatCode, code);
  strcpy(gbCheatList[i].cheatDesc, desc);
  
  gbCheatList[i].code = GBCHEAT_HEX_VALUE(code[0]) << 4 |
    GBCHEAT_HEX_VALUE(code[1]);

  gbCheatList[i].value = GBCHEAT_HEX_VALUE(code[2]) << 4 |
    GBCHEAT_HEX_VALUE(code[3]);

  gbCheatList[i].address = GBCHEAT_HEX_VALUE(code[6]) << 12 |
    GBCHEAT_HEX_VALUE(code[7]) << 8 |
    GBCHEAT_HEX_VALUE(code[4]) << 4 |
    GBCHEAT_HEX_VALUE(code[5]);

  gbCheatList[i].compare = 0;

  gbCheatList[i].enabled = true;
  
  gbCheatMap[gbCheatList[i].address] = true;
  
  gbCheatNumber++;
}

bool gbVerifyGgCode(char *code)
{
  int len = strlen(code);

  if(len != 11 &&
     len != 7 &&
     len != 6 &&
     len != 0)
    return false;

  if(len == 0)
    return true;
  
  if(!GBCHEAT_IS_HEX(code[0]))
    return false;
  if(!GBCHEAT_IS_HEX(code[1]))
    return false;
  if(!GBCHEAT_IS_HEX(code[2]))
    return false;
  if(code[3] != '-')
    return false;
  if(!GBCHEAT_IS_HEX(code[4]))
    return false;
  if(!GBCHEAT_IS_HEX(code[5]))
    return false;
  if(!GBCHEAT_IS_HEX(code[6]))
    return false;
  if(code[7] != 0) {
    if(code[7] != '-')
      return false;
    if(code[8] != 0) {
      if(!GBCHEAT_IS_HEX(code[8]))
        return false;
      if(!GBCHEAT_IS_HEX(code[9]))
        return false;
      if(!GBCHEAT_IS_HEX(code[10]))
        return false;
    }
  }

  //  int replace = (GBCHEAT_HEX_VALUE(code[0]) << 4) +
  //    GBCHEAT_HEX_VALUE(code[1]);

  int address = (GBCHEAT_HEX_VALUE(code[2]) << 8) +
    (GBCHEAT_HEX_VALUE(code[4]) << 4) +
    (GBCHEAT_HEX_VALUE(code[5])) +
    ((GBCHEAT_HEX_VALUE(code[6]) ^ 0x0f) << 12);

  if(address >= 0x8000 && address <= 0x9fff)
    return false;

  if(address >= 0xc000)
    return false;

  if(code[7] == 0 || code[8] == '0')
    return true;

  int compare = (GBCHEAT_HEX_VALUE(code[8]) << 4) +
    (GBCHEAT_HEX_VALUE(code[10]));
  compare = compare ^ 0xff;
  compare = (compare >> 2) | ( (compare << 6) & 0xc0);
  compare ^= 0x45;

  int cloak = (GBCHEAT_HEX_VALUE(code[8])) ^ (GBCHEAT_HEX_VALUE(code[9]));
  
  if(cloak >=1 && cloak <= 7)
    return false;

  return true;
}

void gbAddGgCheat(char *code, char *desc)
{
  if(gbCheatNumber > 99) {
    systemMessage(MSG_MAXIMUM_NUMBER_OF_CHEATS,
                  "Maximum number of cheats reached.");
    return;
  }

  gbCheatToUpper(code);
  
  if(!gbVerifyGgCode(code)) {
    systemMessage(MSG_INVALID_GAMEGENIE_CODE,
                  "Invalid GameGenie code: %s", code);
    return;
  }
  
  int i = gbCheatNumber;

  int len = strlen(code);
  
  strcpy(gbCheatList[i].cheatCode, code);
  strcpy(gbCheatList[i].cheatDesc, desc);

  gbCheatList[i].code = 1;
  gbCheatList[i].value = (GBCHEAT_HEX_VALUE(code[0]) << 4) +
    GBCHEAT_HEX_VALUE(code[1]);
  
  gbCheatList[i].address = (GBCHEAT_HEX_VALUE(code[2]) << 8) +
    (GBCHEAT_HEX_VALUE(code[4]) << 4) +
    (GBCHEAT_HEX_VALUE(code[5])) +
    ((GBCHEAT_HEX_VALUE(code[6]) ^ 0x0f) << 12);

  gbCheatList[i].compare = 0;
  
  if(len != 7 && len != 8) {
    
    int compare = (GBCHEAT_HEX_VALUE(code[8]) << 4) +
      (GBCHEAT_HEX_VALUE(code[10]));
    compare = compare ^ 0xff;
    compare = (compare >> 2) | ( (compare << 6) & 0xc0);
    compare ^= 0x45;

    gbCheatList[i].compare = compare;
    gbCheatList[i].code = 0;
  }

  gbCheatList[i].enabled = true;
  
  gbCheatMap[gbCheatList[i].address] = true;
  
  gbCheatNumber++;
}

void gbCheatRemove(int i)
{
  if(i < 0 || i >= gbCheatNumber) {
    systemMessage(MSG_INVALID_CHEAT_TO_REMOVE,
                  "Invalid cheat to remove %d", i);
    return;
  }
  
  if((i+1) <  gbCheatNumber) {
    memcpy(&gbCheatList[i], &gbCheatList[i+1], sizeof(gbCheat)*
           (gbCheatNumber-i-1));
  }
  
  gbCheatNumber--;

  gbCheatUpdateMap();
}

void gbCheatRemoveAll()
{
  gbCheatNumber = 0;
  gbCheatUpdateMap();
}

void gbCheatEnable(int i)
{
  if(i >=0 && i < gbCheatNumber) {
    if(!gbCheatList[i].enabled) {
      gbCheatList[i].enabled = true;
      gbCheatUpdateMap();
    }
  }
}

void gbCheatDisable(int i)
{
  if(i >=0 && i < gbCheatNumber) {
    if(gbCheatList[i].enabled) {
      gbCheatList[i].enabled = false;
      gbCheatUpdateMap();
    }
  }
}

bool gbCheatReadGSCodeFile(char *fileName)
{
  FILE *file = fopen(fileName, "rb");
    
  if(!file) {
    systemMessage(MSG_CANNOT_OPEN_FILE, "Cannot open file %s", fileName);
    return false;
  }
  
  fseek(file, 0x18, SEEK_SET);
  int count = 0;
  fread(&count, 1, 2, file);
  int dummy = 0;
  gbCheatRemoveAll();
  char desc[13];
  char code[9];
  int i;
  for(i = 0; i < count; i++) {
    fread(&dummy, 1, 2, file);    
    fread(desc, 1, 12, file);
    desc[12] = 0;
    fread(code, 1, 8, file);
    code[8] = 0;
    gbAddGsCheat(code, desc);
  }

  for(i = 0; i < gbCheatNumber; i++)
    gbCheatDisable(i);

  fclose(file);
  return true;
}

u8 gbCheatRead(u16 address)
{
  for(int i = 0; i < gbCheatNumber; i++) {
    if(gbCheatList[i].enabled && gbCheatList[i].address == address) {
      switch(gbCheatList[i].code) {
      case 0x100: // GameGenie support
        if(gbMemoryMap[address>>12][address&0xFFF] == gbCheatList[i].compare)
          return gbCheatList[i].value;
        break;
      case 0x00:
      case 0x01:
      case 0x80:
        return gbCheatList[i].value;
      case 0x90:
      case 0x91:
      case 0x92:
      case 0x93:
      case 0x94:
      case 0x95:
      case 0x96:
      case 0x97:
        if(address >= 0xd000 && address < 0xe000) {
          if(((gbMemoryMap[0x0d] - gbWram)/0x1000) ==
             (gbCheatList[i].code - 0x90))
            return gbCheatList[i].value;
        } else
          return gbCheatList[i].value;
      }
    }
  }
  return gbMemoryMap[address>>12][address&0xFFF];
}

void gbCheatsCleanup()
{
  int i;
  for(i = 0; i < gbCheatsSearchCount; i++) {
    if(gbCheatsSearchMap[i].bits != NULL) {
      free(gbCheatsSearchMap[i].bits);
      gbCheatsSearchMap[i].bits = NULL;
    }
    if(gbCheatsSearchMap[i].data != NULL) {
      free(gbCheatsSearchMap[i].data);
      gbCheatsSearchMap[i].data = NULL;
    }
    gbCheatsSearchMap[i].memory = NULL;
    gbCheatsSearchMap[i].address = 0;
    gbCheatsSearchMap[i].mask = 0;
  }
}

void gbCheatsInitialize()
{
  int i;
  gbCheatsCleanup();
  i = 0;
  
  if(gbRamSize) {
    gbCheatsSearchMap[i].address = 0xa000;
    if(gbRam)
      gbCheatsSearchMap[i].memory = gbRam;
    else
      gbCheatsSearchMap[i].memory = &gbMemory[0xa000];
    gbCheatsSearchMap[i].data = (u8*)malloc(gbRamSize);
    memcpy(gbCheatsSearchMap[i].data, gbCheatsSearchMap[i].memory, gbRamSize);
    gbCheatsSearchMap[i].mask = gbRamSize - 1;
    gbCheatsSearchMap[i].bits = (u32 *)malloc(gbRamSize >> 3);
    memset(gbCheatsSearchMap[i].bits, 255, gbRamSize>>3);
    i++;
  }
  if(gbCgbMode) {
    gbCheatsSearchMap[i].address = 0xc000;
    gbCheatsSearchMap[i].memory = &gbMemory[0xc000];
    gbCheatsSearchMap[i].data = (u8*)malloc(0x1000);
    memcpy(gbCheatsSearchMap[i].data, gbCheatsSearchMap[i].memory, 0x1000);
    gbCheatsSearchMap[i].mask = 0xfff;
    gbCheatsSearchMap[i].bits = (u32 *)malloc(0x1000 >> 3);
    memset(gbCheatsSearchMap[i].bits, 255, 0x1000 >> 3);    
    i++;
    gbCheatsSearchMap[i].address = 0xd000;
    gbCheatsSearchMap[i].memory = gbWram;
    gbCheatsSearchMap[i].data = (u8*)malloc(0x8000);
    memcpy(gbCheatsSearchMap[i].data, gbCheatsSearchMap[i].memory, 0x8000);    
    gbCheatsSearchMap[i].mask = 0x7fff;
    gbCheatsSearchMap[i].bits = (u32 *)malloc(0x8000 >> 3);
    memset(gbCheatsSearchMap[i].bits, 0xff, 0x8000>>3);
    i++;        
  } else {
    gbCheatsSearchMap[i].address = 0xc000;
    gbCheatsSearchMap[i].memory = &gbMemory[0xc000];
    gbCheatsSearchMap[i].data = (u8*)malloc(0x2000);
    memcpy(gbCheatsSearchMap[i].data, gbCheatsSearchMap[i].memory, 0x2000);    
    gbCheatsSearchMap[i].mask = 0x1fff;
    gbCheatsSearchMap[i].bits = (u32 *)malloc(0x2000 >> 3);
    memset(gbCheatsSearchMap[i].bits, 255, 0x2000>>3);    
    i++;    
  }

  gbCheatsSearchCount = i;
}

void gbCheatsSearchChange(int compare, int size, bool isSigned)
{
  int inc = 1;

  if(size == SIZE_16)
    inc = 2;
  else if(size == SIZE_32)
    inc = 4;

  if(isSigned) {
    int i;
    for(i = 0; i < gbCheatsSearchCount; i++) {
      int j;
      int end = gbCheatsSearchMap[i].mask + 1;
      end -= (inc - 1);
      for(j = 0; j < end; j++) {
        if(TEST_BIT(gbCheatsSearchMap[i].bits, j) &&
           COMPARE(compare,
                   SIGNED_DATA(size, gbCheatsSearchMap[i].memory, j),
                   SIGNED_DATA(size, gbCheatsSearchMap[i].data, j))) {
        } else {
          BIT_CLEAR(gbCheatsSearchMap[i].bits, j);
        }
      }
    }
  } else {
    int i;
    for(i = 0; i < gbCheatsSearchCount; i++) {
      int j;
      int end = gbCheatsSearchMap[i].mask + 1;
      end -= (inc - 1);
      for(j = 0; j < end; j++) {
        if(TEST_BIT(gbCheatsSearchMap[i].bits, j) &&
           COMPARE(compare,
                   UNSIGNED_DATA(size, gbCheatsSearchMap[i].memory, j),
                   UNSIGNED_DATA(size, gbCheatsSearchMap[i].data, j))) {
        } else {
          BIT_CLEAR(gbCheatsSearchMap[i].bits, j);
        }
      }
    }
  }
}

void gbCheatsSearchValue(int compare, int size, bool isSigned, u32 value)
{
  int inc = 1;

  if(size == SIZE_16)
    inc = 2;
  else if(size == SIZE_32)
    inc = 4;

  if(isSigned) {
    int i;
    for(i = 0; i < gbCheatsSearchCount; i++) {
      int j;
      int end = gbCheatsSearchMap[i].mask + 1;
      end -= (inc - 1);
      for(j = 0; j < end; j++) {
        if(TEST_BIT(gbCheatsSearchMap[i].bits, j) &&
           COMPARE(compare,
                   SIGNED_DATA(size, gbCheatsSearchMap[i].memory, j),
                   (s32)value)) {
        } else {
          BIT_CLEAR(gbCheatsSearchMap[i].bits, j);
        }
      }
    }
  } else {
    int i;
    for(i = 0; i < gbCheatsSearchCount; i++) {
      int j;
      int end = gbCheatsSearchMap[i].mask + 1;
      end -= (inc - 1);
      for(j = 0; j < end; j++) {
        if(TEST_BIT(gbCheatsSearchMap[i].bits, j) &&
           COMPARE(compare,
                   UNSIGNED_DATA(size, gbCheatsSearchMap[i].memory, j),
                   (u32)value)) {
        } else {
          BIT_CLEAR(gbCheatsSearchMap[i].bits, j);
        }
      }
    }
  }
}

int gbCheatsGetCount(int size)
{
  int count = 0;
  int i;
  int inc = 1;
  if(size == SIZE_16)
    inc = 2;
  else if(size == SIZE_32)
    inc = 4;

  for(i = 0; i < gbCheatsSearchCount; i++) {
    int j;
    int end = gbCheatsSearchMap[i].mask + 1;
    end -= (inc - 1);
    for(j = 0; j < end; j++) {
      if(TEST_BIT(gbCheatsSearchMap[i].bits, j))
        count++;
    }
  }
  return count;
}

void gbCheatsUpdateValues()
{
  int i;
  for(i = 0; i < gbCheatsSearchCount; i++) {
    int j;
    int end = gbCheatsSearchMap[i].mask + 1;
    for(j = 0; j < end; j++) {
      if(TEST_BIT(gbCheatsSearchMap[i].bits, j))
        gbCheatsSearchMap[i].data[j] = gbCheatsSearchMap[i].memory[j];
    }
  }
}
