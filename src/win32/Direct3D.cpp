// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2004 Forgotten and the VBA development team

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or(at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include "stdafx.h"
#include "vba.h"

#define DIRECT3D_VERSION 0x0800
#include <d3d8.h>
#include <d3dx8.h>

#include "MainWnd.h"

#include "../System.h"
#include "../GBA.h"
#include "../Globals.h"
#include "../Text.h"
#include "../gb/gbGlobals.h"

#include "Reg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef MMX
extern "C" bool cpu_mmx;

extern bool detectMMX();
#endif

extern int Init_2xSaI(u32);
extern void winlog(const char *,...);
extern int systemSpeed;

typedef struct _D3DTLVERTEX {
  float sx; /* Screen coordinates */
  float sy;
  float sz;
  float rhw; /* Reciprocal of homogeneous w */
  D3DCOLOR color; /* Vertex color */
  float tu; /* Texture coordinates */
  float tv;
  _D3DTLVERTEX() { }
  _D3DTLVERTEX(const D3DVECTOR& v, float _rhw,
               D3DCOLOR _color, 
               float _tu, float _tv)
  { sx = v.x; sy = v.y; sz = v.z; rhw = _rhw;
  color = _color; 
  tu = _tu; tv = _tv;
  }
} D3DTLVERTEX, *LPD3DTLVERTEX;
 
#define D3DFVF_TLVERTEX D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1

class Direct3DDisplay : public IDisplay {
private:
  HINSTANCE             d3dDLL;
  LPDIRECT3D8           pD3D;
  LPDIRECT3DDEVICE8     pDevice;
  LPDIRECT3DTEXTURE8    pTexture;
  D3DSURFACE_DESC       dsdBackBuffer;
  D3DPRESENT_PARAMETERS dpp;
  D3DFORMAT             screenFormat;
  int                   width;
  int                   height;
  bool                  filterDisabled;
  ID3DXFont             *pFont;
  bool                  failed;
  
  void restoreDeviceObjects();
  void invalidateDeviceObjects();
  bool initializeOffscreen(int w, int h);
  void updateFiltering(int);
public:
  Direct3DDisplay();
  virtual ~Direct3DDisplay();

  virtual bool initialize();
  virtual void cleanup();
  virtual void render();
  virtual void checkFullScreen();
  virtual void renderMenu();
  virtual void clear();
  virtual bool changeRenderSize(int w, int h);
  virtual void resize(int w, int h);
  virtual DISPLAY_TYPE getType() { return DIRECT_3D; };
  virtual void setOption(const char *, int);
  virtual int selectFullScreenMode(GUID **);  
};

Direct3DDisplay::Direct3DDisplay()
{
  d3dDLL = NULL;
  pD3D = NULL;  
  pDevice = NULL;
  pTexture = NULL;
  pFont = NULL;
  screenFormat = D3DFMT_R5G6B5;
  width = 0;
  height = 0;
  filterDisabled = false;
  failed = false;
}

Direct3DDisplay::~Direct3DDisplay()
{
  cleanup();
}

void Direct3DDisplay::cleanup()
{
  if(pD3D != NULL) {
    if(pFont) {
      pFont->Release();
      pFont = NULL;
    }
    
    if(pTexture) {
      pTexture->Release();
      pTexture = NULL;
    }
    
    if(pDevice) {
      pDevice->Release();
      pDevice = NULL;
    }
    
    pD3D->Release();
    pD3D = NULL;

    if(d3dDLL != NULL) {
      FreeLibrary(d3dDLL);
      d3dDLL = NULL;
    }
  }
}

bool Direct3DDisplay::initialize()
{
  theApp.sizeX = 240;
  theApp.sizeY = 160;

  switch(theApp.videoOption) {
  case VIDEO_1X:
    theApp.surfaceSizeX = theApp.sizeX;
    theApp.surfaceSizeY = theApp.sizeY;
    break;
  case VIDEO_2X:
    theApp.surfaceSizeX = theApp.sizeX * 2;
    theApp.surfaceSizeY = theApp.sizeY * 2;
    break;
  case VIDEO_3X:
    theApp.surfaceSizeX = theApp.sizeX * 3;
    theApp.surfaceSizeY = theApp.sizeY * 3;
    break;
  case VIDEO_4X:
    theApp.surfaceSizeX = theApp.sizeX * 4;
    theApp.surfaceSizeY = theApp.sizeY * 4;
    break;
  case VIDEO_320x240:
  case VIDEO_640x480:
  case VIDEO_800x600:
  case VIDEO_OTHER:
    {
      RECT r;
      ::GetWindowRect(GetDesktopWindow(), &r);
      theApp.fsWidth = r.right - r.left;
      theApp.fsHeight = r.bottom - r.top;

      /* Need to fix this code later. For now, Fullscreen takes the whole
         screen.
         int scaleX = (fsWidth / sizeX);
         int scaleY = (fsHeight / sizeY);
         int min = scaleX < scaleY ? scaleX : scaleY;
         surfaceSizeX = sizeX * min;
         surfaceSizeY = sizeY * min;
         if(fullScreenStretch) {
      */
      theApp.surfaceSizeX = theApp.fsWidth;
      theApp.surfaceSizeY = theApp.fsHeight;
      //      }
    }
    break;
  }
  
  theApp.rect.left = 0;
  theApp.rect.top = 0;
  theApp.rect.right = theApp.sizeX;
  theApp.rect.bottom = theApp.sizeY;

  theApp.dest.left = 0;
  theApp.dest.top = 0;
  theApp.dest.right = theApp.surfaceSizeX;
  theApp.dest.bottom = theApp.surfaceSizeY;

  DWORD style = WS_POPUP | WS_VISIBLE;
  DWORD styleEx = 0;
  
  if(theApp.videoOption <= VIDEO_4X)
    style |= WS_OVERLAPPEDWINDOW;
  else
    styleEx = 0;

  if(theApp.videoOption <= VIDEO_4X)
    AdjustWindowRectEx(&theApp.dest, style, TRUE, styleEx);
  else
    AdjustWindowRectEx(&theApp.dest, style, FALSE, styleEx);    

  int winSizeX = theApp.dest.right-theApp.dest.left;
  int winSizeY = theApp.dest.bottom-theApp.dest.top;

  if(theApp.videoOption > VIDEO_4X) {
    winSizeX = theApp.fsWidth;
    winSizeY = theApp.fsHeight;
  }
  
  int x = 0;
  int y = 0;

  if(theApp.videoOption <= VIDEO_4X) {
    x = theApp.windowPositionX;
    y = theApp.windowPositionY;
  }
  
  // Create a window
  MainWnd *pWnd = new MainWnd;
  theApp.m_pMainWnd = pWnd;

  pWnd->CreateEx(styleEx,
                 theApp.wndClass,
                 "VisualBoyAdvance",
                 style,
                 x,y,winSizeX,winSizeY,
                 NULL,
                 0);
  
  if (!(HWND)*pWnd) {
    winlog("Error creating Window %08x\n", GetLastError());
    return FALSE;
  }
  
  theApp.updateMenuBar();
  
  theApp.adjustDestRect();
  
  d3dDLL = LoadLibrary("D3D8.DLL");
  LPDIRECT3D8 (WINAPI *D3DCreate)(UINT);
  if(d3dDLL != NULL) {    
    D3DCreate = (LPDIRECT3D8 (WINAPI *)(UINT))
      GetProcAddress(d3dDLL, "Direct3DCreate8");

    if(D3DCreate == NULL) {
      theApp.directXMessage("Direct3DCreate8");
      return FALSE;
    }
  } else {
    theApp.directXMessage("D3D8.DLL");
    return FALSE;
  }

  pD3D = D3DCreate(120);
    
  if(pD3D == NULL) {
    winlog("Error creating Direct3D object\n");
    return FALSE;
  }
  
  theApp.mode320Available = FALSE;
  theApp.mode640Available = FALSE;
  theApp.mode800Available = FALSE;

  D3DDISPLAYMODE mode;
  pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
  
  switch(mode.Format) {
  case D3DFMT_R8G8B8:
    systemColorDepth = 24;
    systemRedShift = 19;
    systemGreenShift = 11;
    systemBlueShift = 3;
    break;
  case D3DFMT_X8R8G8B8:
    systemColorDepth = 32;
    systemRedShift = 19;
    systemGreenShift = 11;
    systemBlueShift = 3;
    Init_2xSaI(32);
    break;
  case D3DFMT_R5G6B5:
    systemColorDepth = 16;
    systemRedShift = 11;
    systemGreenShift = 6;
    systemBlueShift = 0;    
    Init_2xSaI(565);
    break;
  case D3DFMT_X1R5G5B5:
    systemColorDepth = 16;
    systemRedShift = 10;
    systemGreenShift = 5;
    systemBlueShift = 0;
    Init_2xSaI(555);
    break;
  default:
    systemMessage(0,"Unsupport D3D format %d", mode.Format);
    return false;
  }
  theApp.fsColorDepth = systemColorDepth;

#ifdef MMX
  if(!theApp.disableMMX)
    cpu_mmx = theApp.detectMMX();
  else
    cpu_mmx = 0;
#endif
  
  screenFormat = mode.Format;

  // check for available fullscreen modes
  ZeroMemory(&dpp, sizeof(dpp));
  dpp.Windowed = TRUE;
  dpp.BackBufferFormat = mode.Format;
  dpp.BackBufferCount = 1;
  dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
  dpp.BackBufferWidth = theApp.surfaceSizeX;
  dpp.BackBufferHeight = theApp.surfaceSizeY;
  
  HRESULT hret = pD3D->CreateDevice(D3DADAPTER_DEFAULT,
                                    D3DDEVTYPE_HAL,
                                    pWnd->GetSafeHwnd(),
                                    D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                    &dpp,
                                    &pDevice);
  if(!SUCCEEDED(hret)) {
    winlog("Error creating Direct3DDevice %08x\n", hret);
    return false;
  }
  
  restoreDeviceObjects();

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
  
  theApp.updateFilter();
  theApp.updateIFB();

  if(failed)
    return false;
  
  pWnd->DragAcceptFiles(TRUE);
  
  return TRUE;  
}

bool Direct3DDisplay::initializeOffscreen(int w, int h)
{
  int size = 256;
  if(w > 256 || h > 256)
    size = 512;

  UINT ww = size;
  UINT hh = size;
  D3DFORMAT format = screenFormat;
  
  if(SUCCEEDED(D3DXCheckTextureRequirements(pDevice,
                                            &ww,
                                            &hh,
                                            NULL,
                                            0,
                                            &format,
                                            D3DPOOL_MANAGED))) {
    if((int)ww < w || (int)hh < h) {
      if(theApp.filterFunction) {
        filterDisabled = true;
        theApp.filterFunction = NULL;
        systemMessage(0, "3D card cannot support needed texture size for filter function. Disabling it");
      }
    } else
      filterDisabled = false; 
    if(SUCCEEDED(D3DXCreateTexture(pDevice,
                                   ww,
                                   hh,
                                   0,
                                   0,
                                   format,
                                   D3DPOOL_MANAGED,
                                   &pTexture))) {
      width = w;
      height = h;
      return true;
    }
  }
  return false;
}

void Direct3DDisplay::updateFiltering(int filter)
{
  switch(filter) {
  default:
  case 0:
    // point filtering
    pDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT );
    pDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT );
    pDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_POINT );
    break;
  case 1:
    // bilinear
    pDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    pDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    pDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_POINT );
    break;
  }
}

void Direct3DDisplay::restoreDeviceObjects()
{
  // Store render target surface desc
  LPDIRECT3DSURFACE8 pBackBuffer;
  HRESULT hr;
  if(SUCCEEDED(hr = pDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer ))) {
    pBackBuffer->GetDesc( &dsdBackBuffer );
    pBackBuffer->Release();
  } else
    systemMessage(0, "Failed GetBackBuffer %08x", hr);
  
  // Set up the texture 
  pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
  pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
  pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

  int filter = regQueryDwordValue("d3dFilter", 0);
  if(filter < 0 || filter > 3)
    filter = 0;
  updateFiltering(filter);

  // Set miscellaneous render states
  pDevice->SetRenderState( D3DRS_DITHERENABLE,   TRUE );
  pDevice->SetRenderState( D3DRS_ZENABLE,        FALSE );

  // Set the projection matrix
  D3DXMATRIX matProj;
  FLOAT fAspect = ((FLOAT)dsdBackBuffer.Width) / dsdBackBuffer.Height;
  D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, fAspect, 1.0f, 100.0f );
  pDevice->SetTransform( D3DTS_PROJECTION, &matProj );

  // turn off lighting
  pDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

  if(pFont) {
    pFont->Release();
    pFont = NULL;
  }
  // Create a D3D font using D3DX
  HFONT hFont = CreateFont( 14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                            ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            ANTIALIASED_QUALITY, FF_DONTCARE, "Arial" );      
  D3DXCreateFont( pDevice, hFont, &pFont );
}

void Direct3DDisplay::clear()
{
}

void Direct3DDisplay::renderMenu()
{
  checkFullScreen();
  if(theApp.m_pMainWnd)
    theApp.m_pMainWnd->DrawMenuBar();
}

void Direct3DDisplay::checkFullScreen()
{
  //  if(tripleBuffering)
  //    pDirect3D->FlipToGDISurface();
}

static void BlitRect(LPDIRECT3DDEVICE8 lpDevice,
                     LPDIRECT3DTEXTURE8 lpSrc,
                     float left, float top,
                     float right, float bottom,
                     D3DCOLOR col,float z)
{
  // calculate rhw

  float rhw=1.0f/(z*990.0f+10.0f);

  // set up rectangle

  D3DTLVERTEX verts[4];
  verts[0]=D3DTLVERTEX(D3DXVECTOR3(left-0.5f,  top-0.5f,    z),rhw,col,0.0f,0.0f); 
  verts[1]=D3DTLVERTEX(D3DXVECTOR3(right-0.5f, top-0.5f,    z),rhw,col,1.0f,0.0f);
  verts[2]=D3DTLVERTEX(D3DXVECTOR3(right-0.5f, bottom-0.5f, z),rhw,col,1.0f,1.0f); 
  verts[3]=D3DTLVERTEX(D3DXVECTOR3(left-0.5f,  bottom-0.5f, z),rhw,col,0.0f,1.0f);

  // set the texture

  lpDevice->SetTexture(0,lpSrc);

  // configure shader for vertex type

  lpDevice->SetVertexShader(D3DFVF_TLVERTEX);

  // draw the rectangle

  lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN,2,verts,sizeof(D3DTLVERTEX));
}

void Direct3DDisplay::render()
{
  if(!pDevice)
    return;
  
  // Test the cooperative level to see if it's okay to render
  if( FAILED( pDevice->TestCooperativeLevel() ) )
    {
      return;
    }
  pDevice->Clear( 0L, NULL, D3DCLEAR_TARGET, 0x000000ff, 1.0f, 0L );

  if(SUCCEEDED(pDevice->BeginScene())) {
    D3DLOCKED_RECT locked;
    if(pTexture && SUCCEEDED(pTexture->LockRect(0, &locked, NULL, 0))) {
      if(theApp.filterFunction) {
        if(systemColorDepth == 16)
          theApp.filterFunction(pix+theApp.filterWidth*2+4,
                                theApp.filterWidth*2+4,
                                (u8*)theApp.delta,
                                (u8*)locked.pBits,
                                locked.Pitch,
                                theApp.filterWidth,
                                theApp.filterHeight);
        else
          theApp.filterFunction(pix+theApp.filterWidth*4+4,
                                theApp.filterWidth*4+4,
                                (u8*)theApp.delta,
                                (u8*)locked.pBits,
                                locked.Pitch,
                                theApp.filterWidth,
                                theApp.filterHeight);
      } else {
        int copyX = 240;
        int copyY = 160;
        
        if(theApp.cartridgeType == 1) {
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
          mov edi, locked.pBits;
          mov edx, locked.Pitch;
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

      if(theApp.videoOption > VIDEO_4X && theApp.showSpeed) {
        char buffer[30];
        if(theApp.showSpeed == 1)
          sprintf(buffer, "%3d%%", systemSpeed);
        else
          sprintf(buffer, "%3d%%(%d, %d fps)", systemSpeed,
                  systemFrameSkip,
                  theApp.showRenderedFrames);
        if(theApp.showSpeedTransparent)
          drawTextTransp((u8*)locked.pBits,
                         locked.Pitch,
                         theApp.rect.left+10,
                         theApp.rect.bottom-10,
                         buffer);
        else
          drawText((u8*)locked.pBits,
                   locked.Pitch,
                   theApp.rect.left+10,
                   theApp.rect.bottom-10,
                   buffer);
      }
      
      pTexture->UnlockRect(0);

      float scaleX = (float)theApp.surfaceSizeX / theApp.sizeX;
      float scaleY = (float)theApp.surfaceSizeY / theApp.sizeY;
      BlitRect(pDevice, pTexture, 0, 0, scaleX*256, scaleY*256, 0xffffff, 0.1f);
    }
    
    if(theApp.screenMessage) {
      if(((GetTickCount() - theApp.screenMessageTime) < 3000) &&
         !theApp.disableStatusMessage && pFont) {
        D3DCOLOR color = D3DCOLOR_ARGB(255, 255, 0, 0);
        pFont->Begin();
        RECT r;
        r.left = 10;
        r.top = dpp.BackBufferHeight - 20;
        r.right = dpp.BackBufferWidth - 10;
        r.bottom = r.top + 20;

        pFont->DrawText(theApp.screenMessageBuffer, -1, &r, 0, color);
        pFont->End();
      } else {
        theApp.screenMessage = false;
      }
    }
    
    pDevice->EndScene();

    pDevice->Present( NULL, NULL, NULL, NULL );    
  }
}

void Direct3DDisplay::invalidateDeviceObjects()
{
  if(pFont)
    pFont->Release();
  pFont = NULL;
}

void Direct3DDisplay::resize(int w, int h)
{
  if(pDevice) {
    dpp.BackBufferWidth = w;
    dpp.BackBufferHeight = h;
    HRESULT hr;
    invalidateDeviceObjects();
    if(SUCCEEDED(hr = pDevice->Reset(&dpp))) {
      restoreDeviceObjects();
    } else
      systemMessage(0, "Failed device reset %08x", hr);
  }
}

bool Direct3DDisplay::changeRenderSize(int w, int h)
{
  if(w != width || h != height) {
    if(pTexture) {
      pTexture->Release();
      pTexture = NULL;
    }
    if(!initializeOffscreen(w, h)) {
      failed = true;
      return false;
    }
  }
  if(filterDisabled && theApp.filterFunction)
    theApp.filterFunction = NULL;

  return true;
}

void Direct3DDisplay::setOption(const char *option, int value)
{
  if(!strcmp(option, "d3dFilter"))
    updateFiltering(value);
}

int Direct3DDisplay::selectFullScreenMode(GUID **)
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

IDisplay *newDirect3DDisplay()
{
  return new Direct3DDisplay();
}

