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
#include <windows.h>

char winResBuffer[10][512];
int winResNextBuffer = 0;
HINSTANCE winResInstance = NULL;
WORD winResLangId = 0;
UINT winResCodePage = 1252;
extern HINSTANCE hInstance;

BOOL CALLBACK winResEnumLangProc(HANDLE module,
                                 LPCTSTR type,
                                 LPCTSTR name,
                                 WORD lang,
                                 LONG_PTR lParam)
{
  char buffer[20];
  
  winResLangId = lang;

  if(GetLocaleInfo(MAKELCID(lang, SORT_DEFAULT),
                   LOCALE_IDEFAULTANSICODEPAGE,
                   buffer,
                   20)) {
    winResCodePage = atoi(buffer);
    OSVERSIONINFO info;
    info.dwOSVersionInfoSize = sizeof(info);
    GetVersionEx(&info);
    if(info.dwPlatformId == VER_PLATFORM_WIN32_NT) {
      SetThreadLocale(MAKELCID(lang, SORT_DEFAULT));
    }
  } else
    winResCodePage = GetACP();
  return TRUE;
}

UCHAR *winResGetResource(LPCTSTR resType, LPCTSTR resName)
{
  EnumResourceLanguages(winResInstance, resType, resName,
                        (ENUMRESLANGPROC)winResEnumLangProc,
                        NULL);
  
  HRSRC hRsrc = FindResourceEx(winResInstance, resType, resName, 0);

  if(hRsrc != NULL) {
    HGLOBAL hGlobal = LoadResource(winResInstance, hRsrc);

    if(hGlobal != NULL) {
      UCHAR * b = (UCHAR *)LockResource(hGlobal);

      return b;
    }
  }
  return NULL;
}

HMENU winResLoadMenu(LPCTSTR menuName)
{
  UCHAR * b = winResGetResource(RT_MENU, menuName);
  
  if(b != NULL) {
    HMENU menu = LoadMenuIndirect((CONST MENUTEMPLATE *)b);
    
    if(menu != NULL)
      return menu;
  }

  return LoadMenu(NULL, menuName);
}

int winResDialogBox(LPCTSTR boxName,
                    HWND parent,
                    DLGPROC dlgProc,
                    LPARAM lParam)
{
  UCHAR * b = winResGetResource(RT_DIALOG, boxName);
  
  if(b != NULL) {
    
    return DialogBoxIndirectParam(hInstance,
                                  (LPCDLGTEMPLATE)b,
                                  parent,
                                  dlgProc,
                                  lParam);
  }

  return DialogBoxParam(hInstance,
                        boxName,
                        parent,
                        dlgProc,
                        lParam);
}

int winResDialogBox(LPCTSTR boxName,
                    HWND parent,
                    DLGPROC dlgProc)
{
  return winResDialogBox(boxName,
                         parent,
                         dlgProc,
                         0);
}

const char *winResLoadString(UINT id,
                             WORD &length)
{
  char * buffer  = winResBuffer[winResNextBuffer];
  winResNextBuffer++;
  if(winResNextBuffer >= 10)
    winResNextBuffer = 0;
  
  int stId = id / 16 + 1;
  UCHAR * b = winResGetResource(RT_STRING, MAKEINTRESOURCE(stId));
  
  if(b != NULL) {
    
    int res = id % 16 ;
    
    int i = 0;
    
    while(i != res) {
      WORD len = *((WORD *)b);
      
      b += (2 + 2*len);
      i++;
    }
    
    WORD len = *((WORD *)b);
    
    if(len) {
      b+=2;
      int j = 0;
      if(len >= 512)
        len = 511;
      length = WideCharToMultiByte(winResCodePage,
                                   0,
                                   (LPCWSTR)b,
                                   len,
                                   buffer,
                                   512,
                                   NULL,
                                   NULL);
      buffer[length] = 0;
      return (const char *)buffer;
    }
  }

  int res = LoadString(hInstance,
                       id,
                       buffer,
                       512);

  length = res;
  
  return (const char *)buffer;
}

const char *winResLoadString(UINT id)
{
  WORD length = 0;
  
  return winResLoadString(id,length);
}

void winResSetLanguageModule(HINSTANCE hi)
{
  winResInstance = hi;
}
