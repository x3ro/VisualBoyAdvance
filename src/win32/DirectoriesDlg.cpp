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
#include "Wnd.h"
#include "Reg.h"
#include "WinResUtil.h"
#include "resource.h"

#include <shlobj.h>

extern void winCenterWindow(HWND);

class DirectoriesDlg : public Dlg {
  char initialFolderDir[MAX_PATH];
protected:
  DECLARE_MESSAGE_MAP()
public:
  DirectoriesDlg();

  char *browseForDir(char *);
  
  virtual BOOL OnInitDialog(LPARAM);
  void OnRomDir();
  void OnGBRomDir();
  void OnSaveDir();
  void OnCaptureDir();
  void OnBatteryDir();
  void OnOk();
  void OnCancel();
  void OnRomDirReset();
  void OnGBRomDirReset();
  void OnBatteryDirReset();
  void OnSaveDirReset();
  void OnCaptureDirReset();
};

BEGIN_MESSAGE_MAP(DirectoriesDlg, Dlg)
  ON_BN_CLICKED(IDC_ROM_DIR, OnRomDir)
  ON_BN_CLICKED(IDC_GBROM_DIR, OnGBRomDir)
  ON_BN_CLICKED(IDC_BATTERY_DIR, OnBatteryDir)
  ON_BN_CLICKED(IDC_SAVE_DIR, OnSaveDir)
  ON_BN_CLICKED(IDC_CAPTURE_DIR, OnCaptureDir)
  ON_BN_CLICKED(IDOK, OnOk)
  ON_BN_CLICKED(IDCANCEL, OnCancel)
  ON_BN_CLICKED(IDC_ROM_DIR_RESET, OnRomDirReset)
  ON_BN_CLICKED(IDC_GBROM_DIR_RESET, OnGBRomDirReset)
  ON_BN_CLICKED(IDC_BATTERY_DIR_RESET, OnBatteryDirReset)
  ON_BN_CLICKED(IDC_SAVE_DIR_RESET, OnSaveDirReset)
  ON_BN_CLICKED(IDC_CAPTURE_DIR_RESET, OnCaptureDirReset)
END_MESSAGE_MAP()

DirectoriesDlg::DirectoriesDlg()
  : Dlg()
{
}

static int CALLBACK browseCallbackProc(HWND hWnd, UINT msg,
                                       LPARAM l, LPARAM data)
{
  char *buffer = (char *)data;
  switch(msg) {
  case BFFM_INITIALIZED:
    if(buffer[0])
      SendMessage(hWnd, BFFM_SETSELECTION, TRUE, (LPARAM)buffer);
    break;
  default:
    break;
  }
  return 0;
}

char *DirectoriesDlg::browseForDir(char *title)
{
  static char buffer[1024];
  LPMALLOC pMalloc;
  LPITEMIDLIST pidl;
  
  char * res = NULL;
  
  if(SUCCEEDED(SHGetMalloc(&pMalloc))) {
    BROWSEINFO bi;
    ZeroMemory(&bi, sizeof(bi));
    bi.hwndOwner = getHandle();
    bi.lpszTitle = title;
    bi.pidlRoot = 0;
    bi.ulFlags = BIF_RETURNONLYFSDIRS;
    bi.lpfn = browseCallbackProc;
    bi.lParam = (LPARAM)initialFolderDir;
    
    pidl = SHBrowseForFolder(&bi);
    
    if(pidl) {
      if(SHGetPathFromIDList(pidl, buffer)) {
        res = buffer;
      }
      pMalloc->Free(pidl);
      pMalloc->Release();
    }
  }
  return res;
}

BOOL DirectoriesDlg::OnInitDialog(LPARAM)
{
  char *p = regQueryStringValue("romdir", NULL);
  if(p) {
    int len = strlen(p);
    if(len > 0)
      if(p[len-1] == '\\')
        p[len-1] = 0;
    ::SetWindowText(GetDlgItem(IDC_ROM_PATH), p);
  }
  
  p = regQueryStringValue("gbromdir", NULL);
  if(p) {
    int len = strlen(p);
    if(len > 0)
      if(p[len-1] == '\\')
        p[len-1] = 0;
    ::SetWindowText(GetDlgItem(IDC_GBROM_PATH), p);
  }
  
  p = regQueryStringValue("batteryDir", NULL);
  if(p)
    ::SetWindowText(GetDlgItem(IDC_BATTERY_PATH), p);
  p = regQueryStringValue("saveDir", NULL);
  if(p)
    ::SetWindowText(GetDlgItem(IDC_SAVE_PATH), p);
  p = regQueryStringValue("captureDir", NULL);
  if(p)
    ::SetWindowText(GetDlgItem(IDC_CAPTURE_PATH), p);
  winCenterWindow(getHandle());
  return TRUE;  
}

void DirectoriesDlg::OnRomDir()
{
  char buffer[1024];
  char *p;
  initialFolderDir[0] = 0;      
  GetWindowText(GetDlgItem(IDC_ROM_PATH),buffer, 1024);
  if(buffer[0])
    strcpy(initialFolderDir, buffer);
  p = browseForDir((char *)winResLoadString(IDS_SELECT_ROM_DIR));
  if(p)
    ::SetWindowText(GetDlgItem(IDC_ROM_PATH),p);
}

void DirectoriesDlg::OnGBRomDir()
{
  char buffer[1024];
  char *p;
  initialFolderDir[0] = 0;      
  GetWindowText(GetDlgItem(IDC_GBROM_PATH),buffer, 1024);
  if(buffer[0])
    strcpy(initialFolderDir, buffer);
  p = browseForDir((char *)winResLoadString(IDS_SELECT_ROM_DIR));
  if(p)
    ::SetWindowText(GetDlgItem(IDC_GBROM_PATH),p);
}

void DirectoriesDlg::OnBatteryDir()
{
  char buffer[1024];
  char *p;
  initialFolderDir[0] = 0;      
  GetWindowText(GetDlgItem(IDC_BATTERY_PATH), buffer, 1024);
  if(buffer[0])
    strcpy(initialFolderDir, buffer);
  p = browseForDir((char *)winResLoadString(IDS_SELECT_BATTERY_DIR));
  if(p)
    ::SetWindowText(GetDlgItem(IDC_BATTERY_PATH),p);
}

void DirectoriesDlg::OnSaveDir()
{
  char buffer[1024];
  char *p;
  initialFolderDir[0] = 0;      
  GetWindowText(GetDlgItem(IDC_SAVE_PATH), buffer, 1024);
  if(buffer[0])
    strcpy(initialFolderDir, buffer);
  p = browseForDir((char *)winResLoadString(IDS_SELECT_SAVE_DIR));
  if(p)
    ::SetWindowText(GetDlgItem(IDC_SAVE_PATH),p);
}

void DirectoriesDlg::OnCaptureDir()
{
  char buffer[1024];
  char *p;

  initialFolderDir[0] = 0;      
  GetWindowText(GetDlgItem(IDC_CAPTURE_PATH), buffer, 1024);
  if(buffer[0])
    strcpy(initialFolderDir, buffer);
  p = browseForDir((char *)winResLoadString(IDS_SELECT_CAPTURE_DIR));
  if(p)
    ::SetWindowText(GetDlgItem(IDC_CAPTURE_PATH),p);
}

void DirectoriesDlg::OnRomDirReset()
{
  regDeleteValue("romdir");
  ::SetWindowText(GetDlgItem(IDC_ROM_PATH), "");
}

void DirectoriesDlg::OnGBRomDirReset()
{
  regDeleteValue("gbromdir");
  ::SetWindowText(GetDlgItem(IDC_GBROM_PATH), "");  
}

void DirectoriesDlg::OnBatteryDirReset()
{
  regDeleteValue("batteryDir");
  ::SetWindowText(GetDlgItem(IDC_BATTERY_PATH), "");
}

void DirectoriesDlg::OnSaveDirReset()
{
  regDeleteValue("saveDir");
  ::SetWindowText(GetDlgItem(IDC_SAVE_PATH), "");  
}

void DirectoriesDlg::OnCaptureDirReset()
{
  regDeleteValue("captureDir");
  ::SetWindowText(GetDlgItem(IDC_CAPTURE_PATH), "");  
}

void DirectoriesDlg::OnOk()
{
  char buffer[1024];
  GetWindowText(GetDlgItem(IDC_ROM_PATH), buffer, 1024);
  if(buffer[0])
    regSetStringValue("romdir", buffer);
  GetWindowText(GetDlgItem(IDC_GBROM_PATH), buffer, 1024);
  if(buffer[0])
    regSetStringValue("gbromdir", buffer);      
  GetWindowText(GetDlgItem(IDC_BATTERY_PATH), buffer, 1024);
  if(buffer[0])
    regSetStringValue("batteryDir", buffer);
  GetWindowText(GetDlgItem(IDC_SAVE_PATH), buffer, 1024);
  if(buffer[0])
    regSetStringValue("saveDir", buffer);
  GetWindowText(GetDlgItem(IDC_CAPTURE_PATH), buffer, 1024);
  if(buffer[0])
    regSetStringValue("captureDir", buffer);      
  EndDialog(TRUE);
}

void DirectoriesDlg::OnCancel()
{
  EndDialog(FALSE);
}

void showDirectories(HWND hWindow)
{
  DirectoriesDlg dlg;
  dlg.DoModal(IDD_DIRECTORIES,
              hWindow);
}
