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
extern HWND hWindow;
extern char languageName[4];

class LangSelect : public Dlg {
protected:
  DECLARE_MESSAGE_MAP()
public:
  LangSelect();

  BOOL OnInitDialog(LPARAM);
  void OnOk();
  void OnCancel();
};

BEGIN_MESSAGE_MAP(LangSelect, Dlg)
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(ID_CANCEL, OnCancel)
END_MESSAGE_MAP()

LangSelect::LangSelect()
  : Dlg()
{
}

BOOL LangSelect::OnInitDialog(LPARAM)
{
  char lbuffer[10];
  if(GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SABBREVLANGNAME,
                   lbuffer, 10)) {
    ::SetWindowText(GetDlgItem(IDC_LANG_NAME), lbuffer);
  } else {
    ::SetWindowText(GetDlgItem(IDC_LANG_NAME), "???");
  }
  
  if(languageName[0])
    ::SetWindowText(GetDlgItem(IDC_LANG_STRING), languageName);
      
  SendMessage(GetDlgItem(IDC_LANG_STRING), EM_LIMITTEXT, 3, 0);
  
  winCenterWindow(getHandle());
  
  return TRUE;
}

void LangSelect::OnOk()
{
  GetWindowText(GetDlgItem(IDC_LANG_STRING), languageName, 4);
  EndDialog(TRUE);
}

void LangSelect::OnCancel()
{
  EndDialog(FALSE);
}

int languageSelect()
{
  LangSelect lang;

  return lang.DoModal(IDD_LANG_SELECT, hWindow);
}
