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
#include <stdio.h>
#include <memory.h>
#include "GBA.h"
#include "Globals.h"
#include "Flash.h"
#include "Sram.h"

#define FLASH_READ_ARRAY         0
#define FLASH_CMD_1              1
#define FLASH_CMD_2              2
#define FLASH_AUTOSELECT         3
#define FLASH_CMD_3              4
#define FLASH_CMD_4              5
#define FLASH_CMD_5              6
#define FLASH_ERASE_COMPLETE     7
#define FLASH_PROGRAM            8

u8 flashSaveMemory[0x20000];
int flashState = FLASH_READ_ARRAY;
int flashReadState = FLASH_READ_ARRAY;
int flashSize = 0x10000;
int flashDeviceID = 0x1b;
int flashManufacturerID = 0x32;

variable_desc flashSaveData[] = {
  { &flashState, sizeof(int) },
  { &flashReadState, sizeof(int) },
  { &flashSaveMemory[0], 0x10000 },
  { NULL, 0 }
};

variable_desc flashSaveData2[] = {
  { &flashState, sizeof(int) },
  { &flashReadState, sizeof(int) },
  { &flashSize, sizeof(int) },  
  { &flashSaveMemory[0], 0x20000 },
  { NULL, 0 }
};

void flashReset()
{
  flashState = FLASH_READ_ARRAY;
  flashReadState = FLASH_READ_ARRAY;
}

void flashSaveGame(gzFile gzFile)
{
  CPUWriteData(gzFile, flashSaveData2);
}

void flashReadGame(gzFile gzFile, int version)
{
  if(version < SAVE_GAME_VERSION_5)
    CPUReadData(gzFile, flashSaveData);
  else
    CPUReadData(gzFile, flashSaveData2);
}

void flashSetSize(int size)
{
  flashSize = size;
  if(size == 0x10000) {
    flashDeviceID = 0x1b;
    flashManufacturerID = 0x32;
  } else {
    flashDeviceID = 0x09;
    flashManufacturerID = 0xc2;
  }
}

u8 flashRead(u32 address)
{
  //  printf("Reading %08x from %08x\n", address, reg[15].I);
  //  printf("Current read state is %d\n", flashReadState);
  if(flashSize == 0x10000)
    address &= 0xFFFF;
  else
    address &= 0x1FFFF;

  switch(flashReadState) {
  case FLASH_READ_ARRAY:
    return flashSaveMemory[address];
  case FLASH_AUTOSELECT:
    switch(address & 0xFF) {
    case 0:
      // manufacturer ID
      return flashManufacturerID;
    case 1:
      // device ID
      return flashDeviceID;
    }
    break;
  case FLASH_ERASE_COMPLETE:
    flashState = FLASH_READ_ARRAY;
    flashReadState = FLASH_READ_ARRAY;
    return 0xFF;
  };
  return 0;
}

void flashSaveDecide(u32 address, u8 byte)
{
  if(address == 0x0e005555) {
    saveType = 2;
    cpuSaveGameFunc = flashWrite;
  } else {
    saveType = 1;
    cpuSaveGameFunc = sramWrite;
  }

  (*cpuSaveGameFunc)(address, byte);
}

void flashWrite(u32 address, u8 byte)
{
  //  printf("Writing %02x at %08x\n", byte, address);
  //  printf("Current state is %d\n", flashState);
  if(flashSize == 0x10000)
    address &= 0xFFFF;
  else
    address = address & 0x1FFFF;
  switch(flashState) {
  case FLASH_READ_ARRAY:
    if(address == 0x5555 && byte == 0xAA)
      flashState = FLASH_CMD_1;
    break;
  case FLASH_CMD_1:
    if(address == 0x2AAA && byte == 0x55)
      flashState = FLASH_CMD_2;
    else
      flashState = FLASH_READ_ARRAY;
    break;
  case FLASH_CMD_2:
    if(address == 0x5555) {
      if(byte == 0x90) {
        flashState = FLASH_AUTOSELECT;
        flashReadState = FLASH_AUTOSELECT;
      } else if(byte == 0x80) {
        flashState = FLASH_CMD_3;
      } else if(byte == 0xF0) {
        flashState = FLASH_READ_ARRAY;
        flashReadState = FLASH_READ_ARRAY;
      } else if(byte == 0xA0) {
        flashState = FLASH_PROGRAM;
      } else {
        flashState = FLASH_READ_ARRAY;
        flashReadState = FLASH_READ_ARRAY;
      }
    } else {
      flashState = FLASH_READ_ARRAY;
      flashReadState = FLASH_READ_ARRAY;
    }
    break;
  case FLASH_CMD_3:
    if(address == 0x5555 && byte == 0xAA) {
      flashState = FLASH_CMD_4;
    } else {
      flashState = FLASH_READ_ARRAY;
      flashReadState = FLASH_READ_ARRAY;
    }
    break;
  case FLASH_CMD_4:
    if(address == 0x2AAA && byte == 0x55) {
      flashState = FLASH_CMD_5;
    } else {
      flashState = FLASH_READ_ARRAY;
      flashReadState = FLASH_READ_ARRAY;
    }
    break;
  case FLASH_CMD_5:
    if(byte == 0x30) {
      // SECTOR ERASE
      memset(&flashSaveMemory[address & 0xF000], 0, 0x1000);
      flashReadState = FLASH_ERASE_COMPLETE;
    } else if(byte == 0x10) {
      // CHIP ERASE
      memset(flashSaveMemory, 0, 0x10000);
      flashReadState = FLASH_ERASE_COMPLETE;
    } else {
      flashState = FLASH_READ_ARRAY;
      flashReadState = FLASH_READ_ARRAY;
    }
    break;
  case FLASH_AUTOSELECT:
    if(byte == 0xF0) {
      flashState = FLASH_READ_ARRAY;
      flashReadState = FLASH_READ_ARRAY;
    } else if(address == 0x5555 && byte == 0xAA)
      flashState = FLASH_CMD_1;
    else {
      flashState = FLASH_READ_ARRAY;
      flashReadState = FLASH_READ_ARRAY;
    }
    break;
  case FLASH_PROGRAM:
    flashSaveMemory[address] = byte;
    flashState = FLASH_READ_ARRAY;
    flashReadState = FLASH_READ_ARRAY;
    break;
  }
}
