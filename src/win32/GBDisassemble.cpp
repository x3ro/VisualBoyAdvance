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
#include "../System.h"
#include "../gb/gbGlobals.h"
#include "../gb/GB.h"
#include "../armdis.h"
#include "ResizeDlg.h"

#include "IUpdate.h"

extern gbRegister AF;
extern gbRegister BC;
extern gbRegister DE;
extern gbRegister HL;
extern gbRegister SP;
extern gbRegister PC;
extern u16 IFF;

extern HWND hWindow;
extern HINSTANCE hInstance;
extern int cartridgeType;
extern int emulating;

extern void winAddUpdateListener(IUpdateListener *);
extern void winRemoveUpdateListener(IUpdateListener *);
extern int gbDis(char *, u16);

class GBDisassemble : public ResizeDlg, IUpdateListener {
protected:
  DECLARE_MESSAGE_MAP();
private:
  HWND list;
  u16 address;
  bool autoUpdate;
  int count;
  u16 lastAddress;
public:
  GBDisassemble();
  
  virtual BOOL OnInitDialog(LPARAM);
  virtual void OnClose();
  void OnNext();
  void refresh();
  void OnGo();
  void OnGoPC();

  void OnAutoUpdate();
  virtual void OnVScroll(UINT, UINT, HWND);  

  virtual void update();
};

BEGIN_MESSAGE_MAP(GBDisassemble, ResizeDlg)
  ON_BN_CLICKED(IDC_CLOSE, OnClose)
  ON_BN_CLICKED(IDC_REFRESH, refresh)
  ON_BN_CLICKED(IDC_NEXT, OnNext)
  ON_BN_CLICKED(IDC_GO, OnGo)
  ON_BN_CLICKED(IDC_AUTO_UPDATE, OnAutoUpdate)
  ON_BN_CLICKED(IDC_GOPC, OnGoPC)
  ON_WM_CLOSE()
  ON_WM_VSCROLL()
END_MESSAGE_MAP()

static GBDisassemble *instance = NULL;
  
GBDisassemble::GBDisassemble()
  : ResizeDlg()
{
  address = 0;
  autoUpdate = false;
  count = 1;
  lastAddress = 0;
}

void GBDisassemble::OnVScroll(UINT type, UINT, HWND h)
{
  char buffer[80];
  
  switch(type) {
  case SB_LINEDOWN:
    address += gbDis(buffer, address);
    break;
  case SB_LINEUP:
    address--;
    break;
  case SB_PAGEDOWN:
    address = lastAddress;
    break;
  case SB_PAGEUP:
    address -= count;
    break;
  }
  refresh();
}

void GBDisassemble::refresh()
{
  if(gbRom == NULL)
    return;
  
  int h = ::SendMessage(list, LB_GETITEMHEIGHT, 0, 0);
  RECT r;
  GetClientRect(list, &r);
  count = (r.bottom - r.top+1)/h;

  ::SendMessage(list, LB_RESETCONTENT, 0, 0);
  if(!emulating || cartridgeType != 1)
    return;
  
  char buffer[80];
  u16 addr = address;
  int i;
  int sel = -1;
  for(i = 0; i < count; i++) {
    if(addr == PC.W)
      sel = i;
    addr += gbDis(buffer, addr);
    ::SendMessage(list, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)buffer);
  }
  lastAddress = addr-1;
  if(sel != -1)
    ::SendMessage(list, LB_SETCURSEL, sel, sel);

  sprintf(buffer, "%04x", AF.W);
  ::SetWindowText(GetDlgItem(IDC_R0), buffer);
  sprintf(buffer, "%04x", BC.W);
  ::SetWindowText(GetDlgItem(IDC_R1), buffer);  
  sprintf(buffer, "%04x", DE.W);
  ::SetWindowText(GetDlgItem(IDC_R2), buffer);  
  sprintf(buffer, "%04x", HL.W);
  ::SetWindowText(GetDlgItem(IDC_R3), buffer);  
  sprintf(buffer, "%04x", SP.W);
  ::SetWindowText(GetDlgItem(IDC_R4), buffer);  
  sprintf(buffer, "%04x", PC.W);
  ::SetWindowText(GetDlgItem(IDC_R5), buffer);  
  sprintf(buffer, "%04x", IFF);
  ::SetWindowText(GetDlgItem(IDC_R6), buffer);  

  int v = (AF.B.B0 & 0x80) ? 1 : 0;
  DoCheckbox(false, IDC_Z, v);
  v = (AF.B.B0 & 0x40) ? 1 : 0;
  DoCheckbox(false, IDC_N, v);
  v = (AF.B.B0 & 0x20) ? 1 : 0;
  DoCheckbox(false, IDC_H, v);
  v = (AF.B.B0 & 0x10) ? 1 : 0;
  DoCheckbox(false, IDC_C, v);
}

void GBDisassemble::OnGoPC()
{
  address = PC.W;

  refresh();
}

void GBDisassemble::update()
{
  OnGoPC();
  refresh();
}

BOOL GBDisassemble::OnInitDialog(LPARAM)
{
  DIALOG_SIZER_START( sz )
    DIALOG_SIZER_ENTRY( IDC_DISASSEMBLE, DS_SizeY)
    DIALOG_SIZER_ENTRY( IDC_REFRESH, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_CLOSE, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_NEXT,  DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_AUTO_UPDATE, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_GOPC, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_VSCROLL, DS_SizeY)
  DIALOG_SIZER_END()
  SetData(sz,
          TRUE,
          HKEY_CURRENT_USER,
          "Software\\Emulators\\VisualBoyAdvance\\Viewer\\GBDisassembleView",
          NULL);

  SCROLLINFO si;
  ZeroMemory(&si, sizeof(si));
  si.cbSize = sizeof(si);
  si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
  si.nMin = 0;
  si.nMax = 100;
  si.nPos = 50;
  si.nPage = 0;
  SetScrollInfo(GetDlgItem(IDC_VSCROLL), SB_CTL, &si, TRUE);
  
  list = GetDlgItem(IDC_DISASSEMBLE);
  HFONT font = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);
  
  ::SendMessage(list, WM_SETFONT, (WPARAM)font, 0);
  for(int i = 0; i < 6; i++)
    ::SendMessage(GetDlgItem(IDC_R0+i), WM_SETFONT, (WPARAM)font, 0);

  ::SendMessage(GetDlgItem(IDC_ADDRESS), EM_LIMITTEXT, 4,0);
  refresh();

  return true;
}

void GBDisassemble::OnAutoUpdate()
{
  autoUpdate = !autoUpdate;
  if(autoUpdate) {
    winAddUpdateListener(this);
  } else {
    winRemoveUpdateListener(this);    
  }  
}

void GBDisassemble::OnGo()
{
  char buffer[16];
  ::GetWindowText(GetDlgItem(IDC_ADDRESS), buffer, 16);
  sscanf(buffer, "%x", &address);
  refresh();
}

void GBDisassemble::OnNext()
{
  gbEmulate(1);
  if(PC.W < address || PC.W >= lastAddress)
    OnGoPC();
  refresh();
}

void GBDisassemble::OnClose()
{
  winRemoveUpdateListener(this);
  
  DestroyWindow();
  instance = NULL;
}

void toolsGBDisassemble()
{
  if(instance == NULL) {
    instance = new GBDisassemble();
    instance->setAutoDelete(true);
    instance->MakeDialog(hInstance,
                         IDD_GB_DISASSEMBLE,
                         hWindow);
  } else {
    ::SetForegroundWindow(instance->getHandle());
  }
}
