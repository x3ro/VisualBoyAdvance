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
#include "../System.h"
#include "../gb/gbGlobals.h"
#include "WinResUtil.h"
#include "resource.h"
#include "Controls.h"
#include "CommDlg.h"
#include "IUpdate.h"
#include "PaletteView.h"

#define WM_PALINFO WM_APP+1

class GBPaletteViewControl : public PaletteViewControl {
public:
  virtual void updatePalette();
};

class GBPaletteView : public ResizeDlg, IUpdateListener {
private:
  GBPaletteViewControl paletteView;
  GBPaletteViewControl paletteViewOBJ;
  ColorControl colorControl;
  bool autoUpdate;
protected:
  DECLARE_MESSAGE_MAP()
public:
  GBPaletteView();
  ~GBPaletteView();

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

void GBPaletteViewControl::updatePalette()
{
  if(gbRom) {
    memcpy(palette, &gbPalette[paletteAddress], 64);
  }
}

BEGIN_MESSAGE_MAP(GBPaletteView, ResizeDlg)
  ON_MESSAGE(WM_PALINFO, OnPalInfo)
  ON_WM_CLOSE()
  ON_BN_CLICKED(IDC_SAVE_BG, OnSaveBg)
  ON_BN_CLICKED(IDC_SAVE_OBJ, OnSaveObj)
  ON_BN_CLICKED(IDC_REFRESH2, OnRefresh)
  ON_BN_CLICKED(IDC_CLOSE, OnClose)
  ON_BN_CLICKED(IDC_AUTO_UPDATE, OnAutoUpdate)
END_MESSAGE_MAP()

GBPaletteView::GBPaletteView()
  : ResizeDlg()
{
  PaletteViewControl::registerClass();
  ColorControl::registerClass();
  autoUpdate = false;
}

GBPaletteView::~GBPaletteView()
{
}

BOOL GBPaletteView::OnInitDialog(LPARAM)
{
  DIALOG_SIZER_START( sz )
    DIALOG_SIZER_END()
    SetData(sz,
            FALSE,
            HKEY_CURRENT_USER,
            "Software\\Emulators\\VisualBoyAdvance\\Viewer\\GBPaletteView",
            NULL);

  paletteView.init(32, 64, 128);
  paletteViewOBJ.init(32, 64, 128);
  
  paletteView.Attach(GetDlgItem(IDC_PALETTE_VIEW));
  paletteViewOBJ.Attach(GetDlgItem(IDC_PALETTE_VIEW_OBJ));
  colorControl.Attach(GetDlgItem(IDC_COLOR));

  paletteView.setPaletteAddress(0);
  paletteView.refresh();  
  
  paletteViewOBJ.setPaletteAddress(32);
  paletteViewOBJ.refresh();  
  return TRUE;
}

void GBPaletteView::save(int which)
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

void GBPaletteView::OnSaveBg()
{
  save(0);
}

void GBPaletteView::OnSaveObj()
{
  save(1);
}

void GBPaletteView::OnRefresh()
{
  paletteView.refresh();
  paletteViewOBJ.refresh();  
}

void GBPaletteView::update()
{
  OnRefresh();
}

void GBPaletteView::OnAutoUpdate()
{
  autoUpdate = !autoUpdate;
  if(autoUpdate) {
    winAddUpdateListener(this);
  } else {
    winRemoveUpdateListener(this);    
  }  
}

void GBPaletteView::OnClose()
{
  winRemoveUpdateListener(this);
  
  DestroyWindow();
}

LRESULT GBPaletteView::OnPalInfo(WPARAM wParam, LPARAM lParam)
{
  u16 color = (u16)wParam;
  u32 address = (u32)lParam;
  char buffer[256];

  bool isOBJ = address >= 32;
  address &= 31;
  
  wsprintf(buffer, "%d", address);
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

  if(isOBJ)
    paletteView.setSelected(-1);
  else
    paletteViewOBJ.setSelected(-1);
  
  return TRUE;
}

void toolsGBPaletteView()
{
  GBPaletteView *pal = new GBPaletteView();
  pal->setAutoDelete(true);
  pal->MakeDialog(hInstance,
                  IDD_GB_PALETTE_VIEW,
                  hWindow);
}
