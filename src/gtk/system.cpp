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

#include <stdio.h>
#include <stdarg.h>

#include <SDL.h>

#include "../GBA.h"
#include "../gb/GB.h"
#include "../gb/gbGlobals.h"
#include "../Util.h"

#include "window.h"

int systemRedShift;
int systemGreenShift;
int systemBlueShift;
int systemColorDepth;
int systemDebug;
int systemVerbose;
int systemSaveUpdateCounter;
int systemFrameSkip;
u32 systemColorMap32[0x10000];
u16 systemColorMap16[0x10000];
u16 systemGbPalette[24];
bool systemSoundOn;

int systemRenderedFrames;
int systemFPS;

int emulating;
bool debugger;
int RGB_LOW_BITS_MASK;

inline VBA::Window * gui()
{
  return VBA::Window::poGetInstance();
}

void systemMessage(int _iId, const char * _csFormat, ...)
{
  va_list args;
  va_start(args, _csFormat);
  char * csMsg = g_strdup_vprintf(_csFormat, args);
  va_end(args);

  Gtk::MessageDialog oDialog(*gui(), csMsg,
                             Gtk::MESSAGE_ERROR,
                             Gtk::BUTTONS_OK);
  oDialog.run();
  free(csMsg);
}

void systemDrawScreen()
{
  gui()->vDrawScreen();
  systemRenderedFrames++;
}

bool systemReadJoypads()
{
  return true;
}

u32 systemReadJoypad(int)
{
  return gui()->uiReadJoypad();
}

void systemShowSpeed(int _iSpeed)
{
  systemFPS = systemRenderedFrames;
  systemRenderedFrames = 0;

  gui()->vShowSpeed(_iSpeed);
}

void system10Frames(int _iRate)
{
  gui()->vComputeFrameskip(_iRate);
}

void systemFrame()
{
}

void systemSetTitle(const char * _csTitle)
{
  gui()->set_title(_csTitle);
}

void systemScreenCapture(int _iNum)
{
}

void systemWriteDataToSoundBuffer()
{
}

bool systemSoundInit()
{
  return true;
}

void systemSoundShutdown()
{
}

void systemSoundPause()
{
}

void systemSoundResume()
{
}

void systemSoundReset()
{
}

u32 systemGetClock()
{
  return SDL_GetTicks();
}

void systemUpdateMotionSensor()
{
}

int systemGetSensorX()
{
  return 0;
}

int systemGetSensorY()
{
  return 0;
}

void systemGbPrint(u8 * _puiData,
                   int  _iPages,
                   int  _iFeed,
                   int  _iPalette,
                   int  _iContrast)
{
}

void systemScreenMessage(const char * _csMsg)
{
}

bool systemCanChangeSoundQuality()
{
  return false;
}

bool systemPauseOnFrame()
{
  return false;
}

void systemGbBorderOn()
{
}

void debuggerMain()
{
}

void debuggerSignal(int, int)
{
}

void debuggerOutput(char *, u32)
{
}

void (*dbgMain)() = debuggerMain;
void (*dbgSignal)(int, int) = debuggerSignal;
void (*dbgOutput)(char *, u32) = debuggerOutput;
