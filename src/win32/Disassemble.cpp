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
#include "../GBA.h"
#include "../Globals.h"
#include "../armdis.h"
#include "ResizeDlg.h"

#include "IUpdate.h"

extern HWND hWindow;
extern HINSTANCE hInstance;
extern int cartridgeType;
extern int emulating;

extern void winAddUpdateListener(IUpdateListener *);
extern void winRemoveUpdateListener(IUpdateListener *);

extern void CPUUpdateCPSR();

class Disassemble : public ResizeDlg, IUpdateListener {
protected:
  DECLARE_MESSAGE_MAP();
private:
  HWND list;
  int mode;
  u32 address;
  bool autoUpdate;
  int count;
public:
  Disassemble();
  
  virtual BOOL OnInitDialog(LPARAM);
  virtual void OnClose();
  void OnNext();
  void refresh();
  void OnAutomatic();
  void OnARM();
  void OnTHUMB();
  void OnGo();
  void OnGoPC();

  void OnAutoUpdate();
  virtual void OnVScroll(UINT, UINT, HWND);  

  virtual void update();
};

BEGIN_MESSAGE_MAP(Disassemble, ResizeDlg)
  ON_BN_CLICKED(IDC_AUTOMATIC, OnAutomatic)
  ON_BN_CLICKED(IDC_ARM, OnARM)
  ON_BN_CLICKED(IDC_THUMB, OnTHUMB)
  ON_BN_CLICKED(IDC_CLOSE, OnClose)
  ON_BN_CLICKED(IDC_REFRESH, refresh)
  ON_BN_CLICKED(IDC_NEXT, OnNext)
  ON_BN_CLICKED(IDC_GO, OnGo)
  ON_BN_CLICKED(IDC_AUTO_UPDATE, OnAutoUpdate)
  ON_BN_CLICKED(IDC_GOPC, OnGoPC)
  ON_WM_CLOSE()
  ON_WM_VSCROLL()
END_MESSAGE_MAP()

static Disassemble *instance = NULL;
  
Disassemble::Disassemble()
  : ResizeDlg()
{
  mode = 0;
  address = 0;
  autoUpdate = false;
  count = 1;
}

void Disassemble::OnVScroll(UINT type, UINT, HWND h)
{
  switch(type) {
  case SB_LINEDOWN:
    if(mode == 0) {
      if(armState)
        address += 4;
      else
        address += 2;
    } else if(mode == 1)
      address += 4;
    else
      address += 2;
    break;
  case SB_LINEUP:
    if(mode == 0) {
      if(armState)
        address -= 4;
      else
        address -= 2;
    } else if(mode == 1)
      address -= 4;
    else
      address -= 2;
    break;
  case SB_PAGEDOWN:
    if(mode == 0) {
      if(armState)
        address += count*4;
      else
        address += count*2;
    } else if(mode == 1)
      address += count*4;
    else
      address += count*2;
    break;
  case SB_PAGEUP:
    if(mode == 0) {
      if(armState)
        address -= count*4;
      else
        address -= count*2;
    } else if(mode == 1)
      address -= count*4;
    else
      address -= count*2;
    break;
  }
  refresh();
}

void Disassemble::refresh()
{
  if(rom == NULL)
    return;
  
  bool arm = armState;
  
  if(mode != 0) {
    if(mode == 1)
      arm = true;
    else
      arm = false;
  } else {
  }
  
  int h = ::SendMessage(list, LB_GETITEMHEIGHT, 0, 0);
  RECT r;
  GetClientRect(list, &r);
  count = (r.bottom - r.top+1)/h;

  ::SendMessage(list, LB_RESETCONTENT, 0, 0);
  if(!emulating && cartridgeType == 0)
    return;
  
  char buffer[80];
  u32 addr = address;
  int i;
  int sel = -1;
  for(i = 0; i < count; i++) {
    if(addr == armNextPC)
      sel = i;
    if(arm) {
      addr += disArm(addr, buffer, 3);
    } else {
      addr += disThumb(addr, buffer, 3);
    }
    ::SendMessage(list, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)buffer);
  }
  if(sel != -1)
    ::SendMessage(list, LB_SETCURSEL, sel, sel);

  CPUUpdateCPSR();
  
  for(i = 0; i < 17; i++) {
    sprintf(buffer, "%08x", reg[i].I);
    ::SetWindowText(GetDlgItem(IDC_R0+i), buffer);
  }

  int v = (reg[16].I & 0x80000000) ? 1 : 0;
  DoCheckbox(false, IDC_N, v);
  v = (reg[16].I & 0x40000000) ? 1 : 0;
  DoCheckbox(false, IDC_Z, v);
  v = (reg[16].I & 0x20000000) ? 1 : 0;
  DoCheckbox(false, IDC_C, v);
  v = (reg[16].I & 0x10000000) ? 1 : 0;
  DoCheckbox(false, IDC_V, v);
  v = (reg[16].I & 0x80) ? 1 : 0;
  DoCheckbox(false, IDC_I, v);
  v = (reg[16].I & 0x40) ? 1 : 0;
  DoCheckbox(false, IDC_F, v);
  v = (reg[16].I & 0x20) ? 1 : 0;
  DoCheckbox(false, IDC_T, v);

  v = reg[16].I & 0x1f;
  sprintf(buffer, "%02x", v);
  ::SetWindowText(GetDlgItem(IDC_MODE), buffer);
}

void Disassemble::OnGoPC()
{
  if(armState)
    address = armNextPC - 16;
  else
    address = armNextPC - 8;

  refresh();
}

void Disassemble::update()
{
  OnGoPC();
  refresh();
}

BOOL Disassemble::OnInitDialog(LPARAM)
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
          "Software\\Emulators\\VisualBoyAdvance\\Viewer\\DisassembleView",
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
  for(int i = 0; i < 17; i++)
    ::SendMessage(GetDlgItem(IDC_R0+i), WM_SETFONT, (WPARAM)font, 0);

  ::SendMessage(GetDlgItem(IDC_MODE), WM_SETFONT, (WPARAM)font, 0);
  
  DoRadio(false, IDC_AUTOMATIC, mode);
  ::SendMessage(GetDlgItem(IDC_ADDRESS), EM_LIMITTEXT, 8,0);
  refresh();

  return true;
}

void Disassemble::OnAutoUpdate()
{
  autoUpdate = !autoUpdate;
  if(autoUpdate) {
    winAddUpdateListener(this);
  } else {
    winRemoveUpdateListener(this);    
  }  
}

void Disassemble::OnAutomatic()
{
  mode = 0;
  refresh();
}

void Disassemble::OnARM()
{
  mode = 1;
  refresh();
}

void Disassemble::OnTHUMB()
{
  mode = 2;
  refresh();
}

void Disassemble::OnGo()
{
  char buffer[16];
  ::GetWindowText(GetDlgItem(IDC_ADDRESS), buffer, 16);
  sscanf(buffer, "%x", &address);
  refresh();
}

void Disassemble::OnNext()
{
  CPULoop(1);
  if(armState) {
    u32 total = address+count*4;
    if(armNextPC >= address && armNextPC < total) {
    } else {
      OnGoPC();
    }
  } else {
    u32 total = address+count*2;
    if(armNextPC >= address && armNextPC < total) {
    } else {
      OnGoPC();
    }
  }
  refresh();
}

void Disassemble::OnClose()
{
  winRemoveUpdateListener(this);
  
  DestroyWindow();
  instance = NULL;
}

void toolsDisassemble()
{
  if(instance == NULL) {
    instance = new Disassemble();
    instance->setAutoDelete(true);
    instance->MakeDialog(hInstance,
                         IDD_DISASSEMBLE,
                         hWindow);
  } else {
    ::SetForegroundWindow(instance->getHandle());
  }
}
