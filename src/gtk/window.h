// -*- C++ -*-
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

#ifndef __VBA_WINDOW_H__
#define __VBA_WINDOW_H__

#include <libglademm.h>
#include <gtkmm.h>

#include <string>

#include "configfile.h"
#include "screenarea.h"
#include "filters.h"
#include "input.h"

namespace VBA
{

class Window : public Gtk::Window
{
  friend class Gnome::Glade::Xml;

public:
  virtual ~Window();

  inline static Window * poGetInstance() { return m_poInstance; }

  enum ECartridge
  {
    NO_CARTRIDGE,
    GB_CARTRIDGE,
    GBA_CARTRIDGE
  };

  void vDrawScreen();
  void vComputeFrameskip(int _iRate);
  inline u32 uiReadJoypad() { return m_uiJoypadState; }
  inline ECartridge eGetCartridge() { return m_eCartridge; }

protected:
  virtual void vOnFileOpen();
  virtual void vOnFilePauseToggled(Gtk::CheckMenuItem * _poCMI);
  virtual void vOnFileReset();
  virtual void vOnFileClose();
  virtual void vOnFileExit();
  virtual void vOnFrameskipToggled(Gtk::CheckMenuItem * _poCMI, int _iValue);
  virtual void vOnThrottleToggled(Gtk::CheckMenuItem * _poCMI, int _iPercent);
  virtual void vOnThrottleOther(Gtk::CheckMenuItem * _poCMI);
  virtual void vOnLayerToggled(Gtk::CheckMenuItem * _poCMI, int _iLayer);
  virtual void vOnVideoScaleToggled(Gtk::CheckMenuItem * _poCMI, int _iScale);
  virtual void vOnFilter2xToggled(Gtk::CheckMenuItem * _poCMI, int _iFilter2x);
  virtual void vOnFilterIBToggled(Gtk::CheckMenuItem * _poCMI, int _iFilterIB);
#ifdef MMX
  virtual void vOnDisableMMXToggled(Gtk::CheckMenuItem * _poCMI);
#endif // MMX
  virtual void vOnHelpAbout();
  virtual bool bOnEmuIdle();

  virtual bool on_key_press_event(GdkEventKey * _pstEvent);
  virtual bool on_key_release_event(GdkEventKey * _pstEvent);

private:
  Window(GtkWindow * _pstWindow,
         const Glib::RefPtr<Gnome::Glade::Xml> & _poXml);

  static Window * m_poInstance;

  Glib::RefPtr<Gnome::Glade::Xml> m_poXml;

  std::string       m_sUserDataDir;
  std::string       m_sConfigFile;
  Config::File      m_oConfig;
  Config::Section * m_poCoreConfig;
  Config::Section * m_poDisplayConfig;

  Gtk::FileSelection * m_poFileOpenDialog;
  ScreenArea *         m_poScreenArea;
  Gtk::CheckMenuItem * m_poFilePauseItem;

  SigC::Connection m_oEmuSig;

  int m_iScreenWidth;
  int m_iScreenHeight;

  std::string    m_sRomFile;
  ECartridge     m_eCartridge;
  EmulatedSystem m_stEmulator;
  Keymap         m_oKeymap;
  u32            m_uiJoypadState;
  bool           m_bWasEmulating;
  bool           m_bAutoFrameskip;
  int            m_iThrottle;
  u32            m_uiThrottleLastTime;
  u32            m_uiThrottleDelay;

  void vInitSystem();
  void vInitSDL();
  void vInitConfig();
  void vCheckConfig();
  void vLoadConfig(const std::string & _sFilename);
  void vSaveConfig(const std::string & _sFilename);
  void vLoadKeymap();
  void vUpdateScreen();
  void vDrawDefaultScreen();
  bool bLoadROM(const std::string & _rsFilename);
  void vLoadBattery();
  void vSaveBattery();
  void vLoadState(int _iNum);
  void vSaveState(int _iNum);
  void vStartEmu();
  void vStopEmu();
  void vSetThrottle(int _iPercent);
  void vSelectBestThrottleItem();
};

} // namespace VBA


#endif // __VBA_WINDOW_H__
