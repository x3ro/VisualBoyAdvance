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
#include "Controls.h"
#include "resource.h"
#include "../AutoBuild.h"

extern void winCenterWindow(HWND);

class AboutDlg : public Dlg {
  HyperLink url;
  HyperLink translator;
protected:
  DECLARE_MESSAGE_MAP()
    
public:
  AboutDlg();
  
  virtual BOOL OnInitDialog(LPARAM);

  void OnOk();
};

BEGIN_MESSAGE_MAP(AboutDlg, Dlg)
  ON_BN_CLICKED(IDOK, OnOk)
END_MESSAGE_MAP()

AboutDlg::AboutDlg()
  : Dlg()
{
}

void AboutDlg::OnOk()
{
  EndDialog(TRUE);
}

BOOL AboutDlg::OnInitDialog(LPARAM lParam)
{
  url.SubClassWindow(GetDlgItem(IDC_URL));
  HWND h = GetDlgItem(IDC_TRANSLATOR_URL);
  if(h != NULL) {
    translator.SubClassWindow(h);
  }
  ::SetWindowText(GetDlgItem(IDC_VERSION), VERSION);
  winCenterWindow(getHandle());
  return TRUE;
}

extern HWND hWindow;

void helpAbout()
{
  AboutDlg dlg;
  dlg.DoModal(IDD_ABOUT, hWindow);
}
