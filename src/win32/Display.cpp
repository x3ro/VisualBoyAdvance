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
#include <stdio.h>
#include <windows.h>

#include "Wnd.h"

#include "resource.h"

extern HWND hWindow;
extern void winCenterWindow(HWND);

class ModeConfirmDlg : public Dlg {
  UINT timer;
  int count;
protected:
  DECLARE_MESSAGE_MAP()
public:
  ModeConfirmDlg();
  
  virtual BOOL OnInitDialog(LPARAM);
  void OnDestroy();
  void OnTimer(UINT);
  void OnOk();
  void OnCancel();
};

// Address and size selection

BEGIN_MESSAGE_MAP(ModeConfirmDlg, Dlg)
  ON_WM_CLOSE()
  ON_WM_TIMER()
  ON_WM_DESTROY()
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(ID_CANCEL, OnCancel)
END_MESSAGE_MAP()

ModeConfirmDlg::ModeConfirmDlg()
  : Dlg()
{
  timer = 0;
  count = 10;
}
  
BOOL ModeConfirmDlg::OnInitDialog(LPARAM)
{
  char buffer[16];
  
  timer = SetTimer(getHandle(), 0, 1000, NULL);
  sprintf(buffer, "%d", count);
  ::SetWindowText(GetDlgItem(IDC_TIMER), buffer);

  winCenterWindow(getHandle());  
  return TRUE;
}

void ModeConfirmDlg::OnDestroy()
{
  KillTimer(getHandle(), timer);
  timer = 0;
}

void ModeConfirmDlg::OnOk()
{
  EndDialog(TRUE);
}

void ModeConfirmDlg::OnCancel()
{
  EndDialog(FALSE);
}

void ModeConfirmDlg::OnTimer(UINT)
{
  char buffer[16];  
  count--;
  if(count == 0)
    EndDialog(FALSE);
  sprintf(buffer, "%d", count);
  ::SetWindowText(GetDlgItem(IDC_TIMER), buffer);
}

bool winDisplayConfirmMode()
{
  ModeConfirmDlg dlg;

  return (dlg.DoModal(IDD_MODE_CONFIRM,
                      hWindow)) ? true : false;
}


  
