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
#include "ResizeDlg.h"
#include "Controls.h"
#include "CommDlg.h"
#include "WinResUtil.h"
#include "resource.h"
#include "Reg.h"

#include "../GBA.h"
#include "../Globals.h"
#include "../NLS.h"
#include "IUpdate.h"

extern "C" {
#include <png.h>
}

extern HINSTANCE hInstance;
extern HWND hWindow;

extern char *winLoadFilter(int id);
extern void utilPutDword(u8 *, u32);
extern void utilPutWord(u8 *, u16);

extern void winAddUpdateListener(IUpdateListener *);
extern void winRemoveUpdateListener(IUpdateListener *);

extern int videoOption;
extern int captureFormat;

enum {
  VIDEO_1X, VIDEO_2X, VIDEO_3X, VIDEO_4X,
  VIDEO_320x240, VIDEO_640x480
};


class TileViewer : public ResizeDlg, IUpdateListener {
  int charBase;
  int is256Colors;
  int palette;
  BitmapControl tileView;
  BITMAPINFO bmpInfo;
  u8 *data;
  ZoomControl zoom;
  ColorControl color;
  int w;
  int h;
  bool autoUpdate;
protected:
  DECLARE_MESSAGE_MAP()
public:
  TileViewer();
  virtual ~TileViewer();

  virtual BOOL OnInitDialog(LPARAM);
  virtual void OnClose();
  virtual void OnHScroll(UINT, UINT, HWND);

  virtual LRESULT OnMapInfo(WPARAM, LPARAM);
  virtual LRESULT OnColInfo(WPARAM, LPARAM);

  void saveBMP(char *);
  void savePNG(char *);
  
  void save();
  void paint();
  void render();
  void renderTile256(int, int, int, u8 *,u16 *);
  void renderTile16(int, int, int, u8 *, u16 *);
  void On16Colors();
  void On256Colors();
  void OnCharBase0();
  void OnCharBase1();
  void OnCharBase2();
  void OnCharBase3();
  void OnCharBase4();
  void OnStretch();
  void OnAutoUpdate();

  virtual void update();
};

BEGIN_MESSAGE_MAP(TileViewer, ResizeDlg)
  ON_WM_CLOSE()
  ON_WM_HSCROLL()
  ON_MESSAGE(WM_MAPINFO, OnMapInfo)
  ON_MESSAGE(WM_COLINFO, OnColInfo)
  ON_BN_CLICKED(IDC_CLOSE, OnClose)
  ON_BN_CLICKED(IDC_REFRESH, paint)
  ON_BN_CLICKED(IDC_16_COLORS, On16Colors)
  ON_BN_CLICKED(IDC_256_COLORS, On256Colors)
  ON_BN_CLICKED(IDC_CHARBASE_0, OnCharBase0)
  ON_BN_CLICKED(IDC_CHARBASE_1, OnCharBase1)
  ON_BN_CLICKED(IDC_CHARBASE_2, OnCharBase2)
  ON_BN_CLICKED(IDC_CHARBASE_3, OnCharBase3)
  ON_BN_CLICKED(IDC_CHARBASE_4, OnCharBase4)
  ON_BN_CLICKED(IDC_STRETCH, OnStretch)
  ON_BN_CLICKED(IDC_SAVE, save)
  ON_BN_CLICKED(IDC_AUTO_UPDATE, OnAutoUpdate)
END_MESSAGE_MAP()

TileViewer::TileViewer()
 : ResizeDlg()
{
  BitmapControl::registerClass();
  ZoomControl::registerClass();
  ColorControl::registerClass();

  autoUpdate = false;
  
  memset(&bmpInfo, 0, sizeof(bmpInfo));

  bmpInfo.bmiHeader.biSize = sizeof(bmpInfo.bmiHeader);
  bmpInfo.bmiHeader.biWidth = 32*8;
  bmpInfo.bmiHeader.biHeight = 32*8;
  bmpInfo.bmiHeader.biPlanes = 1;
  bmpInfo.bmiHeader.biBitCount = 24;
  bmpInfo.bmiHeader.biCompression = BI_RGB;
  data = (u8 *)calloc(1, 3 * 32*32 * 64);

  tileView.setData(data);
  tileView.setBmpInfo(&bmpInfo);

  charBase = 0;
  is256Colors = 0;
  palette = 0;
  w = h = 0;
}

TileViewer::~TileViewer()
{
  free(data);
  data = NULL;
}

void TileViewer::saveBMP(char *name)
{
  u8 writeBuffer[1024 * 3];
  
  FILE *fp = fopen(name,"wb");

  if(!fp) {
    systemMessage(MSG_ERROR_CREATING_FILE, "Error creating file %s", name);
    return;
  }

  struct {
    u8 ident[2];
    u8 filesize[4];
    u8 reserved[4];
    u8 dataoffset[4];
    u8 headersize[4];
    u8 width[4];
    u8 height[4];
    u8 planes[2];
    u8 bitsperpixel[2];
    u8 compression[4];
    u8 datasize[4];
    u8 hres[4];
    u8 vres[4];
    u8 colors[4];
    u8 importantcolors[4];
    u8 pad[2];
  } bmpheader;
  memset(&bmpheader, 0, sizeof(bmpheader));

  bmpheader.ident[0] = 'B';
  bmpheader.ident[1] = 'M';

  u32 fsz = sizeof(bmpheader) + w*h*3;
  utilPutDword(bmpheader.filesize, fsz);
  utilPutDword(bmpheader.dataoffset, 0x38);
  utilPutDword(bmpheader.headersize, 0x28);
  utilPutDword(bmpheader.width, w);
  utilPutDword(bmpheader.height, h);
  utilPutDword(bmpheader.planes, 1);
  utilPutDword(bmpheader.bitsperpixel, 24);
  utilPutDword(bmpheader.datasize, 3*w*h);

  fwrite(&bmpheader, 1, sizeof(bmpheader), fp);

  u8 *b = writeBuffer;

  int sizeX = w;
  int sizeY = h;

  u8 *pixU8 = (u8 *)data+3*w*(h-1);
  for(int y = 0; y < sizeY; y++) {
    for(int x = 0; x < sizeX; x++) {
      *b++ = *pixU8++; // B
      *b++ = *pixU8++; // G
      *b++ = *pixU8++; // R
    }
    pixU8 -= 2*3*w;
    fwrite(writeBuffer, 1, 3*w, fp);
    
    b = writeBuffer;
  }

  fclose(fp);
}

void TileViewer::savePNG(char *name)
{
  u8 writeBuffer[1024 * 3];
  
  FILE *fp = fopen(name,"wb");

  if(!fp) {
    systemMessage(MSG_ERROR_CREATING_FILE, "Error creating file %s", name);
    return;
  }
  
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                NULL,
                                                NULL,
                                                NULL);
  if(!png_ptr) {
    fclose(fp);
    return;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);

  if(!info_ptr) {
    png_destroy_write_struct(&png_ptr,NULL);
    fclose(fp);
    return;
  }

  if(setjmp(png_ptr->jmpbuf)) {
    png_destroy_write_struct(&png_ptr,NULL);
    fclose(fp);
    return;
  }

  png_init_io(png_ptr,fp);

  png_set_IHDR(png_ptr,
               info_ptr,
               w,
               h,
               8,
               PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  png_write_info(png_ptr,info_ptr);

  u8 *b = writeBuffer;

  int sizeX = w;
  int sizeY = h;

  u8 *pixU8 = (u8 *)data;
  for(int y = 0; y < sizeY; y++) {
    for(int x = 0; x < sizeX; x++) {
      int blue = *pixU8++;
      int green = *pixU8++;
      int red = *pixU8++;
      
      *b++ = red;
      *b++ = green;
      *b++ = blue;
    }
    png_write_row(png_ptr,writeBuffer);
    
    b = writeBuffer;
  }
  
  png_write_end(png_ptr, info_ptr);

  png_destroy_write_struct(&png_ptr, &info_ptr);

  fclose(fp);
}

void TileViewer::save()
{
    char captureBuffer[2048];

  if(captureFormat == 0)
    strcpy(captureBuffer, "tiles.png");
  else
    strcpy(captureBuffer, "tiles.bmp");

  char *exts[] = {".png", ".bmp" };

  FileDlg dlg(getHandle(),
              (char *)captureBuffer,
              (int)sizeof(captureBuffer),
              (char *)winLoadFilter(IDS_FILTER_PNG),
              captureFormat ? 2 : 1,
              captureFormat ? "BMP" : "PNG",
              exts,
              (char *)NULL, 
              (char *)winResLoadString(IDS_SELECT_CAPTURE_NAME),
              TRUE);

  BOOL res = dlg.DoModal();  
  if(res == FALSE) {
    DWORD res = CommDlgExtendedError();
    return;
  }

  if(captureFormat)
    saveBMP(captureBuffer);
  else
    savePNG(captureBuffer);  
}

void TileViewer::renderTile256(int tile, int x, int y, u8 *charBase, u16 *palette)
{
  u8 *bmp = &data[24*x + 8*32*24*y];

  for(int j = 0; j < 8; j++) {
    for(int i = 0; i < 8; i++) {
      u8 c = charBase[tile*64 + j * 8 + i];

      u16 color = palette[c];

      *bmp++ = ((color >> 10) & 0x1f) << 3;
      *bmp++ = ((color >> 5) & 0x1f) << 3;
      *bmp++ = (color & 0x1f) << 3;      

    }
    bmp += 31*24; // advance line
  }
}

void TileViewer::renderTile16(int tile, int x, int y, u8 *charBase, u16 *palette)
{
  u8 *bmp = &data[24*x + 8*32*24*y];

  int pal = this->palette;

  if(this->charBase == 4)
    pal += 16;

  for(int j = 0; j < 8; j++) {
    for(int i = 0; i < 8; i++) {
      u8 c = charBase[tile*32 + j * 4 + (i>>1)];

      if(i & 1)
        c = c>>4;
      else
        c = c & 15;

      u16 color = palette[pal*16+c];

      *bmp++ = ((color >> 10) & 0x1f) << 3;
      *bmp++ = ((color >> 5) & 0x1f) << 3;
      *bmp++ = (color & 0x1f) << 3;
    }
    bmp += 31*24; // advance line
  }
}

void TileViewer::render()
{
  u16 *palette = (u16 *)paletteRAM;
  u8 *charBase = &vram[this->charBase * 0x4000];
 
  int maxY;

  if(is256Colors) {
    int tile = 0;
    maxY = 32;
    if(this->charBase == 3)
      maxY = 16;
    for(int y = 0; y < maxY; y++) {
      for(int x = 0; x < 32; x++) {
        if(this->charBase == 4)
          renderTile256(tile, x, y, charBase, &palette[256]);
        else
          renderTile256(tile, x, y, charBase, palette);
        tile++;
      }
    }
    tileView.setSize(32*8, maxY*8);
    w = 32*8;
    h = maxY*8;
    tileView.SetScrollSize(32*8, maxY*8);
    if(tileView.getStretch())
      tileView.SetScrollSize(1,1);
  } else {
    int tile = 0;
    maxY = 32;
    if(this->charBase == 3)
      maxY = 16;
    for(int y = 0; y < maxY; y++) {
      for(int x = 0; x < 32; x++) {
        renderTile16(tile, x, y, charBase, palette);    
        tile++;
      }
    }
    
    tileView.setSize(32*8, maxY*8);
    w = 32*8;
    h = maxY*8;
    tileView.SetScrollSize(32*8, maxY*8);
    if(tileView.getStretch())
      tileView.SetScrollSize(1,1);
  }
}

void TileViewer::update()
{
  paint();
}

BOOL TileViewer::OnInitDialog(LPARAM)
{
  DIALOG_SIZER_START( sz )
    DIALOG_SIZER_ENTRY( IDC_TILE_VIEW, DS_SizeX | DS_SizeY )
    DIALOG_SIZER_ENTRY( IDC_COLOR, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_R, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_G, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_B, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_REFRESH, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_CLOSE, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_SAVE, DS_MoveY)
  DIALOG_SIZER_END()
  SetData(sz,
    TRUE,
    HKEY_CURRENT_USER,
    "Software\\Emulators\\VisualBoyAdvance\\Viewer\\TileView",
    NULL);
  tileView.Attach(GetDlgItem(IDC_TILE_VIEW));
  zoom.Attach(GetDlgItem(IDC_MAP_VIEW_ZOOM));
  color.Attach(GetDlgItem(IDC_COLOR));

  DoRadio(false, IDC_16_COLORS, is256Colors);
  DoRadio(false, IDC_CHARBASE_0, charBase);
  HWND h = GetDlgItem(IDC_PALETTE_SLIDER);
  SendMessage(h, TBM_SETRANGE, FALSE, MAKELONG(0, 15));
  SendMessage(h, TBM_SETPAGESIZE, 0, 4);
  SendMessage(h, TBM_SETTICFREQ, 1, 0);
  paint();

  int s = regQueryDwordValue("tileViewStretch", 0);
  if(s)
    tileView.setStretch(true);
  DoCheckbox(false, IDC_STRETCH, s);
  
  return TRUE;
}

void TileViewer::OnClose()
{
  winRemoveUpdateListener(this);
  
  DestroyWindow();
}

void TileViewer::OnAutoUpdate()
{
  autoUpdate = !autoUpdate;
  if(autoUpdate) {
    winAddUpdateListener(this);
  } else {
    winRemoveUpdateListener(this);    
  }  
}

void TileViewer::paint()
{
  if(vram != NULL && paletteRAM != NULL) {
    render();
    tileView.refresh();
  }
}

void TileViewer::On16Colors()
{
  is256Colors = 0;
  paint();
}

void TileViewer::On256Colors()
{
  is256Colors = 1;
  paint();
}

void TileViewer::OnCharBase0()
{
  charBase = 0;
  paint();
}

void TileViewer::OnCharBase1()
{
  charBase = 1;
  paint();
}

void TileViewer::OnCharBase2()
{
  charBase = 2;
  paint();
}

void TileViewer::OnCharBase3()
{
  charBase = 3;
  paint();
}

void TileViewer::OnCharBase4()
{
  charBase = 4;
  paint();
}

void TileViewer::OnStretch()
{
  tileView.setStretch(!tileView.getStretch());
  paint();
  regSetDwordValue("tileViewStretch", tileView.getStretch());  
}

LRESULT TileViewer::OnMapInfo(WPARAM wParam, LPARAM lParam)
{
  u8 *colors = (u8 *)lParam;
  zoom.setColors(colors);

  int x = (wParam & 0xFFFF)/8;
  int y = ((wParam >> 16) & 0xFFFF)/8;

  u32 address = 0x6000000 + 0x4000 * charBase;
  int tile = 32 * y + x;

  address += 32 * tile;

  char buffer[16];
  sprintf(buffer, "%d", tile);
  ::SetWindowText(GetDlgItem(IDC_TILE_NUMBER), buffer);

  sprintf(buffer, "%08x", address);
  ::SetWindowText(GetDlgItem(IDC_ADDRESS), buffer);
  
  return TRUE;
}

LRESULT TileViewer::OnColInfo(WPARAM wParam, LPARAM)
{
  u16 c = (u16)wParam;
  char buffer[16];

  color.setColor(c);  

  int r = (c & 0x1f);
  int g = (c & 0x3e0) >> 5;
  int b = (c & 0x7c00) >> 10;

  sprintf(buffer, "R: %d", r);
  ::SetWindowText(GetDlgItem(IDC_R), buffer);

  sprintf(buffer, "G: %d", g);
  ::SetWindowText(GetDlgItem(IDC_G), buffer);

  sprintf(buffer, "B: %d", b);
  ::SetWindowText(GetDlgItem(IDC_B), buffer);

  return TRUE;
}

void TileViewer::OnHScroll(UINT type, UINT pos, HWND)
{
  switch(type) {
  case TB_THUMBPOSITION:
    palette = pos;
    break;
  default:
    palette = ::SendMessage(GetDlgItem(IDC_PALETTE_SLIDER),
      TBM_GETPOS, 0, 0);
    break;
  }
  paint();
}

void toolsTileViewer()
{
  TileViewer *dlg = new TileViewer();

  dlg->setAutoDelete(true);
  dlg->MakeDialog(hInstance, IDD_TILE_VIEWER, hWindow);
}
