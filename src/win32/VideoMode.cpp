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
#include "resource.h"

#include <ddraw.h>

extern void winCenterWindow(HWND);

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

int winVideoModeSelect(HWND hWindow, LPDIRECTDRAW7 pDirectDraw)
{
  VideoModeSelect dlg;
  return dlg.DoModal(IDD_MODES,
                     hWindow,
                     (LPARAM)pDirectDraw);
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
  char buffer[1024];
  
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
