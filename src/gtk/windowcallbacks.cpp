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

#include <sys/stat.h>

#include <stdio.h>
#include <time.h>

#include <SDL.h>

#include "../GBA.h"
#include "../gb/GB.h"
#include "../gb/gbGlobals.h"
#include "../gb/gbPrinter.h"
#include "../Sound.h"
#include "../Util.h"

#include "tools.h"
#include "intl.h"

extern int systemRenderedFrames;
extern int systemFPS;
extern bool debugger;
extern int RGB_LOW_BITS_MASK;

#ifdef MMX
extern "C" bool cpu_mmx;
#endif // MMX

namespace VBA
{

using Gnome::Glade::Xml;

void Window::vOnFileOpen()
{
  if (m_poFileOpenDialog == NULL)
  {
    m_poFileOpenDialog = new Gtk::FileSelection(_("Open"));
    m_poFileOpenDialog->set_transient_for(*this);

    std::string sDir = m_poDirConfig->sGetKey("gba_roms");
    if (sDir != "")
    {
      m_poFileOpenDialog->set_filename(sDir + "/");
    }
  }

  while (m_poFileOpenDialog->run() == Gtk::RESPONSE_OK)
  {
    if (bLoadROM(m_poFileOpenDialog->get_filename()))
    {
      break;
    }
  }
  m_poFileOpenDialog->hide();
}

void Window::vOnLoadGameMostRecent()
{
  int    iMostRecent = -1;
  time_t uiTimeMax;

  for (int i = 0; i < 10; i++)
  {
    if (! m_astGameSlot[i].m_bEmpty
        && (iMostRecent < 0 || m_astGameSlot[i].m_uiTime > uiTimeMax))
    {
      iMostRecent = i;
      uiTimeMax = m_astGameSlot[i].m_uiTime;
    }
  }

  if (iMostRecent >= 0)
  {
    vOnLoadGame(iMostRecent + 1);
  }
}

void Window::vOnLoadGameAutoToggled(Gtk::CheckMenuItem * _poCMI)
{
  m_poCoreConfig->vSetKey("load_game_auto", _poCMI->get_active());
}

void Window::vOnLoadGame(int _iSlot)
{
  int i = _iSlot - 1;
  if (! m_astGameSlot[i].m_bEmpty)
  {
    m_stEmulator.emuReadState(m_astGameSlot[i].m_sFile.c_str());
    m_poFilePauseItem->set_active(false);
  }
}

void Window::vOnSaveGameOldest()
{
  int    iOldest = -1;
  time_t uiTimeMin;

  for (int i = 0; i < 10; i++)
  {
    if (! m_astGameSlot[i].m_bEmpty
        && (iOldest < 0 || m_astGameSlot[i].m_uiTime < uiTimeMin))
    {
      iOldest = i;
      uiTimeMin = m_astGameSlot[i].m_uiTime;
    }
  }

  if (iOldest >= 0)
  {
    vOnSaveGame(iOldest + 1);
  }
  else
  {
    vOnSaveGame(1);
  }
}

void Window::vOnSaveGame(int _iSlot)
{
  int i = _iSlot - 1;
  m_stEmulator.emuWriteState(m_astGameSlot[i].m_sFile.c_str());
  vUpdateGameSlots();
}

void Window::vOnFilePauseToggled(Gtk::CheckMenuItem * _poCMI)
{
  m_bPaused = _poCMI->get_active();
  if (emulating)
  {
    if (m_bPaused)
    {
      vStopEmu();
      soundPause();
    }
    else
    {
      vStartEmu();
      soundResume();
    }
  }
}

void Window::vOnFileReset()
{
  if (emulating)
  {
    m_stEmulator.emuReset();
    m_poFilePauseItem->set_active(false);
  }
}

void Window::vOnRecentReset()
{
  m_listHistory.clear();
  vClearHistoryMenu();
}

void Window::vOnRecentFreezeToggled(Gtk::CheckMenuItem * _poCMI)
{
  m_poHistoryConfig->vSetKey("freeze", _poCMI->get_active());
}

void Window::vOnRecent(std::string _sFile)
{
  bLoadROM(_sFile);
}

void Window::vOnFileClose()
{
  if (emulating)
  {
    soundPause();
    vStopEmu();
    vSetDefaultTitle();
    vDrawDefaultScreen();
    vSaveBattery();
    m_stEmulator.emuCleanUp();
    m_eCartridge = CartridgeNone;
    emulating = 0;

    vUpdateGameSlots();

    for (std::list<Gtk::Widget *>::iterator it = m_listSensitiveWhenPlaying.begin();
         it != m_listSensitiveWhenPlaying.end();
         it++)
    {
      (*it)->set_sensitive(false);
    }

    m_poFilePauseItem->set_active(false);
  }
}

void Window::vOnFileExit()
{
  hide();
}

void Window::vOnFrameskipToggled(Gtk::CheckMenuItem * _poCMI, int _iValue)
{
  if (! _poCMI->get_active())
  {
    return;
  }

  if (_iValue >= 0 && _iValue <= 9)
  {
    m_poCoreConfig->vSetKey("frameskip", _iValue);
    gbFrameSkip      = _iValue;
    systemFrameSkip  = _iValue;
    m_bAutoFrameskip = false;
  }
  else
  {
    m_poCoreConfig->vSetKey("frameskip", "auto");
    gbFrameSkip      = 0;
    systemFrameSkip  = 0;
    m_bAutoFrameskip = true;
  }
}

void Window::vOnThrottleToggled(Gtk::CheckMenuItem * _poCMI, int _iPercent)
{
  if (! _poCMI->get_active())
  {
    return;
  }

  vSetThrottle(_iPercent);

  // Initialize the frameskip adjustment each time throttle is changed
  if (m_bAutoFrameskip)
  {
    systemFrameSkip = 0;
  }
}

void Window::vOnThrottleOther(Gtk::CheckMenuItem * _poCMI)
{
  if (! _poCMI->get_active())
  {
    return;
  }

  Glib::RefPtr<Xml> poXml;
  poXml = Xml::create(PKGDATADIR "/vba.glade", "ThrottleDialog");

  Gtk::Dialog * poDialog = dynamic_cast<Gtk::Dialog *>(poXml->get_widget("ThrottleDialog"));
  Gtk::SpinButton * poSpin = dynamic_cast<Gtk::SpinButton *>(poXml->get_widget("ThrottleSpin"));

  poDialog->set_transient_for(*this);

  if (m_iThrottle != 0)
  {
    poSpin->set_value(m_iThrottle);
  }
  else
  {
    poSpin->set_value(100);
  }

  if (poDialog->run() == Gtk::RESPONSE_OK)
  {
    vSetThrottle(poSpin->get_value_as_int());
  }

  delete poDialog;
  vSelectBestThrottleItem();
}

void Window::vOnVideoScaleToggled(Gtk::CheckMenuItem * _poCMI, int _iScale)
{
  if (! _poCMI->get_active())
  {
    return;
  }

  m_poDisplayConfig->vSetKey("scale", _iScale);
  vUpdateScreen();
}

void Window::vOnLayerToggled(Gtk::CheckMenuItem * _poCMI, int _iLayer)
{
  int iMask = (0x0100 << _iLayer);
  if (_poCMI->get_active())
  {
    layerSettings |= iMask;
  }
  else
  {
    layerSettings &= ~iMask;
  }
  layerEnable = DISPCNT & layerSettings;

  const char * acsLayers[] =
  {
    "layer_bg0",
    "layer_bg1",
    "layer_bg2",
    "layer_bg3",
    "layer_obj",
    "layer_win0",
    "layer_win1",
    "layer_objwin"
  };
  m_poCoreConfig->vSetKey(acsLayers[_iLayer], _poCMI->get_active());
}

void Window::vOnDirectories()
{
  Glib::RefPtr<Xml> poXml;
  poXml = Xml::create(PKGDATADIR "/vba.glade", "DirectoriesDialog");

  struct
  {
    const char * m_csKey;
    const char * m_csEntry;
    const char * m_csResetButton;
    const char * m_csSelectButton;
  }
  astRow[] =
  {
    { "gba_roms",  "GBARomsDirEntry",   "GBARomsDirResetButton",   "GBARomsDirSelectButton"   },
    { "gb_roms",   "GBRomsDirEntry",    "GBRomsDirResetButton",    "GBRomsDirSelectButton"    },
    { "batteries", "BatteriesDirEntry", "BatteriesDirResetButton", "BatteriesDirSelectButton" },
    { "saves",     "SavesDirEntry",     "SavesDirResetButton",     "SavesDirSelectButton"     },
    { "captures",  "CapturesDirEntry",  "CapturesDirResetButton",  "CapturesDirSelectButton"  }
  };

  for (guint i = 0; i < sizeof(astRow) / sizeof(astRow[0]); i++)
  {
    Gtk::Entry *  poEntry  = dynamic_cast<Gtk::Entry *>(poXml->get_widget(astRow[i].m_csEntry));
    Gtk::Button * poReset  = dynamic_cast<Gtk::Button *>(poXml->get_widget(astRow[i].m_csResetButton));
    Gtk::Button * poSelect = dynamic_cast<Gtk::Button *>(poXml->get_widget(astRow[i].m_csSelectButton));

    poEntry->set_text(m_poDirConfig->sGetKey(astRow[i].m_csKey));

    poReset->signal_clicked().connect(SigC::bind<Gtk::Entry *>(
                                        SigC::slot(*this, &Window::vOnDirectoryReset),
                                        poEntry));
    poSelect->signal_clicked().connect(SigC::bind<Gtk::Entry *>(
                                         SigC::slot(*this, &Window::vOnDirectorySelect),
                                         poEntry));
  }

  Gtk::Dialog * poDialog = dynamic_cast<Gtk::Dialog *>(poXml->get_widget("DirectoriesDialog"));
  poDialog->set_transient_for(*this);

  if (poDialog->run() == Gtk::RESPONSE_OK)
  {
    for (guint i = 0; i < sizeof(astRow) / sizeof(astRow[0]); i++)
    {
      Gtk::Entry * poEntry = dynamic_cast<Gtk::Entry *>(poXml->get_widget(astRow[i].m_csEntry));
      Glib::ustring sDir = poEntry->get_text();
      if (! Glib::file_test(sDir, Glib::FILE_TEST_IS_DIR))
      {
        sDir = "";
      }
      m_poDirConfig->vSetKey(astRow[i].m_csKey, sDir);
    }

    // Needed if saves dir changed
    vUpdateGameSlots();
  }

  delete poDialog;
}

void Window::vOnDirectoryReset(Gtk::Entry * _poEntry)
{
  _poEntry->set_text("");
}

void Window::vOnDirectorySelect(Gtk::Entry * _poEntry)
{
  Gtk::FileSelection * poDialog = new Gtk::FileSelection(_("Select directory"));
  poDialog->set_transient_for(*this);

  if (_poEntry->get_text() != "")
  {
    poDialog->set_filename(_poEntry->get_text() + "/");
  }

  if (poDialog->run() == Gtk::RESPONSE_OK)
  {
    std::string sFile = poDialog->get_filename();
    if (! Glib::file_test(sFile, Glib::FILE_TEST_IS_DIR))
    {
      sFile = Glib::path_get_dirname(sFile);
    }
    _poEntry->set_text(sFile);
  }

  delete poDialog;
}

void Window::vOnPauseWhenInactiveToggled(Gtk::CheckMenuItem * _poCMI)
{
  m_poDisplayConfig->vSetKey("pause_when_inactive", _poCMI->get_active());
}

void Window::vOnSelectBios()
{
  Gtk::FileSelection * poDialog = new Gtk::FileSelection(_("Select BIOS file"));
  poDialog->set_transient_for(*this);

  if (m_poCoreConfig->sGetKey("bios_file") != "")
  {
    poDialog->set_filename(m_poCoreConfig->sGetKey("bios_file"));
  }

  while (poDialog->run() == Gtk::RESPONSE_OK)
  {
    if (Glib::file_test(poDialog->get_filename(), Glib::FILE_TEST_IS_REGULAR))
    {
      m_poCoreConfig->vSetKey("bios_file", poDialog->get_filename());
      m_poUseBiosItem->set_sensitive();
      break;
    }
  }

  delete poDialog;
}

void Window::vOnUseBiosToggled(Gtk::CheckMenuItem * _poCMI)
{
  m_poCoreConfig->vSetKey("use_bios_file", _poCMI->get_active());
}

void Window::vOnShowSpeedToggled(Gtk::CheckMenuItem * _poCMI, int _iShowSpeed)
{
  if (! _poCMI->get_active())
  {
    return;
  }

  m_eShowSpeed = (EShowSpeed)_iShowSpeed;
  if (m_eShowSpeed == ShowNone)
  {
    vSetDefaultTitle();
  }
  m_poDisplayConfig->vSetKey("show_speed", _iShowSpeed);
}

void Window::vOnSaveTypeToggled(Gtk::CheckMenuItem * _poCMI, int _iSaveType)
{
  if (! _poCMI->get_active())
  {
    return;
  }

  cpuSaveType = _iSaveType;
  m_poCoreConfig->vSetKey("save_type", _iSaveType);
}

void Window::vOnFlashSizeToggled(Gtk::CheckMenuItem * _poCMI, int _iFlashSize)
{
  if (! _poCMI->get_active())
  {
    return;
  }

  if (_iFlashSize == 64)
  {
    flashSetSize(0x10000);
  }
  else
  {
    flashSetSize(0x20000);
  }
  m_poCoreConfig->vSetKey("flash_size", _iFlashSize);
}

void Window::vOnSoundStatusToggled(Gtk::CheckMenuItem * _poCMI, int _iSoundStatus)
{
  if (! _poCMI->get_active())
  {
    return;
  }

  std::string sSoundStatus;
  switch (_iSoundStatus)
  {
  case SoundOff:
    soundOffFlag = true;
    if (systemSoundOn)
    {
      soundShutdown();
    }
    sSoundStatus = "off";
    break;
  case SoundMute:
    soundDisable(0x30f);
    sSoundStatus = "mute";
    break;
  case SoundOn:
    if (soundOffFlag)
    {
      soundOffFlag = false;
      if (! soundInit())
      {
        m_poSoundOffItem->set_active();
        return;
      }
    }
    soundEnable(0x30f);
    sSoundStatus = "on";
    break;
  }
  m_poSoundConfig->vSetKey("status", sSoundStatus);
}

void Window::vOnSoundEchoToggled(Gtk::CheckMenuItem * _poCMI)
{
  soundEcho = _poCMI->get_active();
  m_poSoundConfig->vSetKey("echo", soundEcho);
}

void Window::vOnSoundLowPassToggled(Gtk::CheckMenuItem * _poCMI)
{
  soundLowPass = _poCMI->get_active();
  m_poSoundConfig->vSetKey("low_pass", soundLowPass);
}

void Window::vOnSoundReverseToggled(Gtk::CheckMenuItem * _poCMI)
{
  soundReverse = _poCMI->get_active();
  m_poSoundConfig->vSetKey("reverse_stereo", soundReverse);
}

void Window::vOnSoundChannelToggled(Gtk::CheckMenuItem * _poCMI, int _iSoundChannel)
{
  int iShift = _iSoundChannel;
  if (_iSoundChannel > 3)
  {
    iShift += 4;
  }
  int iFlag = 1 << iShift;
  int iActive = soundGetEnable() & 0x30f;
  if (_poCMI->get_active())
  {
    iActive |= iFlag;
  }
  else
  {
    iActive &= ~iFlag;
  }
  soundEnable(iActive);
  soundDisable(~iActive & 0x30f);

  const char * acsChannels[] =
  {
    "channel_1",
    "channel_2",
    "channel_3",
    "channel_4",
    "channel_A",
    "channel_B"
  };
  m_poSoundConfig->vSetKey(acsChannels[_iSoundChannel], _poCMI->get_active());
}

void Window::vOnSoundQualityToggled(Gtk::CheckMenuItem * _poCMI, int _iSoundQuality)
{
  if (! _poCMI->get_active())
  {
    return;
  }

  m_eSoundQuality = (ESoundQuality)_iSoundQuality;
  if (m_eCartridge == CartridgeGBA)
  {
    soundSetQuality(_iSoundQuality);
  }
  else if (m_eCartridge == CartridgeGB)
  {
    gbSoundSetQuality(_iSoundQuality);
  }
  m_poSoundConfig->vSetKey("quality", _iSoundQuality);
}

void Window::vOnSoundVolumeToggled(Gtk::CheckMenuItem * _poCMI, int _iSoundVolume)
{
  if (! _poCMI->get_active())
  {
    return;
  }

  soundVolume = _iSoundVolume;
  m_poSoundConfig->vSetKey("volume", _iSoundVolume);
}

void Window::vOnGBBorderToggled(Gtk::CheckMenuItem * _poCMI)
{
  gbBorderOn = _poCMI->get_active();
  if (emulating && m_eCartridge == CartridgeGB && _poCMI->get_active())
  {
    gbSgbRenderBorder();
  }
  vUpdateScreen();
  m_poCoreConfig->vSetKey("gb_border", _poCMI->get_active());
}

void Window::vOnGBPrinterToggled(Gtk::CheckMenuItem * _poCMI)
{
  if (_poCMI->get_active())
  {
    gbSerialFunction = gbPrinterSend;
  }
  else
  {
    gbSerialFunction = NULL;
  }
  m_poCoreConfig->vSetKey("gb_printer", _poCMI->get_active());
}

void Window::vOnEmulatorTypeToggled(Gtk::CheckMenuItem * _poCMI, int _iEmulatorType)
{
  gbEmulatorType = _iEmulatorType;
  m_poCoreConfig->vSetKey("emulator_type", _iEmulatorType);
}

void Window::vOnFilter2xToggled(Gtk::CheckMenuItem * _poCMI, int _iFilter2x)
{
  if (! _poCMI->get_active())
  {
    return;
  }

  m_poScreenArea->vSetFilter2x((EFilter2x)_iFilter2x);
  if (emulating)
  {
    vDrawScreen();
  }
  m_poDisplayConfig->vSetKey("filter2x", _iFilter2x);
}

void Window::vOnFilterIBToggled(Gtk::CheckMenuItem * _poCMI, int _iFilterIB)
{
  if (! _poCMI->get_active())
  {
    return;
  }

  m_poScreenArea->vSetFilterIB((EFilterIB)_iFilterIB);
  if (emulating)
  {
    vDrawScreen();
  }
  m_poDisplayConfig->vSetKey("filterIB", _iFilterIB);
}

#ifdef MMX
void Window::vOnDisableMMXToggled(Gtk::CheckMenuItem * _poCMI)
{
  cpu_mmx = ! _poCMI->get_active();
  m_poDisplayConfig->vSetKey("filter_disable_mmx", _poCMI->get_active());
}
#endif // MMX

void Window::vOnHelpAbout()
{
  Glib::RefPtr<Xml> poXml;
  poXml = Xml::create(PKGDATADIR "/vba.glade", "AboutDialog");

  Gtk::Dialog * poDialog = dynamic_cast<Gtk::Dialog *>(poXml->get_widget("AboutDialog"));
  Gtk::Label *  poLabel  = dynamic_cast<Gtk::Label *>(poXml->get_widget("VersionLabel"));

  poDialog->set_transient_for(*this);
  poLabel->set_markup("<b><big>" PACKAGE " " VERSION "</big></b>");
  poDialog->run();
  delete poDialog;
}

bool Window::bOnEmuIdle()
{
  if (m_uiThrottleDelay != 0)
  {
    u32 uiTime = SDL_GetTicks();
    if (uiTime - m_uiThrottleLastTime >= m_uiThrottleDelay)
    {
      m_uiThrottleDelay = 0;
      m_uiThrottleLastTime = uiTime;
    }
    else
    {
      return true;
    }
  }

  m_stEmulator.emuMain(m_stEmulator.emuCount);
  return true;
}

bool Window::on_focus_in_event(GdkEventFocus * _pstEvent)
{
  if (emulating
      && ! m_bPaused
      && m_poDisplayConfig->oGetKey<bool>("pause_when_inactive"))
  {
    vStartEmu();
    soundResume();
  }
  return false;
}

bool Window::on_focus_out_event(GdkEventFocus * _pstEvent)
{
  if (emulating
      && ! m_bPaused
      && m_poDisplayConfig->oGetKey<bool>("pause_when_inactive"))
  {
    vStopEmu();
    soundPause();
  }
  return false;
}

bool Window::on_key_press_event(GdkEventKey * _pstEvent)
{
  EKey eKey;

  if ((_pstEvent->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK))
      || (eKey = m_oKeymap.eGetKey(_pstEvent->keyval)) == KeyNone)
  {
    return Gtk::Window::on_key_press_event(_pstEvent);
  }

  switch (eKey)
  {
  case KeyA:
    m_uiJoypadState |= KeyFlagA;
    break;
  case KeyB:
    m_uiJoypadState |= KeyFlagB;
    break;
  case KeySelect:
    m_uiJoypadState |= KeyFlagSelect;
    break;
  case KeyStart:
    m_uiJoypadState |= KeyFlagStart;
    break;
  case KeyRight:
    m_uiJoypadState |= KeyFlagRight;
    m_uiJoypadState &= ~KeyFlagLeft;
    break;
  case KeyLeft:
    m_uiJoypadState |= KeyFlagLeft;
    m_uiJoypadState &= ~KeyFlagRight;
    break;
  case KeyUp:
    m_uiJoypadState |= KeyFlagUp;
    m_uiJoypadState &= ~KeyFlagDown;
    break;
  case KeyDown:
    m_uiJoypadState |= KeyFlagDown;
    m_uiJoypadState &= ~KeyFlagUp;
    break;
  case KeyR:
    m_uiJoypadState |= KeyFlagR;
    break;
  case KeyL:
    m_uiJoypadState |= KeyFlagL;
    break;
  case KeySpeed:
    m_uiJoypadState |= KeyFlagSpeed;
    break;
  case KeyCapture:
    m_uiJoypadState |= KeyFlagCapture;
    break;
  case KeyNone:
    break;
  }
  return true;
}

bool Window::on_key_release_event(GdkEventKey * _pstEvent)
{
  EKey eKey;

  if ((_pstEvent->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK))
      || (eKey = m_oKeymap.eGetKey(_pstEvent->keyval)) == KeyNone)
  {
    return Gtk::Window::on_key_release_event(_pstEvent);
  }

  switch (eKey)
  {
  case KeyA:
    m_uiJoypadState &= ~KeyFlagA;
    break;
  case KeyB:
    m_uiJoypadState &= ~KeyFlagB;
    break;
  case KeySelect:
    m_uiJoypadState &= ~KeyFlagSelect;
    break;
  case KeyStart:
    m_uiJoypadState &= ~KeyFlagStart;
    break;
  case KeyRight:
    m_uiJoypadState &= ~KeyFlagRight;
    break;
  case KeyLeft:
    m_uiJoypadState &= ~KeyFlagLeft;
    break;
  case KeyUp:
    m_uiJoypadState &= ~KeyFlagUp;
    break;
  case KeyDown:
    m_uiJoypadState &= ~KeyFlagDown;
    break;
  case KeyR:
    m_uiJoypadState &= ~KeyFlagR;
    break;
  case KeyL:
    m_uiJoypadState &= ~KeyFlagL;
    break;
  case KeySpeed:
    m_uiJoypadState &= ~KeyFlagSpeed;
    break;
  case KeyCapture:
    m_uiJoypadState &= ~KeyFlagCapture;
    break;
  case KeyNone:
    break;
  }
  return true;
}

} // namespace VBA
