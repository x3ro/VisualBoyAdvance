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
#include "Wnd.h"
#include "Reg.h"
#include "resource.h"

extern void winCenterWindow(HWND);
extern HWND hWindow;

class Associate : public Dlg {
protected:
  DECLARE_MESSAGE_MAP()
public:
  Associate();

  void makeAssociations(int);

  virtual BOOL OnInitDialog(LPARAM l);
  void OnOk();
  void OnCancel();
};

BEGIN_MESSAGE_MAP(Associate, Dlg)
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(ID_CANCEL, OnCancel)
END_MESSAGE_MAP()

Associate::Associate()
  : Dlg()
{
}

void Associate::makeAssociations(int mask)
{
  char applicationPath[2048];
  char commandPath[2048];
  char *types[] = { ".gb", ".sgb", ".cgb", ".gbc", ".gba", ".agb", ".bin" };
  GetModuleFileName(NULL, applicationPath, 2048);
  sprintf(commandPath,"\"%s\" \"%%1\"", applicationPath);
  regAssociateType("VisualBoyAdvance.Binary",
                   "Binary",
                   commandPath);
  
  for(int i = 0; i < 7; i++) {
    if(mask & (1<<i)) {
      regCreateFileType(types[i],"VisualBoyAdvance.Binary");
    }
  }
}

BOOL Associate::OnInitDialog(LPARAM l)
{
  winCenterWindow(getHandle());
  return TRUE;
}

void Associate::OnOk()
{
  int mask = 0;
  if(SendMessage(GetDlgItem(IDC_GB),
                 BM_GETCHECK,
                 0,
                 0) & BST_CHECKED)
    mask |= 1;
  if(SendMessage(GetDlgItem(IDC_SGB),
                 BM_GETCHECK,
                 0,
                 0) & BST_CHECKED)
    mask |= 2;
  if(SendMessage(GetDlgItem(IDC_CGB),
                 BM_GETCHECK,
                 0,
                 0) & BST_CHECKED)
    mask |= 4;
  if(SendMessage(GetDlgItem(IDC_GBC),
                 BM_GETCHECK,
                 0,
                 0) & BST_CHECKED)
    mask |= 8;
  if(SendMessage(GetDlgItem(IDC_GBA),
                 BM_GETCHECK,
                 0,
                 0) & BST_CHECKED)
    mask |= 16;
  if(SendMessage(GetDlgItem(IDC_AGB),
                 BM_GETCHECK,
                 0,
                 0) & BST_CHECKED)
    mask |= 32;
  if(SendMessage(GetDlgItem(IDC_BIN),
                 BM_GETCHECK,
                 0,
                 0) & BST_CHECKED)
    mask |= 64;
  if(mask) {
    makeAssociations(mask);
  }
  
  EndDialog(TRUE);
}

void Associate::OnCancel()
{
  EndDialog(FALSE);
}

void emulatorAssociate()
{
  Associate assoc;
  assoc.DoModal(IDD_ASSOCIATIONS, hWindow);
}
