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
#include "ResizeDlg.h"
#include "MemoryViewer.h"
#include <stdio.h>
#include <memory.h>
#include "../gb/gbGlobals.h"
#include "Reg.h"
#include "WinResUtil.h"
#include "resource.h"
#include "IUpdate.h"
#include "CommDlg.h"

extern HINSTANCE hInstance;
extern HWND hWindow;
extern int emulating;

extern void winAddUpdateListener(IUpdateListener *);
extern void winRemoveUpdateListener(IUpdateListener *);
extern char *winLoadFilter(int id);

class GBMemoryViewer : public MemoryViewer {
public:
  GBMemoryViewer();
  virtual void readData(u32, int, u8 *);
  virtual void editData(u32, int, int, u32);
};

class GBMemoryViewerDlg : public ResizeDlg, IUpdateListener, IMemoryViewerDlg {
  GBMemoryViewer viewer;
  bool autoUpdate;
protected:
  DECLARE_MESSAGE_MAP()
public:
  GBMemoryViewerDlg();

  virtual BOOL OnInitDialog(LPARAM);
  virtual void OnClose();
  void On8Bit();
  void On16Bit();
  void On32Bit();
  void OnGo();
  void OnAddressesSelChange();
  void OnRefresh();
  void OnAutoUpdate();
  void OnSave();
  void OnLoad();

  virtual void setCurrentAddress(u32);

  virtual void update();
};

GBMemoryViewer::GBMemoryViewer()
  : MemoryViewer()
{
  setAddressSize(1);
}

void GBMemoryViewer::readData(u32 address, int len, u8 *data)
{
  u16 addr = address & 0xffff;
  if(emulating && gbRom != NULL) {
    for(int i = 0; i < len; i++) {
      *data++ = gbMemoryMap[addr >> 12][addr & 0xfff];
      addr++;
    }
  } else {
    for(int i = 0; i < len; i++) {
      *data++ = 0;
      addr++;
    }    
  }
}

#define GB_READBYTE_QUICK(addr) \
  gbMemoryMap[(addr) >> 12][(addr) & 0xfff]

#define GB_WRITEBYTE_QUICK(addr,v) \
  gbMemoryMap[(addr) >> 12][(addr) & 0xfff] = (v)

void GBMemoryViewer::editData(u32 address, int size, int mask, u32 value)
{
  u32 oldValue;
  u16 addr = (u16)address & 0xffff;
  switch(size) {
  case 8:
    oldValue = GB_READBYTE_QUICK(addr);
    oldValue &= mask;
    oldValue |= (u8)value;
    GB_WRITEBYTE_QUICK(addr, oldValue);
    break;
  case 16:
    oldValue = GB_READBYTE_QUICK(addr) |
      (GB_READBYTE_QUICK(addr + 1) << 8);
    oldValue &= mask;
    oldValue |= (u16)value;
    GB_WRITEBYTE_QUICK(addr, (oldValue & 255));
    GB_WRITEBYTE_QUICK(addr+1, (oldValue >> 8));
    break;
  case 32:
    oldValue = GB_READBYTE_QUICK(addr) |
      (GB_READBYTE_QUICK(addr + 1) << 8) |
      (GB_READBYTE_QUICK(addr + 2) << 16) |
      (GB_READBYTE_QUICK(addr + 3) << 24);
    oldValue &= mask;
    oldValue |= (u32)value;
    GB_WRITEBYTE_QUICK(addr, (oldValue & 255));
    GB_WRITEBYTE_QUICK(addr+1, (oldValue >> 8));
    GB_WRITEBYTE_QUICK(addr+2, (oldValue >> 16));
    GB_WRITEBYTE_QUICK(addr+3, (oldValue >> 24));
    break;
  }    
}

BEGIN_MESSAGE_MAP(GBMemoryViewerDlg, ResizeDlg)
  ON_WM_CLOSE()
  ON_BN_CLICKED(IDC_CLOSE, OnClose)
  ON_BN_CLICKED(IDC_8_BIT, On8Bit)
  ON_BN_CLICKED(IDC_16_BIT, On16Bit)
  ON_BN_CLICKED(IDC_32_BIT, On32Bit)
  ON_BN_CLICKED(IDC_GO, OnGo)
  ON_BN_CLICKED(IDC_AUTO_UPDATE, OnAutoUpdate)
  ON_BN_CLICKED(IDC_SAVE, OnSave)
  ON_BN_CLICKED(IDC_LOAD, OnLoad)
  ON_CONTROL(CBN_SELCHANGE, IDC_ADDRESSES, OnAddressesSelChange)
END_MESSAGE_MAP()

GBMemoryViewerDlg::GBMemoryViewerDlg()
  : ResizeDlg()
{
  MemoryViewer::registerClass();
  autoUpdate = false;
}

BOOL GBMemoryViewerDlg::OnInitDialog(LPARAM)
{
  DIALOG_SIZER_START( sz )
    DIALOG_SIZER_ENTRY( IDC_VIEWER, DS_SizeX | DS_SizeY )
    DIALOG_SIZER_ENTRY( IDC_REFRESH, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_CLOSE, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_LOAD, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_SAVE, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_AUTO_UPDATE, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_CURRENT_ADDRESS_LABEL, DS_MoveY | DS_MoveX)
    DIALOG_SIZER_ENTRY( IDC_CURRENT_ADDRESS, DS_MoveY | DS_MoveX)
    DIALOG_SIZER_END()
    SetData(sz,
            TRUE,
            HKEY_CURRENT_USER,
            "Software\\Emulators\\VisualBoyAdvance\\Viewer\\GBMemoryView",
            NULL);
  
  viewer.Attach(GetDlgItem(IDC_VIEWER));
  viewer.setDialog(this);
  ShowScrollBar(viewer.getHandle(), SB_VERT, TRUE);
  EnableScrollBar(viewer.getHandle(), SB_VERT, ESB_ENABLE_BOTH);

  HWND h = GetDlgItem(IDC_ADDRESSES);

  char *s[] = {
    "0x0000 - ROM",
    "0x4000 - ROM",
    "0x8000 - VRAM",
    "0xA000 - SRAM",
    "0xC000 - RAM",
    "0xD000 - WRAM",
    "0xFF00 - I/O",
    "0xFF80 - RAM"
  };

  for(int i = 0; i < 8; i++)
    SendMessage(h, CB_ADDSTRING,
                0,
                (LPARAM)s[i]);

  SendMessage(h, CB_SETCURSEL,
              0,
              0);

  RECT cbSize;
  int Height;
  
  GetClientRect(h, &cbSize);
  Height = SendMessage(h, CB_GETITEMHEIGHT, (WPARAM)-1, 0);
  Height += SendMessage(h, CB_GETITEMHEIGHT, 0, 0) * (9);
  
  // Note: The use of SM_CYEDGE assumes that we're using Windows '95
  // Now add on the height of the border of the edit box
  Height += GetSystemMetrics(SM_CYEDGE) * 2;  // top & bottom edges
  
  // The height of the border of the drop-down box
  Height += GetSystemMetrics(SM_CYEDGE) * 2;  // top & bottom edges
  
  // now set the size of the window
  SetWindowPos(h,
               NULL,
               0, 0,
               cbSize.right, Height,
               SWP_NOMOVE | SWP_NOZORDER);

  SendMessage(GetDlgItem(IDC_ADDRESS),
              EM_LIMITTEXT, 8, 0);

  int size = regQueryDwordValue("memViewerDataSize", 0);
  if(size < 0 || size > 2)
    size = 0;
  viewer.setSize(size);
  DoRadio(false, IDC_8_BIT, size);

  HFONT font = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);
  
  ::SendMessage(GetDlgItem(IDC_CURRENT_ADDRESS), WM_SETFONT, (WPARAM)font, 0);
  
  return TRUE;
}

void GBMemoryViewerDlg::OnClose()
{
  winRemoveUpdateListener(this);
  
  DestroyWindow();
}

void GBMemoryViewerDlg::OnRefresh()
{
  viewer.Invalidate();
};

void GBMemoryViewerDlg::update()
{
  OnRefresh();
}

void GBMemoryViewerDlg::On8Bit()
{
  viewer.setSize(0);
  regSetDwordValue("memViewerDataSize", 0);
}

void GBMemoryViewerDlg::On16Bit()
{
  viewer.setSize(1);
  regSetDwordValue("memViewerDataSize", 1);
}

void GBMemoryViewerDlg::On32Bit()
{
  viewer.setSize(2);
  regSetDwordValue("memViewerDataSize", 2);
}

void GBMemoryViewerDlg::OnAutoUpdate()
{
  autoUpdate = !autoUpdate;
  if(autoUpdate) {
    winAddUpdateListener(this);
  } else {
    winRemoveUpdateListener(this);    
  }  
}

void GBMemoryViewerDlg::OnGo()
{
  char buffer[16];
  
  GetWindowText(GetDlgItem(IDC_ADDRESS), buffer, 16);
  
  u32 address;
  sscanf(buffer, "%x", &address);
  if(viewer.getSize() == 1)
    address &= ~1;
  else if(viewer.getSize() == 2)
    address &= ~3;
  viewer.setAddress(address);
}

void GBMemoryViewerDlg::OnAddressesSelChange()
{
  HWND h = (HWND)GetDlgItem(IDC_ADDRESSES);

  int cur = SendMessage(h, CB_GETCURSEL, 0, 0);
  
  switch(cur) {
  case 0:
    viewer.setAddress(0x0000);
    break;
  case 1:
    viewer.setAddress(0x4000);
    break;
  case 2:
    viewer.setAddress(0x8000);
    break;
  case 3:
    viewer.setAddress(0xa000);
    break;
  case 4:
    viewer.setAddress(0xc000);
    break;
  case 5:
    viewer.setAddress(0xd000);
    break;
  case 6:
    viewer.setAddress(0xff00);
    break;
  case 7:
    viewer.setAddress(0xff80);
    break;
  }
}

void GBMemoryViewerDlg::setCurrentAddress(u32 address)
{
  char buffer[20];

  sprintf(buffer, "0x%08X", address);
  ::SetWindowText(GetDlgItem(IDC_CURRENT_ADDRESS), buffer);
}

void GBMemoryViewerDlg::OnSave()
{
  MemoryViewerAddressSize dlg;
  char buffer[2048];

  dlg.setAddress(viewer.getCurrentAddress());

  char *exts[] = { ".dmp" };
  
  if(dlg.DoModal(IDD_ADDR_SIZE,
                 getHandle())) {
    buffer[0] = 0;
    FileDlg file(getHandle(),
                 buffer,
                 2048,
                 (char *)winLoadFilter(IDS_FILTER_DUMP),
                 0,
                 "DMP",
                 exts,
                 NULL,
                 (char *)winResLoadString(IDS_SELECT_DUMP_FILE),
                 TRUE);
    if(file.DoModal()) {
      FILE *f = fopen(buffer, "wb");
      
      if(f == NULL) {
        systemMessage(IDS_ERROR_CREATING_FILE, buffer);
        return;
      }

      int size = dlg.getSize();
      u16 addr = dlg.getAddress() & 0xffff;

      for(int i = 0; i < size; i++) {
        fputc(gbMemoryMap[addr >> 12][addr & 0xfff], f);
        addr++;
      }

      fclose(f);
    }
  }
}

void GBMemoryViewerDlg::OnLoad()
{
  char buffer[2048];
  buffer[0] = 0;
  char *exts[] = { ".dmp" };
  
  FileDlg file(getHandle(),
               buffer,
               2048,
               (char *)winLoadFilter(IDS_FILTER_DUMP),
               0,
               "DMP",
               exts,
               NULL,
               (char *)winResLoadString(IDS_SELECT_DUMP_FILE),
               FALSE);
  
  if(file.DoModal()) {
    FILE *f = fopen(buffer, "rb");
    if(f == NULL) {
      systemMessage(IDS_CANNOT_OPEN_FILE,
                    "Cannot open file %s",
                    buffer);
      return;
    }
    
    MemoryViewerAddressSize dlg;    

    fseek(f, 0, SEEK_END);
    int size = ftell(f);

    fseek(f, 0, SEEK_SET);
    
    dlg.setAddress(viewer.getCurrentAddress());
    dlg.setSize(size);
    
    if(dlg.DoModal(IDD_ADDR_SIZE,
                   getHandle())) {
      int size = dlg.getSize();
      u16 addr = dlg.getAddress() & 0xffff;

      for(int i = 0; i < size; i++) {
        int c = fgetc(f);
        if(c == -1)
          break;
        gbMemoryMap[addr >> 12][addr & 0xfff] = c;
        addr++;
      }
      OnRefresh();
    }
    fclose(f);    
  }  
}

void toolsGBMemoryViewer()
{
  GBMemoryViewerDlg *dlg = new GBMemoryViewerDlg();
  dlg->setAutoDelete(true);
  dlg->MakeDialog(hInstance,
                  IDD_MEM_VIEWER,
                  hWindow);
}
