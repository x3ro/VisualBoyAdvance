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
#include "WinResUtil.h"
#include "resource.h"
#include "Controls.h"
#include "CommDlg.h"
#include "IUpdate.h"
#include "PaletteView.h"
#include "../Util.h"

#define WM_PALINFO WM_APP+1

class GBAPaletteViewControl : public PaletteViewControl {
public:
  virtual void updatePalette();
};

class PaletteView : public ResizeDlg, IUpdateListener {
private:
  GBAPaletteViewControl paletteView;
  GBAPaletteViewControl paletteViewOBJ;
  ColorControl colorControl;
  bool autoUpdate;
protected:
  DECLARE_MESSAGE_MAP()
public:
  PaletteView();
  ~PaletteView();

  void save(int);
  
  virtual LRESULT OnPalInfo(WPARAM, LPARAM);
  virtual BOOL OnInitDialog(LPARAM);
  virtual void OnClose();  

  void OnSaveBg();
  void OnSaveObj();
  void OnRefresh();
  void OnAutoUpdate();

  virtual void update();
};

enum {
  VIDEO_1X, VIDEO_2X, VIDEO_3X, VIDEO_4X,
  VIDEO_320x240, VIDEO_640x480
};

extern char *winLoadFilter(int);
extern HINSTANCE hInstance;
extern HWND hWindow;
extern int videoOption;

extern void winAddUpdateListener(IUpdateListener *);
extern void winRemoveUpdateListener(IUpdateListener *);

void GBAPaletteViewControl::updatePalette()
{
  if(paletteRAM != NULL)
    memcpy(palette, &paletteRAM[paletteAddress], 512);
}

BEGIN_MESSAGE_MAP(PaletteViewControl, Wnd)
  ON_WM_PAINT()
  ON_WM_ERASEBKGND()
  ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

bool PaletteViewControl::isRegistered = false;

PaletteViewControl::PaletteViewControl()
  : Wnd()
{
  memset(&bmpInfo.bmiHeader, 0, sizeof(bmpInfo.bmiHeader));
  
  bmpInfo.bmiHeader.biSize = sizeof(bmpInfo.bmiHeader);
  bmpInfo.bmiHeader.biWidth = 256;
  bmpInfo.bmiHeader.biHeight = -256;
  bmpInfo.bmiHeader.biPlanes = 1;
  bmpInfo.bmiHeader.biBitCount = 24;
  bmpInfo.bmiHeader.biCompression = BI_RGB;
  data = (u8 *)malloc(3 * 256 * 256);

  w = 256;
  h = 256;

  colors = 256;

  paletteAddress = 0;
  
  ZeroMemory(palette, 512);

  selected = -1;
}

PaletteViewControl::~PaletteViewControl()
{
  free(data);
}

void PaletteViewControl::init(int c, int w, int h)
{
  this->w = w;
  this->h = h;
  this->colors = c;

  bmpInfo.bmiHeader.biWidth = w;
  bmpInfo.bmiHeader.biHeight = -h;  
}

bool PaletteViewControl::saveAdobe(char *name)
{
  FILE *f = fopen(name, "wb");

  if(!f)
    return false;

  for(int i = 0; i < colors; i++) {
    u16 c = palette[i];
    int r = (c & 0x1f) << 3;
    int g = (c & 0x3e0) >> 2;
    int b = (c & 0x7c00) >> 7;

    u8 data[3] = { r, g, b };
    fwrite(data, 1, 3, f);
  }
  if(colors < 256) {
    for(int i = colors; i < 256; i++) {
      u8 data[3] = { 0, 0, 0 };
      fwrite(data, 1, 3, f);
    }
  }
  fclose(f);

  return true;
}

bool PaletteViewControl::saveMSPAL(char *name)
{
  FILE *f = fopen(name, "wb");

  if(!f)
    return false;

  u8 data[4] = { 'R', 'I', 'F', 'F' };

  fwrite(data, 1, 4, f);
  utilPutDword(data, 256 * 4 + 16);
  fwrite(data, 1, 4, f);
  u8 data3[4] = { 'P', 'A', 'L', ' ' };
  fwrite(data3, 1, 4, f);
  u8 data4[4] = { 'd', 'a', 't', 'a' };
  fwrite(data4, 1, 4, f);
  utilPutDword(data, 256*4+4);
  fwrite(data, 1, 4, f);
  utilPutWord(&data[0], 0x0300);
  utilPutWord(&data[2], 256); // causes problems if not 16 or 256
  fwrite(data, 1, 4, f);
  
  for(int i = 0; i < colors; i++) {
    u16 c = palette[i];
    int r = (c & 0x1f) << 3;
    int g = (c & 0x3e0) >> 2;
    int b = (c & 0x7c00) >> 7;

    u8 data7[4] = { r, g, b, 0 };
    fwrite(data7, 1, 4, f);
  }
  if(colors < 256) {
    for(int i = colors; i < 256; i++) {
      u8 data7[4] = { 0, 0, 0, 0 };
      fwrite(data7, 1, 4, f);
    }
  }
  fclose(f);

  return true;
}

bool PaletteViewControl::saveJASCPAL(char *name)
{
  FILE *f = fopen(name, "wb");

  if(!f)
    return false;

  fprintf(f, "JASC-PAL\r\n0100\r\n256\r\n");
  
  for(int i = 0; i < colors; i++) {
    u16 c = palette[i];
    int r = (c & 0x1f) << 3;
    int g = (c & 0x3e0) >> 2;
    int b = (c & 0x7c00) >> 7;

    fprintf(f, "%d %d %d\r\n", r, g, b);
  }
  if(colors < 256) {
    for(int i = colors; i < 256; i++)
      fprintf(f, "0 0 0\r\n");
  }
  fclose(f);

  return true;  
}

void PaletteViewControl::setPaletteAddress(int address)
{
  paletteAddress = address;
}

void PaletteViewControl::setSelected(int s)
{
  selected = s;
  InvalidateRect(NULL, FALSE);
}

void PaletteViewControl::render(u16 color, int x, int y)
{
  u8 *start = data + y*16*w*3 + x*16*3;
  int skip = w*3-16*3;

  int r = (color & 0x1f) << 3;
  int g = (color & 0x3e0) >> 2;
  int b = (color & 0x7c00) >> 7;

  for(int i = 0; i < 16; i++) {
    *start++ = b;
    *start++ = g;
    *start++ = r;

    *start++ = b;
    *start++ = g;
    *start++ = r;

    *start++ = b;
    *start++ = g;
    *start++ = r;

    *start++ = b;
    *start++ = g;
    *start++ = r;

    *start++ = b;
    *start++ = g;
    *start++ = r;

    *start++ = b;
    *start++ = g;
    *start++ = r;

    *start++ = b;
    *start++ = g;
    *start++ = r;

    *start++ = b;
    *start++ = g;
    *start++ = r;

    *start++ = b;
    *start++ = g;
    *start++ = r;

    *start++ = b;
    *start++ = g;
    *start++ = r;

    *start++ = b;
    *start++ = g;
    *start++ = r;

    *start++ = b;
    *start++ = g;
    *start++ = r;

    *start++ = b;
    *start++ = g;
    *start++ = r;

    *start++ = b;
    *start++ = g;
    *start++ = r;

    *start++ = b;
    *start++ = g;
    *start++ = r;

    *start++ = b;
    *start++ = g;
    *start++ = r;
    
    start += skip;
  }
}

void PaletteViewControl::refresh()
{
  updatePalette();
  int sw = w/16;
  int sh = h/16;
  for(int i = 0; i < colors; i++) {
    render(palette[i], i & (sw-1), i/sw);
  }
  InvalidateRect(NULL, FALSE);
}

void PaletteViewControl::OnLButtonDown(UINT, int x, int y)
{
  RECT rect;
  GetClientRect(getHandle(), &rect);
  int h = rect.bottom - rect.top;
  int w = rect.right - rect.left;
  int sw = (this->w/16);
  int sh = (this->h/16);
  int mult = w / sw;
  int multY = h / sh;

  setSelected(x/mult + (y/multY)*sw);
  
  SendMessage(GetParent(getHandle()),
              WM_PALINFO,
              palette[x/mult+(y/multY)*sw],
              paletteAddress+(x/mult+(y/multY)*sw));
}

BOOL PaletteViewControl::OnEraseBkgnd(HDC)
{
  return TRUE;
}

void PaletteViewControl::OnPaint()
{
  RECT rect;
  GetClientRect(getHandle(), &rect);
  int w = rect.right - rect.left;
  int h = rect.bottom - rect.top;
  PAINTSTRUCT ps;
  HDC dc = BeginPaint(getHandle(), &ps);
  
  HDC memDC = CreateCompatibleDC(dc);
  HBITMAP bitmap = CreateCompatibleBitmap(dc, w, h);
  SelectObject(memDC, bitmap);
  
  StretchDIBits(memDC,
                0,
                0,
                w,
                h,
                0,
                0,
                this->w,
                this->h,
                data,
                &bmpInfo,
                DIB_RGB_COLORS,
                SRCCOPY);
  int sw = this->w / 16;
  int sh = this->h / 16;
  int mult  = w / sw;
  int multY = h / sh;
  HPEN pen = CreatePen(PS_SOLID, 1, RGB(192,192,192));
  HPEN old = (HPEN)SelectObject(memDC, pen);
  int i;
  for(i = 1; i < sh; i++) {
    MoveToEx(memDC, 0, i * multY, NULL);
    LineTo(memDC, w, i * multY);
  }
  for(i = 1; i < sw; i++) {
    MoveToEx(memDC, i * mult, 0, NULL);
    LineTo(memDC, i * mult, h);
  }
  DrawEdge(memDC, &rect, EDGE_SUNKEN, BF_RECT);
  SelectObject(memDC, old);
  DeleteObject(pen);

  if(selected != -1) {
    pen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
    old = (HPEN)SelectObject(memDC, pen);

    int startX = (selected & (sw-1))*mult+1;
    int startY = (selected / sw)*multY+1;
    int endX = startX + mult-2;
    int endY = startY + multY-2;
    
    MoveToEx(memDC, startX, startY, NULL);
    LineTo(memDC, endX, startY);
    LineTo(memDC, endX, endY);
    LineTo(memDC, startX, endY);
    LineTo(memDC, startX, startY-1);

    SelectObject(memDC, old);
    DeleteObject(pen);
  }
  
  BitBlt(dc,0,0,w,h,
         memDC,0,0,SRCCOPY);

  DeleteObject(bitmap);
  DeleteDC(memDC);
  
  EndPaint(getHandle(), &ps);
}

extern LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
extern HINSTANCE hInstance;

void PaletteViewControl::registerClass()
{
  if(!isRegistered) {
    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH )GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "VbaPaletteViewControl";
    RegisterClass(&wc);
    isRegistered = true;
  }
}

BEGIN_MESSAGE_MAP(PaletteView, ResizeDlg)
  ON_MESSAGE(WM_PALINFO, OnPalInfo)
  ON_WM_CLOSE()
  ON_BN_CLICKED(IDC_SAVE_BG, OnSaveBg)
  ON_BN_CLICKED(IDC_SAVE_OBJ, OnSaveObj)
  ON_BN_CLICKED(IDC_REFRESH2, OnRefresh)
  ON_BN_CLICKED(IDC_CLOSE, OnClose)
  ON_BN_CLICKED(IDC_AUTO_UPDATE, OnAutoUpdate)
END_MESSAGE_MAP()

PaletteView::PaletteView()
  : ResizeDlg()
{
  PaletteViewControl::registerClass();
  ColorControl::registerClass();
  autoUpdate = false;
}

PaletteView::~PaletteView()
{
}

BOOL PaletteView::OnInitDialog(LPARAM)
{
  DIALOG_SIZER_START( sz )
    DIALOG_SIZER_END()
    SetData(sz,
            FALSE,
            HKEY_CURRENT_USER,
            "Software\\Emulators\\VisualBoyAdvance\\Viewer\\PaletteView",
            NULL);
  
  paletteView.Attach(GetDlgItem(IDC_PALETTE_VIEW));
  paletteViewOBJ.Attach(GetDlgItem(IDC_PALETTE_VIEW_OBJ));
  colorControl.Attach(GetDlgItem(IDC_COLOR));

  paletteView.setPaletteAddress(0);
  paletteView.refresh();  
  
  paletteViewOBJ.setPaletteAddress(0x200);
  paletteViewOBJ.refresh();  
  return TRUE;
}

void PaletteView::save(int which)
{
  char captureBuffer[2048];

  if(which == 0)
    strcpy(captureBuffer, "bg.pal");
  else
    strcpy(captureBuffer, "obj.pal");

  char *exts[] = {".pal", ".pal", ".act" };

  FileDlg dlg(getHandle(),
              (char *)captureBuffer,
              (int)sizeof(captureBuffer),
              (char *)winLoadFilter(IDS_FILTER_PAL),
              1,
              "PAL",
        exts,
              (char *)NULL, 
              (char *)winResLoadString(IDS_SELECT_PALETTE_NAME),
              TRUE);

  BOOL res = dlg.DoModal();  
  if(res == FALSE) {
    DWORD res = CommDlgExtendedError();
    return;
  }

  PaletteViewControl *p = NULL;

  if(which == 0)
    p = &paletteView;
  else
    p = &paletteViewOBJ;
  
  switch(dlg.getFilterIndex()) {
  case 0:
  case 1:
    p->saveMSPAL(captureBuffer);
    break;
  case 2:
    p->saveJASCPAL(captureBuffer);
    break;
  case 3:
    p->saveAdobe(captureBuffer);
    break;
  }
}

void PaletteView::OnSaveBg()
{
  save(0);
}

void PaletteView::OnSaveObj()
{
  save(1);
}

void PaletteView::OnRefresh()
{
  paletteView.refresh();
  paletteViewOBJ.refresh();  
}

void PaletteView::update()
{
  OnRefresh();
}

void PaletteView::OnAutoUpdate()
{
  autoUpdate = !autoUpdate;
  if(autoUpdate) {
    winAddUpdateListener(this);
  } else {
    winRemoveUpdateListener(this);    
  }  
}

void PaletteView::OnClose()
{
  winRemoveUpdateListener(this);
  
  DestroyWindow();
}

LRESULT PaletteView::OnPalInfo(WPARAM wParam, LPARAM lParam)
{
  u16 color = (u16)wParam;
  u32 address = (u32)lParam;
  char buffer[256];

  if(address >= 0x200)
    address = 0x5000200 + 2*(address & 255);
  else
    address = 0x5000000 + 2*(address & 255); 
  
  wsprintf(buffer, "0x%08X", address);
  ::SetWindowText(GetDlgItem(IDC_ADDRESS), buffer);

  int r = (color & 0x1f);
  int g = (color & 0x3e0) >> 5;
  int b = (color & 0x7c00) >> 10;

  wsprintf(buffer, "%d", r);
  ::SetWindowText(GetDlgItem(IDC_R), buffer);

  wsprintf(buffer, "%d", g);
  ::SetWindowText(GetDlgItem(IDC_G), buffer);

  wsprintf(buffer, "%d", b);
  ::SetWindowText(GetDlgItem(IDC_B), buffer);


  wsprintf(buffer, "0x%04X", color);
  ::SetWindowText(GetDlgItem(IDC_VALUE), buffer);

  colorControl.setColor(color);

  if(address >= 0x5000200) {
    paletteView.setSelected(-1);
  } else
    paletteViewOBJ.setSelected(-1);
  
  return TRUE;
}

void toolsPaletteView()
{
  PaletteView *pal = new PaletteView();
  pal->setAutoDelete(true);
  pal->MakeDialog(hInstance,
                  IDD_PALETTE_VIEW,
                  hWindow);
}
