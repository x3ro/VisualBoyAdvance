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
#define C_FLAG 0x10
#define H_FLAG 0x20
#define N_FLAG 0x40
#define Z_FLAG 0x80

typedef union {
  struct {
#ifdef WORDS_BIGENDIAN
    u8 B1, B0;
#else
    u8 B0,B1;
#endif
  } B;
  u16 W;
} gbRegister;

extern bool gbLoadRom(char *);
extern void gbEmulate(int);
extern bool gbIsGameboyRom(char *);
extern void gbSoundReset();
extern void gbSoundSetQuality(int);
extern void gbReset();
extern void gbCleanUp();
extern bool gbWriteBatteryFile(char *);
extern bool gbWriteBatteryFile(char *, bool);
extern bool gbReadBatteryFile(char *);
extern bool gbWriteSaveState(char *);
extern bool gbReadSaveState(char *);
extern void gbSgbRenderBorder();
extern bool gbWritePNGFile(char *);
extern bool gbWriteBMPFile(char *);
extern bool gbReadGSASnapshot(char *);
