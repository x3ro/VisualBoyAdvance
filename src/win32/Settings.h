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
#ifndef VBA_WIN32_SETTINGS_H
#define VBA_WIN32_SETTINGS_H

enum {
  VIDEO_1X, VIDEO_2X, VIDEO_3X, VIDEO_4X,
  VIDEO_320x240, VIDEO_640x480, VIDEO_800x600, VIDEO_OTHER
};

typedef struct {
  int sizeX;
  int sizeY;
  int surfaceSizeX;
  int surfaceSizeY;
  int videoOption;
  bool fullScreenStretch;
  int fsWidth;
  int fsHeight;
  int fsColorDepth;
  RECT rect;
  RECT dest;
  int windowPositionX;
  int windowPositionY;
  bool ddrawEmulationOnly;
  bool ddrawUsingEmulationOnly;
  bool ddrawUseVideoMemory;
  bool tripleBuffering;
  int ddrawDebug;
  bool mode320Available;
  bool mode640Available;
  bool mode800Available;
  int nCmdShow;
  GUID *pVideoDriverGUID;
  void (*filterFunction)(u8*,u32,u8*,u8*,u32,int,int);
  void (*ifbFunction)(u8*,u32,int,int);
  bool vsync;
  int filterWidth;
  int filterHeight;
  u8 *delta[257*244*4];
  int showSpeed;
  bool showSpeedTransparent;
  bool disableMMX;
  int showRenderedFrames;
  bool menuToggle;
  bool active;
  bool screenMessage;
  char screenMessageBuffer[41];
  DWORD screenMessageTime;
  bool disableStatusMessage;
} Settings;

#endif
