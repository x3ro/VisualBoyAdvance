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
    std::string sGBDir  = m_poDirConfig->sGetKey("gb_roms");
    std::string sGBADir = m_poDirConfig->sGetKey("gba_roms");

#ifdef GTKMM20

    Gtk::FileSelection * poDialog = new Gtk::FileSelection(_("Open"));
    poDialog->set_transient_for(*this);

    if (sGBADir != "")
    {
      poDialog->set_filename(sGBADir + "/");
    }
    else if (sGBDir != "")
    {
      poDialog->set_filename(sGBDir + "/");
    }

#else // ! GTKMM20

    Gtk::FileChooserDialog * poDialog = new Gtk::FileChooserDialog(*this, _("Open"));
    poDialog->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    poDialog->add_button(Gtk::Stock::OPEN,   Gtk::RESPONSE_OK);

    if (sGBDir != "")
    {
      poDialog->add_shortcut_folder(sGBDir);
      poDialog->set_current_folder(sGBDir);
    }

    if (sGBADir != "" && sGBADir != sGBDir)
    {
      poDialog->add_shortcut_folder(sGBADir);
      poDialog->set_current_folder(sGBADir);
    }

    const char * acsPattern[] =
    {
      // GBA
      "*.[bB][iI][nN]", "*.[aA][gG][bB]", "*.[gG][bB][aA]",
      // GB
      "*.[gG][bB]", "*.[sS][gG][bB]", "*.[cC][gG][bB]", "*.[gG][bB][cC]",
      // Both
      "*.[mM][bB]", "*.[eE][lL][fF]", "*.[zZ][iI][pP]", "*.[zZ]", "*.[gG][zZ]"
    };

    Gtk::FileFilter oAllGBAFilter;
    oAllGBAFilter.set_name(_("All Gameboy Advance files"));
    for (guint i = 0; i < sizeof(acsPattern) / sizeof(acsPattern[0]); i++)
    {
      oAllGBAFilter.add_pattern(acsPattern[i]);
    }

    Gtk::FileFilter oGBAFilter;
    oGBAFilter.set_name(_("Gameboy Advance files"));
    for (int i = 0; i < 3; i++)
    {
      oGBAFilter.add_pattern(acsPattern[i]);
    }

    Gtk::FileFilter oGBFilter;
    oGBFilter.set_name(_("Gameboy files"));
    for (int i = 3; i < 7; i++)
    {
      oGBFilter.add_pattern(acsPattern[i]);
    }

    poDialog->add_filter(oAllGBAFilter);
    poDialog->add_filter(oGBAFilter);
    poDialog->add_filter(oGBFilter);

#endif // ! GTKMM20

    m_poFileOpenDialog = poDialog;
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

void Window::vOnLoadGame()
{
  std::string sSaveDir = m_poDirConfig->sGetKey("saves");

#ifdef GTKMM20

  Gtk::FileSelection oDialog(_("Load game"));
  oDialog.set_transient_for(*this);

  if (sSaveDir == "")
  {
    oDialog.set_filename(Glib::path_get_dirname(m_sRomFile) + "/");
  }
  else
  {
    oDialog.set_filename(sSaveDir + "/");
  }

#else // ! GTKMM20

  Gtk::FileChooserDialog oDialog(*this, _("Load game"));
  oDialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  oDialog.add_button(Gtk::Stock::OPEN,   Gtk::RESPONSE_OK);

  if (sSaveDir == "")
  {
    oDialog.set_current_folder(Glib::path_get_dirname(m_sRomFile));
  }
  else
  {
    oDialog.set_current_folder(sSaveDir);
    oDialog.add_shortcut_folder(sSaveDir);
  }

  Gtk::FileFilter oSaveFilter;
  oSaveFilter.set_name(_("VisualBoyAdvance save game"));
  oSaveFilter.add_pattern("*.[sS][gG][mM]");

  oDialog.add_filter(oSaveFilter);

#endif // ! GTKMM20

  while (oDialog.run() == Gtk::RESPONSE_OK)
  {
    if (m_stEmulator.emuReadState(oDialog.get_filename().c_str()))
    {
      break;
    }
  }
}

void Window::vOnSaveGame()
{
  Glib::ustring sSaveDir = m_poDirConfig->sGetKey("saves");

#ifdef GTKMM20

  Gtk::FileSelection oDialog(_("Save game"));
  oDialog.set_transient_for(*this);

  Glib::ustring sDefaultFile;
  if (sSaveDir == "")
  {
    sDefaultFile = sCutSuffix(m_sRomFile) + ".sgm";
  }
  else
  {
    sDefaultFile = sSaveDir + "/"
      + sCutSuffix(Glib::path_get_basename(m_sRomFile)) + ".sgm";
  }
  oDialog.set_filename(sDefaultFile);

#else // ! GTKMM20

  Gtk::FileChooserDialog oDialog(*this, _("Save game"),
                                 Gtk::FILE_CHOOSER_ACTION_SAVE);
  oDialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  oDialog.add_button(Gtk::Stock::SAVE,   Gtk::RESPONSE_OK);

  if (sSaveDir == "")
  {
    oDialog.set_current_folder(Glib::path_get_dirname(m_sRomFile));
  }
  else
  {
    oDialog.set_current_folder(sSaveDir);
    oDialog.add_shortcut_folder(sSaveDir);
  }
  oDialog.set_current_name(sCutSuffix(Glib::path_get_basename(m_sRomFile)) + ".sgm");

  Gtk::FileFilter oSaveFilter;
  oSaveFilter.set_name(_("VisualBoyAdvance save game"));
  oSaveFilter.add_pattern("*.[sS][gG][mM]");

  oDialog.add_filter(oSaveFilter);

#endif // ! GTKMM20

  while (oDialog.run() == Gtk::RESPONSE_OK)
  {
    Glib::ustring sFile = oDialog.get_filename();
    if (! bHasSuffix(sFile, ".sgm", false))
    {
      sFile += ".sgm";
    }

    if (Glib::file_test(sFile, Glib::FILE_TEST_EXISTS))
    {
      Gtk::MessageDialog oConfirmDialog(*this,
                                        _("File already exists. Overwrite it?"),
#ifndef GTKMM20
                                        false,
#endif // ! GTKMM20
                                        Gtk::MESSAGE_QUESTION,
                                        Gtk::BUTTONS_YES_NO);
      if (oConfirmDialog.run() != Gtk::RESPONSE_YES)
      {
        continue;
      }
    }

    if (m_stEmulator.emuWriteState(sFile.c_str()))
    {
      break;
    }
  }
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
    vOnLoadGameSlot(iMostRecent + 1);
  }
}

void Window::vOnLoadGameAutoToggled(Gtk::CheckMenuItem * _poCMI)
{
  m_poCoreConfig->vSetKey("load_game_auto", _poCMI->get_active());
}

void Window::vOnLoadGameSlot(int _iSlot)
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
    vOnSaveGameSlot(iOldest + 1);
  }
  else
  {
    vOnSaveGameSlot(1);
  }
}

void Window::vOnSaveGameSlot(int _iSlot)
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
  m_poRecentResetItem->set_sensitive(! _poCMI->get_active());
  m_poHistoryConfig->vSetKey("freeze", _poCMI->get_active());
}

void Window::vOnRecentFile(std::string _sFile)
{
  bLoadROM(_sFile);
}

void Window::vOnImportBatteryFile()
{
  std::string BatteryDir = m_poDirConfig->sGetKey("batteries");

#ifdef GTKMM20

  Gtk::FileSelection oDialog(_("Import battery file"));
  oDialog.set_transient_for(*this);

  if (BatteryDir == "")
  {
    oDialog.set_filename(Glib::path_get_dirname(m_sRomFile) + "/");
  }
  else
  {
    oDialog.set_filename(BatteryDir + "/");
  }

#else // ! GTKMM20

  Gtk::FileChooserDialog oDialog(*this, _("Import battery file"));
  oDialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  oDialog.add_button(Gtk::Stock::OPEN,   Gtk::RESPONSE_OK);

  if (BatteryDir == "")
  {
    oDialog.set_current_folder(Glib::path_get_dirname(m_sRomFile));
  }
  else
  {
    oDialog.set_current_folder(BatteryDir);
    oDialog.add_shortcut_folder(BatteryDir);
  }

  Gtk::FileFilter oBatteryFilter;
  oBatteryFilter.set_name(_("Battery file"));
  oBatteryFilter.add_pattern("*.[sS][aA][vV]");

  Gtk::FileFilter oFlashFilter;
  oFlashFilter.set_name(_("Flash save"));
  oFlashFilter.add_pattern("*.[dD][aA][tT]");

  oDialog.add_filter(oBatteryFilter);
  oDialog.add_filter(oFlashFilter);

#endif // ! GTKMM20

  while (oDialog.run() == Gtk::RESPONSE_OK)
  {
    Gtk::MessageDialog oConfirmDialog(*this,
                                      _("Importing a battery file will erase any saved games and reset the emulator. Do you want to continue?"),
#ifndef GTKMM20
                                      false,
#endif // ! GTKMM20
                                      Gtk::MESSAGE_QUESTION,
                                      Gtk::BUTTONS_YES_NO);
    if (oConfirmDialog.run() != Gtk::RESPONSE_YES)
    {
      continue;
    }

    if (m_stEmulator.emuReadBattery(oDialog.get_filename().c_str()))
    {
      m_stEmulator.emuReset();
      break;
    }
    else
    {
      vPopupError(_("Failed to import battery file %s."),
                  oDialog.get_filename().c_str());
    }
  }
}

void Window::vOnExportBatteryFile()
{
  std::string sBatteryDir = m_poDirConfig->sGetKey("batteries");

#ifdef GTKMM20

  Gtk::FileSelection oDialog(_("Export battery file"));
  oDialog.set_transient_for(*this);

  if (sBatteryDir == "")
  {
    oDialog.set_filename(Glib::path_get_dirname(m_sRomFile) + "/");
  }
  else
  {
    oDialog.set_filename(sBatteryDir + "/");
  }

#else // ! GTKMM20

  Gtk::FileChooserDialog oDialog(*this, _("Export battery file"),
                                 Gtk::FILE_CHOOSER_ACTION_SAVE);
  oDialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  oDialog.add_button(Gtk::Stock::SAVE,   Gtk::RESPONSE_OK);

  if (sBatteryDir == "")
  {
    oDialog.set_current_folder(Glib::path_get_dirname(m_sRomFile));
  }
  else
  {
    oDialog.set_current_folder(sBatteryDir);
    oDialog.add_shortcut_folder(sBatteryDir);
  }

  Gtk::FileFilter oBatteryFilter;
  oBatteryFilter.set_name(_("Battery file"));
  oBatteryFilter.add_pattern("*.[sS][aA][vV]");

  Gtk::FileFilter oFlashFilter;
  oFlashFilter.set_name(_("Flash save"));
  oFlashFilter.add_pattern("*.[dD][aA][tT]");

  oDialog.add_filter(oBatteryFilter);
  oDialog.add_filter(oFlashFilter);

#endif // ! GTKMM20

  while (oDialog.run() == Gtk::RESPONSE_OK)
  {
    Glib::ustring sFile = oDialog.get_filename();
    Glib::ustring sExt;

#ifdef GTKMM20

    sExt = ".sav";

#else // ! GTKMM20

    if (oDialog.get_filter() == &oBatteryFilter)
    {
      sExt = ".sav";
    }
    else
    {
      sExt = ".dat";
    }

#endif // ! GTKMM20

    if (! bHasSuffix(sFile, sExt, false))
    {
      sFile += sExt;
    }

    if (Glib::file_test(sFile, Glib::FILE_TEST_EXISTS))
    {
      Gtk::MessageDialog oConfirmDialog(*this,
                                        _("File already exists. Overwrite it?"),
#ifndef GTKMM20
                                        false,
#endif // ! GTKMM20
                                        Gtk::MESSAGE_QUESTION,
                                        Gtk::BUTTONS_YES_NO);
      if (oConfirmDialog.run() != Gtk::RESPONSE_YES)
      {
        continue;
      }
    }

    bool bResult;
    if (m_eCartridge == CartridgeGB)
    {
      bResult = gbWriteBatteryFile(sFile.c_str(), false);
    }
    else
    {
      bResult = m_stEmulator.emuWriteBattery(sFile.c_str());
    }

    if (bResult)
    {
      break;
    }
    else
    {
      vPopupError(_("Failed to export battery file %s."),
                  sFile.c_str());
    }
  }
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
#ifdef GTKMM20

  Gtk::FileSelection oDialog(_("Select directory"));
  oDialog.set_transient_for(*this);

  if (_poEntry->get_text() != "")
  {
    oDialog.set_filename(_poEntry->get_text() + "/");
  }

  if (oDialog.run() == Gtk::RESPONSE_OK)
  {
    std::string sFile = oDialog.get_filename();
    if (! Glib::file_test(sFile, Glib::FILE_TEST_IS_DIR))
    {
      sFile = Glib::path_get_dirname(sFile);
    }
    _poEntry->set_text(sFile);
  }

#else // ! GTKMM20

  Gtk::FileChooserDialog oDialog(*this, _("Select directory"),
                                 Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
  oDialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  oDialog.add_button(Gtk::Stock::OK,     Gtk::RESPONSE_OK);

  if (_poEntry->get_text() != "")
  {
    oDialog.add_shortcut_folder(_poEntry->get_text());
    oDialog.set_current_folder(_poEntry->get_text());
  }

  if (oDialog.run() == Gtk::RESPONSE_OK)
  {
    _poEntry->set_text(oDialog.get_filename());
  }

#endif // ! GTKMM20
}

void Window::vOnPauseWhenInactiveToggled(Gtk::CheckMenuItem * _poCMI)
{
  m_poDisplayConfig->vSetKey("pause_when_inactive", _poCMI->get_active());
}

void Window::vOnSelectBios()
{
#ifdef GTKMM20

  Gtk::FileSelection oDialog(_("Select BIOS file"));
  oDialog.set_transient_for(*this);

  if (m_poCoreConfig->sGetKey("bios_file") != "")
  {
    oDialog.set_filename(m_poCoreConfig->sGetKey("bios_file"));
  }

#else // ! GTKMM20

  Gtk::FileChooserDialog oDialog(*this, _("Select BIOS file"));
  oDialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  oDialog.add_button(Gtk::Stock::OPEN,   Gtk::RESPONSE_OK);

  if (m_poCoreConfig->sGetKey("bios_file") != "")
  {
    oDialog.set_filename(m_poCoreConfig->sGetKey("bios_file"));
  }

  const char * acsPattern[] =
  {
    "*.[bB][iI][nN]", "*.[aA][gG][bB]", "*.[gG][bB][aA]",
    "*.[bB][iI][oO][sS]", "*.[zZ][iI][pP]", "*.[zZ]", "*.[gG][zZ]"
  };

  Gtk::FileFilter oAllFilter;
  oAllFilter.set_name(_("All files"));
  oAllFilter.add_pattern("*");

  Gtk::FileFilter oBiosFilter;
  oBiosFilter.set_name(_("Gameboy Advance BIOS"));
  for (guint i = 0; i < sizeof(acsPattern) / sizeof(acsPattern[0]); i++)
  {
    oBiosFilter.add_pattern(acsPattern[i]);
  }

  oDialog.add_filter(oAllFilter);
  oDialog.add_filter(oBiosFilter);

  oDialog.set_filter(oBiosFilter);

#endif // ! GTKMM20

  while (oDialog.run() == Gtk::RESPONSE_OK)
  {
    if (Glib::file_test(oDialog.get_filename(), Glib::FILE_TEST_IS_REGULAR))
    {
      m_poCoreConfig->vSetKey("bios_file", oDialog.get_filename());
      m_poUseBiosItem->set_sensitive();
      break;
    }
  }
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

  if ((_pstEvent->state & Gtk::AccelGroup::get_default_mod_mask())
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

  if ((_pstEvent->state & Gtk::AccelGroup::get_default_mod_mask())
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
