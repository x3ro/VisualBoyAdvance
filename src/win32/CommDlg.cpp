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
// FileDlg.cpp: implementation of the FileDlg class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "CommDlg.h"
#include <commdlg.h>
#include <dlgs.h>
#include "../System.h"
#include "resource.h"

extern HINSTANCE hInstance;
extern int videoOption;
extern BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
extern void WndHookCreate(Wnd *);

enum {
  VIDEO_1X, VIDEO_2X, VIDEO_3X, VIDEO_4X,
  VIDEO_320x240, VIDEO_640x480
};

static Wnd *fileDlgWndInit = NULL;

UINT CALLBACK FileDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if (hWnd == NULL)
    return 0;
  
  if(fileDlgWndInit != NULL && Wnd::FromMap(hWnd) == NULL) {
    fileDlgWndInit->SubClassWindow(hWnd);
    fileDlgWndInit = NULL;
  }
  
  if (message == WM_INITDIALOG) {
    return (UINT)DlgProc(hWnd, message, wParam, lParam);
  }
  
  return 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FileDlg::FileDlg(HWND parent, char *file, int maxFile, char *filter,
                 int filterIndex, char *ext, char **exts, char *initialDir, 
                 char *title, bool save)
: Dlg()
{
  ZeroMemory(&ofn, sizeof(ofn));

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = parent;
  ofn.hInstance = hInstance;
  ofn.lpstrFile = file;
  ofn.nMaxFile = maxFile;
  ofn.lpstrFilter = filter;
  ofn.nFilterIndex = filterIndex;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrDefExt = ext;
  ofn.lpstrInitialDir = initialDir;
  ofn.lpstrTitle = title;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_ENABLEHOOK | OFN_ENABLESIZING;

  if(videoOption == VIDEO_320x240) {
    ofn.lpTemplateName = MAKEINTRESOURCE(IDD_OPENDLG);
    ofn.Flags |= OFN_ENABLETEMPLATE;
  }

  ofn.Flags |= OFN_EXPLORER;

  ofn.lpfnHook = (LPOFNHOOKPROC)FileDlgProc;

  isSave = save;
  extensions = exts;
}

FileDlg::~FileDlg()
{

}

BOOL FileDlg::OnInitDialog(LPARAM)
{
  return TRUE;
}


BOOL FileDlg::DoModal()
{
  ASSERT(ofn.Flags & OFN_ENABLEHOOK);
  ASSERT(ofn.lpfnHook != NULL);

  if(ofn.Flags & OFN_EXPLORER)
    fileDlgWndInit = this;
  else
    WndHookCreate(this);

  BOOL res;

  if(isSave)
    res = GetSaveFileName(&ofn);
  else
    res = GetOpenFileName(&ofn);
  
  ASSERT(fileDlgWndInit == NULL);
  fileDlgWndInit = NULL;

  return res;
}

BOOL FileDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
  if(Dlg::OnNotify(wParam, lParam, pResult))
    return TRUE;

  OFNOTIFY* pNotify = (OFNOTIFY*)lParam;
  
  switch(pNotify->hdr.code) {
  case CDN_TYPECHANGE:
    OnTypeChange();
    return TRUE;
  }
  return FALSE;
}

void FileDlg::OnTypeChange()
{
  HWND parent = GetParent(getHandle());
  HWND h = ::GetDlgItem(parent, edt1);

  ASSERT(IsWindow(h));

  char filename[2048];

  ::GetWindowText(h, filename, sizeof(filename));

  HWND combo = ::GetDlgItem(parent, cmb1);
  ASSERT(IsWindow(combo));

  int sel = ::SendMessage(combo, CB_GETCURSEL, 0, 0);

  ASSERT(sel != -1);

  char *typeName = extensions[sel];

  if(strlen(filename) == 0) {
    sprintf(filename, "*%s", typeName);
  } else {
    char *p = strrchr(filename, '.');
    if(p == NULL) {
      strcat(filename, typeName);
    } else {
      strcpy(p, typeName);
    }
  }

  ::SetWindowText(h, filename);
}

int FileDlg::getFilterIndex()
{
  return ofn.nFilterIndex;
}

COLORREF ColorDlg::customColors[16];

ColorDlg::ColorDlg(COLORREF clrInit, DWORD dwFlags, Wnd *parent)
{
  memset(&cc, 0, sizeof(cc));
  cc.lStructSize = sizeof(cc);
  cc.lpCustColors = customColors;
  cc.Flags = dwFlags | CC_ENABLEHOOK;
  cc.lpfnHook = (LPOFNHOOKPROC)FileDlgProc;
  cc.hwndOwner = parent->getHandle();

  if((cc.rgbResult = clrInit) != 0)
    cc.Flags |= CC_RGBINIT;
}

int ColorDlg::DoModal()
{
  ASSERT(cc.Flags & CC_ENABLEHOOK);
  ASSERT(cc.lpfnHook != NULL);

  WndHookCreate(this);

  int res = ::ChooseColor(&cc);

  return res;
}

COLORREF ColorDlg::GetColor() const
{
  return cc.rgbResult;
}

