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

#include "screenarea.h"
#include "input.h"
#include "filters.h"

namespace VBA
{

class Window : public Gtk::Window
{
  friend class Gnome::Glade::Xml;

public:
  inline static Window * poGetInstance() { return m_poInstance; }

  void vDrawScreen();
  inline u32 uiReadJoypad() { return m_uiJoypadState; }

  enum ECartridge
  {
    NO_CARTRIDGE,
    GB_CARTRIDGE,
    GBA_CARTRIDGE
  };

  inline ECartridge eGetCartridge() { return m_eCartridge; }

protected:
  virtual bool on_key_press_event(GdkEventKey * _pstEvent);
  virtual bool on_key_release_event(GdkEventKey * _pstEvent);

private:
  Window(GtkWindow * _pstWindow,
         const Glib::RefPtr<Gnome::Glade::Xml> & _poXml);
  ~Window();

  static Window * m_poInstance;

  Gtk::FileSelection * m_poFileOpenDialog;
  ScreenArea *         m_poScreenArea;
  Gtk::CheckMenuItem * m_poFilePauseItem;

  SigC::Connection m_oEmuSig;

  std::string    m_sFilename;
  ECartridge     m_eCartridge;
  EmulatedSystem m_stEmulator;
  Keymap         m_oKeymap;
  u32            m_uiJoypadState;

  int m_iScreenWidth;
  int m_iScreenHeight;
  int m_iScreenScale;

  Filter2x m_vFilter2x;
  FilterIB m_vFilterIB;

  void vInitSystem();
  void vInitSDL();
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

  void vOnFileOpen();
  void vOnFilePause();
  void vOnFileReset();
  void vOnFileClose();
  void vOnFileQuit();
  void vOnVideoZoom1x();
  void vOnVideoZoom2x();
  void vOnVideoZoom3x();
  void vOnVideoZoom4x();
  void vOnHelpAbout();
  bool bOnEmuIdle();
};

} // namespace VBA


#endif // __VBA_WINDOW_H__
