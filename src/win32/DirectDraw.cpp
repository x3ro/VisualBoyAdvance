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
#define DIRECTDRAW_VERSION 0x0700
#include <ddraw.h>
#include <stdio.h>

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
extern int winVideoModeSelect(HWND, GUID **);

class DirectDrawDisplay : public IDisplay {
private:
  HINSTANCE            ddrawDLL;
  LPDIRECTDRAW7        pDirectDraw;
  LPDIRECTDRAWSURFACE7 ddsPrimary;
  LPDIRECTDRAWSURFACE7 ddsOffscreen;
  LPDIRECTDRAWSURFACE7 ddsFlip;
  LPDIRECTDRAWCLIPPER  ddsClipper;
  int                  width;
  int                  height;
  bool                 failed;

  bool initializeOffscreen(int w, int h);
public:
  DirectDrawDisplay();
  virtual ~DirectDrawDisplay();

  virtual bool initialize();
  virtual void cleanup();
  virtual void render();
  virtual void checkFullScreen();
  virtual void renderMenu();
  virtual void clear();
  virtual bool changeRenderSize(int w, int h);
  virtual DISPLAY_TYPE getType() { return DIRECT_DRAW; };
  virtual void setOption(const char *, int) {}
  virtual bool isSkinSupported() { return true; }
  virtual int selectFullScreenMode(GUID **);  
};

static HRESULT WINAPI checkModesAvailable(LPDDSURFACEDESC2 surf, LPVOID lpContext)
{
  if(surf->dwWidth == 320 &&
     surf->dwHeight == 240 &&
     surf->ddpfPixelFormat.dwRGBBitCount == 16) {
    mode320Available = TRUE;
  }
  if(surf->dwWidth == 640 &&
     surf->dwHeight == 480 &&
     surf->ddpfPixelFormat.dwRGBBitCount == 16) {
    mode640Available = TRUE;
  }
  if(surf->dwWidth == 800 &&
     surf->dwHeight == 600 &&
     surf->ddpfPixelFormat.dwRGBBitCount == 16) {
    mode800Available = TRUE;
  }
  return DDENUMRET_OK;
}

static int ffs(UINT mask)
{
  int m = 0;
  if (mask) {
    while (!(mask & (1 << m)))
      m++;
    
    return (m);
  }
  
  return (0);
}

DirectDrawDisplay::DirectDrawDisplay()
{
  pDirectDraw = NULL;
  ddsPrimary = NULL;
  ddsOffscreen = NULL;
  ddsFlip = NULL;
  ddsClipper = NULL;
  ddrawDLL = NULL;
  width = 0;
  height = 0;
  failed = false;
}

DirectDrawDisplay::~DirectDrawDisplay()
{
  cleanup();
}

void DirectDrawDisplay::cleanup()
{
  if(pDirectDraw != NULL) {
    if(ddsClipper != NULL) {
      ddsClipper->Release();
      ddsClipper = NULL;
    }

    if(ddsFlip != NULL) {
      ddsFlip->Release();
      ddsFlip = NULL;
    }

    if(ddsOffscreen != NULL) {
      ddsOffscreen->Release();
      ddsOffscreen = NULL;
    }
    
    if(ddsPrimary != NULL) {
      ddsPrimary->Release();
      ddsPrimary = NULL;
    }
    
    pDirectDraw->Release();
    pDirectDraw = NULL;
  }

  if(ddrawDLL != NULL) {
    FreeLibrary(ddrawDLL);
    ddrawDLL = NULL;
  }
  width = 0;
  height = 0;
}

bool DirectDrawDisplay::initialize()
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
  
  GUID *guid = NULL;
  if(ddrawEmulationOnly)
    guid = (GUID *)DDCREATE_EMULATIONONLY;

  if(pVideoDriverGUID)
    guid = pVideoDriverGUID;

  ddrawDLL = LoadLibrary("DDRAW.DLL");
  HRESULT (WINAPI *DDrawCreateEx)(GUID *,LPVOID *,REFIID,IUnknown *);  
  if(ddrawDLL != NULL) {    
    DDrawCreateEx = (HRESULT (WINAPI *)(GUID *,LPVOID *,REFIID,IUnknown *))
      GetProcAddress(ddrawDLL, "DirectDrawCreateEx");

    if(DDrawCreateEx == NULL) {
      directXMessage("DirectDrawCreateEx");
      return FALSE;
    }
  } else {
    directXMessage("DDRAW.DLL");
    return FALSE;
  }

  ddrawUsingEmulationOnly = ddrawEmulationOnly;
  
  HRESULT hret = DDrawCreateEx(guid,
                               (void **)&pDirectDraw,
                               IID_IDirectDraw7,
                               NULL);
    
  if(hret != DD_OK) {
    winlog("Error creating DirectDraw object %08x\n", hret);
    if(ddrawEmulationOnly) {
      // disable emulation only setting in case of failure
      regSetDwordValue("ddrawEmulationOnly", 0);
    }
    //    errorMessage(myLoadString(IDS_ERROR_DISP_DRAWCREATE), hret);
    return FALSE;
  }

  if(ddrawDebug) {
    DDCAPS driver;
    DDCAPS hel;
    ZeroMemory(&driver, sizeof(driver));
    ZeroMemory(&hel, sizeof(hel));
    driver.dwSize = sizeof(driver);
    hel.dwSize = sizeof(hel);
    pDirectDraw->GetCaps(&driver, &hel);
    int i;
    DWORD *p = (DWORD *)&driver;
    for(i = 0; i < (int)driver.dwSize; i+=4)
      winlog("Driver CAPS %2d: %08x\n", i>>2, *p++);
    p = (DWORD *)&hel;
    for(i = 0; i < (int)hel.dwSize; i+=4)
      winlog("HEL CAPS %2d: %08x\n", i>>2, *p++);
  }
  
  mode320Available = FALSE;
  mode640Available = FALSE;
  mode800Available = FALSE;
  // check for available fullscreen modes
  pDirectDraw->EnumDisplayModes(DDEDM_STANDARDVGAMODES, NULL, NULL,
                                checkModesAvailable);
  
  DWORD flags = DDSCL_NORMAL;

  if(videoOption >= VIDEO_320x240)
    flags = DDSCL_ALLOWMODEX |
      DDSCL_ALLOWREBOOT |
      DDSCL_EXCLUSIVE |
      DDSCL_FULLSCREEN;
  
  hret = pDirectDraw->SetCooperativeLevel(hWindow,  
                                          flags);

  if(hret != DD_OK) {
    winlog("Error SetCooperativeLevel %08x\n", hret);    
    //    errorMessage(myLoadString(IDS_ERROR_DISP_DRAWLEVEL), hret);
    return FALSE;
  }
  
  if(videoOption > VIDEO_4X) {
    hret = pDirectDraw->SetDisplayMode(fsWidth,
                                       fsHeight,
                                       fsColorDepth,
                                       0,
                                       0);
    if(hret != DD_OK) {
      winlog("Error SetDisplayMode %08x\n", hret);
      //      errorMessage(myLoadString(IDS_ERROR_DISP_DRAWSET), hret);
      return FALSE;
    }
  }
  
  DDSURFACEDESC2 ddsd;
  ZeroMemory(&ddsd,sizeof(ddsd));
  ddsd.dwSize = sizeof(ddsd);
  ddsd.dwFlags = DDSD_CAPS;
  ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
  if(videoOption > VIDEO_4X) {
    if(tripleBuffering) {
      // setup triple buffering
      ddsd.dwFlags |= DDSD_BACKBUFFERCOUNT;
      ddsd.ddsCaps.dwCaps |= DDSCAPS_COMPLEX | DDSCAPS_FLIP;
      ddsd.dwBackBufferCount = 2;
    }
  }
  
  hret = pDirectDraw->CreateSurface(&ddsd, &ddsPrimary, NULL);
  if(hret != DD_OK) {
    winlog("Error primary CreateSurface %08x\n", hret);    
    //    errorMessage(myLoadString(IDS_ERROR_DISP_DRAWSURFACE), hret);
    return FALSE;
  }

  if(ddrawDebug) {
    DDSCAPS2 caps;
    ZeroMemory(&caps, sizeof(caps));
    ddsPrimary->GetCaps(&caps);

    winlog("Primary CAPS 1: %08x\n", caps.dwCaps);
    winlog("Primary CAPS 2: %08x\n", caps.dwCaps2);
    winlog("Primary CAPS 3: %08x\n", caps.dwCaps3);
    winlog("Primary CAPS 4: %08x\n", caps.dwCaps4);
  }

  if(videoOption > VIDEO_4X && tripleBuffering) {
    DDSCAPS2 caps;
    ZeroMemory(&caps, sizeof(caps));
    // this gets the third surface. The front one is the primary,
    // the second is the backbuffer and the third is the flip
    // surface
    caps.dwCaps = DDSCAPS_BACKBUFFER;
    
    hret = ddsPrimary->GetAttachedSurface(&caps, &ddsFlip);
    if(hret != DD_OK) {
      winlog("Failed to get attached surface %08x", hret);
      return FALSE;
    }

    ddsFlip->AddRef();
    clear();
  }

  // create clipper in all modes to avoid paint problems
  //  if(videoOption <= VIDEO_4X) {
    hret = pDirectDraw->CreateClipper(0, &ddsClipper, NULL);
    if(hret == DD_OK) {
      ddsClipper->SetHWnd(0, hWindow);
      if(videoOption > VIDEO_4X) {
        if(tripleBuffering)
          ddsFlip->SetClipper(ddsClipper);
        else
          ddsPrimary->SetClipper(ddsClipper);
      } else
        ddsPrimary->SetClipper(ddsClipper);
    }
    //  }

  DDPIXELFORMAT px;

  px.dwSize = sizeof(px);

  hret = ddsPrimary->GetPixelFormat(&px);

  switch(px.dwRGBBitCount) {
  case 15:
  case 16:
    systemColorDepth = 16;
    break;
  case 24:
    systemColorDepth = 24;
    filterFunction = NULL;
    break;
  case 32:
    systemColorDepth = 32;
    break;
  default:
    systemMessage(IDS_ERROR_DISP_COLOR, "Unsupported display setting for color depth: %d bits. \nWindows desktop must be in either 16-bit, 24-bit or 32-bit mode for this program to work in window mode.",px.dwRGBBitCount);
    return FALSE;
  }
  updateFilter();
  updateIFB();

  if(failed)
    return false;

  DragAcceptFiles(hWindow, TRUE);
  
  return true;  
}

bool DirectDrawDisplay::changeRenderSize(int w, int h)
{
  if(w != width || h != height) {
    if(ddsOffscreen) {
      ddsOffscreen->Release();
      ddsOffscreen = NULL;
    }
    if(!initializeOffscreen(w, h)) {
      failed = true;
      return false;
    }
  }
  return true;
}

bool DirectDrawDisplay::initializeOffscreen(int w, int h)
{
  DDSURFACEDESC2 ddsd;
  
  ZeroMemory(&ddsd, sizeof(ddsd));
  ddsd.dwSize = sizeof(ddsd);
  ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
  ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
  if(ddrawUseVideoMemory)
    ddsd.ddsCaps.dwCaps |= (DDSCAPS_LOCALVIDMEM|DDSCAPS_VIDEOMEMORY);
  ddsd.dwWidth = w;
  ddsd.dwHeight = h;

  HRESULT hret = pDirectDraw->CreateSurface(&ddsd, &ddsOffscreen, NULL);

  if(hret != DD_OK) {
    winlog("Error offscreen CreateSurface %08x\n", hret);    
    if(ddrawUseVideoMemory) {
      regSetDwordValue("ddrawUseVideoMemory", 0);
    }    
    //    errorMessage(myLoadString(IDS_ERROR_DISP_DRAWSURFACE2), hret);
    return false;
  }

  if(ddrawDebug) {
    DDSCAPS2 caps;
    ZeroMemory(&caps, sizeof(caps));
    ddsOffscreen->GetCaps(&caps);

    winlog("Offscreen CAPS 1: %08x\n", caps.dwCaps);
    winlog("Offscreen CAPS 2: %08x\n", caps.dwCaps2);
    winlog("Offscreen CAPS 3: %08x\n", caps.dwCaps3);
    winlog("Offscreen CAPS 4: %08x\n", caps.dwCaps4);
  }
  
  DDPIXELFORMAT px;

  px.dwSize = sizeof(px);

  hret = ddsOffscreen->GetPixelFormat(&px);

  if(ddrawDebug) {
    DWORD *pdword = (DWORD *)&px;
    for(int ii = 0; ii < 8; ii++) {
      winlog("Pixel format %d %08x\n", ii, pdword[ii]);
    }
  }
  
  switch(px.dwRGBBitCount) {
  case 15:
  case 16:
    systemColorDepth = 16;
    break;
  case 24:
    systemColorDepth = 24;
    filterFunction = NULL;
    break;
  case 32:
    systemColorDepth = 32;
    break;
  default:
    systemMessage(IDS_ERROR_DISP_COLOR, "Unsupported display setting for color depth: %d bits. \nWindows desktop must be in either 16-bit, 24-bit or 32-bit mode for this program to work in window mode.",px.dwRGBBitCount);
    return FALSE;
  }
  if(ddrawDebug) {
    winlog("R Mask: %08x\n", px.dwRBitMask);
    winlog("G Mask: %08x\n", px.dwGBitMask);
    winlog("B Mask: %08x\n", px.dwBBitMask);
  }
  
  systemRedShift = ffs(px.dwRBitMask);
  systemGreenShift = ffs(px.dwGBitMask);
  systemBlueShift = ffs(px.dwBBitMask);

#ifdef MMX
  if(!disableMMX)
    cpu_mmx = detectMMX();
  else
    cpu_mmx = 0;
#endif
  
  if((px.dwFlags&DDPF_RGB) != 0 &&
     px.dwRBitMask == 0xF800 &&
     px.dwGBitMask == 0x07E0 &&
     px.dwBBitMask == 0x001F) {
    systemGreenShift++;
    Init_2xSaI(565);
    RGB_LOW_BITS_MASK=0x821;
  } else if((px.dwFlags&DDPF_RGB) != 0 &&
            px.dwRBitMask == 0x7C00 &&
            px.dwGBitMask == 0x03E0 &&
            px.dwBBitMask == 0x001F) {
    Init_2xSaI(555);
    RGB_LOW_BITS_MASK=0x421;
  } else if((px.dwFlags&DDPF_RGB) != 0 &&
            px.dwRBitMask == 0x001F &&
            px.dwGBitMask == 0x07E0 &&
            px.dwBBitMask == 0xF800) {
    systemGreenShift++;
    Init_2xSaI(565);
    RGB_LOW_BITS_MASK=0x821;
  } else if((px.dwFlags&DDPF_RGB) != 0 &&
            px.dwRBitMask == 0x001F &&
            px.dwGBitMask == 0x03E0 &&
            px.dwBBitMask == 0x7C00) {
    Init_2xSaI(555);
    RGB_LOW_BITS_MASK=0x421;
  } else {
    // 32-bit or 24-bit
    if(systemColorDepth == 32 || systemColorDepth == 24) {
      systemRedShift += 3;
      systemGreenShift += 3;
      systemBlueShift += 3;
      if(systemColorDepth == 32)
        Init_2xSaI(32);
    }
  }

  if(ddrawDebug) {
    winlog("R shift: %d\n", systemRedShift);
    winlog("G shift: %d\n", systemGreenShift);
    winlog("B shift: %d\n", systemBlueShift);
  }
  
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
  width = w;
  height = h;
  return true;
}

void DirectDrawDisplay::clear()
{
  if(videoOption <= VIDEO_4X || !tripleBuffering || ddsFlip == NULL)
    return;

  DDBLTFX fx;
  ZeroMemory(&fx, sizeof(fx));
  fx.dwSize = sizeof(fx);
  fx.dwFillColor = 0;
  ddsFlip->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &fx);
  ddsPrimary->Flip(NULL, 0);
  ddsFlip->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &fx);
  ddsPrimary->Flip(NULL, 0);
  ddsFlip->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &fx);
  ddsPrimary->Flip(NULL, 0);    
}

void DirectDrawDisplay::renderMenu()
{
  checkFullScreen();
  DrawMenuBar(hWindow);
}

void DirectDrawDisplay::checkFullScreen()
{
  if(tripleBuffering)
    pDirectDraw->FlipToGDISurface();
}

void DirectDrawDisplay::render()
{
  HRESULT hret;

  if(pDirectDraw == NULL ||
     ddsOffscreen == NULL ||
     ddsPrimary == NULL)
    return;

  if(vsync && !speedup) {
    hret = pDirectDraw->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);
  }
  
  DDSURFACEDESC2 ddsDesc;
  
  ZeroMemory(&ddsDesc, sizeof(ddsDesc));

  ddsDesc.dwSize = sizeof(ddsDesc);

  hret = ddsOffscreen->Lock(NULL,
                            &ddsDesc,
                            DDLOCK_WRITEONLY|
#ifndef FINAL_VERSION
                            DDLOCK_NOSYSLOCK|
#endif
                            DDLOCK_SURFACEMEMORYPTR,
                            NULL);

  if(hret == DDERR_SURFACELOST) {
    hret = ddsPrimary->Restore();
    if(hret == DD_OK) {
      hret = ddsOffscreen->Restore();
      
      if(hret == DD_OK) {
        hret = ddsOffscreen->Lock(NULL,
                                  &ddsDesc,
                                  DDLOCK_WRITEONLY|
#ifndef FINAL_VERSION
                                  DDLOCK_NOSYSLOCK|
#endif
                                  DDLOCK_SURFACEMEMORYPTR,
                                  NULL);
        
      }
    }
  }
    
  if(hret == DD_OK) {
    if(filterFunction) {
      if(systemColorDepth == 16)
        (*filterFunction)(pix+filterWidth*2+4,
                          filterWidth*2+4,
                          (u8*)delta,
                          (u8*)ddsDesc.lpSurface,
                          ddsDesc.lPitch,
                          filterWidth,
                          filterHeight);
      else
        (*filterFunction)(pix+filterWidth*4+4,
                          filterWidth*4+4,
                          (u8*)delta,
                          (u8*)ddsDesc.lpSurface,
                          ddsDesc.lPitch,
                          filterWidth,
                          filterHeight);
        
    } else {
      int copyX = 240;
      int copyY = 160;
      
      if(cartridgeType == 1) {
        if(gbBorderOn) {
          copyX = 256;
          copyY = 224;
        } else {
          copyX = 160;
          copyY = 144;
        }
      }
      // MMX doesn't seem to be faster to copy the data
      __asm {
        mov eax, copyX;
        mov ebx, copyY;
        
        mov esi, pix;
        mov edi, ddsDesc.lpSurface;
        mov edx, ddsDesc.lPitch;
        cmp systemColorDepth, 16;
        jnz gbaOtherColor;
        sub edx, eax;
        sub edx, eax;
        lea esi,[esi+2*eax+4];
        shr eax, 1;
      gbaLoop16bit:
        mov ecx, eax;
        repz movsd;
        inc esi;
        inc esi;
        inc esi;
        inc esi;
        add edi, edx;
        dec ebx;
        jnz gbaLoop16bit;
        jmp gbaLoopEnd;
      gbaOtherColor:
        cmp systemColorDepth, 32;
        jnz gbaOtherColor2;
        
        sub edx, eax;
        sub edx, eax;
        sub edx, eax;
        sub edx, eax;
        lea esi, [esi+4*eax+4];
      gbaLoop32bit:
        mov ecx, eax;
        repz movsd;
        add esi, 4;
        add edi, edx;
        dec ebx;
        jnz gbaLoop32bit;
        jmp gbaLoopEnd;
      gbaOtherColor2:
        lea eax, [eax+2*eax];
        sub edx, eax;
      gbaLoop24bit:
        mov ecx, eax;
        shr ecx, 2;
        repz movsd;
        add edi, edx;
        dec ebx;
        jnz gbaLoop24bit;
      gbaLoopEnd:
      }
    }
    if(videoOption > VIDEO_4X && showSpeed) {
      char buffer[30];
      if(showSpeed == 1)
        sprintf(buffer, "%3d%%", systemSpeed);
      else
        sprintf(buffer, "%3d%%(%d, %d fps)", systemSpeed,
                systemFrameSkip,
                showRenderedFrames);
      if(showSpeedTransparent)
        fontDisplayStringTransp((u8*)ddsDesc.lpSurface,
                                ddsDesc.lPitch,
                                rect.left+10,
                                rect.bottom-10,
                                buffer);
      else
        fontDisplayString((u8*)ddsDesc.lpSurface,
                          ddsDesc.lPitch,
                          rect.left+10,
                          rect.bottom-10,
                          buffer);        
    }
  } else if(ddrawDebug)
    winlog("Error during lock: %08x\n", hret);

  hret = ddsOffscreen->Unlock(NULL);

  if(hret == DD_OK) {
    ddsOffscreen->PageLock(0);
    if(tripleBuffering && videoOption > VIDEO_4X) {
      hret = ddsFlip->Blt(&dest, ddsOffscreen, NULL, DDBLT_WAIT, NULL);
      if(hret == DD_OK) {
        if(menuToggle || !active) {
          pDirectDraw->FlipToGDISurface();
          ddsPrimary->SetClipper(ddsClipper);
          hret = ddsPrimary->Blt(&dest, ddsFlip, NULL, DDBLT_ASYNC, NULL);
          // if using emulation only, then we have to redraw the menu
          // everytime. It seems like a bug in DirectDraw to me as we not
          // overwritting the menu area at all.
          if(ddrawUsingEmulationOnly)
            DrawMenuBar(hWindow);
        } else
          hret = ddsPrimary->Flip(NULL, 0);
      }
    } else {
      hret = ddsPrimary->Blt(&dest, ddsOffscreen, NULL,DDBLT_ASYNC,NULL);
      
      if(hret == DDERR_SURFACELOST) {
        hret = ddsPrimary->Restore();
        
        if(hret == DD_OK) {
          hret = ddsPrimary->Blt(&dest, ddsOffscreen, NULL, DDBLT_ASYNC, NULL);
        }
      }
    }
    ddsOffscreen->PageUnlock(0);
  } else if(ddrawDebug)
    winlog("Error during unlock: %08x\n", hret);

  if(screenMessage) {
    if(((GetTickCount() - screenMessageTime) < 3000) &&
       !disableStatusMessage) {
      ddsPrimary->SetClipper(ddsClipper);
      HDC hdc;
      ddsPrimary->GetDC(&hdc);
      SetTextColor(hdc, RGB(255,0,0));
      SetBkMode(hdc,TRANSPARENT);      
      TextOut(hdc, dest.left+10, dest.bottom - 20, screenMessageBuffer,
              strlen(screenMessageBuffer));
      ddsPrimary->ReleaseDC(hdc);
    } else {
      screenMessage = false;
    }
  }
  
  if(hret != DD_OK) {
    if(ddrawDebug)
      winlog("Error on update screen: %08x\n", hret);
  }  
}

int DirectDrawDisplay::selectFullScreenMode(GUID **pGUID)
{
  return winVideoModeSelect(hWindow, pGUID);
}

IDisplay *newDirectDrawDisplay()
{
  return new DirectDrawDisplay();
}

