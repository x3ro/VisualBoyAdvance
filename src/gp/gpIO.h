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
#ifndef VBA_GP_GPIO_H
#define VBA_GP_GPIO_H

struct VideoControl1 {
  unsigned int envid:1;
  unsigned int bppmode:4;
  unsigned int pnrmode:2;
  unsigned int mmmode:1;
  unsigned int clkval:10;
  unsigned int linecnt:10;
};

struct VideoControl2 {
  unsigned int vspw:6;
  unsigned int vfpd:8;
  unsigned int lineval:10;
  unsigned int vbpd:8;
};

struct VideoControl3 {
  unsigned int hfpd:8;
  unsigned int hozval:11;
  unsigned int hbpd:7;
  unsigned int unused:6;
};

struct VideoControl4 {
  unsigned int hspw:8;
  unsigned int mval:8;
  unsigned int addval:8;
  unsigned int paladden:1;
  unsigned int unused:7;
};

struct VideoControl5 {
  unsigned int hwswp:1;
  unsigned int bswp:1;
  unsigned int enlend:1;
  unsigned int reserved:1;
  unsigned int invendline:1;
  unsigned int reserved2:1;
  unsigned int invvden:1;
  unsigned int invvd:1;
  unsigned int invvframe:1;
  unsigned int invvline:1;
  unsigned int invvclk:1;
  unsigned int reserved3:2;
  unsigned int selfref:1;
  unsigned int slowclksync:1;
  unsigned int reserved4:2;
  unsigned int hstatus:2;
  unsigned int vstatus:2;
  unsigned int unused:11;
};

struct VideoAddress1 {
  unsigned int lcdbaseu:21;
  unsigned int lcdbank:7;
  unsigned int unsed:4;
};

struct VideoAddress2 {
  unsigned int lcdbasel:21;
  unsigned int unused:11;
};

struct VideoAddress3 {
  unsigned int pagewidth:11;
  unsigned int offsize:11;
  unsigned int unused:10;
};

struct TimerConfig0 {
  unsigned int prescalar0:8;
  unsigned int prescalar1:8;
  unsigned int deadzone:8;
};

struct TimerConfig1 {
  unsigned int mux0:4;
  unsigned int mux1:4;
  unsigned int mux2:4;
  unsigned int mux3:4;
  unsigned int mux4:4;
  unsigned int dmamode:4;
};

struct TimerControl {
  unsigned int timer0start:1;
  unsigned int timer0manual:1;
  unsigned int timer0inverter:1;
  unsigned int timer0auto:1;
  unsigned int deadzone:1;
  unsigned int unused:3;
  unsigned int timer1start:1;
  unsigned int timer1manual:1;
  unsigned int timer1inverter:1;
  unsigned int timer1auto:1;
  unsigned int timer2start:1;
  unsigned int timer2manual:1;
  unsigned int timer2inverter:1;
  unsigned int timer2auto:1;
  unsigned int timer3start:1;
  unsigned int timer3manual:1;
  unsigned int timer3inverter:1;
  unsigned int timer3auto:1;
  unsigned int timer4start:1;
  unsigned int timer4manual:1;
  unsigned int timer4auto:1;
};

struct TimerCount {
  unsigned int count:16;
};

struct IoPortB {
  unsigned int smc_data:8;
  unsigned int nLeft:1;
  unsigned int nDown:1;
  unsigned int nRight:1;
  unsigned int nUp:1;
  unsigned int nButtonL:1;
  unsigned int nButtonB:1;
  unsigned int nButtonA:1;
  unsigned int nButtonR:1;
  unsigned int unused2:16;
};

struct IoPortD {
  unsigned int unused:6;
  unsigned int smc_nWP:1;
  unsigned int smc_nCE:1;
  unsigned int smc_nRE:1;
  unsigned int smc_nRB:1;
  unsigned int unused3:22;
};

struct IoPortE {
  unsigned int unsed:2;
  unsigned int smc_nIN:1;
  unsigned int smc_nWE:1;
  unsigned int smc_ALE:1;
  unsigned int smc_CLE:1;
  unsigned int nButtonStart:1;
  unsigned int nButtonSelect:1;
  unsigned int unused2:24;
};

extern TimerConfig0 *TCFG0;
extern TimerConfig1 *TCFG1;
extern TimerControl *TCON;
extern IoPortB *IODATB;
extern IoPortE *IODATE;
extern VideoControl1 *LCDCON1;
extern VideoControl2 *LCDCON2;
extern VideoControl3 *LCDCON3;
extern VideoControl4 *LCDCON4;
extern VideoControl5 *LCDCON5;
extern VideoAddress1 *LCDSADDR1;
extern VideoAddress2 *LCDSADDR2;
extern VideoAddress3 *LCDSADDR3;

#endif
