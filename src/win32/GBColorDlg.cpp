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
#include "Controls.h"
#include "CommDlg.h"
#include "../System.h"
#include "resource.h"
#include "Reg.h"

extern void winCenterWindow(HWND h);
extern int gbPaletteOption;
extern int emulating;
extern int cartridgeType;
extern u16 gbPalette[128];

class GBColorDlg : public Dlg {
  ColorButton colorControls[8];
  u16 colors[24];
  int which;
protected:
  DECLARE_MESSAGE_MAP()
public:
  GBColorDlg();

  u16 *getColors();
  void setWhich(int);
  int getWhich() { return which; }

  virtual BOOL OnInitDialog(LPARAM);
  virtual void OnDrawItem(int, LPDRAWITEMSTRUCT);

  void OnReset();
  void OnDefault();
  void OnUser1();
  void OnUser2();
  void OnColorClicked(UINT);
  void OnOk();
  void OnCancel();
};

BEGIN_MESSAGE_MAP(GBColorDlg, Dlg)
  ON_WM_DRAWITEM()
  ON_BN_CLICKED(IDC_RESET, OnReset)
  ON_BN_CLICKED(IDC_DEFAULT, OnDefault)
  ON_BN_CLICKED(IDC_USER1, OnUser1)
  ON_BN_CLICKED(IDC_USER2, OnUser2)
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(ID_CANCEL, OnCancel)
  ON_CONTROL_RANGE(BN_CLICKED, IDC_COLOR_BG0, IDC_COLOR_OB3, OnColorClicked)
END_MESSAGE_MAP()  

GBColorDlg::GBColorDlg()
  : Dlg()
{
  which = gbPaletteOption;
}

void GBColorDlg::setWhich(int w)
{
  which = w;

  for(int i = 0; i < 8; i++) {
    colorControls[i].setColor(colors[which*8+i]);
  }
}

void GBColorDlg::OnDrawItem(int id, LPDRAWITEMSTRUCT itemStruct)
{
  ColorButton *b = (ColorButton *)Wnd::FromMap(itemStruct->hwndItem);
  ASSERT(b != NULL);
  b->OnDrawItem(id, itemStruct);
}

BOOL GBColorDlg::OnInitDialog(LPARAM)
{
  colorControls[0].SubClassDlgItem(IDC_COLOR_BG0, this);
  colorControls[1].SubClassDlgItem(IDC_COLOR_BG1, this);
  colorControls[2].SubClassDlgItem(IDC_COLOR_BG2, this);
  colorControls[3].SubClassDlgItem(IDC_COLOR_BG3, this);
  colorControls[4].SubClassDlgItem(IDC_COLOR_OB0, this);
  colorControls[5].SubClassDlgItem(IDC_COLOR_OB1, this);
  colorControls[6].SubClassDlgItem(IDC_COLOR_OB2, this);
  colorControls[7].SubClassDlgItem(IDC_COLOR_OB3, this);

  for(int i = 0; i < 24; i++) {
    colors[i] = systemGbPalette[i];
  }

  setWhich(which);

  DoRadio(false, IDC_DEFAULT, which);

  winCenterWindow(hWnd);
  
  return TRUE;
}

void GBColorDlg::OnDefault()
{
  setWhich(0);
}

void GBColorDlg::OnUser1()
{
  setWhich(1);
}

void GBColorDlg::OnUser2()
{
  setWhich(2);
}

void GBColorDlg::OnReset()
{
  int s = which * 8;
  colors[s++] = (0x1f) | (0x1f << 5) | (0x1f << 10);
  colors[s++] = (0x15) | (0x15 << 5) | (0x15 << 10);
  colors[s++] = (0x0c) | (0x0c << 5) | (0x0c << 10);
  colors[s++] = 0;
  
  colors[s++] = (0x1f) | (0x1f << 5) | (0x1f << 10);
  colors[s++] = (0x15) | (0x15 << 5) | (0x15 << 10);
  colors[s++] = (0x0c) | (0x0c << 5) | (0x0c << 10);
  colors[s] = 0;
  setWhich(which);
}

void GBColorDlg::OnColorClicked(UINT id)
{
  id -= IDC_COLOR_BG0;
  
  u16 color = colors[id];
  
  ColorDlg dlg(RGB(color & 0x1f, (color >> 5) & 0x1f, (color >> 10) & 0x1f),
               CC_FULLOPEN | CC_ANYCOLOR, this);
  if(dlg.DoModal()) {
    COLORREF c = dlg.GetColor();
    
    colors[which*8+id] = (c >> 3) & 0x1f | ((c >> 11) & 0x1f) << 5 |
      ((c >> 19) & 0x1f) << 10;
    colorControls[id].setColor(colors[which*8+id]);
  }  
}

void GBColorDlg::OnOk()
{
  EndDialog(TRUE);
}

void GBColorDlg::OnCancel()
{
  EndDialog(FALSE);
}

u16 *GBColorDlg::getColors()
{
  return colors;
}

extern HWND hWindow;

void optionsGameboyColors()
{
  GBColorDlg dlg;
  if(dlg.DoModal(IDD_GB_COLORS, hWindow)) {
    gbPaletteOption = dlg.getWhich();
    memcpy(systemGbPalette, dlg.getColors(), 24*sizeof(u16));
    if(emulating && cartridgeType == 1) {
      memcpy(gbPalette, &systemGbPalette[dlg.getWhich()*8], 8*sizeof(u16));
    }
    regSetDwordValue("gbPaletteOption", gbPaletteOption);
    regSetBinaryValue("gbPalette", (char *)systemGbPalette,
                      24*sizeof(u16));
  }
}
