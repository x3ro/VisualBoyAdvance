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
#include "stdafx.h"

#include "Wnd.h"
#include "../System.h"
#include "resource.h"

extern void winCenterWindow(HWND h);

class RewindDlg : public Dlg {
  int interval;
protected:
  DECLARE_MESSAGE_MAP()
public:
  RewindDlg();

  virtual BOOL OnInitDialog(LPARAM);

  void OnOk();
  void OnCancel();
};

BEGIN_MESSAGE_MAP(RewindDlg, Dlg)
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(ID_CANCEL, OnCancel)
END_MESSAGE_MAP()  

RewindDlg::RewindDlg()
  : Dlg()
{
  interval = 0;
}

BOOL RewindDlg::OnInitDialog(LPARAM value)
{
  HWND h = GetDlgItem(IDC_INTERVAL);

  ::SendMessage(h, EM_LIMITTEXT, 3, 0);

  if(value != 0) {
    char buffer[16];
    sprintf(buffer, "%d", value);
    ::SetWindowText(h, buffer);
    interval = value;
  }
  winCenterWindow(hWnd);
  
  return TRUE;
}

void RewindDlg::OnOk()
{
  char buffer[16];

  ::GetWindowText(GetDlgItem(IDC_INTERVAL), buffer, 16);
  int v = atoi(buffer);

  if(v >= 10 && v <= 600) {
    EndDialog(v);
  } else
    systemMessage(IDS_INVALID_INTERVAL_VALUE, 
                  "Invalid rewind interval value. Please enter a number "
                  "between 10 and 600 seconds");
}

void RewindDlg::OnCancel()
{
  EndDialog(FALSE);
}

extern HWND hWindow;

int optionsRewindInterval(int interval)
{
  RewindDlg dlg;
  return dlg.DoModal(IDD_REWIND_INTERVAL, hWindow, interval);
}
