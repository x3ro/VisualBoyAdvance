/*
 * VisualBoyAdvanced - Nintendo Gameboy/GameboyAdvance (TM) emulator
 * Copyrigh(c) 1999-2003 Forgotten (vb@emuhq.com)
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
#include "stdafx.h"
#include "MainWnd.h"

#include "AboutDialog.h"

#include "../agbprint.h"
#include "../AutoBuild.h"
#include "../GBA.h"
#include "../Globals.h"
#include "../RTC.h"
#include "../Sound.h"
#include "../gb/gbCheats.h"
#include "../gb/gbGlobals.h"

extern int emulating;

void MainWnd::OnHelpAbout() 
{
  theApp.winCheckFullscreen();
  AboutDialog dlg;

  dlg.DoModal();
}

void MainWnd::OnHelpFaq() 
{
  ::ShellExecute(0, _T("open"), "http://vboy.emuhq.com/faq.shtml", 
                 0, 0, SW_SHOWNORMAL);
}

static void AppendFormat(CString& report, const char *format, ...)
{
  CString buffer;
  va_list valist;

  va_start(valist, format);
  buffer.FormatV(format, valist);
  va_end(valist);
  report += buffer;
}

void MainWnd::OnHelpBugreport() 
{
  theApp.winCheckFullscreen();
  OpenClipboard();

  EmptyClipboard();

  CString report = "";
  AppendFormat(report, "Emu version  : %s\r\n", VERSION);
  AppendFormat(report, "Emu Type     : %s\r\n",
#ifdef FINAL_VERSION
#ifdef DEV_VERSION
               "Development Version"
#else
               "Normal Version"
#endif
#else
               "Debug Version"
#endif
               );

  if(emulating) {
    AppendFormat(report, "File         : %s\r\n", theApp.szFile);

    char buffer[20];
    if(theApp.cartridgeType == 0) {
      u32 check = 0;
      for(int i = 0; i < 0x4000; i += 4) {
        check += *((u32 *)&bios[i]);
      }
      AppendFormat(report, "BIOS Checksum: %08X\r\n", check);

      strncpy(buffer, (const char *)&rom[0xa0], 12);
      buffer[12] = 0;
      AppendFormat(report, "Internal name: %s\r\n", buffer);
      
      strncpy(buffer, (const char *)&rom[0xac], 4);
      buffer[4] = 0;
      AppendFormat(report, "Game code    : %s\r\n", buffer);
    } else if(theApp.cartridgeType == 1) {
      strncpy(buffer, (const char *)&gbRom[0x134], 15);
      buffer[15] = 0;
      AppendFormat(report, "Game title   : %s\r\n", buffer);
    }
  }
  
  AppendFormat(report, "Using BIOS   : %d\r\n", theApp.useBiosFile);
  AppendFormat(report, "Skip BIOS    : %d\r\n", theApp.skipBiosFile);
  AppendFormat(report, "Disable SFX  : %d\r\n", cpuDisableSfx);
  AppendFormat(report, "Skip intro   : %d\r\n", theApp.removeIntros);
  AppendFormat(report, "Throttle     : %d\r\n", theApp.throttle);
  AppendFormat(report, "Rewind       : %d\r\n", theApp.rewindTimer);
  AppendFormat(report, "Auto frame   : %d\r\n", theApp.autoFrameSkip);
  AppendFormat(report, "Video option : %d\r\n", theApp.videoOption);
  AppendFormat(report, "Render type  : %d\r\n", theApp.renderMethod);
  AppendFormat(report, "Color depth  : %d\r\n", systemColorDepth);
  AppendFormat(report, "Red shift    : %08x\r\n", systemRedShift);
  AppendFormat(report, "Green shift  : %08x\r\n", systemGreenShift);
  AppendFormat(report, "Blue shift   : %08x\r\n", systemBlueShift);
  AppendFormat(report, "Layer setting: %04X\r\n", layerSettings);
  AppendFormat(report, "Save type    : %d (%d)\r\n", 
               theApp.winSaveType, cpuSaveType);
  AppendFormat(report, "Flash size   : %08X (%08x)\r\n", 
               theApp.winFlashSize, flashSize);
  AppendFormat(report, "RTC          : %d (%d)\r\n", theApp.winRtcEnable,
               rtcIsEnabled());
  AppendFormat(report, "AGBPrint     : %d\r\n", agbPrintIsEnabled());
  AppendFormat(report, "Speed toggle : %d\r\n", theApp.speedupToggle);
  AppendFormat(report, "Synchronize  : %d\r\n", synchronize);
  AppendFormat(report, "Sound OFF    : %d\r\n", soundOffFlag);
  AppendFormat(report, "Channels     : %04x\r\n", soundGetEnable() & 0x30f);
  AppendFormat(report, "Old Sync     : %d\r\n", theApp.useOldSync);
  AppendFormat(report, "Priority     : %d\r\n", theApp.threadPriority);
  AppendFormat(report, "Filters      : %d (%d)\r\n", theApp.filterType, theApp.ifbType);
  AppendFormat(report, "Cheats       : %d\r\n", cheatsNumber);
  AppendFormat(report, "GB Cheats    : %d\r\n", gbCheatNumber);
  AppendFormat(report, "GB Emu Type  : %d\r\n", gbEmulatorType);

  HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, 
                                 (report.GetLength() + 1) * sizeof(CHAR)); 
  if (hglbCopy == NULL) { 
    CloseClipboard(); 
    return;
  } 
 
  // Lock the handle and copy the text to the buffer. 
 
  LPSTR lptstrCopy = (LPSTR)GlobalLock(hglbCopy); 
  memcpy(lptstrCopy, (const char *)report, 
         report.GetLength() * sizeof(CHAR)); 
  lptstrCopy[report.GetLength()] = (TCHAR) 0;    // null character 
  GlobalUnlock(hglbCopy); 
 
  // Place the handle on the clipboard. 
  
  SetClipboardData(CF_TEXT, hglbCopy);   
  CloseClipboard();

  systemMessage(IDS_BUG_REPORT, "Bug report has been copied to the Clipboard");
}
