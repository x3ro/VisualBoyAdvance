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

#include <memory.h>

#include "../GBA.h"
#include "../Globals.h"
#include "WinResUtil.h"
#include "Reg.h"
#include "resource.h"
#include "../NLS.h"

#include "Controls.h"
#include "CommDlg.h"
#include "IUpdate.h"

extern "C" {
#include <png.h>
}

class MapView : public ResizeDlg, IUpdateListener {
private:
  BITMAPINFO bmpInfo;
  u8 *data;
  int frame;
  u16 control;
  int bg;
  int w;
  int h;
  BitmapControl mapView;
  ZoomControl mapViewZoom;
  ColorControl color;
  bool autoUpdate;

protected:
  DECLARE_MESSAGE_MAP()
  
public:
  MapView();
  ~MapView();

  void save();
  void saveBMP(char *);
  void savePNG(char *);
  void enableButtons(int);
  void renderPixel(u32, u32);
  void renderView(u32, u32, u32, u32, u32, u32, u32, u32, bool);
  void renderTextScreen(u16);
  void renderRotScreen(u16);
  void renderMode0();
  void renderMode1();
  void renderMode2();
  void renderMode3();
  void renderMode4();
  void renderMode5();
  void paint();
  u32 GetClickAddress(int x, int y);
  u32 GetTextClickAddress(u32, int, int);

  void OnFrame0();
  void OnFrame1();
  void OnBg0();
  void OnBg1();
  void OnBg2();
  void OnBg3();
  void OnStretch();
  void OnAutoUpdate();

  virtual void update();
  
  virtual BOOL OnInitDialog(LPARAM);
  virtual void OnClose();
  virtual LRESULT OnMapInfo(WPARAM, LPARAM);
  virtual LRESULT OnColInfo(WPARAM, LPARAM);
};

extern char *winLoadFilter(int id);
extern void utilPutDword(u8 *, u32);
extern void utilPutWord(u8 *, u16);

extern void winAddUpdateListener(IUpdateListener *);
extern void winRemoveUpdateListener(IUpdateListener *);

extern HWND hWindow;
extern HINSTANCE hInstance;
extern int videoOption;
extern int captureFormat;

enum {
  VIDEO_1X, VIDEO_2X, VIDEO_3X, VIDEO_4X,
  VIDEO_320x240, VIDEO_640x480
};

BEGIN_MESSAGE_MAP(MapView, ResizeDlg)
  ON_WM_CLOSE()
  ON_MESSAGE( WM_MAPINFO, OnMapInfo)
  ON_MESSAGE( WM_COLINFO, OnColInfo)
  ON_BN_CLICKED(IDC_FRAME_0, OnFrame0)
  ON_BN_CLICKED(IDC_FRAME_1, OnFrame1)
  ON_BN_CLICKED(IDC_BG0, OnBg0)
  ON_BN_CLICKED(IDC_BG1, OnBg1)
  ON_BN_CLICKED(IDC_BG2, OnBg2)
  ON_BN_CLICKED(IDC_BG3, OnBg3)
  ON_BN_CLICKED(IDC_REFRESH, paint)
  ON_BN_CLICKED(IDC_CLOSE, OnClose)
  ON_BN_CLICKED(IDC_SAVE, save)
  ON_BN_CLICKED(IDC_STRETCH, OnStretch)
  ON_BN_CLICKED(IDC_AUTO_UPDATE, OnAutoUpdate)
END_MESSAGE_MAP()

MapView::MapView()
  : ResizeDlg()
{
  BitmapControl::registerClass();
  ZoomControl::registerClass();
  ColorControl::registerClass();

  autoUpdate = false;
  
  memset(&bmpInfo.bmiHeader, 0, sizeof(bmpInfo.bmiHeader));
  
  bmpInfo.bmiHeader.biSize = sizeof(bmpInfo.bmiHeader);
  bmpInfo.bmiHeader.biWidth = 1024;
  bmpInfo.bmiHeader.biHeight = -1024;
  bmpInfo.bmiHeader.biPlanes = 1;
  bmpInfo.bmiHeader.biBitCount = 24;
  bmpInfo.bmiHeader.biCompression = BI_RGB;
  data = (u8 *)calloc(1, 3 * 1024 * 1024);

  mapView.setData(data);
  mapView.setBmpInfo(&bmpInfo);
  
  control = BG0CNT;

  bg = 0;
  frame = 0;
}

MapView::~MapView()
{
  free(data);
  data = NULL;
}

void MapView::saveBMP(char *name)
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

void MapView::savePNG(char *name)
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

void MapView::save()
{
    char captureBuffer[2048];

  if(captureFormat == 0)
    strcpy(captureBuffer, "map.png");
  else
    strcpy(captureBuffer, "map.bmp");

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

void inline MapView::renderPixel(u32 xx, u32 yy)
{
  int x = xx;
  int y = yy;
  x >>= 8;
  y >>= 8;
  if(x >= 0 && x < w && y >= 0 && y < h) {
    u8 *bmp = &data[w * y *3 + 3 * x];
    *bmp++ = 0x00;
    *bmp++ = 0x00;
    *bmp++ = 0xff;
  }
}

void MapView::renderView(u32 startX, u32 startY,
                         u32 dx, u32 dy,
                         u32 dmx, u32 dmy,
                         u32 maskX, u32 maskY,
                         bool wrap)
{
  int x,y;
  u32 xx, yy;
  if(dx & 0x8000)
    dx |= 0xFFFF0000;
  if(dy & 0x8000)
    dy |= 0xffff0000;
  if(dmx & 0x8000)
    dmx |= 0xffff0000;
  if(dmy & 0x8000)
    dmy |= 0xffff0000;
  
  if(wrap) {
    startX &= maskX;
    startY &= maskY;
  }
  // top line
  xx = startX;
  yy = startY;
  for(x = 0; x < 240; x++) {
    renderPixel(xx, yy);
    xx += dx;
    yy += dy;
    if(wrap) {
      xx &= maskX;
      yy &= maskY;
    }
  }
  // bottom line
  xx = (startX+159*dmx) & maskX;
  yy = (startY+159*dmy) & maskY;
  for(x = 0; x < 240; x++) {
    renderPixel(xx, yy);
    xx += dx;
    yy += dy;
    if(wrap) {
      xx &= maskX;
      yy &= maskY;
    }
  }

  // left line
  xx = startX;
  yy = startY;
  for(y = 0; y < 160; y++) {
    renderPixel(xx, yy);
    xx += dmx;
    yy += dmy;
    if(wrap) {
      xx &= maskX;
      yy &= maskY;
    }
  }
  // right line
  xx = (startX + 239*dx) & maskX;
  yy = (startY + 239*dy) & maskY;
  for(y = 0; y < 160; y++) {
    renderPixel(xx, yy);
    xx += dmx;
    yy += dmy;
    if(wrap) {
      xx &= maskX;
      yy &= maskY;
    }
  }
}

void MapView::renderTextScreen(u16 control)
{
  u16 *palette = (u16 *)paletteRAM;
  u8 *charBase = &vram[((control >> 2) & 0x03) * 0x4000];
  u16 *screenBase = (u16 *)&vram[((control >> 8) & 0x1f) * 0x800];
  u8 *bmp = data;

  int sizeX = 256;
  int sizeY = 256;
  switch((control >> 14) & 3) {
  case 0:
    break;
  case 1:
    sizeX = 512;
    break;
  case 2:
    sizeY = 512;
    break;
  case 3:
    sizeX = 512;
    sizeY = 512;
    break;
  }

  w = sizeX;
  h = sizeY;
  
  if(control & 0x80) {
    for(int y = 0; y < sizeY; y++) {
      u16 *screenSource = screenBase + ((y>>3)*32);
      for(int x = 0; x < sizeX; x++) {
        u16 data = *screenSource;
      
        int tile = data & 0x3FF;
        int tileX = (x & 7);
        int tileY = y & 7;
        
        if(data & 0x0400)
          tileX = 7 - tileX;
        if(data & 0x0800)
          tileY = 7 - tileY;

        u8 c = charBase[tile * 64 + tileY * 8 + tileX];
        
        u16 color = palette[c];
        
        *bmp++ = ((color >> 10) & 0x1f) << 3;
        *bmp++ = ((color >> 5) & 0x1f) << 3;
        *bmp++ = (color & 0x1f) << 3;      
      
        if(data & 0x0400) {
          if(tileX == 0)
            screenSource++;
        } else if(tileX == 7)
          screenSource++;
        if(x == 255 && sizeX > 256) {
          screenSource = screenBase + 0x400 + ((y>>3)*32);
        }
      }
    }
  } else {
    for(int y = 0; y < sizeY; y++) {
      u16 *screenSource = screenBase + ((y>>3)*32);
      for(int x = 0; x < sizeX; x++) {
        u16 data = *screenSource;
        
        int tile = data & 0x3FF;
        int tileX = (x & 7);
        int tileY = y & 7;

        if(data & 0x0400)
          tileX = 7 - tileX;
        if(data & 0x0800)
          tileY = 7 - tileY;
        
        u8 color = charBase[tile * 32 + tileY * 4 + (tileX>>1)];
        
        if(tileX & 1) {
          color = (color >> 4);
        } else {
          color &= 0x0F;
        }

        int pal = (*screenSource>>8) & 0xF0;
        u16 color2 = palette[pal + color];

        *bmp++ = ((color2 >> 10) & 0x1f) << 3;
        *bmp++ = ((color2 >> 5) & 0x1f) << 3;
        *bmp++ = (color2 & 0x1f) << 3;
        
        if(data & 0x0400) {
          if(tileX == 0)
            screenSource++;
        } else if(tileX == 7)
          screenSource++;

        if(x == 255 && sizeX > 256) {
          screenSource = screenBase + 0x400 + ((y>>3)*32);
        }        
      }
    }
  }
  /*
  switch(bg) {
  case 0:
    renderView(BG0HOFS<<8, BG0VOFS<<8,
               0x100, 0x000,
               0x000, 0x100,
               (sizeX -1) <<8,
               (sizeY -1) << 8,
               true);
    break;
  case 1:
    renderView(BG1HOFS<<8, BG1VOFS<<8,
               0x100, 0x000,
               0x000, 0x100,
               (sizeX -1) <<8,
               (sizeY -1) << 8,
               true);
    break;
  case 2:
    renderView(BG2HOFS<<8, BG2VOFS<<8,
               0x100, 0x000,
               0x000, 0x100,
               (sizeX -1) <<8,
               (sizeY -1) << 8,
               true);
    break;
  case 3:
    renderView(BG3HOFS<<8, BG3VOFS<<8,
               0x100, 0x000,
               0x000, 0x100,
               (sizeX -1) <<8,
               (sizeY -1) << 8,
               true);
    break;
  }
  */
}

void MapView::renderRotScreen(u16 control)
{
  u16 *palette = (u16 *)paletteRAM;
  u8 *charBase = &vram[((control >> 2) & 0x03) * 0x4000];
  u8 *screenBase = (u8 *)&vram[((control >> 8) & 0x1f) * 0x800];
  u8 *bmp = data;

  int sizeX = 128;
  int sizeY = 128;
  switch((control >> 14) & 3) {
  case 0:
    break;
  case 1:
    sizeX = sizeY = 256;
    break;
  case 2:
    sizeX = sizeY = 512;
    break;
  case 3:
    sizeX = sizeY = 1024;
    break;
  }

  w = sizeX;
  h = sizeY;
  
  if(control & 0x80) {
    for(int y = 0; y < sizeY; y++) {
      for(int x = 0; x < sizeX; x++) {
        int tile = screenBase[(x>>3) + (y>>3)*(w>>3)];
        
        int tileX = (x & 7);
        int tileY = y & 7;
        
        u8 color = charBase[tile * 64 + tileY * 8 + tileX];
        u16 color2 = palette[color];

        *bmp++ = ((color2 >> 10) & 0x1f) << 3;
        *bmp++ = ((color2 >> 5) & 0x1f) << 3;
        *bmp++ = (color2 & 0x1f) << 3;
      }
    }
  } else {
    for(int y = 0; y < sizeY; y++) {
      for(int x = 0; x < sizeX; x++) {
        int tile = screenBase[(x>>3) + (y>>3)*(w>>3)];
        
        int tileX = (x & 7);
        int tileY = y & 7;
        
        u8 color = charBase[tile * 64 + tileY * 8 + tileX];
        u16 color2 = palette[color];

        *bmp++ = ((color2 >> 10) & 0x1f) << 3;
        *bmp++ = ((color2 >> 5) & 0x1f) << 3;
        *bmp++ = (color2 & 0x1f) << 3;        
      }
    }    
  }

  u32 xx;
  u32 yy;
  
  switch(bg) {
  case 2:
    xx = BG2X_L | BG2X_H << 16;
    yy = BG2Y_L | BG2Y_H << 16;

    /*    
    renderView(xx, yy, 
               BG2PA, BG2PC,
               BG2PB, BG2PD,
               (sizeX -1) <<8,
               (sizeY -1) << 8,
               (control & 0x2000) != 0);
    */
    break;
  case 3:
    xx = BG3X_L | BG3X_H << 16;
    yy = BG3Y_L | BG3Y_H << 16;
    /*    
    renderView(xx, yy, 
               BG3PA, BG3PC,
               BG3PB, BG3PD,
               (sizeX -1) <<8,
               (sizeY -1) << 8,
               (control & 0x2000) != 0);
    */
    break;
  }
}

void MapView::renderMode0()
{
  renderTextScreen(control);
}

void MapView::renderMode1()
{
  switch(bg) {
  case 0:
  case 1:
    renderTextScreen(control);
    break;
  case 2:
    renderRotScreen(control);
    break;
  default:
    bg = 0;
    control = BG0CNT;
    renderTextScreen(control);
    break;
  }
}

void MapView::renderMode2()
{
  switch(bg) {
  case 2:
  case 3:
    renderRotScreen(control);
    break;
  default:
    bg = 2;
    control = BG2CNT;
    renderRotScreen(control);
    break;
  }  
}
  
void MapView::renderMode3()
{
  u8 *bmp = data;
  u16 *src = (u16 *)&vram[0];

  w = 240;
  h = 160;
  
  for(int y = 0; y < 160; y++) {
    for(int x = 0; x < 240; x++) {
      u16 data = *src++;
      *bmp++ = ((data >> 10) & 0x1f) << 3;
      *bmp++ = ((data >> 5) & 0x1f) << 3;
      *bmp++ = (data & 0x1f) << 3;      
    }
  }
  bg = 2;
}

void MapView::renderMode4()
{
  u8 *bmp = data;
  u8 *src = frame ? &vram[0xa000] : &vram[0];
  u16 *pal = (u16 *)&paletteRAM[0];
  
  w = 240;
  h = 160;
  
  for(int y = 0; y < 160; y++) {
    for(int x = 0; x < 240; x++) {
      u8 c = *src++;
      u16 data = pal[c];
      *bmp++ = ((data >> 10) & 0x1f) << 3;
      *bmp++ = ((data >> 5) & 0x1f) << 3;
      *bmp++ = (data & 0x1f) << 3;      
    }
  }
  bg = 2;
}

void MapView::renderMode5()
{
  u8 *bmp = data;
  u16 *src = (u16 *)(frame ? &vram[0xa000] : &vram[0]);

  w = 160;
  h = 128;
  
  for(int y = 0; y < 128; y++) {
    for(int x = 0; x < 160; x++) {
      u16 data = *src++;
      *bmp++ = ((data >> 10) & 0x1f) << 3;
      *bmp++ = ((data >> 5) & 0x1f) << 3;
      *bmp++ = (data & 0x1f) << 3;      
    }
  }
  bg = 2;
}

void MapView::enableButtons(int mode)
{
  bool enable[6] = { true, true, true, true, true, true };

  switch(mode) {
  case 0:
    enable[4] = false;
    enable[5] = false;
    break;
  case 1:
    enable[3] = false;
    enable[4] = false;
    enable[5] = false;
    break;
  case 2:
    enable[0] = false;
    enable[1] = false;
    enable[4] = false;
    enable[5] = false;
    break;
  case 3:
    enable[0] = false;
    enable[1] = false;
    enable[2] = false;
    enable[3] = false;
    enable[4] = false;
    enable[5] = false;
    break;
  case 4:
    enable[0] = false;
    enable[1] = false;
    enable[2] = false;
    enable[3] = false;
    break;    
  case 5:
    enable[0] = false;
    enable[1] = false;
    enable[2] = false;
    enable[3] = false;
    break;    
  }
  EnableWindow(GetDlgItem(IDC_BG0), enable[0]);
  EnableWindow(GetDlgItem(IDC_BG1), enable[1]);
  EnableWindow(GetDlgItem(IDC_BG2), enable[2]);
  EnableWindow(GetDlgItem(IDC_BG3), enable[3]);
  EnableWindow(GetDlgItem(IDC_FRAME_0), enable[4]);
  EnableWindow(GetDlgItem(IDC_FRAME_1), enable[5]);
  int id = IDC_BG0;
  switch(bg) {
  case 1:
    id = IDC_BG1;
    break;
  case 2:
    id = IDC_BG2;
    break;
  case 3:
    id = IDC_BG3;
    break;
  }
  SendMessage(GetDlgItem(id), BM_SETCHECK, BST_CHECKED, 0);
  id = IDC_FRAME_0;
  if(frame != 0)
    id = IDC_FRAME_1;
  SendMessage(GetDlgItem(id), BM_SETCHECK, BST_CHECKED, 0);
}

void MapView::paint()
{
  if(vram == NULL)
    return;
  int mode = DISPCNT & 7;

  switch(bg) {
  default:
  case 0:
    control = BG0CNT;
    break;
  case 1:
    control = BG1CNT;
    break;
  case 2:
    control = BG2CNT;
    break;
  case 3:
    control = BG3CNT;
    break;
  }
  
  switch(mode) {
  case 0:
    renderMode0();
    break;
  case 1:
    renderMode1();
    break;
  case 2:
    renderMode2();
    break;
  case 3:
    renderMode3();
    break;
  case 4:
    renderMode4();
    break;
  case 5:
    renderMode5();
    break;
  }
  enableButtons(mode);
  SIZE s;
  mapView.GetScrollSize(s);
  if(s.cx != w || s.cy != h)
    mapView.setSize(w, h);
  if(mapView.getStretch())
    mapView.SetScrollSize(1,1);
  mapView.refresh();

  char buffer[32];
  
  u32 charBase = ((control >> 2) & 0x03) * 0x4000 + 0x6000000;
  u32 screenBase = ((control >> 8) & 0x1f) * 0x800 + 0x6000000;  

  sprintf(buffer, "%d", mode);
  ::SetWindowText(GetDlgItem(IDC_MODE), buffer);

  if(mode >= 3) {
    ::SetWindowText(GetDlgItem(IDC_MAPBASE), "");
    ::SetWindowText(GetDlgItem(IDC_CHARBASE), "");
  } else {
    sprintf(buffer, "0x%08X", screenBase);
    ::SetWindowText(GetDlgItem(IDC_MAPBASE), buffer);
    
    sprintf(buffer, "0x%08X", charBase);
    ::SetWindowText(GetDlgItem(IDC_CHARBASE), buffer);
  }

  sprintf(buffer, "%dx%d", w, h);
  ::SetWindowText(GetDlgItem(IDC_DIM), buffer);

  ::SetWindowText(GetDlgItem(IDC_NUMCOLORS), control & 0x80 ? "256" : "16");

  sprintf(buffer, "%d", control & 3);
  ::SetWindowText(GetDlgItem(IDC_PRIORITY), buffer);

  ::SetWindowText(GetDlgItem(IDC_MOSAIC), control & 0x40 ? "1" : "0");

  ::SetWindowText(GetDlgItem(IDC_OVERFLOW), bg <= 1 ? "" :
                  control & 0x2000 ? "1" : "0");
}

void MapView::update()
{
  paint();
}

BOOL MapView::OnInitDialog(LPARAM)
{
  DIALOG_SIZER_START( sz )
    DIALOG_SIZER_ENTRY( IDC_MAP_VIEW, DS_SizeX | DS_SizeY )
    DIALOG_SIZER_ENTRY( IDC_REFRESH, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_CLOSE, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_SAVE,  DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_COLOR, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_R, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_G, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_B, DS_MoveY)    
    DIALOG_SIZER_END()
    SetData(sz,
            TRUE,
            HKEY_CURRENT_USER,
            "Software\\Emulators\\VisualBoyAdvance\\Viewer\\MapView",
            NULL);
  mapView.Attach(GetDlgItem(IDC_MAP_VIEW));
  mapViewZoom.Attach(GetDlgItem(IDC_MAP_VIEW_ZOOM));
  color.Attach(GetDlgItem(IDC_COLOR));
  int s = regQueryDwordValue("mapViewStretch", 0);
  if(s)
    mapView.setStretch(true);
  DoCheckbox(false, IDC_STRETCH, s);
  paint();
  return TRUE;
}

void MapView::OnFrame0()
{
  frame = 0;
  paint();  
}

void MapView::OnFrame1()
{
  frame = 1;
  paint();  
}

void MapView::OnBg0()
{
  bg = 0;
  control = BG0CNT;
  paint();
}

void MapView::OnBg1()
{
  bg = 1;
  control = BG1CNT;
  paint();
}

void MapView::OnBg2()
{
  bg = 2;
  control = BG2CNT;
  paint();
}

void MapView::OnBg3()
{
  bg = 3;
  control = BG3CNT;
  paint();
}

void MapView::OnStretch()
{
  mapView.setStretch(!mapView.getStretch());
  paint();
  regSetDwordValue("mapViewStretch", mapView.getStretch());  
}

void MapView::OnAutoUpdate()
{
  autoUpdate = !autoUpdate;
  if(autoUpdate) {
    winAddUpdateListener(this);
  } else {
    winRemoveUpdateListener(this);    
  }
}

void MapView::OnClose()
{
  winRemoveUpdateListener(this);
  
  DestroyWindow();
}

u32 MapView::GetTextClickAddress(u32 base, int x, int y)
{
  if(x >= 255 && y < 256)
    base += 0x800;
  else if(x < 256 && y >= 256)
    base += 0x1000;
  else if(x >= 256 && y >= 256)
    base += 0x1800;
  x &= 255;
  y &= 255;
  base += (x>>3)*2 + 64*(y>>3);

  return base;
}

u32 MapView::GetClickAddress(int x, int y)
{
  int mode = DISPCNT & 7;

  u32 base = ((control >> 8) & 0x1f) * 0x800 + 0x6000000;
  
  if(bg == 0 || bg == 1) {
    return GetTextClickAddress(base, x, y);
  }
  if(mode == 0) {
    return GetTextClickAddress(base, x, y);
  }
  if(bg == 2 && mode < 3) {
    return base + (x>>3)*2+64*(y>>3);
  }
  if(mode != 4) {
    return 0x6000000 + 0xa000*frame + 2*x + w*y*2;
  }
  return 0x6000000 + 0xa000*frame + x + w*y;
}

LRESULT MapView::OnMapInfo(WPARAM wParam, LPARAM lParam)
{
  u8 *colors = (u8 *)lParam;
  mapViewZoom.setColors(colors);

  int x = wParam & 0xffff;
  int y = (wParam >> 16);
  
  char buffer[16];
  sprintf(buffer, "(%d,%d)", x, y);
  ::SetWindowText(GetDlgItem(IDC_XY), buffer);

  u32 address = GetClickAddress(x,y);
  sprintf(buffer, "0x%08X", address);
  ::SetWindowText(GetDlgItem(IDC_ADDRESS), buffer);

  int mode = DISPCNT & 7;  
  if(mode < 3) {
    u16 value = *((u16 *)&vram[address - 0x6000000]);

    int tile = value & 1023;
    sprintf(buffer, "%d", tile);
    ::SetWindowText(GetDlgItem(IDC_TILE_NUM), buffer);

    buffer[0] = value & 1024 ? 'H' : '-';
    buffer[1] = value & 2048 ? 'V' : '-';
    buffer[2] = 0;
    ::SetWindowText(GetDlgItem(IDC_FLIP), buffer);

    if(!(control & 0x80)) {
      sprintf(buffer, "%d", (value >> 12) & 15);
    } else
      strcpy(buffer, "---");
    ::SetWindowText(GetDlgItem(IDC_PALETTE_NUM), buffer);
  } else {
    ::SetWindowText(GetDlgItem(IDC_TILE_NUM), "---");
    ::SetWindowText(GetDlgItem(IDC_FLIP), "--");
    ::SetWindowText(GetDlgItem(IDC_PALETTE_NUM), "---");
  }
  
  return TRUE;
}

LRESULT MapView::OnColInfo(WPARAM wParam, LPARAM)
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

void toolsMapView()
{
  MapView *map = new MapView();
  map->setAutoDelete(true);
  map->MakeDialog(hInstance,
                  IDD_MAP_VIEW,
                  hWindow);
}
