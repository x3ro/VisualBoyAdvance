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
#include "resource.h"

extern void winCenterWindow(HWND);

class GSACodeSelect : public Dlg {
protected:
  DECLARE_MESSAGE_MAP()
public:
  GSACodeSelect();

  virtual BOOL OnInitDialog(LPARAM);
  virtual void OnClose();

  void OnOk();
  void OnSelChange();
};

int winGSACodeSelect(HWND hWindow,
                     LPARAM l)
{
  GSACodeSelect dlg;
  return dlg.DoModal(IDD_CODE_SELECT,
                     hWindow,
                     l);
}

BEGIN_MESSAGE_MAP(GSACodeSelect, Dlg)
  ON_WM_CLOSE()
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(ID_CANCEL, OnClose)
  ON_CONTROL(LBN_SELCHANGE, IDC_GAME_LIST, OnSelChange)
END_MESSAGE_MAP()

GSACodeSelect::GSACodeSelect()
  : Dlg()
{
}

BOOL GSACodeSelect::OnInitDialog(LPARAM lParam)
{
  char buffer[1024];
  
  HWND h = GetDlgItem(IDC_GAME_LIST);
  
  FILE *f = (FILE *)lParam;
  int games = 0;
  int len = 0;
  fseek(f, -4, SEEK_CUR);
  fread(&games, 1, 4, f);
  while(games > 0) {
    fread(&len, 1, 4, f);
    fread(buffer, 1, len, f);
    buffer[len] = 0;
    SendMessage(h, LB_ADDSTRING, 0, (LPARAM)buffer);
    int codes = 0;
    fread(&codes, 1, 4, f);
    
    while(codes > 0) {
      fread(&len, 1, 4, f);
      fseek(f, len, SEEK_CUR);
      fread(&len, 1, 4, f);
      fseek(f, len, SEEK_CUR);
      fseek(f, 4, SEEK_CUR);
      fread(&len, 1, 4, f);
      fseek(f, len*12, SEEK_CUR);
      codes--;
    }
    games--;
  }
  EnableWindow(GetDlgItem(ID_OK), FALSE);      
  winCenterWindow(getHandle());
  return TRUE;  
}

void GSACodeSelect::OnClose()
{
  EndDialog(-1);
}

void GSACodeSelect::OnOk()
{
  int cur = SendMessage(GetDlgItem(IDC_GAME_LIST),
                        LB_GETCURSEL,
                        0,
                        0);
  EndDialog(cur);
}

void GSACodeSelect::OnSelChange()
{
  int item = SendMessage(GetDlgItem(IDC_GAME_LIST),
                         LB_GETCURSEL,
                         0,
                         0);
  if(item != -1)
    EnableWindow(GetDlgItem(ID_OK), TRUE);
  else
    EnableWindow(GetDlgItem(ID_OK), FALSE);
}
