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

#include <sys/types.h>

#include <libglademm.h>
#include <gtkmm.h>

#ifndef GTKMM20
# include "sigccompat.h"
#endif // ! GTKMM20

#include <string>
#include <list>

#include "../System.h"

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
    CartridgeNone,
    CartridgeGB,
    CartridgeGBA
  };

  // GB/GBA screen sizes
  const int m_iGBScreenWidth;
  const int m_iGBScreenHeight;
  const int m_iSGBScreenWidth;
  const int m_iSGBScreenHeight;
  const int m_iGBAScreenWidth;
  const int m_iGBAScreenHeight;

  void vDrawScreen();
  void vComputeFrameskip(int _iRate);
  void vShowSpeed(int _iSpeed);

  inline u32        uiReadJoypad()  const { return m_uiJoypadState; }
  inline ECartridge eGetCartridge() const { return m_eCartridge; }
  inline int        iGetThrottle()  const { return m_iThrottle; }

protected:
  enum EShowSpeed
  {
    ShowNone,
    ShowPercentage,
    ShowDetailed
  };

  enum ESaveType
  {
    SaveAuto,
    SaveEEPROM,
    SaveSRAM,
    SaveFlash,
    SaveEEPROMSensor,
    SaveNone
  };

  enum ESoundStatus
  {
    SoundOff,
    SoundMute,
    SoundOn
  };

  enum ESoundQuality
  {
    Sound44K = 1,
    Sound22K = 2,
    Sound11K = 4
  };

  enum ESoundVolume
  {
    Sound100,
    Sound200,
    Sound300,
    Sound400,
    Sound25,
    Sound50
  };

  enum EEmulatorType
  {
    EmulatorAuto,
    EmulatorCGB,
    EmulatorSGB,
    EmulatorGB,
    EmulatorGBA,
    EmulatorSGB2
  };

  virtual void vOnFileOpen();
  virtual void vOnLoadGame();
  virtual void vOnSaveGame();
  virtual void vOnLoadGameMostRecent();
  virtual void vOnLoadGameAutoToggled(Gtk::CheckMenuItem * _poCMI);
  virtual void vOnLoadGameSlot(int _iSlot);
  virtual void vOnSaveGameOldest();
  virtual void vOnSaveGameSlot(int _iSlot);
  virtual void vOnFilePauseToggled(Gtk::CheckMenuItem * _poCMI);
  virtual void vOnFileReset();
  virtual void vOnRecentReset();
  virtual void vOnRecentFreezeToggled(Gtk::CheckMenuItem * _poCMI);
  virtual void vOnRecentFile(std::string _sFile);
  virtual void vOnImportBatteryFile();
  virtual void vOnExportBatteryFile();
  virtual void vOnFileClose();
  virtual void vOnFileExit();
  virtual void vOnFrameskipToggled(Gtk::CheckMenuItem * _poCMI, int _iValue);
  virtual void vOnThrottleToggled(Gtk::CheckMenuItem * _poCMI, int _iPercent);
  virtual void vOnThrottleOther(Gtk::CheckMenuItem * _poCMI);
  virtual void vOnVideoScaleToggled(Gtk::CheckMenuItem * _poCMI, int _iScale);
  virtual void vOnLayerToggled(Gtk::CheckMenuItem * _poCMI, int _iLayer);
  virtual void vOnDirectories();
  virtual void vOnDirectoryReset(Gtk::Entry * _poEntry);
  virtual void vOnDirectorySelect(Gtk::Entry * _poEntry);
  virtual void vOnPauseWhenInactiveToggled(Gtk::CheckMenuItem * _poCMI);
  virtual void vOnSelectBios();
  virtual void vOnUseBiosToggled(Gtk::CheckMenuItem * _poCMI);
  virtual void vOnShowSpeedToggled(Gtk::CheckMenuItem * _poCMI, int _iShowSpeed);
  virtual void vOnSaveTypeToggled(Gtk::CheckMenuItem * _poCMI, int _iSaveType);
  virtual void vOnFlashSizeToggled(Gtk::CheckMenuItem * _poCMI, int _iFlashSize);
  virtual void vOnSoundStatusToggled(Gtk::CheckMenuItem * _poCMI, int _iSoundStatus);
  virtual void vOnSoundEchoToggled(Gtk::CheckMenuItem * _poCMI);
  virtual void vOnSoundLowPassToggled(Gtk::CheckMenuItem * _poCMI);
  virtual void vOnSoundReverseToggled(Gtk::CheckMenuItem * _poCMI);
  virtual void vOnSoundChannelToggled(Gtk::CheckMenuItem * _poCMI, int _iSoundChannel);
  virtual void vOnSoundQualityToggled(Gtk::CheckMenuItem * _poCMI, int _iSoundQuality);
  virtual void vOnSoundVolumeToggled(Gtk::CheckMenuItem * _poCMI, int _iSoundVolume);
  virtual void vOnGBBorderToggled(Gtk::CheckMenuItem * _poCMI);
  virtual void vOnGBPrinterToggled(Gtk::CheckMenuItem * _poCMI);
  virtual void vOnEmulatorTypeToggled(Gtk::CheckMenuItem * _poCMI, int _iEmulatorType);
  virtual void vOnFilter2xToggled(Gtk::CheckMenuItem * _poCMI, int _iFilter2x);
  virtual void vOnFilterIBToggled(Gtk::CheckMenuItem * _poCMI, int _iFilterIB);
#ifdef MMX
  virtual void vOnDisableMMXToggled(Gtk::CheckMenuItem * _poCMI);
#endif // MMX
  virtual void vOnHelpAbout();
  virtual bool bOnEmuIdle();

  virtual bool on_focus_in_event(GdkEventFocus * _pstEvent);
  virtual bool on_focus_out_event(GdkEventFocus * _pstEvent);
  virtual bool on_key_press_event(GdkEventKey * _pstEvent);
  virtual bool on_key_release_event(GdkEventKey * _pstEvent);

private:
  Window(GtkWindow * _pstWindow,
         const Glib::RefPtr<Gnome::Glade::Xml> & _poXml);

  // Config limits
  const int m_iFrameskipMin;
  const int m_iFrameskipMax;
  const int m_iThrottleMin;
  const int m_iThrottleMax;
  const int m_iScaleMin;
  const int m_iScaleMax;
  const int m_iShowSpeedMin;
  const int m_iShowSpeedMax;
  const int m_iSaveTypeMin;
  const int m_iSaveTypeMax;
  const int m_iSoundQualityMin;
  const int m_iSoundQualityMax;
  const int m_iSoundVolumeMin;
  const int m_iSoundVolumeMax;
  const int m_iEmulatorTypeMin;
  const int m_iEmulatorTypeMax;
  const int m_iFilter2xMin;
  const int m_iFilter2xMax;
  const int m_iFilterIBMin;
  const int m_iFilterIBMax;

  static Window * m_poInstance;

  Glib::RefPtr<Gnome::Glade::Xml> m_poXml;

  std::string       m_sUserDataDir;
  std::string       m_sConfigFile;
  Config::File      m_oConfig;
  Config::Section * m_poHistoryConfig;
  Config::Section * m_poDirConfig;
  Config::Section * m_poCoreConfig;
  Config::Section * m_poDisplayConfig;
  Config::Section * m_poSoundConfig;

#ifdef GTKMM20
  Gtk::FileSelection * m_poFileOpenDialog;
#else // ! GTKMM20
  Gtk::FileChooserDialog * m_poFileOpenDialog;
#endif // ! GTKMM20
  ScreenArea *         m_poScreenArea;
  Gtk::Menu *          m_poRecentMenu;
  Gtk::MenuItem *      m_poRecentResetItem;
  Gtk::CheckMenuItem * m_poFilePauseItem;
  Gtk::CheckMenuItem * m_poUseBiosItem;
  Gtk::CheckMenuItem * m_poSoundOffItem;

  struct SGameSlot
  {
    bool        m_bEmpty;
    std::string m_sFile;
    time_t      m_uiTime;
  };

  Gtk::MenuItem * m_apoLoadGameItem[10];
  Gtk::MenuItem * m_apoSaveGameItem[10];
  SGameSlot       m_astGameSlot[10];

  std::list<std::string> m_listHistory;

  std::list<Gtk::Widget *> m_listSensitiveWhenPlaying;

  Gtk::Tooltips m_oTooltips;

  SigC::Connection m_oEmuSig;

  int m_iScreenWidth;
  int m_iScreenHeight;

  std::string    m_sRomFile;
  ECartridge     m_eCartridge;
  EmulatedSystem m_stEmulator;
  Keymap         m_oKeymap;
  u32            m_uiJoypadState;
  bool           m_bPaused;
  bool           m_bWasEmulating;
  bool           m_bAutoFrameskip;
  int            m_iThrottle;
  u32            m_uiThrottleLastTime;
  u32            m_uiThrottleDelay;
  EShowSpeed     m_eShowSpeed;
  ESoundQuality  m_eSoundQuality;

  void vInitSystem();
  void vInitSDL();
  void vInitConfig();
  void vCheckConfig();
  void vLoadConfig(const std::string & _rsFile);
  void vSaveConfig(const std::string & _rsFile);
  void vLoadHistoryFromConfig();
  void vSaveHistoryToConfig();
  void vHistoryAdd(const std::string & _rsFile);
  void vClearHistoryMenu();
  void vUpdateHistoryMenu();
  void vLoadKeymap();
  void vUpdateScreen();
  void vDrawDefaultScreen();
  void vSetDefaultTitle();
  bool bLoadROM(const std::string & _rsFile);
  void vLoadBattery();
  void vSaveBattery();
  void vStartEmu();
  void vStopEmu();
  void vSetThrottle(int _iPercent);
  void vSelectBestThrottleItem();
  void vUpdateGameSlots();
};

} // namespace VBA


#endif // __VBA_WINDOW_H__
