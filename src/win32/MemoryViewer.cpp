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
#include <stdio.h>
#include <memory.h>
#include "../GBA.h"
#include "../Globals.h"
#include "Reg.h"
#include "WinResUtil.h"
#include "resource.h"
#include "IUpdate.h"

#define CPUReadByteQuick(addr) \
  ::map[(addr)>>24].address[(addr) & ::map[(addr)>>24].mask]
#define CPUWriteByteQuick(addr, b) \
  ::map[(addr)>>24].address[(addr) & ::map[(addr)>>24].mask] = (b)
#define CPUReadHalfWordQuick(addr) \
  *((u16 *)&::map[(addr)>>24].address[(addr) & ::map[(addr)>>24].mask])
#define CPUWriteHalfWordQuick(addr, b) \
  *((u16 *)&::map[(addr)>>24].address[(addr) & ::map[(addr)>>24].mask]) = (b)
#define CPUReadMemoryQuick(addr) \
  *((u32 *)&::map[(addr)>>24].address[(addr) & ::map[(addr)>>24].mask])
#define CPUWriteMemoryQuick(addr, b) \
  *((u32 *)&::map[(addr)>>24].address[(addr) & ::map[(addr)>>24].mask]) = (b)

extern LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
extern HINSTANCE hInstance;
extern HWND hWindow;
extern int emulating;

extern void winAddUpdateListener(IUpdateListener *);
extern void winRemoveUpdateListener(IUpdateListener *);

class MemoryViewer : public Wnd {
  u32 address;
  int dataSize;
  bool hasCaret;
  int caretWidth;
  int caretHeight;
  HFONT font;
  SIZE fontSize;
  u32 editAddress;
  int editNibble;
  int maxNibble;
  int displayedLines;
  int beginAscii;
  int beginHex;
  bool editAscii;

  static bool isRegistered;
protected:
  DECLARE_MESSAGE_MAP()
public:
  void beep();
  BOOL OnEditInput(UINT c);
  LRESULT OnWMChar(WPARAM wParam, LPARAM lParam);
  
  void setCaretPos();
  void destroyEditCaret();
  void createEditCaret(int w, int h);
  void moveAddress(s32 off, int nibble);
  MemoryViewer();

  static void registerClass();

  void readData(u32,int,u8 *);
  void setAddress(u32);
  void setSize(int);
  int getSize() { return dataSize; }
  void updateScrollInfo(int lines);

  virtual BOOL OnEraseBkgnd(HDC);
  virtual void OnPaint();
  virtual void OnVScroll(UINT, UINT, HWND);
  virtual void OnKeyDown(UINT virtKey, UINT, UINT);
  virtual void OnKillFocus(HWND next);
  virtual void OnSetFocus(HWND old);
  virtual void OnLButtonDown(UINT flags, int x, int y);
  virtual UINT OnGetDlgCode();
};

class MemoryViewerDlg : public ResizeDlg, IUpdateListener {
  MemoryViewer viewer;
  bool autoUpdate;
protected:
  DECLARE_MESSAGE_MAP()
public:
  MemoryViewerDlg();

  virtual BOOL OnInitDialog(LPARAM);
  virtual void OnClose();
  void On8Bit();
  void On16Bit();
  void On32Bit();
  void OnGo();
  void OnAddressesSelChange();
  void OnRefresh();
  void OnAutoUpdate();

  virtual void update();
};

bool MemoryViewer::isRegistered = false;

BEGIN_MESSAGE_MAP(MemoryViewer, Wnd)
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
  ON_WM_VSCROLL()
  ON_WM_KEYDOWN()
  ON_WM_KILLFOCUS()
  ON_WM_SETFOCUS()
  ON_WM_LBUTTONDOWN()
  ON_WM_GETDLGCODE()
  ON_MESSAGE(WM_CHAR, OnWMChar)
END_MESSAGE_MAP()

MemoryViewer::MemoryViewer()
  : Wnd()
{
  address = 0;
  dataSize = 0;
  editAddress = 0;
  editNibble = 0;
  displayedLines = 0;
  hasCaret = false;
  maxNibble = 0;
  font = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);
  fontSize.cx = fontSize.cy = 0;
  beginAscii = 0;
  beginHex = 0;
}

void MemoryViewer::readData(u32 address, int len, u8 *data)
{
  if(emulating && rom != NULL) {
    for(int i = 0; i < len; i++) {
      *data++ = CPUReadByteQuick(address);
      address++;
    }
  } else {
    for(int i = 0; i < len; i++) {
      *data++ = 0;
      address++;
    }    
  }
}

void MemoryViewer::setAddress(u32 a)
{
  address = a;
  if(displayedLines) {
    if((address+(displayedLines<<4)) < address) {
      address = 0xffffffff - (displayedLines<<4) + 1;
    }
  }
  InvalidateRect(NULL, TRUE);
}

void MemoryViewer::setSize(int s)
{
  dataSize = s;
  if(s == 0)
    maxNibble = 1;
  else if(s == 1)
    maxNibble = 3;
  else
    maxNibble = 7;
  
  InvalidateRect(NULL, TRUE);
}

BOOL MemoryViewer::OnEraseBkgnd(HDC)
{
  return TRUE;
}

void MemoryViewer::updateScrollInfo(int lines)
{
  int page = lines * 16;
  SCROLLINFO si;
  ZeroMemory(&si, sizeof(si));
  si.cbSize = sizeof(si);
  si.fMask = SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL | SIF_POS;
  si.nMin = 0;
  si.nMax = 0xa000000 / page;
  si.nPage = page;
  si.nPos = address / page;
  SetScrollInfo(getHandle(),
                SB_VERT,
                &si,
                TRUE);
}

void MemoryViewer::OnPaint()
{
  RECT rect;
  GetClientRect(getHandle(), &rect);
  int w = rect.right - rect.left;
  int h = rect.bottom - rect.top - 6;
  
  PAINTSTRUCT ps;
  HDC dc = BeginPaint(getHandle(), &ps);
  
  HDC memDC = CreateCompatibleDC(dc);
  HBITMAP bitmap = CreateCompatibleBitmap(dc, w, rect.bottom - rect.top);
  SelectObject(memDC, bitmap);
  
  FillRect(memDC, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
  DrawEdge(memDC, &rect, EDGE_ETCHED, BF_RECT);
  
  HFONT oldFont = (HFONT)SelectObject(memDC,
    font);
  
  GetTextExtentPoint(memDC, "0", 1, &fontSize);
  
  int lines = h / fontSize.cy;
  
  displayedLines = lines;
  
  updateScrollInfo(lines);
  
  u32 addr = address;
  
  SetTextColor(memDC, RGB(0,0,0));
  
  u8 data[32];
  
  RECT r;
  r.top = 3;
  r.left = 3;
  r.bottom = r.top+fontSize.cy;
  r.right = rect.right-3;
  
  int line = 0;
  
  for(int i = 0; i < lines; i++) {
    char buffer[33];
    sprintf(buffer, "%08X", addr);
    DrawText(memDC, buffer, 8, &r, DT_TOP | DT_LEFT | DT_NOPREFIX);
    r.left += 10*fontSize.cx;
    beginHex = r.left;
    readData(addr, 16, data);
    
    int j;
    
    if(dataSize == 0) {
      for(j = 0; j < 16; j++) {
        sprintf(buffer, "%02X", data[j]);
        DrawText(memDC, buffer, 2, &r, DT_TOP | DT_LEFT | DT_NOPREFIX);
        r.left += 3*fontSize.cx;
      }
    }
    if(dataSize == 1) {
      for(j = 0; j < 16; j += 2) {
        sprintf(buffer, "%04X", data[j] | data[j+1]<<8);
        DrawText(memDC, buffer, 4, &r, DT_TOP | DT_LEFT | DT_NOPREFIX);
        r.left += 5*fontSize.cx;
      }      
    }
    if(dataSize == 2) {
      for(j = 0; j < 16; j += 4) {
        sprintf(buffer, "%08X", data[j] | data[j+1]<<8 |
          data[j+2] << 16 | data[j+3] << 24);
        DrawText(memDC, buffer, 8, &r, DT_TOP | DT_LEFT | DT_NOPREFIX);
        r.left += 9*fontSize.cx;
      }            
    }
    
    line = r.left;
    
    r.left += fontSize.cx;
    beginAscii = r.left;

    for(j = 0; j < 16; j++) {
      char c = data[j];
      if(c >= 32 && c <= 127) {
        buffer[j] = c;
      } else
        buffer[j] = '.';
    }
    
    buffer[16] = 0;
    DrawText(memDC, buffer, 16, &r, DT_TOP | DT_LEFT | DT_NOPREFIX);
    addr += 16;
    r.top += fontSize.cy;
    r.bottom += fontSize.cy;
    r.left = 3;
  }
  HPEN pen = CreatePen(PS_SOLID, 1, RGB(0,0,0));
  HPEN old = (HPEN)SelectObject(memDC, pen);
  
  MoveToEx(memDC, 3+fontSize.cx*9, 3, NULL);
  LineTo(memDC, 3+fontSize.cx*9, 3+displayedLines*fontSize.cy);
  
  MoveToEx(memDC, line, 3, NULL);
  LineTo(memDC, line, 3+displayedLines*fontSize.cy);
  
  SelectObject(memDC, old);
  DeleteObject(pen);
  
  SelectObject(memDC, oldFont);
  
  BitBlt(dc, 0, 0, w, rect.bottom - rect.top, memDC, 0, 0, SRCCOPY);
  
  DeleteDC(memDC);
  DeleteObject(bitmap);
  
  EndPaint(getHandle(), &ps);
}

void MemoryViewer::OnVScroll(UINT type, UINT pos, HWND)
{
  int address = this->address;
  switch(type) {
  case SB_BOTTOM:
    address = 0xffffff00;
    break;
  case SB_LINEDOWN:
    address += 0x10;
    break;
  case SB_LINEUP:
    address -= 0x10;
    break;
  case SB_PAGEDOWN:
    address += (displayedLines<<4);
    break;
  case SB_PAGEUP:
    address -= (displayedLines<<4);
    break;
  case SB_TOP:
    address = 0;
    break;
  case SB_THUMBTRACK:
    {
      SCROLLINFO si;
      ZeroMemory(&si, sizeof(si));
      si.cbSize = sizeof(si);
      si.fMask = SIF_TRACKPOS | SIF_PAGE;
      GetScrollInfo(getHandle(), SB_VERT, &si);
      address = si.nPage * si.nTrackPos;
    }
    break;
  }
  setAddress(address);
}

UINT MemoryViewer::OnGetDlgCode()
{
  return DLGC_WANTALLKEYS;
}

void MemoryViewer::createEditCaret(int w, int h)
{
  if(!hasCaret || caretWidth != w || caretHeight != h) {
    hasCaret = true;
    caretWidth = w;
    caretHeight = h;
    ::CreateCaret(hWnd, (HBITMAP)0, w, h);
  }
}

void MemoryViewer::destroyEditCaret()
{
  hasCaret = false;
  DestroyCaret();
}

void MemoryViewer::setCaretPos()
{
  if(GetFocus() != hWnd) {
    destroyEditCaret();
    return;
  }

  if(editAddress < address || editAddress > (-1+address + (displayedLines<<4))) {
    destroyEditCaret();
    return;
  }

  int subAddress = (editAddress - address);

  int x = 3+10*fontSize.cx+editNibble*fontSize.cx;
  int y = 3+fontSize.cy*((editAddress-address)>>4);

  if(editAscii) {
    x = beginAscii + fontSize.cx*(subAddress&15);
  } else {
    switch(dataSize) {
    case 0:
      x += 3*fontSize.cx*(subAddress & 15);
      break;
    case 1:
      x += 5*fontSize.cx*((subAddress>>1) & 7);
      break;
    case 2:
      x += 9*fontSize.cx*((subAddress>>2) & 3);
      break;
    }
  }

  RECT r;
  GetClientRect(hWnd, &r);
  r.right -= 3;
  if(x >= r.right) {
    destroyEditCaret();
    return;
  }
  int w = fontSize.cx;
  if((x+fontSize.cx)>=r.right)
    w = r.right - x;
  createEditCaret(w, fontSize.cy);
  SetCaretPos(x, y);
  ShowCaret(hWnd);
}


void MemoryViewer::OnLButtonDown(UINT flags, int x, int y)
{
  int line = (y-3)/fontSize.cy;
  int beforeAscii = beginHex;
  int inc = 1;
  int sub = 3*fontSize.cx;
  switch(dataSize) {
  case 0:
    beforeAscii += 47*fontSize.cx;
    break;
  case 1:
    beforeAscii += 39*fontSize.cx;
    inc = 2;
    sub = 5*fontSize.cx;
    break;
  case 2:
    beforeAscii += 35*fontSize.cx;
    inc = 4;
    sub = 9*fontSize.cx;
    break;
  }
  
  editAddress = address + (line<<4);
  if(x >= beginHex && x < beforeAscii) {
    x -= beginHex;
    editNibble = 0;
    while(x > 0) {
      x -= sub;
      if(x >= 0)
        editAddress += inc;
      else {
        editNibble = (x + sub)/fontSize.cx;
      }
    }
    editAscii = false;
  } else if(x >= beginAscii) {
    int afterAscii = beginAscii+16*fontSize.cx;
    if(x >= afterAscii) 
      x = afterAscii-1;
    editAddress += (x-beginAscii)/fontSize.cx;
    editNibble = 0;
    editAscii = true;
  } else {
    return;
  }

  if(editNibble > maxNibble)
    editNibble = maxNibble;
  SetFocus(hWnd);
  setCaretPos();
  return;
}

void MemoryViewer::OnSetFocus(HWND old)
{
  setCaretPos();
  InvalidateRect(NULL, TRUE);
}

void MemoryViewer::OnKillFocus(HWND next)
{
  destroyEditCaret();
  InvalidateRect(NULL, TRUE);
}

void MemoryViewer::OnKeyDown(UINT virtKey, UINT, UINT)
{
  bool isShift = (GetKeyState(VK_SHIFT) & 0x80000000) == 0x80000000;

  HWND h;
  switch(virtKey) {
  case VK_RIGHT:
    if(editAscii)
      moveAddress(1,0);
    else if(isShift)
      moveAddress((maxNibble+1)>>1,0);
    else
      moveAddress(0, 1);
    break;
  case VK_LEFT:
    if(editAscii)
      moveAddress(-1, 0);
    else if(isShift)
      moveAddress(-((maxNibble+1)>>1),0);
    else
      moveAddress(0, -1);
    break;  
  case VK_DOWN:
    moveAddress(16, 0);
    break;
  case VK_UP:
    moveAddress(-16, 0);
    break;
  case VK_TAB:
    h = GetNextDlgTabItem(GetParent(getHandle()), getHandle(), isShift);
    SetFocus(h);
    break;
  }
}

void MemoryViewer::moveAddress(s32 offset, int nibbleOff)
{
  if(offset == 0) {
    if(nibbleOff == -1) {
      editNibble--;
      if(editNibble == -1) {
        editAddress -= (maxNibble + 1) >> 1;
        editNibble = maxNibble;
      }
      if(address == 0 && editAddress >= (displayedLines<<4)) {
        editAddress = 0;
        editNibble = 0;
        beep();
      }
      if(editAddress < address)
        setAddress(address - 16);
    } else {
      editNibble++;
      if(editNibble > maxNibble) {
        editNibble = 0;
        editAddress += (maxNibble + 1) >> 1;
      }
      if(editAddress < address) {
        editAddress -= (maxNibble + 1) >> 1;
        editNibble = maxNibble;
        beep();
      }
      if(editAddress >= (address+(displayedLines<<4)))
        setAddress(address+16);
    }
  } else {
    editAddress += offset;
    if(offset < 0 && editAddress > (-1+address+(displayedLines<<4))) {
      editAddress -= offset;
      beep();
      return;
    }
    if(offset > 0 && (editAddress < address)) {
      editAddress -= offset;
      beep();
      return;
    }
    if(editAddress < address) {
      if(offset & 15)
        setAddress((address+offset-16) & ~15);
      else
        setAddress(address+offset);
    } else if(editAddress > (address - 1 + (displayedLines<<4))) {
      if(offset & 15)
        setAddress((address+offset+16) & ~15);
      else
        setAddress(address+offset);
    }
  }

  setCaretPos();
}

LRESULT MemoryViewer::OnWMChar(WPARAM wParam, LPARAM)
{
  if(OnEditInput(wParam))
    return 0;
  return 1;
}

BOOL MemoryViewer::OnEditInput(UINT c)
{
  if(c > 255 || !emulating) {
    beep();
    return FALSE;
  }

  if(!editAscii)
    c = tolower(c);

  u32 value = 256;

  if(c >= 'a' && c <= 'f')
    value = 10 + (c - 'a');
  else if(c >= '0' && c <= '9')
    value = (c - '0');
  if(editAscii) {
    CPUWriteByteQuick(editAddress, c);
    moveAddress(1, 0);
    InvalidateRect(NULL, TRUE);
  } else {
    if(value != 256) {
      value <<= 4*(maxNibble-editNibble);
      u32 mask = ~(15 << 4*(maxNibble - editNibble));
      u32 oldValue;
      switch(dataSize) {
      case 0:
        oldValue = CPUReadByteQuick(editAddress);
        oldValue &= mask;
        oldValue |= value;
        CPUWriteByteQuick(editAddress, oldValue);
        break;
      case 1:
        oldValue = CPUReadHalfWordQuick(editAddress);
        oldValue &= mask;
        oldValue |= value;
        CPUWriteHalfWordQuick(editAddress, oldValue);
        break;
      case 2:
        oldValue = CPUReadMemoryQuick(editAddress);
        oldValue &= mask;
        oldValue |= value;
        CPUWriteMemoryQuick(editAddress, oldValue);
        break;
      }
      moveAddress(0, 1);
      InvalidateRect(NULL, TRUE);
    }
  }
  return TRUE;
}


void MemoryViewer::beep()
{
  MessageBeep((UINT)-1);
}

void MemoryViewer::registerClass()
{
  if(!isRegistered) {
    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_PARENTDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH )GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "VbaMemoryViewer";
    RegisterClass(&wc);
    isRegistered = true;
  }
}

BEGIN_MESSAGE_MAP(MemoryViewerDlg, ResizeDlg)
  ON_WM_CLOSE()
  ON_BN_CLICKED(IDC_CLOSE, OnClose)
  ON_BN_CLICKED(IDC_8_BIT, On8Bit)
  ON_BN_CLICKED(IDC_16_BIT, On16Bit)
  ON_BN_CLICKED(IDC_32_BIT, On32Bit)
  ON_BN_CLICKED(IDC_GO, OnGo)
  ON_BN_CLICKED(IDC_AUTO_UPDATE, OnAutoUpdate)
  ON_CONTROL(CBN_SELCHANGE, IDC_ADDRESSES, OnAddressesSelChange)
END_MESSAGE_MAP()

MemoryViewerDlg::MemoryViewerDlg()
  : ResizeDlg()
{
  MemoryViewer::registerClass();
  autoUpdate = false;
}

BOOL MemoryViewerDlg::OnInitDialog(LPARAM)
{
  DIALOG_SIZER_START( sz )
    DIALOG_SIZER_ENTRY( IDC_VIEWER, DS_SizeX | DS_SizeY )
    DIALOG_SIZER_ENTRY( IDC_REFRESH, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_CLOSE, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_AUTO_UPDATE, DS_MoveY)
    DIALOG_SIZER_END()
    SetData(sz,
            TRUE,
            HKEY_CURRENT_USER,
            "Software\\Emulators\\VisualBoyAdvance\\Viewer\\MemoryView",
            NULL);
  
  viewer.Attach(GetDlgItem(IDC_VIEWER));
  ShowScrollBar(viewer.getHandle(), SB_VERT, TRUE);
  EnableScrollBar(viewer.getHandle(), SB_VERT, ESB_ENABLE_BOTH);

  HWND h = GetDlgItem(IDC_ADDRESSES);

  char *s[] = {
    "0x00000000 - BIOS",
    "0x02000000 - WRAM",
    "0x03000000 - IRAM",
    "0x04000000 - I/O",
    "0x05000000 - PALETTE",
    "0x06000000 - VRAM",
    "0x07000000 - OAM",
    "0x08000000 - ROM"
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
  
  return TRUE;
}

void MemoryViewerDlg::OnClose()
{
  winRemoveUpdateListener(this);
  
  DestroyWindow();
}

void MemoryViewerDlg::OnRefresh()
{
  viewer.Invalidate();
};

void MemoryViewerDlg::update()
{
  OnRefresh();
}

void MemoryViewerDlg::On8Bit()
{
  viewer.setSize(0);
  regSetDwordValue("memViewerDataSize", 0);
}

void MemoryViewerDlg::On16Bit()
{
  viewer.setSize(1);
  regSetDwordValue("memViewerDataSize", 1);
}

void MemoryViewerDlg::On32Bit()
{
  viewer.setSize(2);
  regSetDwordValue("memViewerDataSize", 2);
}

void MemoryViewerDlg::OnAutoUpdate()
{
  autoUpdate = !autoUpdate;
  if(autoUpdate) {
    winAddUpdateListener(this);
  } else {
    winRemoveUpdateListener(this);    
  }  
}

void MemoryViewerDlg::OnGo()
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

void MemoryViewerDlg::OnAddressesSelChange()
{
  HWND h = (HWND)GetDlgItem(IDC_ADDRESSES);

  int cur = SendMessage(h, CB_GETCURSEL, 0, 0);
  
  switch(cur) {
  case 0:
    viewer.setAddress(0);
    break;
  case 1:
    viewer.setAddress(0x2000000);
    break;
  case 2:
    viewer.setAddress(0x3000000);
    break;
  case 3:
    viewer.setAddress(0x4000000);
    break;
  case 4:
    viewer.setAddress(0x5000000);
    break;
  case 5:
    viewer.setAddress(0x6000000);
    break;
  case 6:
    viewer.setAddress(0x7000000);
    break;
  case 7:
    viewer.setAddress(0x8000000);
    break;
  }
}

void toolsMemoryViewer()
{
  MemoryViewerDlg *dlg = new MemoryViewerDlg();
  dlg->setAutoDelete(true);
  dlg->MakeDialog(hInstance,
                  IDD_MEM_VIEWER,
                  hWindow);
}
