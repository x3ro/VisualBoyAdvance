// -*- C++ -*-
// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2004-2006 Forgotten and the VBA development team

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

#include <time.h>

typedef struct mapperMBC1 {
  int mapperRAMEnable;
  int mapperROMBank;
  int mapperRAMBank;
  int mapperMemoryModel;
  int mapperROMHighAddress;
  int mapperRAMAddress;
  int mapperRomBank0Remapping;
} mapperMBC1;

typedef struct mapperMBC2 {
  int mapperRAMEnable;
  int mapperROMBank;
} mapperMBC2;

typedef struct mapperMBC3 {
  int mapperRAMEnable;
  int mapperROMBank;
  int mapperRAMBank;
  int mapperRAMAddress;
  int mapperClockLatch;
  int mapperClockRegister;
  int mapperSeconds;
  int mapperMinutes;
  int mapperHours;
  int mapperDays;
  int mapperControl;
  int mapperLSeconds;
  int mapperLMinutes;
  int mapperLHours;
  int mapperLDays;
  int mapperLControl;
  time_t mapperLastTime;
} mapperMBC3;

typedef struct mapperMBC5 {
  int mapperRAMEnable;
  int mapperROMBank;
  int mapperRAMBank;
  int mapperROMHighAddress;
  int mapperRAMAddress;
  int isRumbleCartridge;
} mapperMBC5;

typedef struct mapperMBC7 {
  int mapperRAMEnable;
  int mapperROMBank;
  int mapperRAMBank;
  int mapperRAMAddress;
  int cs;
  int sk;
  int state;
  int buffer;
  int idle;
  int count;
  int code;
  int address;
  int writeEnable;
  int value;
} mapperMBC7;

typedef struct mapperHuC1 {
  int mapperRAMEnable;
  int mapperROMBank;
  int mapperRAMBank;
  int mapperMemoryModel;
  int mapperROMHighAddress;
  int mapperRAMAddress;
} mapperHuC1;

typedef struct mapperHuC3 {
  int mapperRAMEnable;
  int mapperROMBank;
  int mapperRAMBank;
  int mapperRAMAddress;
  int mapperAddress;
  int mapperRAMFlag;
  int mapperRAMValue;
  int mapperRegister1;
  int mapperRegister2;
  int mapperRegister3;
  int mapperRegister4;
  int mapperRegister5;
  int mapperRegister6;
  int mapperRegister7;
  int mapperRegister8;
} mapperHuC3;

typedef struct mapperTAMA5 {
  int mapperRAMEnable;
  int mapperROMBank;
  int mapperRAMBank;
  int mapperRAMAddress;
  int mapperRamByteSelect;
  int mapperCommandNumber;
  int mapperLastCommandNumber;
  int mapperCommands[0x10];
  int mapperRegister;
  int mapperClockLatch;
  int mapperClockRegister;
  int mapperSeconds;
  int mapperMinutes;
  int mapperHours;
  int mapperDays;
  int mapperMonths;
  int mapperYears;
  int mapperControl;
  int mapperLSeconds;
  int mapperLMinutes;
  int mapperLHours;
  int mapperLDays;
  int mapperLMonths;
  int mapperLYears;
  int mapperLControl;
  time_t mapperLastTime;
} mapperTAMA5;

typedef struct mapperMMM01 {
  int mapperRAMEnable;
  int mapperROMBank;
  int mapperRAMBank;
  int mapperMemoryModel;
  int mapperROMHighAddress;
  int mapperRAMAddress;
  int mapperRomBank0Remapping;
} mapperMMM01;

typedef struct mapperGS3 {
  int mapperROMBank;
} mapperGS3;

extern mapperMBC1 gbDataMBC1;
extern mapperMBC2 gbDataMBC2;
extern mapperMBC3 gbDataMBC3;
extern mapperMBC5 gbDataMBC5;
extern mapperHuC1 gbDataHuC1;
extern mapperHuC3 gbDataHuC3;
extern mapperTAMA5 gbDataTAMA5;
extern mapperMMM01 gbDataMMM01;
extern mapperGS3 gbDataGS3;

void mapperMBC1ROM(u16,u8);
void mapperMBC1RAM(u16,u8);
u8 mapperMBC1ReadRAM(u16);
void mapperMBC2ROM(u16,u8);
void mapperMBC2RAM(u16,u8);
void mapperMBC3ROM(u16,u8);
void mapperMBC3RAM(u16,u8);
u8 mapperMBC3ReadRAM(u16);
void mapperMBC5ROM(u16,u8);
void mapperMBC5RAM(u16,u8);
u8 mapperMBC5ReadRAM(u16);
void mapperMBC7ROM(u16,u8);
void mapperMBC7RAM(u16,u8);
u8 mapperMBC7ReadRAM(u16);
void mapperHuC1ROM(u16,u8);
void mapperHuC1RAM(u16,u8);
void mapperHuC3ROM(u16,u8);
void mapperHuC3RAM(u16,u8);
u8 mapperHuC3ReadRAM(u16);
void mapperTAMA5RAM(u16,u8);
u8 mapperTAMA5ReadRAM(u16);
void memoryUpdateTAMA5Clock();
void mapperMMM01ROM(u16,u8);
void mapperMMM01RAM(u16,u8);
void mapperGGROM(u16,u8);
void mapperGS3ROM(u16,u8);
//extern void (*mapper)(u16,u8);
//extern void (*mapperRAM)(u16,u8);
//extern u8 (*mapperReadRAM)(u16);

extern void memoryUpdateMapMBC1();
extern void memoryUpdateMapMBC2();
extern void memoryUpdateMapMBC3();
extern void memoryUpdateMapMBC5();
extern void memoryUpdateMapMBC7();
extern void memoryUpdateMapHuC1();
extern void memoryUpdateMapHuC3();
extern void memoryUpdateMapTAMA5();
extern void memoryUpdateMapMMM01();
extern void memoryUpdateMapGS3();




