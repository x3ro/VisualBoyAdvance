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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../GBA.h"
#include "../unzip.h"
#include "../Util.h"
#include "../NLS.h"
#include "../Globals.h"
#include "gpIO.h"

#ifdef __GNUC__
#define _stricmp strcasecmp
#endif

#define CPUUpdateTicksAccess32(a) 0
#define CPUUpdateTicksAccessSeq32(a) 0
#define CPUUpdateTicksAccess16(a) 0
#define CPUUpdateTicksAccessSeq16(a) 0
#define CPUReadMemory         GPReadMemory
#define CPUReadHalfWord       GPReadHalfWord
#define CPUReadByte           GPReadByte
#define CPUReadHalfWordSigned GPReadHalfWordSigned
#define CPUWriteMemory        GPWriteMemory
#define CPUWriteHalfWord      GPWriteHalfWord
#define CPUWriteByte          GPWriteByte
#define CPUSwitchMode         GPSwitchMode
#define CPUSoftwareInterrupt  GPSoftwareInterrupt
#define CPUUpdateCPSR         GPUpdateCPSR
#define CPUUpdateFlags        GPUpdateFlags

#define CPUReadMemoryQuick   GPReadMemory
#define CPUReadHalfWordQuick GPReadHalfWord
#define CPUReadByteQuick     GPReadByte

// this enables the MRC/MCR instructions in arm-new.cpp
#ifndef GP_SUPPORT
#define GP_SUPPORT
#endif

extern int thumbCycles[];
extern u8 cpuBitsSet[256];
extern u8 cpuBitsSet[256];
extern u8 cpuLowestBitSet[256];

#define IO(address) *((u32 *)&gpIo[address])

TimerConfig0 *TCFG0 = NULL;
TimerConfig1 *TCFG1 = NULL;
TimerControl *TCON = NULL;
IoPortB *IODATB = NULL;
IoPortE *IODATE = NULL;
VideoControl1 *LCDCON1 = NULL;
VideoControl2 *LCDCON2 = NULL;
VideoControl3 *LCDCON3 = NULL;
VideoControl4 *LCDCON4 = NULL;
VideoControl5 *LCDCON5 = NULL;
VideoAddress1 *LCDSADDR1 = NULL;
VideoAddress2 *LCDSADDR2 = NULL;
VideoAddress3 *LCDSADDR3 = NULL;

static int dummyAddress = 0;

u8 *gpBios = NULL;
u8 *gpRam = NULL;
u8 *gpIo = NULL;

int gpCapture = 0;
int gpCapturePrevious = 0;
int gpCaptureNumber = 0;

u32 gpLastTime = 0;
int gpCount = 0;
int gpFrameCount = 0;

int gpLcdTicks = 200000/320;
int gpLcdTicksReload = 200000/320;

int gpTimer0Ticks = 0;
int gpTimer0TicksReload = 0;
int gpTimer1Ticks = 0;
int gpTimer1TicksReload = 0;
int gpTimer2Ticks = 0;
int gpTimer2TicksReload = 0;
int gpTimer3Ticks = 0;
int gpTimer3TicksReload = 0;
int gpTimer4Ticks = 0;
int gpTimer4TicksReload = 0;

u32 GPReadMemory(u32 address)
{
  u32 value;
  
#ifdef DEV_VERSION
  if(address & 3) {  
    if(systemVerbose & VERBOSE_UNALIGNED_MEMORY) {
      log("Unaligned word read: %08x at %08x\n", address, armMode ?
          armNextPC - 4 : armNextPC - 2);
    }
  }
#endif
  
  switch(address >> 24) {
  case 0x00:
    value = *((u32 *)&gpBios[address & 0x7fffc]);
    break;
  case 0x0c:
    value = *((u32 *)&gpRam[address & 0x7ffffc]);
    break;
  case 0x14:
  case 0x15:
    //    if(address >= 0x15100000 && address < 0x15200000)
    //      printf("32-bit read %08x from %08x\n", address, armNextPC);    
    value = *((u32 *)&gpIo[address & 0x1fffffc]);
    break;
  default:
#ifdef DEV_VERSION
    if(systemVerbose & VERBOSE_ILLEGAL_READ) {
      log("Illegal word read: %08x at %08x\n", address, armMode ?
          armNextPC - 4 : armNextPC - 2);
    }
#endif
    value = 0; // check if right value
  }

  if(address & 3) {
#ifdef __GNUC__
    asm("and $3, %%ecx;"
        "shl $3 ,%%ecx;"
        "ror %%cl, %0"
        : "=r" (value)
        : "r" (value), "c" (address));
#else
    __asm {
      mov ecx, address;
      and ecx, 3;
      shl ecx, 3;
      ror [dword ptr value], cl;
    }
#endif
  }
  
  return value;
}

u32 GPReadHalfWord(u32 address)
{
  u16 value;
  
#ifdef DEV_VERSION      
  if(address & 1) {
    if(systemVerbose & VERBOSE_UNALIGNED_MEMORY) {
      log("Unaligned halfword read: %08x at %08x\n", address, armMode ?
          armNextPC - 4 : armNextPC - 2);
    }
  }
#endif
  
  switch(address >> 24) {
  case 0x00:
    value = *((u16 *)&gpBios[address & 0x7fffe]);
    break;
  case 0x0c:
    value = *((u16 *)&gpRam[address & 0x7ffffe]);
    break;
  case 0x14:
  case 0x15:
    value = *((u16 *)&gpIo[address & 0x1fffffe]);
    break;
  default:
#ifdef DEV_VERSION
    if(systemVerbose & VERBOSE_ILLEGAL_READ) {
      log("Illegal halfword read: %08x at %08x\n", address, armMode ?
          armNextPC - 4 : armNextPC - 2);
    }
#endif
    value = 0;
  }

  // verify this is valid on GP32
  if(address & 1)
    value = value >> 8;
  
  return value;
}

u32 GPReadByte(u32 address)
{
  switch(address >> 24) {
  case 0x00:
    return gpBios[address & 0x7ffff];
  case 0x0c:
    return gpRam[address & 0x7fffff];
  case 0x14:
  case 0x15:
    return gpIo[address & 0x1ffffff];    
  default:
#ifdef DEV_VERSION
    if(systemVerbose & VERBOSE_ILLEGAL_READ) {
      log("Illegal byte read: %08x at %08x\n", address, armMode ?
          armNextPC - 4 : armNextPC - 2);
    }
#endif
  }
  
  return 0;
}

s16 GPReadHalfWordSigned(u32 address)
{
  return (s16)GPReadHalfWord(address);
}

void GPWriteMemory(u32 address, u32 value)
{
#ifdef DEV_VERSION
  if(address & 3) {
    if(systemVerbose & VERBOSE_UNALIGNED_MEMORY) {
      log("Unaliagned word write: %08x to %08x from %08x\n",
          value,
          address,
          armMode ? armNextPC - 4 : armNextPC - 2);
    }
  }
#endif
  
  switch(address >> 24) {
  case 0x0c:
    *((u32 *)&gpRam[address & 0x7ffffc]) = value;
    break;
  case 0x14:
  case 0x15:
    if(address != 0x15100008)
      *((u32 *)&gpIo[address & 0x1fffffc]) = value;
    else {
      u32 old = IO(0x1100008);
      IO(0x1100008) = value;
      
      u32 change = value ^ old; // see what changed

      // timer 0
      
      if((change & old) & 0x00000002) {
        // move count buffer to actual count buffer
      }
      
      if((change & value) & 0x00000001) {
        int mux = TCFG1->mux0;
        switch(mux) {
        case 0:
          mux = 2;
          break;
        case 1:
          mux = 4;
          break;
        case 2:
          mux = 8;
          break;
        case 3:
          mux = 16;
          break;
        default:
          mux = 1;
          break;
        }
        int hz = (66000000/(TCFG0->prescalar0+1))/mux;
        int count = IO(0x110000c) & 0xffff;
        if(count == 0)
          count = 0x10000;
        
        gpTimer0Ticks = gpTimer0TicksReload = (12000000/hz)*count;
      } else if((change & old) & 1) {
        gpTimer0Ticks = gpTimer0TicksReload = 0;
      }

      // timer 1
      
      if((change & old) & 0x00000200) {
        // move count buffer to actual count buffer
      }
      
      if((change & value) & 0x00000100) {
        int mux = TCFG1->mux1;
        switch(mux) {
        case 0:
          mux = 2;
          break;
        case 1:
          mux = 4;
          break;
        case 2:
          mux = 8;
          break;
        case 3:
          mux = 16;
          break;
        default:
          mux = 1;
          break;
        }
        int hz = (66000000/(TCFG0->prescalar0+1))/mux;
        int count = IO(0x1100018) & 0xffff;
        if(count == 0)
          count = 0x10000;
        
        gpTimer1Ticks = gpTimer1TicksReload = (12000000/hz)*count;
      } else if((change & old) & 1) {
        gpTimer1Ticks = gpTimer1TicksReload = 0;
      }

      // timer 2
      
      if((change & old) & 0x00002000) {
        // move count buffer to actual count buffer
      }
      
      if((change & value) & 0x00001000) {
        int mux = TCFG1->mux2;
        switch(mux) {
        case 0:
          mux = 2;
          break;
        case 1:
          mux = 4;
          break;
        case 2:
          mux = 8;
          break;
        case 3:
          mux = 16;
          break;
        default:
          mux = 1;
          break;
        }
        int hz = (66000000/(TCFG0->prescalar1+1))/mux;
        int count = IO(0x1100024) & 0xffff;
        if(count == 0)
          count = 0x10000;
        
        gpTimer2Ticks = gpTimer2TicksReload = (12000000/hz)*count;
      } else if((change & old) & 1) {
        gpTimer2Ticks = gpTimer2TicksReload = 0;
      }      

      // timer 3
      
      if((change & old) & 0x00020000) {
        // move count buffer to actual count buffer
      }
      
      if((change & value) & 0x00010000) {
        int mux = TCFG1->mux3;
        switch(mux) {
        case 0:
          mux = 2;
          break;
        case 1:
          mux = 4;
          break;
        case 2:
          mux = 8;
          break;
        case 3:
          mux = 16;
          break;
        default:
          mux = 1;
          break;
        }
        int hz = (66000000/(TCFG0->prescalar1+1))/mux;
        int count = IO(0x1100030) & 0xffff;
        if(count == 0)
          count = 0x10000;
        
        gpTimer3Ticks = gpTimer3TicksReload = (12000000/hz)*count;
      } else if((change & old) & 1) {
        gpTimer3Ticks = gpTimer3TicksReload = 0;
      }      

      // timer 4
      
      if((change & old) & 0x00200000) {
        // move count buffer to actual count buffer
      }
      
      if((change & value) & 0x00100000) {
        int mux = TCFG1->mux4;
        switch(mux) {
        case 0:
          mux = 2;
          break;
        case 1:
          mux = 4;
          break;
        case 2:
          mux = 8;
          break;
        case 3:
          mux = 16;
          break;
        default:
          mux = 1;
          break;
        }
        int hz = (66000000/(TCFG0->prescalar1+1))/mux;
        int count = IO(0x110003c) & 0xffff;
        if(count == 0)
          count = 0x10000;
        
        gpTimer4Ticks = gpTimer4TicksReload = (12000000/hz)*count;
      } else if((change & old) & 1) {
        gpTimer4Ticks = gpTimer4TicksReload = 0;
      }      
    }
    break;
  default:
#ifdef DEV_VERSION
    if(systemVerbose & VERBOSE_ILLEGAL_WRITE) {
      log("Illegal word write: %08x to %08x from %08x\n",
          value,
          address,
          armMode ? armNextPC - 4 : armNextPC - 2);
    }
#endif
  }
}

void GPWriteHalfWord(u32 address, u16 value)
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
  case 0x0c:
    *((u16 *)&gpRam[address & 0x7ffffe]) = value;
    break;
  case 0x14:
  case 0x15:
    *((u16 *)&gpIo[address & 0x1fffffe]) = value;
    break;
  default:
#ifdef DEV_VERSION
    if(systemVerbose & VERBOSE_ILLEGAL_WRITE) {
      log("Illegal halfword write: %04x to %08x from %08x\n",
          value,
          address,
          armMode ? armNextPC - 4 : armNextPC - 2);
    }
#endif
  }
}

void GPWriteByte(u32 address, u8 value)
{
  switch(address >> 24) {
  case 0x0c:
    gpRam[address & 0x7fffff] = value;
    break;
  case 0x14:
  case 0x15:
    gpIo[address & 0x1ffffff] = value;
    break;
  default:
#ifdef DEV_VERSION
    if(systemVerbose & VERBOSE_ILLEGAL_WRITE) {
      log("Illegal byte write: %02x to %08x from %08x\n",
          value,
          address,
          armMode ? armNextPC - 4 : armNextPC -2 );
    }
#endif
  }
}

void GPUpdateCPSR()
{
  u32 CPSR =  reg[16].I & 0x40;
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

void GPUpdateFlags(bool breakLoop)
{
  u32 CPSR = reg[16].I;
  
  N_FLAG = (CPSR & 0x80000000) ? true: false;
  Z_FLAG = (CPSR & 0x40000000) ? true: false;
  C_FLAG = (CPSR & 0x20000000) ? true: false;
  V_FLAG = (CPSR & 0x10000000) ? true: false;
  armState = (CPSR & 0x20) ? false : true;
  armIrqEnable = (CPSR & 0x80) ? false : true;
  /*
  if(breakLoop) {
    if(armIrqEnable && (IF & IE) && (IME & 1)) {
      CPU_BREAK_LOOP_2;
    }
  }
  */
}

void GPUpdateFlags()
{
  GPUpdateFlags(true);
}

void GPSwap(u32& a, u32& b)
{
  u32 c = b;
  b = a;
  a = c;
}

void GPSwitchMode(int mode, bool saveState, bool breakLoop)
{
  //  if(armMode == mode)
  //    return;
  
  GPUpdateCPSR();

  switch(armMode) {
  case 0x10:
  case 0x1F:
    reg[R13_USR].I = reg[13].I;
    reg[R14_USR].I = reg[14].I;
    reg[17].I = reg[16].I;
    break;
  case 0x11:
    GPSwap(reg[R8_FIQ].I, reg[8].I);
    GPSwap(reg[R9_FIQ].I, reg[9].I);
    GPSwap(reg[R10_FIQ].I, reg[10].I);
    GPSwap(reg[R11_FIQ].I, reg[11].I);
    GPSwap(reg[R12_FIQ].I, reg[12].I);
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
    GPSwap(reg[8].I, reg[R8_FIQ].I);
    GPSwap(reg[9].I, reg[R9_FIQ].I);
    GPSwap(reg[10].I, reg[R10_FIQ].I);
    GPSwap(reg[11].I, reg[R11_FIQ].I);
    GPSwap(reg[12].I, reg[R12_FIQ].I);
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
    systemMessage(MSG_UNSUPPORTED_ARM_MODE,"Unsupported ARM mode %02x %08x", mode, armNextPC);
    break;
  }
  armMode = mode;
  GPUpdateFlags(breakLoop);
  GPUpdateCPSR();
}

void GPSwitchMode(int mode, bool saveState)
{
  GPSwitchMode(mode, saveState, true);
}

void GPSoftwareInterrupt()
{
  u32 PC = reg[15].I;
  bool savedArmState = armState;
  GPSwitchMode(0x13, true, false);
  reg[14].I = PC - (savedArmState ? 4 : 2);
  reg[15].I = 0x08;
  armState = true;
  armIrqEnable = false;
  armNextPC = 0x08;
  reg[15].I += 4;
}

void GPSoftwareInterrupt(int comment)
{
#ifdef BKPT_SUPPORT
  if(comment == 0xff || comment == 0x00ff0000) {
    extern void (*dbgOutput)(char *, u32);
    dbgOutput(NULL, reg[0].I);
    return;
  }
#endif  
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
  GPSoftwareInterrupt();
}

bool GPIsGPImage(char *file)
{
  if(strlen(file) > 4) {
    char * p = strrchr(file,'.');

    if(p != NULL) {
      if(_stricmp(p, ".gxb") == 0)
        return true;
      if(_stricmp(p, ".fxe") == 0)
        return true;
    }
  }

  return false;
}

bool GPLoadRom(char *name)
{
  char *enckey;
  unsigned char a,b,c,d;
  int i,j,keylen;

  FILE *f = fopen("fw157e.bin", "rb");
  if(!f) {
    systemMessage(0, "Error opening firmware");
    return false;
  }

  gpBios = (u8 *)malloc(0x80000);
  fread(gpBios, 1, 0x80000, f);

  fclose(f);

  gpRam = (u8 *)malloc(0x8000000);
  
  f = fopen(name, "rb");
  
  if(f != NULL) {
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    a = fgetc(f);
    b = fgetc(f);
    c = fgetc(f);
    
    if ((a=='f') && (b=='x') && (c=='e')) {
      fseek (f, 1116, SEEK_SET); 
      a = fgetc(f);
      b = fgetc(f);
      c = fgetc(f);
      d = fgetc(f);
      
      size = (int)(a | (b<<8) | (c<<16) | (d<<24));
      
      a = fgetc(f);
      b = fgetc(f);
      c = fgetc(f);
      d = fgetc(f);
      
      keylen = (int)(a | (b<<8) | (c<<16) | (d<<24));
      enckey = (char*)malloc (keylen);
      
      fread (enckey, 1, keylen, f);

      for(i = 0; i < keylen; i++)
        enckey[i] = ~enckey[i];
      
      j=0;
      fread(&gpBios[0x8004], 1, size, f);
      for (i=0;i<size;i++) {
        gpBios[0x8004+i] ^= enckey[j];
        j++;
        if (j>= keylen) j=0;
      }
      free(enckey);
      u32 zeroStart = gpBios[0x8018] | gpBios[0x8019]<<8 |
        gpBios[0x801a] << 16 | gpBios[0x801b] << 24;
      u32 zeroEnd = gpBios[0x801c] | gpBios[0x801d]<<8 |
        gpBios[0x801e] << 16 | gpBios[0x801f] << 24;
      u32 rwStart = gpBios[0x8010] | gpBios[0x8011]<<8 |
        gpBios[0x8012] << 16 | gpBios[0x8013] << 24;      
      
      *((u32 *)&gpBios[0x8000]) = zeroStart - 0xc000000;

      u32 addr = 0x8004 + rwStart - 0xc000000;
      memset(&gpBios[addr],
             0,
             zeroEnd - rwStart);
      
    } else {
      fseek(f, 0, SEEK_SET);
      fread(&gpBios[0x8004], 1, size, f);
      *((u32 *)&gpBios[0x8000]) = size;
    }
    fclose(f);
  }
  
  gpIo = (u8 *)malloc(0x2000000);

  TCFG0 = (TimerConfig0 *)&gpIo[0x1100000];
  TCFG1 = (TimerConfig1 *)&gpIo[0x1100004];
  TCON  = (TimerControl *)&gpIo[0x1100008];
  IODATB = (IoPortB *)&gpIo[0x160000c];
  IODATE = (IoPortE *)&gpIo[0x1600030];
  LCDCON1 = (VideoControl1 *)&gpIo[0x0a00000];
  LCDCON2 = (VideoControl2 *)&gpIo[0x0a00004];
  LCDCON3 = (VideoControl3 *)&gpIo[0x0a00008];
  LCDCON4 = (VideoControl4 *)&gpIo[0x0a0000c];
  LCDCON5 = (VideoControl5 *)&gpIo[0x0a00010];
  LCDSADDR1 = (VideoAddress1 *)&gpIo[0x0a00014];
  LCDSADDR2 = (VideoAddress2 *)&gpIo[0x0a00018];
  LCDSADDR3 = (VideoAddress3 *)&gpIo[0x0a0001c];
  
  pix = (u8 *)malloc(4*320*240);
  
  return true;
}

bool GPWriteState(char *)
{
  return true;
}

bool GPReadState(char *)
{
  return true;
}

bool GPWriteBatteryFile(char *)
{
  return true;
}

bool GPReadBatteryFile(char *)
{
  return true;
}

void GPReset()
{
  // clen registers
  memset(&reg[0], 0, sizeof(reg));
  reg[15].I = 0x00000000;
  armState = true;
  armMode = 0x13;
  armIrqEnable = false; // IRQ is disabled
  C_FLAG = V_FLAG = N_FLAG = Z_FLAG = false;

  reg[16].I |= 0x40; // FIQ is disabled
  GPUpdateCPSR();
  
  armNextPC = reg[15].I;
  reg[15].I += 4;

  gpLastTime = systemGetClock();
}

void GPCleanUp()
{
  free(gpBios);
  gpBios = NULL;
  free(gpRam);
  gpRam = NULL;
  free(gpIo);
  gpIo = NULL;
  free(pix);
  pix = NULL;
}

bool GPWritePNGFile(char *fileName)
{
  return utilWritePNGFile(fileName, 320, 240, pix);
}

bool GPWriteBMPFile(char *fileName)
{
  return utilWriteBMPFile(fileName, 320, 240, pix);
}

void GPInit()
{
  int i;
  for(i = 0; i < 256; i++) {
    map[i].address = (u8 *)&dummyAddress;
    map[i].mask = 0;
  }

  map[0].address = gpBios;
  map[0].mask = 0x7FFFF;
  map[12].address = gpRam;
  map[12].mask = 0x7FFFFF;
  map[0x14].address = gpIo;
  map[0x14].mask = 0x1ffffff;
  map[0x15].address = gpIo;
  map[0x15].mask = 0x1ffffff;

  IO(0x160000c) = 0x0000ff00;
  IO(0x1600030) = 0x3fffffff;
  
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
}

void GPInterrupt()
{
  u32 PC = reg[15].I;
  bool savedState = armState;
  GPSwitchMode(0x12, true, false);
  reg[14].I = PC;
  if(!savedState)
    reg[14].I += 2;
  reg[15].I = 0x18;
  armState = true;
  armIrqEnable = false;

  armNextPC = reg[15].I;
  reg[15].I += 4;
}

int gpInterrupt = 0;

void GPLoop(int ticks)
{
  int clockTicks;
  // variables used by the CPU core
  u32 value;
  int base;
  int dest;
  int source;
  int shift;
  int offset;
  int mult;
  int rs;
  u32 address;
  u32 temp;
  u32 opcode;
  bool C_OUT;
  bool cond_res;
  int destLo;
  int destHi;
  u64 uTemp;
  s64 sTemp;
  s64 m;
  s64 s;
  u32 newValue;
  u32 umult;
  u32 usource;

  while(ticks > 0) {
#ifndef FINAL_VERSION
    if(systemDebug) {
      char buffer[256];
      if(systemDebug >= 10) {
        GPUpdateCPSR();
        sprintf(buffer, "R00=%08x R01=%08x R02=%08x R03=%08x R04=%08x R05=%08x R06=%08x R07=%08x R08=%08x R09=%08x R10=%08x R11=%08x R12=%08x R13=%08x R14=%08x R15=%08x R16=%08x R17=%08x\n",
                 reg[0].I, reg[1].I, reg[2].I, reg[3].I, reg[4].I, reg[5].I,
                 reg[6].I, reg[7].I, reg[8].I, reg[9].I, reg[10].I, reg[11].I,
                 reg[12].I, reg[13].I, reg[14].I, reg[15].I, reg[16].I,
                 reg[17].I);
        log(buffer);
      } else {
        sprintf(buffer, "PC=%08x\n", armNextPC);
        log(buffer);
      }
    }
#endif
    
    if(armState) {
      clockTicks = 1;
#include "../arm-new.cpp"
    } else {
#include "../thumb.cpp"
    }
    
    clockTicks += 3;
    
    gpLcdTicks -= clockTicks;
    if(gpLcdTicks <= 0) {
      gpLcdTicks += gpLcdTicksReload;
      VideoControl1 *lcdcon1 = (VideoControl1 *)&gpIo[0xa00000];
      int line = lcdcon1->linecnt;
      line--;
      if(line == -1) {
        gpCount++;
        line = 319;
        IO(0x160000c) |= 0x0000ff00;
        IO(0x1600030) |= 0x000000c0;
        u32 keys = systemReadJoypad();
        if(keys & 1)
          IODATB->nButtonA = 0;
        if(keys & 2)
          IODATB->nButtonB = 0;
        if(keys & 512)
          IODATB->nButtonL = 0;
        if(keys & 256)
          IODATB->nButtonR = 0;
        if(keys & 64)
          IODATB->nUp = 0;
        if(keys & 16)
          IODATB->nRight = 0;
        if(keys & 128)
          IODATB->nDown = 0;
        if(keys & 32)
          IODATB->nLeft = 0;
        if(keys & 4)
          IODATE->nButtonSelect = 0;
        if(keys & 8)
          IODATE->nButtonStart = 0;


        u32 ext = systemReadJoypadExtended();
        gpCapture = (ext & 2) ? true: false;

        if(gpCapture && !gpCapturePrevious) {
          gpCaptureNumber++;
          systemScreenCapture(gpCaptureNumber);
        }
        gpCapturePrevious = gpCapture;

        if(gpFrameCount >= 1) {
          extern void gpGfxRender();
          
          gpGfxRender();
        
          systemDrawScreen();
          gpFrameCount = 0;
        } else
          gpFrameCount++;
        
        if(gpCount >= 60) {
          char buffer[256];
          u32 currentTime = systemGetClock();
          if(currentTime != gpLastTime)
            sprintf(buffer,"VisualBoyAdvance - %d%%", 
                    100000/(currentTime - gpLastTime));
          else
            sprintf(buffer,"VisualBoyAdvance - 0%%");
          systemSetTitle(buffer);
          gpLastTime = currentTime;
          gpCount = 0;
          
        }
      }
      lcdcon1->linecnt = line;
    }

    if(gpTimer0Ticks) {
      gpTimer0Ticks -= clockTicks;

      if(gpTimer0Ticks <= 0) {
        if(TCON->timer0auto) {
          gpTimer0Ticks += gpTimer0TicksReload;
        } else
          gpTimer0Ticks = gpTimer0TicksReload = 0;
        gpInterrupt |= 1024;
      }
    }

    if(gpTimer1Ticks) {
      gpTimer1Ticks -= clockTicks;

      if(gpTimer1Ticks <= 0) {
        if(TCON->timer1auto) {
          gpTimer1Ticks += gpTimer1TicksReload;
        } else
          gpTimer1Ticks = gpTimer1TicksReload = 0;
        gpInterrupt |= 2048;
      }
    }

    if(gpTimer2Ticks) {
      gpTimer2Ticks -= clockTicks;

      if(gpTimer2Ticks <= 0) {
        if(TCON->timer2auto) {
          gpTimer2Ticks += gpTimer2TicksReload;
        } else
          gpTimer2Ticks = gpTimer2TicksReload = 0;
        gpInterrupt |= 4096;
      }
    }

    if(gpTimer3Ticks) {
      gpTimer3Ticks -= clockTicks;

      if(gpTimer3Ticks <= 0) {
        if(TCON->timer3auto) {
          gpTimer3Ticks += gpTimer3TicksReload;
        } else
          gpTimer3Ticks = gpTimer3TicksReload = 0;
        gpInterrupt |= 8192;
      }
    }

    if(gpTimer4Ticks) {
      gpTimer4Ticks -= clockTicks;

      if(gpTimer4Ticks <= 0) {
        if(TCON->timer4auto) {
          gpTimer4Ticks += gpTimer4TicksReload;
        } else
          gpTimer4Ticks = gpTimer4TicksReload = 0;
        gpInterrupt |= 16384;
      }
    }    
    if(gpInterrupt) {
      IO(0x0400000) |= gpInterrupt;
      u32 res = (~IO(0x0400008)) & gpInterrupt;
      if(res) {
        int irq = 0;
        if(res & 1024) {
          res = 1024;
          irq = 10;
        } else if(res & 2048) {
          res = 2048;
          irq = 11;
        } else if(res & 4196) {
          res = 4196;
          irq = 12;
        } else if(res & 8192) {
          res = 8192;
          irq = 13;
        } else if(res & 16384) {
          res = 16384;
          irq = 14;
        } else {
          res = 0;
        }
        gpInterrupt &= ~res;
        IO(0x0400010) = res;
        IO(0x0400014) = irq;
        if(armIrqEnable)
          GPInterrupt();
      }
    }
    
    ticks -= clockTicks;
  }
}
#endif
