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
#define DIRECTDRAW_VERSION 0x0700

#include "Wnd.h"
#include "../System.h"
#include "resource.h"

#include <ddraw.h>

extern void winCenterWindow(HWND);

//-----------------------------------------------------------------------------
// Default settings
//-----------------------------------------------------------------------------
#define MAX_DRIVERS         32                  // 32 drivers maximum

//-----------------------------------------------------------------------------
// Local structures
//-----------------------------------------------------------------------------
// Keeps data on the available DDraw drivers
struct
{
    char        szDescription[128];
    char        szName[128];
    GUID        *pGUID;
    GUID        GUIDcopy;
    HMONITOR    hm;
} Drivers[MAX_DRIVERS];

//-----------------------------------------------------------------------------
// Local data
//-----------------------------------------------------------------------------
static int                  gDriverCnt = 0;     // Total number of drivers
static GUID                *gpSelectedDriverGUID;

class VideoModeSelect : public Dlg {
  LPDIRECTDRAW7 pDirectDraw;  
protected:
  DECLARE_MESSAGE_MAP()
public:
  VideoModeSelect();

  virtual BOOL OnInitDialog(LPARAM);
  virtual void OnClose();

  void OnOk();
  void OnSelChange();
};

class VideoDriverSelect : public Dlg {
protected:
  DECLARE_MESSAGE_MAP()
public:
  VideoDriverSelect();

  virtual BOOL OnInitDialog(LPARAM);
  virtual void OnClose();

  void OnOk();
  void OnSelChange();
};

//-----------------------------------------------------------------------------
// Name: DDEnumCallbackEx()
// Desc: This call back is used to determine the existing available DDraw
//       devices, so the user can pick which one to run on.
//-----------------------------------------------------------------------------
BOOL WINAPI 
DDEnumCallbackEx(GUID *pGUID, LPSTR pDescription, LPSTR pName, LPVOID pContext, HMONITOR hm)
{
    if (pGUID)
    {
        Drivers[gDriverCnt].GUIDcopy = *pGUID;
        Drivers[gDriverCnt].pGUID = &Drivers[gDriverCnt].GUIDcopy;
    }
    else
        Drivers[gDriverCnt].pGUID = NULL;
    Drivers[gDriverCnt].szDescription[127] = '\0';
    Drivers[gDriverCnt].szName[127] = '\0';
    strncpy(Drivers[gDriverCnt].szDescription,pDescription,127);
    strncpy(Drivers[gDriverCnt].szName,pName,127);
    Drivers[gDriverCnt].hm = hm;
    if (gDriverCnt < MAX_DRIVERS)
        gDriverCnt++;
    else
        return DDENUMRET_CANCEL;
    return DDENUMRET_OK;
}




//-----------------------------------------------------------------------------
// Name: DDEnumCallback()
// Desc: This callback is used only with old versions of DDraw.
//-----------------------------------------------------------------------------
BOOL WINAPI 
DDEnumCallback(GUID *pGUID, LPSTR pDescription, LPSTR pName, LPVOID context)
{
    return (DDEnumCallbackEx(pGUID, pDescription, pName, context, NULL));
}

int winVideoModeSelect(HWND hWindow, GUID **guid)
{
  HINSTANCE h = LoadLibrary("ddraw.dll");
 
  // If ddraw.dll doesn't exist in the search path,
  // then DirectX probably isn't installed, so fail.
  if (!h)
    return -1;
  
  gDriverCnt = 0;
  
  // Note that you must know which version of the
  // function to retrieve (see the following text).
  // For this example, we use the ANSI version.
  LPDIRECTDRAWENUMERATEEX lpDDEnumEx;
  lpDDEnumEx = (LPDIRECTDRAWENUMERATEEX)
    GetProcAddress(h,"DirectDrawEnumerateExA");
 
  // If the function is there, call it to enumerate all display 
  // devices attached to the desktop, and any non-display DirectDraw
  // devices.
  if (lpDDEnumEx)
    lpDDEnumEx(DDEnumCallbackEx, NULL, 
               DDENUM_ATTACHEDSECONDARYDEVICES |
               DDENUM_NONDISPLAYDEVICES 
               );
  else {
    /*
     * We must be running on an old version of DirectDraw.
     * Therefore MultiMon isn't supported. Fall back on
     * DirectDrawEnumerate to enumerate standard devices on a 
     * single-monitor system.
     */
    BOOL (WINAPI *lpDDEnum)(LPDDENUMCALLBACK, LPVOID);
    
    lpDDEnum = (BOOL (WINAPI *)(LPDDENUMCALLBACK, LPVOID))
      GetProcAddress(h, "DirectDrawEnumerateA");
    if(lpDDEnum)
      lpDDEnum(DDEnumCallback,NULL);
    
    /* Note that it could be handy to let the OldCallback function
     * be a wrapper for a DDEnumCallbackEx. 
     * 
     * Such a function would look like:
     *    BOOL FAR PASCAL OldCallback(GUID FAR *lpGUID,
     *                                LPSTR pDesc,
     *                                LPSTR pName,
     *                                LPVOID pContext)
     *    {
     *         return Callback(lpGUID,pDesc,pName,pContext,NULL);
     *    }
     */
  }

  int selected = 0;

  if(gDriverCnt > 1) {
    VideoDriverSelect d;

    selected = d.DoModal(IDD_DRIVERS,
                         hWindow,
                         NULL);
    if(selected == -1) {
      // If the library was loaded by calling LoadLibrary(),
      // then you must use FreeLibrary() to let go of it.
      FreeLibrary(h);
      
      return -1;
    }
  }

  HRESULT (WINAPI *DDrawCreateEx)(GUID *,LPVOID *,REFIID,IUnknown *);  
  DDrawCreateEx = (HRESULT (WINAPI *)(GUID *,LPVOID *,REFIID,IUnknown *))
    GetProcAddress(h, "DirectDrawCreateEx");

  LPDIRECTDRAW7 ddraw = NULL;
  if(DDrawCreateEx) {
    HRESULT hret = DDrawCreateEx(Drivers[selected].pGUID,
                                 (void **)&ddraw,
                                 IID_IDirectDraw7,
                                 NULL);
    if(hret != DD_OK) {
      systemMessage(0, "Error during DirectDrawCreateEx: %08x", hret);
      FreeLibrary(h);
      return -1;
    }
  } else {
    // should not happen....
    systemMessage(0, "Error getting DirectDrawCreateEx");
    FreeLibrary(h);
    return -1;
  }  
  
  VideoModeSelect dlg;
  int res = dlg.DoModal(IDD_MODES,
                        hWindow,
                        (LPARAM)ddraw);
  if(res != -1) {
    *guid = Drivers[selected].pGUID;
  }
  ddraw->Release();
  ddraw = NULL;

  // If the library was loaded by calling LoadLibrary(),
  // then you must use FreeLibrary() to let go of it.
  FreeLibrary(h);

  return res;
}

static HRESULT WINAPI addVideoMode(LPDDSURFACEDESC2 surf, LPVOID lpContext)
{
  HWND h = (HWND)lpContext;
  char buffer[50];
  
  switch(surf->ddpfPixelFormat.dwRGBBitCount) {
  case 16:
  case 24:
  case 32:
    if(surf->dwWidth >= 640 && surf->dwHeight >= 480) {
      sprintf(buffer, "%4dx%4dx%2d", surf->dwWidth, surf->dwHeight,
              surf->ddpfPixelFormat.dwRGBBitCount);
      int pos = ::SendMessage(h, LB_ADDSTRING, 0, (LPARAM)buffer);
      ::SendMessage(h, LB_SETITEMDATA, pos,
                    (surf->ddpfPixelFormat.dwRGBBitCount << 24) |
                    ((surf->dwWidth & 4095) << 12) |
                    (surf->dwHeight & 4095));
    }
  }

  return DDENUMRET_OK;
}


BEGIN_MESSAGE_MAP(VideoModeSelect, Dlg)
  ON_WM_CLOSE()
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(ID_CANCEL, OnClose)
  ON_CONTROL(LBN_SELCHANGE, IDC_MODES, OnSelChange)
END_MESSAGE_MAP()

VideoModeSelect::VideoModeSelect()
  : Dlg()
{
  pDirectDraw = NULL;
}

BOOL VideoModeSelect::OnInitDialog(LPARAM lParam)
{
  HWND h = GetDlgItem(IDC_MODES);

  pDirectDraw = (LPDIRECTDRAW7)lParam;
  
  // check for available fullscreen modes
  pDirectDraw->EnumDisplayModes(DDEDM_STANDARDVGAMODES, NULL, h,
                                addVideoMode);
  
  EnableWindow(GetDlgItem(ID_OK), FALSE);      
  winCenterWindow(getHandle());
  return TRUE;  
}

void VideoModeSelect::OnClose()
{
  EndDialog(-1);
}

void VideoModeSelect::OnOk()
{
  int cur = SendMessage(GetDlgItem(IDC_MODES),
                        LB_GETCURSEL,
                        0,
                        0);
  if(cur != -1) {
    cur = ::SendMessage(GetDlgItem(IDC_MODES),
                        LB_GETITEMDATA,
                        cur,
                        0);
  }
  EndDialog(cur);
}

void VideoModeSelect::OnSelChange()
{
  int item = SendMessage(GetDlgItem(IDC_MODES),
                         LB_GETCURSEL,
                         0,
                         0);
  if(item != -1)
    EnableWindow(GetDlgItem(ID_OK), TRUE);
  else
    EnableWindow(GetDlgItem(ID_OK), FALSE);
}

/*********************************************************************/
BEGIN_MESSAGE_MAP(VideoDriverSelect, Dlg)
  ON_WM_CLOSE()
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(ID_CANCEL, OnClose)
  ON_CONTROL(LBN_SELCHANGE, IDC_DRIVERS, OnSelChange)
END_MESSAGE_MAP()

VideoDriverSelect::VideoDriverSelect()
  : Dlg()
{
}

BOOL VideoDriverSelect::OnInitDialog(LPARAM lParam)
{
  HWND h = GetDlgItem(IDC_DRIVERS);

  for(int i = 0; i < gDriverCnt; i++) {
    ::SendMessage(h, LB_ADDSTRING, 0, (LPARAM)Drivers[i].szDescription);
  }
  
  EnableWindow(GetDlgItem(ID_OK), FALSE);      
  winCenterWindow(getHandle());
  return TRUE;  
}

void VideoDriverSelect::OnClose()
{
  EndDialog(-1);
}

void VideoDriverSelect::OnOk()
{
  int cur = SendMessage(GetDlgItem(IDC_DRIVERS),
                        LB_GETCURSEL,
                        0,
                        0);
  EndDialog(cur);
}

void VideoDriverSelect::OnSelChange()
{
  int item = SendMessage(GetDlgItem(IDC_DRIVERS),
                         LB_GETCURSEL,
                         0,
                         0);
  if(item != -1)
    EnableWindow(GetDlgItem(ID_OK), TRUE);
  else
    EnableWindow(GetDlgItem(ID_OK), FALSE);
}
