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

#include "window.h"

#include <SDL.h>

#include "../GBA.h"
#include "../gb/GB.h"
#include "../gb/gbGlobals.h"
#include "../Util.h"

#include "tools.h"
#include "intl.h"

extern bool debugger;
extern int RGB_LOW_BITS_MASK;

namespace VBA
{

using Gnome::Glade::Xml;

Window * Window::m_poInstance = NULL;

const int iGBScreenWidth   = 160;
const int iGBScreenHeight  = 144;
const int iSGBScreenWidth  = 256;
const int iSGBScreenHeight = 224;
const int iGBAScreenWidth  = 240;
const int iGBAScreenHeight = 160;

Window::Window(GtkWindow * _pstWindow, const Glib::RefPtr<Xml> & _poXml) :
  Gtk::Window(_pstWindow),
  m_poFileOpenDialog(NULL),
  m_eCartridge(NO_CARTRIDGE),
  m_uiJoypadState(0),
  m_iScreenWidth(iGBAScreenWidth),
  m_iScreenHeight(iGBAScreenHeight),
  m_iScreenScale(1),
  m_vFilter2x(NULL),
  m_vFilterIB(NULL)
{
  vInitSystem();
  vInitSDL();
  vLoadKeymap();

  Gtk::Container * poC;
  poC = dynamic_cast<Gtk::Container *>(_poXml->get_widget("ScreenContainer"));
  m_poScreenArea = Gtk::manage(new ScreenArea(m_iScreenWidth, m_iScreenHeight));
  poC->add(*m_poScreenArea);
  vDrawDefaultScreen();
  m_poScreenArea->vSetFilter2x(SuperEagle32); // TEST
  m_poScreenArea->show();

  m_poFilePauseItem = dynamic_cast<Gtk::CheckMenuItem *>(_poXml->get_widget("FilePause"));
  m_poFilePauseItem->signal_toggled().connect(SigC::slot(*this, &Window::vOnFilePause));

  Gtk::MenuItem * poMI;
  poMI = dynamic_cast<Gtk::MenuItem *>(_poXml->get_widget("FileOpen"));
  poMI->signal_activate().connect(SigC::slot(*this, &Window::vOnFileOpen));
  poMI = dynamic_cast<Gtk::MenuItem *>(_poXml->get_widget("FileReset"));
  poMI->signal_activate().connect(SigC::slot(*this, &Window::vOnFileReset));
  poMI = dynamic_cast<Gtk::MenuItem *>(_poXml->get_widget("FileClose"));
  poMI->signal_activate().connect(SigC::slot(*this, &Window::vOnFileClose));
  poMI = dynamic_cast<Gtk::MenuItem *>(_poXml->get_widget("FileQuit"));
  poMI->signal_activate().connect(SigC::slot(*this, &Window::vOnFileQuit));

  poMI = dynamic_cast<Gtk::MenuItem *>(_poXml->get_widget("VideoZoom1x"));
  poMI->signal_activate().connect(SigC::slot(*this, &Window::vOnVideoZoom1x));
  poMI = dynamic_cast<Gtk::MenuItem *>(_poXml->get_widget("VideoZoom2x"));
  poMI->signal_activate().connect(SigC::slot(*this, &Window::vOnVideoZoom2x));
  poMI = dynamic_cast<Gtk::MenuItem *>(_poXml->get_widget("VideoZoom3x"));
  poMI->signal_activate().connect(SigC::slot(*this, &Window::vOnVideoZoom3x));
  poMI = dynamic_cast<Gtk::MenuItem *>(_poXml->get_widget("VideoZoom4x"));
  poMI->signal_activate().connect(SigC::slot(*this, &Window::vOnVideoZoom4x));

  poMI = dynamic_cast<Gtk::MenuItem *>(_poXml->get_widget("HelpAbout"));
  poMI->signal_activate().connect(SigC::slot(*this, &Window::vOnHelpAbout));

  if (m_poInstance == NULL)
  {
    m_poInstance = this;
  }
  else
  {
    abort();
  }
}

Window::~Window()
{
  vOnFileClose();

  if (m_poFileOpenDialog != NULL)
  {
    delete m_poFileOpenDialog;
  }

  m_poInstance = NULL;
}

void Window::vInitSystem()
{
  Init_2xSaI(32);

#if G_BYTE_ORDER == G_LITTLE_ENDIAN
  systemRedShift    = 3;
  systemGreenShift  = 11;
  systemBlueShift   = 19;
  RGB_LOW_BITS_MASK = 0x00010101;
#else
  systemRedShift    = 27;
  systemGreenShift  = 19;
  systemBlueShift   = 11;
  RGB_LOW_BITS_MASK = 0x01010100;
#endif

  systemColorDepth = 32;
  systemDebug = 0;
  systemVerbose = 0;
  systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
  systemFrameSkip = 5; // TEST
  systemSoundOn = false;

  emulating = 0;
  debugger = true;

  for (int i = 0; i < 0x10000; i++)
  {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
    systemColorMap32[i] = (((i & 0x1f) << systemRedShift)
                           | (((i & 0x3e0) >> 5) << systemGreenShift)
                           | (((i & 0x7c00) >> 10) << systemBlueShift)
                           | 0xff000000);
#else
    systemColorMap32[i] = (((i & 0x1f) << systemRedShift)
                           | (((i & 0x3e0) >> 5) << systemGreenShift)
                           | (((i & 0x7c00) >> 10) << systemBlueShift)
                           | 0xff);
#endif
  }

  // TODO : GB init and 16-bit color map (?)
}

void Window::vInitSDL()
{
  static bool bDone = false;

  if (bDone)
    return;

  int iFlags = (SDL_INIT_AUDIO
                | SDL_INIT_TIMER
                | SDL_INIT_NOPARACHUTE);

  if (SDL_Init(iFlags) < 0)
  {
    fprintf(stderr, "Failed to init SDL: %s", SDL_GetError());
    abort();
  }

  bDone = true;
}

void Window::vLoadKeymap()
{
  // TODO : load from prefs

  m_oKeymap.vRegister(GDK_z,            KEY_A);
  m_oKeymap.vRegister(GDK_Z,            KEY_A);
  m_oKeymap.vRegister(GDK_x,            KEY_B);
  m_oKeymap.vRegister(GDK_X,            KEY_B);
  m_oKeymap.vRegister(GDK_BackSpace,    KEY_SELECT);
  m_oKeymap.vRegister(GDK_Return,       KEY_START);
  m_oKeymap.vRegister(GDK_Right,        KEY_RIGHT);
  m_oKeymap.vRegister(GDK_Left,         KEY_LEFT);
  m_oKeymap.vRegister(GDK_Up,           KEY_UP);
  m_oKeymap.vRegister(GDK_Down,         KEY_DOWN);
  m_oKeymap.vRegister(GDK_s,            KEY_R);
  m_oKeymap.vRegister(GDK_S,            KEY_R);
  m_oKeymap.vRegister(GDK_a,            KEY_L);
  m_oKeymap.vRegister(GDK_A,            KEY_L);
  m_oKeymap.vRegister(GDK_space,        KEY_SPEED);
  m_oKeymap.vRegister(GDK_F12,          KEY_CAPTURE);
}

void Window::vUpdateScreen()
{
  g_return_if_fail(m_iScreenWidth >= 1
                   && m_iScreenHeight >= 1
                   && m_iScreenScale >= 1);

  m_poScreenArea->vSetSize(m_iScreenWidth, m_iScreenHeight);
  m_poScreenArea->vSetScale(m_iScreenScale);

  resize(1, 1);

  if (emulating)
  {
    vDrawScreen();
  }
  else
  {
    vDrawDefaultScreen();
  }
}

void Window::vDrawScreen()
{
  m_poScreenArea->vDrawPixels(pix);
}

void Window::vDrawDefaultScreen()
{
  m_poScreenArea->vDrawColor(0x000000); // Black
}

bool Window::bLoadROM(const std::string & _rsFilename)
{
  vOnFileClose();

  m_sFilename = _rsFilename;
  const char * csFilename = _rsFilename.c_str();

  IMAGE_TYPE eType = utilFindType(csFilename);
  if (eType == IMAGE_UNKNOWN)
  {
    systemMessage(0, _("Unknown file type %s"), csFilename);
    return false;
  }

  bool bLoaded = false;
  if (eType == IMAGE_GB)
  {
    bLoaded = gbLoadRom(csFilename);
    if (bLoaded)
    {
      m_eCartridge = GB_CARTRIDGE;
      m_stEmulator = GBSystem;

      //if(sdlAutoIPS) {
      //  int size = gbRomSize;
      //  utilApplyIPS(ipsname, &gbRom, &size);
      //  if(size != gbRomSize) {
      //    extern bool gbUpdateSizes();
      //    gbUpdateSizes();
      //    gbReset();
      //  }
      //}

      if (gbBorderOn)
      {
        m_iScreenWidth     = iSGBScreenWidth;
        m_iScreenHeight    = iSGBScreenHeight;
        gbBorderLineSkip   = iSGBScreenWidth;
        gbBorderColumnSkip = (iSGBScreenWidth - iGBScreenWidth) / 2;
        gbBorderRowSkip    = (iSGBScreenHeight - iGBScreenHeight) / 2;
      }
      else
      {
        m_iScreenWidth     = iGBScreenWidth;
        m_iScreenHeight    = iGBScreenHeight;
        gbBorderLineSkip   = iGBScreenWidth;
        gbBorderColumnSkip = 0;
        gbBorderRowSkip    = 0;
      }

      // TODO
      //systemFrameSkip = gbFrameSkip;
    }
  }
  else if (eType == IMAGE_GBA)
  {
    int iSize = CPULoadRom(csFilename);
    bLoaded = (iSize > 0);
    if (bLoaded)
    {
      //sdlApplyPerImagePreferences();

      m_eCartridge = GBA_CARTRIDGE;
      m_stEmulator = GBASystem;

      // TODO
      //CPUInit(biosFileName, useBios);
      useBios = false;
      CPUInit(NULL, useBios);
      CPUReset();

      //if(sdlAutoIPS) {
      //  int size = 0x2000000;
      //  utilApplyIPS(ipsname, &rom, &size);
      //  if(size != 0x2000000) {
      //    CPUReset();
      //  }
      //}

      m_iScreenWidth  = iGBAScreenWidth;
      m_iScreenHeight = iGBAScreenHeight;

      // TODO
      //systemFrameSkip = frameSkip;
    }
  }

  if (! bLoaded)
  {
    systemMessage(0, _("Failed to load file %s"), csFilename);
    return false;
  }

  vLoadBattery();
  vUpdateScreen();

  emulating = 1;

  if (m_poFilePauseItem->get_active())
  {
    m_poFilePauseItem->set_active(false);
  }
  else
  {
    vStartEmu();
  }

  return true;
}

void Window::vLoadBattery()
{
  // TODO : from battery dir

  std::string sBattery = sCutSuffix(m_sFilename) + ".sav";
  if (m_stEmulator.emuReadBattery(sBattery.c_str()))
  {
    systemScreenMessage(_("Loaded battery"));
  }
}

void Window::vSaveBattery()
{
  // TODO : from battery dir

  std::string sBattery = sCutSuffix(m_sFilename) + ".sav";
  if (m_stEmulator.emuWriteBattery(sBattery.c_str()))
  {
    systemScreenMessage(_("Saved battery"));
  }
}

void Window::vLoadState(int _iNum)
{
  // TODO
}

void Window::vSaveState(int _iNum)
{
  // TODO
}

void Window::vStartEmu()
{
  if (m_oEmuSig.connected())
  {
    return;
  }

  m_oEmuSig = Glib::signal_idle().connect(SigC::slot(*this, &Window::bOnEmuIdle),
                                          Glib::PRIORITY_DEFAULT_IDLE);
}

void Window::vStopEmu()
{
  m_oEmuSig.disconnect();
}

void Window::vOnFileOpen()
{
  if (m_poFileOpenDialog == NULL)
  {
    m_poFileOpenDialog = new Gtk::FileSelection(_("Open a ROM"));
  }

  m_poFileOpenDialog->show();

  int iResponse = m_poFileOpenDialog->run();
  if (iResponse == Gtk::RESPONSE_OK)
  {
    if (! bLoadROM(m_poFileOpenDialog->get_filename()))
    {
      return;
    }
  }

  m_poFileOpenDialog->hide();
}

void Window::vOnFilePause()
{
  if (emulating)
  {
    if (m_poFilePauseItem->get_active())
    {
      vStopEmu();
    }
    else
    {
      vStartEmu();
    }
  }
}

void Window::vOnFileReset()
{
  if (emulating)
  {
    m_stEmulator.emuReset();
  }
}

void Window::vOnFileClose()
{
  if (emulating)
  {
    vSaveBattery();
    m_stEmulator.emuCleanUp();
    emulating = 0;
  }

  m_eCartridge = NO_CARTRIDGE;

  vStopEmu();
  vDrawDefaultScreen();
}

void Window::vOnFileQuit()
{
  hide();
}

void Window::vOnVideoZoom1x()
{
  m_iScreenScale = 1;
  vUpdateScreen();
}

void Window::vOnVideoZoom2x()
{
  m_iScreenScale = 2;
  vUpdateScreen();
}

void Window::vOnVideoZoom3x()
{
  m_iScreenScale = 3;
  vUpdateScreen();
}

void Window::vOnVideoZoom4x()
{
  m_iScreenScale = 4;
  vUpdateScreen();
}

void Window::vOnHelpAbout()
{
  Glib::RefPtr<Xml> poXml;
  poXml = Xml::create(PKGDATADIR "/vba.glade", "AboutDialog");

  Gtk::Dialog * poDialog = dynamic_cast<Gtk::Dialog *>(poXml->get_widget("AboutDialog"));
  Gtk::Label *  poLabel  = dynamic_cast<Gtk::Label *>(poXml->get_widget("VersionLabel"));

  poLabel->set_markup("<b><big>" PACKAGE " " VERSION "</big></b>");
  poDialog->run();
  delete poDialog;
}

bool Window::bOnEmuIdle()
{
  if (emulating)
  {
    m_stEmulator.emuMain(m_stEmulator.emuCount);
  }

  return true;
}

bool Window::on_key_press_event(GdkEventKey * _pstEvent)
{
  EKey eKey;

  if ((_pstEvent->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK))
      || (eKey = m_oKeymap.eGetKey(_pstEvent->keyval)) == KEY_NONE)
  {
    return Gtk::Window::on_key_press_event(_pstEvent);
  }

  switch (eKey)
  {
  case KEY_A:
    m_uiJoypadState |= KEYFLAG_A;
    break;
  case KEY_B:
    m_uiJoypadState |= KEYFLAG_B;
    break;
  case KEY_SELECT:
    m_uiJoypadState |= KEYFLAG_SELECT;
    break;
  case KEY_START:
    m_uiJoypadState |= KEYFLAG_START;
    break;
  case KEY_RIGHT:
    m_uiJoypadState |= KEYFLAG_RIGHT;
    m_uiJoypadState &= ~KEYFLAG_LEFT;
    break;
  case KEY_LEFT:
    m_uiJoypadState |= KEYFLAG_LEFT;
    m_uiJoypadState &= ~KEYFLAG_RIGHT;
    break;
  case KEY_UP:
    m_uiJoypadState |= KEYFLAG_UP;
    m_uiJoypadState &= ~KEYFLAG_DOWN;
    break;
  case KEY_DOWN:
    m_uiJoypadState |= KEYFLAG_DOWN;
    m_uiJoypadState &= ~KEYFLAG_UP;
    break;
  case KEY_R:
    m_uiJoypadState |= KEYFLAG_R;
    break;
  case KEY_L:
    m_uiJoypadState |= KEYFLAG_L;
    break;
  case KEY_SPEED:
    m_uiJoypadState |= KEYFLAG_SPEED;
    break;
  case KEY_CAPTURE:
    m_uiJoypadState |= KEYFLAG_CAPTURE;
    break;
  case KEY_NONE:
    break;
  }
  return true;
}

bool Window::on_key_release_event(GdkEventKey * _pstEvent)
{
  EKey eKey;

  if ((_pstEvent->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK))
      || (eKey = m_oKeymap.eGetKey(_pstEvent->keyval)) == KEY_NONE)
  {
    return Gtk::Window::on_key_press_event(_pstEvent);
  }

  switch (eKey)
  {
  case KEY_A:
    m_uiJoypadState &= ~KEYFLAG_A;
    break;
  case KEY_B:
    m_uiJoypadState &= ~KEYFLAG_B;
    break;
  case KEY_SELECT:
    m_uiJoypadState &= ~KEYFLAG_SELECT;
    break;
  case KEY_START:
    m_uiJoypadState &= ~KEYFLAG_START;
    break;
  case KEY_RIGHT:
    m_uiJoypadState &= ~KEYFLAG_RIGHT;
    break;
  case KEY_LEFT:
    m_uiJoypadState &= ~KEYFLAG_LEFT;
    break;
  case KEY_UP:
    m_uiJoypadState &= ~KEYFLAG_UP;
    break;
  case KEY_DOWN:
    m_uiJoypadState &= ~KEYFLAG_DOWN;
    break;
  case KEY_R:
    m_uiJoypadState &= ~KEYFLAG_R;
    break;
  case KEY_L:
    m_uiJoypadState &= ~KEYFLAG_L;
    break;
  case KEY_SPEED:
    m_uiJoypadState &= ~KEYFLAG_SPEED;
    break;
  case KEY_CAPTURE:
    m_uiJoypadState &= ~KEYFLAG_CAPTURE;
    break;
  case KEY_NONE:
    break;
  }
  return true;
}

} // VBA namespace
