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
#include <stdio.h>
#include <windows.h>

#include "../System.h"
#include "../GBA.h"
#include "../Globals.h"
#include "../Font.h"

#include "Display.h"
#include "Reg.h"
#include "resource.h"

enum {
  VIDEO_1X, VIDEO_2X, VIDEO_3X, VIDEO_4X,
  VIDEO_320x240, VIDEO_640x480, VIDEO_800x600, VIDEO_OTHER
};

extern int sizeX;
extern int sizeY;
extern int surfaceSizeX;
extern int surfaceSizeY;
extern int videoOption;
extern BOOL fullScreenStretch;
extern HWND hWindow;
extern int fsWidth;
extern int fsHeight;
extern int fsColorDepth;
extern RECT rect;
extern RECT dest;
extern HINSTANCE hInstance;
extern int windowPositionX;
extern int windowPositionY;
extern BOOL ddrawEmulationOnly;
extern BOOL ddrawUsingEmulationOnly;
extern BOOL ddrawUseVideoMemory;
extern BOOL tripleBuffering;
extern int ddrawDebug;
extern BOOL mode320Available;
extern BOOL mode640Available;
extern BOOL mode800Available;
extern GUID *pVideoDriverGUID;
extern void (*filterFunction)(u8*,u32,u8*,u8*,u32,int,int);
extern void (*ifbFunction)(u8*,u32,int,int);
extern int RGB_LOW_BITS_MASK;
extern BOOL vsync;
extern int filterWidth;
extern int filterHeight;
extern u8 *delta[257*244*4];
extern int cartridgeType;
extern int gbBorderOn;
extern int showSpeed;
extern BOOL showSpeedTransparent;
extern int systemSpeed;
extern bool disableMMX;
extern bool speedup;
extern int showRenderedFrames;
extern BOOL menuToggle;
extern BOOL active;
extern bool screenMessage;
extern char screenMessageBuffer[41];
extern DWORD screenMessageTime;
extern bool disableStatusMessage;

#ifdef MMX
extern "C" bool cpu_mmx;

extern bool detectMMX();
#endif

extern void updateFilter();
extern void updateIFB();
extern int Init_2xSaI(u32);
extern void winlog(const char *,...);
extern void updateMenuBar();
extern void adjustDestRect();
extern void directXMessage(char *msg);

class GDIDisplay : public IDisplay {
private:
  u8 *filterData;
  u8 info[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];
  
public:
  GDIDisplay();
  virtual ~GDIDisplay();

  virtual bool initialize();
  virtual void cleanup();
  virtual void render();
  virtual void checkFullScreen();
  virtual void renderMenu();
  virtual void clear();
  virtual DISPLAY_TYPE getType() { return GDI; };
  virtual void setOption(const char *, int) {}
  virtual bool isSkinSupported() { return true; }
  virtual int selectFullScreenMode(GUID **);
};

static int calculateShift(u32 mask)
{
  int m = 0;
  
  while(mask) {
    m++;
    mask >>= 1;
  }

  return m-5;
}

GDIDisplay::GDIDisplay()
{
  filterData = (u8 *)malloc(4*4*256*240);
}

GDIDisplay::~GDIDisplay()
{
  cleanup();
}

void GDIDisplay::cleanup()
{
  if(filterData) {
    free(filterData);
    filterData = NULL;
  }
}

bool GDIDisplay::initialize()
{
  sizeX = 240;
  sizeY = 160;
  switch(videoOption) {
  case VIDEO_1X:
    surfaceSizeX = sizeX;
    surfaceSizeY = sizeY;
    break;
  case VIDEO_2X:
    surfaceSizeX = sizeX * 2;
    surfaceSizeY = sizeY * 2;
    break;
  case VIDEO_3X:
    surfaceSizeX = sizeX * 3;
    surfaceSizeY = sizeY * 3;
    break;
  case VIDEO_4X:
    surfaceSizeX = sizeX * 4;
    surfaceSizeY = sizeY * 4;
    break;
  case VIDEO_320x240:
  case VIDEO_640x480:
  case VIDEO_800x600:
  case VIDEO_OTHER:
    {
      int scaleX = (fsWidth / sizeX);
      int scaleY = (fsHeight / sizeY);
      int min = scaleX < scaleY ? scaleX : scaleY;
      surfaceSizeX = sizeX * min;
      surfaceSizeY = sizeY * min;
      if(fullScreenStretch) {
        surfaceSizeX = fsWidth;
        surfaceSizeY = fsHeight;
      }
    }
    break;
  }
  
  rect.left = 0;
  rect.top = 0;
  rect.right = sizeX;
  rect.bottom = sizeY;

  dest.left = 0;
  dest.top = 0;
  dest.right = surfaceSizeX;
  dest.bottom = surfaceSizeY;

  DWORD style = WS_POPUP | WS_VISIBLE;
  DWORD styleEx = 0;
  
  if(videoOption <= VIDEO_4X)
    style |= WS_OVERLAPPEDWINDOW;
  else
    styleEx = WS_EX_TOPMOST;

  if(videoOption <= VIDEO_4X)
    AdjustWindowRectEx(&dest, style, TRUE, styleEx);
  else
    AdjustWindowRectEx(&dest, style, FALSE, styleEx);    

  int winSizeX = dest.right-dest.left;
  int winSizeY = dest.bottom-dest.top;

  if(videoOption > VIDEO_4X) {
    winSizeX = fsWidth;
    winSizeY = fsHeight;
  }
  
  int x = 0;
  int y = 0;

  if(videoOption <= VIDEO_4X) {
    x = windowPositionX;
    y = windowPositionY;
  }
  
  // Create a window
  hWindow = CreateWindowEx(styleEx,
                           "GBA",
                           "VisualBoyAdvance",
                           style,
                           x,
                           y,
                           winSizeX,
                           winSizeY,
                           NULL,
                           NULL,
                           hInstance,
                           NULL);
  
  if (!hWindow) {
    winlog("Error creating Window %08x\n", GetLastError());
    //    errorMessage(myLoadString(IDS_ERROR_DISP_FAILED));
    return FALSE;
  }
  
  updateMenuBar();
  
  adjustDestRect();
  
  mode320Available = FALSE;
  mode640Available = FALSE;
  mode800Available = FALSE;
  
  HDC dc = GetDC(NULL);
  HBITMAP hbm = CreateCompatibleBitmap(dc, 1, 1);
  BITMAPINFO *bi = (BITMAPINFO *)info;
  ZeroMemory(bi, sizeof(info));
  bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  GetDIBits(dc, hbm, 0, 1, NULL, (LPBITMAPINFO)info, DIB_RGB_COLORS);  
  GetDIBits(dc, hbm, 0, 1, NULL, (LPBITMAPINFO)info, DIB_RGB_COLORS);
  DeleteObject(hbm);
  ReleaseDC(NULL, dc);

  if(bi->bmiHeader.biCompression == BI_BITFIELDS) {
    systemColorDepth = bi->bmiHeader.biBitCount;
    if(systemColorDepth == 15)
      systemColorDepth = 16;
    systemRedShift = calculateShift(*((DWORD *)&bi->bmiColors[0]));
    systemGreenShift = calculateShift(*((DWORD *)&bi->bmiColors[1]));
    systemBlueShift = calculateShift(*((DWORD *)&bi->bmiColors[2]));
    if(systemColorDepth == 16) {
      if(systemGreenShift == 6) {
        Init_2xSaI(565);
        RGB_LOW_BITS_MASK=0x821;
      } else {
        Init_2xSaI(555);
        RGB_LOW_BITS_MASK=0x421;        
      }
    } else if(systemColorDepth == 32)
      Init_2xSaI(32);
  } else {
    systemColorDepth = 32;
    systemRedShift = 19;
    systemGreenShift = 11;
    systemBlueShift = 3;

    Init_2xSaI(32);    
  }
  fsColorDepth = systemColorDepth;
  if(systemColorDepth == 24)
    filterFunction = NULL;
#ifdef MMX
  if(!disableMMX)
    cpu_mmx = detectMMX();
  else
    cpu_mmx = 0;
#endif
  
  switch(systemColorDepth) {
  case 16:
    {
      for(int i = 0; i < 0x10000; i++) {
        systemColorMap16[i] = ((i & 0x1f) << systemRedShift) |
          (((i & 0x3e0) >> 5) << systemGreenShift) |
          (((i & 0x7c00) >> 10) << systemBlueShift);
      }
    }
    break;
  case 24:
  case 32:
    {
      for(int i = 0; i < 0x10000; i++) {
        systemColorMap32[i] = ((i & 0x1f) << systemRedShift) |
          (((i & 0x3e0) >> 5) << systemGreenShift) |
          (((i & 0x7c00) >> 10) << systemBlueShift);
      }      
    }
    break;
  }
  for(int i = 0; i < 24;) {
    systemGbPalette[i++] = (0x1f) | (0x1f << 5) | (0x1f << 10);
    systemGbPalette[i++] = (0x15) | (0x15 << 5) | (0x15 << 10);
    systemGbPalette[i++] = (0x0c) | (0x0c << 5) | (0x0c << 10);
    systemGbPalette[i++] = 0;
  }
  updateFilter();
  updateIFB();
  
  DragAcceptFiles(hWindow, TRUE);
  
  return TRUE;  
}

void GDIDisplay::clear()
{
}

void GDIDisplay::renderMenu()
{
  checkFullScreen();
  DrawMenuBar(hWindow);
}

void GDIDisplay::checkFullScreen()
{
}

void GDIDisplay::render()
{ 
  BITMAPINFO *bi = (BITMAPINFO *)info;
  bi->bmiHeader.biWidth = filterWidth+1;
  bi->bmiHeader.biHeight = -filterHeight;

  int pitch = filterWidth * 2 + 4;
  if(systemColorDepth == 24)
    pitch = filterWidth * 3;
  else if(systemColorDepth == 32)
    pitch = filterWidth * 4 + 4;
  
  if(filterFunction) {
    bi->bmiHeader.biWidth = filterWidth * 2;
    bi->bmiHeader.biHeight = -filterHeight * 2;
    
    if(systemColorDepth == 16)
      (*filterFunction)(pix+pitch,
                        pitch,
                        (u8*)delta,
                        (u8*)filterData,
                        filterWidth*2*2,
                        filterWidth,
                        filterHeight);
    else
      (*filterFunction)(pix+pitch,
                        pitch,
                        (u8*)delta,
                        (u8*)filterData,
                        filterWidth*4*2,
                        filterWidth,
                        filterHeight);
  }

  if(videoOption > VIDEO_4X && showSpeed) {
    char buffer[30];
    if(showSpeed == 1)
      sprintf(buffer, "%3d%%", systemSpeed);
    else
      sprintf(buffer, "%3d%%(%d, %d fps)", systemSpeed,
              systemFrameSkip,
              showRenderedFrames);

    if(filterFunction) {
      int p = filterWidth * 4;
      if(systemColorDepth == 24)
        p = filterWidth * 6;
      else if(systemColorDepth == 32)
        p = filterWidth * 8;
      if(showSpeedTransparent)
        fontDisplayStringTransp((u8*)filterData,
                                p,
                                10,
                                filterHeight*2-10,
                                buffer);
      else
        fontDisplayString((u8*)filterData,
                          p,
                          10,
                          filterHeight-10,
                          buffer);      
    } else {
      if(showSpeedTransparent)
        fontDisplayStringTransp((u8*)pix,
                                pitch,
                                10,
                                filterHeight-10,
                                buffer);
      else
        fontDisplayString((u8*)pix,
                          pitch,
                          10,
                          filterHeight-10,
                          buffer);
    }
  }

  POINT p;
  p.x = dest.left;
  p.y = dest.top;
  ScreenToClient(hWindow, &p);
  POINT p2;
  p2.x = dest.right;
  p2.y = dest.bottom;
  ScreenToClient(hWindow, &p2);
  
  HDC dc = GetDC(hWindow);

  StretchDIBits(dc,
                p.x,
                p.y,
                p2.x - p.x,
                p2.y - p.y,
                0,
                0,
                rect.right,
                rect.bottom,
                filterFunction ? filterData : pix+pitch,
                bi,
                DIB_RGB_COLORS,
                SRCCOPY);

  if(screenMessage) {
    if(((GetTickCount() - screenMessageTime) < 3000) &&
       !disableStatusMessage) {
      SetTextColor(dc, RGB(255,0,0));
      SetBkMode(dc,TRANSPARENT);
      TextOut(dc, p.x+10, p2.y - 20, screenMessageBuffer,
              strlen(screenMessageBuffer));
    } else {
      screenMessage = false;
    }
  }
  
  ReleaseDC(hWindow, dc);
}

int GDIDisplay::selectFullScreenMode(GUID **)
{
  HWND wnd = GetDesktopWindow();
  RECT r;
  GetWindowRect(wnd, &r);
  int w = (r.right - r.left) & 4095;
  int h = (r.bottom - r.top) & 4095;
  HDC dc = GetDC(wnd);
  int c = GetDeviceCaps(dc, BITSPIXEL);
  ReleaseDC(wnd, dc);

  return (c << 24) | (w << 12) | h;
}

IDisplay *newGDIDisplay()
{
  return new GDIDisplay();
}

