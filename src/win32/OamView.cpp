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

class OamView : public ResizeDlg, IUpdateListener {
private:
  BITMAPINFO bmpInfo;
  u8 *data;
  int w;
  int h;
  int number;
  bool autoUpdate;
  BitmapControl oamView;
  ZoomControl oamZoom;
  ColorControl color;

protected:
  DECLARE_MESSAGE_MAP()
  
public:
  OamView();
  ~OamView();

  void paint();
  void render();
  void setAttributes(u16, u16, u16);
  void save();
  void saveBMP(char *);
  void savePNG(char *);
  void updateScrollInfo();

  void OnStretch();
  void OnSprite();
  void OnAutoUpdate();

  virtual void update();
  
  virtual BOOL OnInitDialog(LPARAM);
  virtual void OnClose();
  virtual LRESULT OnMapInfo(WPARAM, LPARAM);
  virtual LRESULT OnColInfo(WPARAM, LPARAM);
  virtual void OnHScroll(UINT, UINT, HWND);
};

extern char *winLoadFilter(int id);
extern void utilPutDword(u8 *, u32);
extern void utilPutWord(u8 *, u16);

extern HWND hWindow;
extern HINSTANCE hInstance;
extern int videoOption;
extern int captureFormat;

extern void winAddUpdateListener(IUpdateListener *);
extern void winRemoveUpdateListener(IUpdateListener *);

enum {
  VIDEO_1X, VIDEO_2X, VIDEO_3X, VIDEO_4X,
  VIDEO_320x240, VIDEO_640x480
};

BEGIN_MESSAGE_MAP(OamView,ResizeDlg)
  ON_WM_HSCROLL()
  ON_WM_CLOSE()
  ON_MESSAGE(WM_MAPINFO, OnMapInfo)
  ON_MESSAGE(WM_COLINFO, OnColInfo)
  ON_BN_CLICKED(IDC_REFRESH, paint)
  ON_BN_CLICKED(IDC_CLOSE, OnClose)
  ON_BN_CLICKED(IDC_SAVE, save)
  ON_BN_CLICKED(IDC_STRETCH, OnStretch)
  ON_CONTROL(EN_CHANGE, IDC_SPRITE, OnSprite)
  ON_BN_CLICKED(IDC_AUTO_UPDATE, OnAutoUpdate)
END_MESSAGE_MAP()

OamView::OamView()
  : ResizeDlg()
{
  BitmapControl::registerClass();
  ZoomControl::registerClass();
  ColorControl::registerClass();

  autoUpdate = false;
  
  memset(&bmpInfo.bmiHeader, 0, sizeof(bmpInfo.bmiHeader));
  
  bmpInfo.bmiHeader.biSize = sizeof(bmpInfo.bmiHeader);
  bmpInfo.bmiHeader.biWidth = 32;
  bmpInfo.bmiHeader.biHeight = 32;
  bmpInfo.bmiHeader.biPlanes = 1;
  bmpInfo.bmiHeader.biBitCount = 24;
  bmpInfo.bmiHeader.biCompression = BI_RGB;
  data = (u8 *)calloc(1, 3 * 64 * 64);

  oamView.setData(data);
  oamView.setBmpInfo(&bmpInfo);

  number = 0;
}

OamView::~OamView()
{
  free(data);
  data = NULL;
}

void OamView::paint()
{
  if(oam == NULL || paletteRAM == NULL || vram == NULL)
    return;
  
  render();
  oamView.setSize(w,h);
  oamView.refresh();
}

void OamView::update()
{
  paint();
}

void OamView::setAttributes(u16 a0, u16 a1, u16 a2)
{
  char buffer[256];
  
  int y = a0 & 255;
  int rot = a0 & 512;
  int mode = (a0 >> 10) & 3;
  int mosaic = a0 & 4096;
  int color = a0 & 8192;
  int duple = a0 & 1024;
  int shape = (a0 >> 14) & 3;
  int x = a1 & 511;
  int rotParam = (a1 >> 9) & 31;
  int flipH = a1 & 4096;
  int flipV = a1 & 8192;
  int size = (a1 >> 14) & 3;
  int tile = a2 & 1023;
  int prio = (a2 >> 10) & 3;
  int pal = (a2 >> 12) & 15;

  wsprintf(buffer, "%d,%d", x,y);
  ::SetWindowText(GetDlgItem(IDC_POS), buffer);

  wsprintf(buffer, "%d", mode);
  ::SetWindowText(GetDlgItem(IDC_MODE), buffer);

  ::SetWindowText(GetDlgItem(IDC_COLORS), color ? "256" : "16");

  wsprintf(buffer, "%d", pal);
  ::SetWindowText(GetDlgItem(IDC_PALETTE), buffer);

  wsprintf(buffer, "%d", tile);
  ::SetWindowText(GetDlgItem(IDC_TILE), buffer);

  wsprintf(buffer, "%d", prio);
  ::SetWindowText(GetDlgItem(IDC_PRIO), buffer);

  wsprintf(buffer, "%d,%d", w,h);
  ::SetWindowText(GetDlgItem(IDC_SIZE2), buffer);

  if(rot) {
    wsprintf(buffer, "%d", rotParam);
  } else
    buffer[0] = 0;
  ::SetWindowText(GetDlgItem(IDC_ROT), buffer);

  if(rot)
    buffer[0] = 'R';
  else buffer[0] = ' ';
  if(!rot) {
    if(flipH)
      buffer[1] = 'H';
    else
      buffer[1] = ' ';
    if(flipV)
      buffer[2] = 'V';
    else
      buffer[2] = ' ';
  } else {
    buffer[1] = ' ';
    buffer[2] = ' ';
  }
  if(mosaic)
    buffer[3] = 'M';
  else
    buffer[3] = ' ';
  if(duple)
    buffer[4] = 'D';
  else
    buffer[4] = ' ';
  
  buffer[5] = 0;
  ::SetWindowText(GetDlgItem(IDC_FLAGS), buffer);
}

void OamView::render()
{
  int m=0;
  if(oam == NULL || paletteRAM == NULL || vram == NULL)
    return;
  
  u16 *sprites = &((u16 *)oam)[4*number];
  u16 *spritePalette = &((u16 *)paletteRAM)[0x100];
  u8 *bmp = data;
  
  u16 a0 = *sprites++;
  u16 a1 = *sprites++;
  u16 a2 = *sprites++;
  
  int sizeY = 8;
  int sizeX = 8;
  
  switch(((a0 >>12) & 0x0c)|(a1>>14)) {
  case 0:
    break;
  case 1:
    sizeX = sizeY = 16;
    break;
  case 2:
    sizeX = sizeY = 32;
    break;
  case 3:
    sizeX = sizeY = 64;
    break;
  case 4:
    sizeX = 16;
    break;
  case 5:
    sizeX = 32;
    break;
  case 6:
    sizeX = 32;
    sizeY = 16;
    break;
  case 7:
    sizeX = 64;
    sizeY = 32;
    break;
  case 8:
    sizeY = 16;
    break;
  case 9:
    sizeY = 32;
    break;
  case 10:
    sizeX = 16;
    sizeY = 32;
    break;
  case 11:
    sizeX = 32;
    sizeY = 64;
    break;
  default:
    return;
  }

  w = sizeX;
  h = sizeY;

  setAttributes(a0,a1,a2);
  
  int sy = (a0 & 255);
  
  if(a0 & 0x2000) {
    int c = (a2 & 0x3FF);
    //          if((DISPCNT & 7) > 2 && (c < 512))
    //            return;
    int inc = 32;
    if(DISPCNT & 0x40)
      inc = sizeX >> 2;
    else
      c &= 0x3FE;
    
    for(int y = 0; y < sizeY; y++) {
      for(int x = 0; x < sizeX; x++) {
        u32 color = vram[0x10000 + (((c + (y>>3) * inc)*
                                     32 + (y & 7) * 8 + (x >> 3) * 64 +
                                     (x & 7))&0x7FFF)];
        color = spritePalette[color];
        *bmp++ = ((color >> 10) & 0x1f) << 3;
        *bmp++ = ((color >> 5) & 0x1f) << 3;
        *bmp++ = (color & 0x1f) << 3;
      }
    }
  } else {
    int c = (a2 & 0x3FF);
    //      if((DISPCNT & 7) > 2 && (c < 512))
    //          continue;
    
    int inc = 32;
    if(DISPCNT & 0x40)
      inc = sizeX >> 3;
    int palette = (a2 >> 8) & 0xF0;
    for(int y = 0; y < sizeY; y++) {
      for(int x = 0; x < sizeX; x++) {
        u32 color = vram[0x10000 + (((c + (y>>3) * inc)*
                                     32 + (y & 7) * 4 + (x >> 3) * 32 +
                                     ((x & 7)>>1))&0x7FFF)];
        if(x & 1)
          color >>= 4;
        else
          color &= 0x0F;
        
        color = spritePalette[palette+color];
        *bmp++ = ((color >> 10) & 0x1f) << 3;
        *bmp++ = ((color >> 5) & 0x1f) << 3;
        *bmp++ = (color & 0x1f) << 3;            
      }
    }
  }
}

void OamView::saveBMP(char *name)
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

void OamView::savePNG(char *name)
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

void OamView::save()
{
  char captureBuffer[2048];

  if(captureFormat == 0)
    strcpy(captureBuffer, "oam.png");
  else
    strcpy(captureBuffer, "oam.bmp");

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

BOOL OamView::OnInitDialog(LPARAM)
{
  // winCenterWindow(getHandle());
  DIALOG_SIZER_START( sz )
    DIALOG_SIZER_ENTRY( IDC_OAM_VIEW, DS_SizeX | DS_SizeY )
    DIALOG_SIZER_ENTRY( IDC_OAM_VIEW_ZOOM, DS_MoveX)
    DIALOG_SIZER_ENTRY( IDC_REFRESH, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_SAVE,  DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_CLOSE, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_COLOR, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_R, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_G, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_B, DS_MoveY)    
    DIALOG_SIZER_END()
    SetData(sz,
            TRUE,
            HKEY_CURRENT_USER,
            "Software\\Emulators\\VisualBoyAdvance\\Viewer\\OamView",
            NULL);
  oamView.Attach(GetDlgItem(IDC_OAM_VIEW));
  oamZoom.Attach(GetDlgItem(IDC_OAM_VIEW_ZOOM));
  color.Attach(GetDlgItem(IDC_COLOR));
  ::SetWindowText(GetDlgItem(IDC_SPRITE), "0");

  updateScrollInfo();

  int s = regQueryDwordValue("oamViewStretch", 0);
  if(s)
    oamView.setStretch(true);
  DoCheckbox(false, IDC_STRETCH, s);
  
  paint();
  return TRUE;
}

void OamView::OnStretch()
{
  oamView.setStretch(!oamView.getStretch());
  paint();
  regSetDwordValue("oamViewStretch", oamView.getStretch());  
}

void OamView::OnAutoUpdate()
{
  autoUpdate = !autoUpdate;
  if(autoUpdate) {
    winAddUpdateListener(this);
  } else {
    winRemoveUpdateListener(this);    
  }  
}

void OamView::OnSprite()
{
  HWND h = GetDlgItem(IDC_SPRITE);
  char buffer[10];
  GetWindowText(h,  buffer, 10);
  int n = atoi(buffer);
  if(n < 0 || n > 127) {
    sprintf(buffer, "%d", number);
    ::SetWindowText(h, buffer);
    return;
  }
  number = n;
  paint();
  updateScrollInfo();
}

void OamView::OnClose()
{
  winRemoveUpdateListener(this);
  
  DestroyWindow();
}

LRESULT OamView::OnMapInfo(WPARAM, LPARAM lParam)
{
  u8 *colors = (u8 *)lParam;
  oamZoom.setColors(colors);
  
  return TRUE;
}

LRESULT OamView::OnColInfo(WPARAM wParam, LPARAM)
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

void OamView::updateScrollInfo()
{
  SCROLLINFO si;
  ZeroMemory(&si, sizeof(si));
  si.cbSize = sizeof(si);
  si.fMask = SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL | SIF_POS;
  si.nMin = 0;
  si.nMax = 127;
  si.nPage = 1;
  si.nPos = number;
  SetScrollInfo(GetDlgItem(IDC_SCROLLBAR),
                SB_CTL,
                &si,
                TRUE);    
}

void OamView::OnHScroll(UINT type, UINT pos, HWND)
{
  switch(type) {
  case SB_BOTTOM:
    number = 127;
    break;
  case SB_LINEDOWN:
    number++;
    if(number > 127)
      number = 127;
    break;
  case SB_LINEUP:
    number--;
    if(number < 0)
      number = 0;
    break;
  case SB_PAGEDOWN:
    number += 16;
    if(number > 127)
      number = 127;
    break;
  case SB_PAGEUP:
    number -= 16;
    if(number < 0)
      number = 0;
    break;
  case SB_TOP:
    number = 0;
    break;
  case SB_THUMBTRACK:
    number = pos;
    if(number < 0)
      number = 0;
    if(number > 127)
      number = 127;
    break;
  }

  updateScrollInfo();
  
  char buffer[10];
  sprintf(buffer, "%d", number);
  ::SetWindowText(GetDlgItem(IDC_SPRITE), buffer);
  paint();
}                       

void toolsOamViewer()
{
  OamView *dlg = new OamView();
  dlg->setAutoDelete(true);
  dlg->MakeDialog(hInstance,
                  IDD_OAM_VIEW,
                  hWindow);  
}
