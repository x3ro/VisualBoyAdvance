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
#include <windows.h>
#include <gl/GL.h>
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
extern int glType;

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

class OpenGLDisplay : public IDisplay {
private:
  HDC hDC;
  HGLRC hglrc;
  GLuint texture;
  int width;
  int height;
  float size;
  u8 *filterData;
  bool failed;
  
  bool initializeTexture(int w, int h);
  void updateFiltering(int);
public:
  OpenGLDisplay();
  virtual ~OpenGLDisplay();

  virtual bool initialize();
  virtual void cleanup();
  virtual void render();
  virtual void checkFullScreen();
  virtual void renderMenu();
  virtual void clear();
  virtual bool changeRenderSize(int w, int h);
  virtual void resize(int w, int h);
  virtual DISPLAY_TYPE getType() { return OPENGL; };
  virtual void setOption(const char *, int);
  virtual int selectFullScreenMode(GUID **);  
};

OpenGLDisplay::OpenGLDisplay()
{
  hDC = NULL;
  hglrc = NULL;
  texture = 0;
  width = 0;
  height = 0;
  size = 0.0f;
  filterData = (u8 *)malloc(4*4*256*240);
  failed = false;
}

OpenGLDisplay::~OpenGLDisplay()
{
  cleanup();
}

void OpenGLDisplay::cleanup()
{
  if(texture != 0) {
    glDeleteTextures(1, &texture);
    texture = 0;
  }
  if(hglrc != NULL) {
    wglDeleteContext(hglrc);
    wglMakeCurrent(NULL, NULL);
    hglrc = NULL;
  }
  if(hDC != NULL) {
    ReleaseDC(hWindow, hDC);
    hDC = NULL;
  }
  if(filterData) {
    free(filterData);
    filterData = NULL;
  }
  width = 0;
  height = 0;
  size = 0.0f;
}

bool OpenGLDisplay::initialize()
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
      RECT r;
      GetWindowRect(GetDesktopWindow(), &r);
      fsWidth = r.right - r.left;
      fsHeight = r.bottom - r.top;
      
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

  hDC = GetDC(hWindow);
 
  PIXELFORMATDESCRIPTOR pfd = { 
    sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd 
    1,                     // version number 
    PFD_DRAW_TO_WINDOW |   // support window 
    PFD_SUPPORT_OPENGL |   // support OpenGL 
    PFD_DOUBLEBUFFER,      // double buffered 
    PFD_TYPE_RGBA,         // RGBA type 
    16,                    // 16-bit color depth 
    0, 0, 0, 0, 0, 0,      // color bits ignored 
    0,                     // no alpha buffer 
    0,                     // shift bit ignored 
    0,                     // no accumulation buffer 
    0, 0, 0, 0,            // accum bits ignored 
    32,                    // 32-bit z-buffer     
    0,                     // no stencil buffer 
    0,                     // no auxiliary buffer 
    PFD_MAIN_PLANE,        // main layer 
    0,                     // reserved 
    0, 0, 0                // layer masks ignored 
  }; 
  int  iPixelFormat; 
  
  if(!(iPixelFormat = ChoosePixelFormat(hDC, &pfd))) {
    winlog("Failed ChoosePixelFormat\n");
    return false;
  }

  // obtain detailed information about 
  // the device context's first pixel format
  if(!(DescribePixelFormat(hDC, iPixelFormat,  
                           sizeof(PIXELFORMATDESCRIPTOR), &pfd))) {
    winlog("Failed DescribePixelFormat\n");
    return false;
  }

  if(!SetPixelFormat(hDC, iPixelFormat, &pfd)) {
    winlog("Failed SetPixelFormat\n");
    return false;
  }

  if(!(hglrc = wglCreateContext(hDC))) {
    winlog("Failed wglCreateContext\n");
    return false;
  }

  if(!wglMakeCurrent(hDC, hglrc)) {
    winlog("Failed wglMakeCurrent\n");
    return false;
  }

  // setup 2D gl environment
  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);

  glViewport(0, 0, surfaceSizeX, surfaceSizeY);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho(0.0, (GLdouble)(surfaceSizeX), (GLdouble)(surfaceSizeY),
          0.0, 0.0,1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  systemRedShift = 3;
  systemGreenShift = 11;
  systemBlueShift = 19;
  systemColorDepth = 32;
  fsColorDepth = 32;
  
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
  updateFilter();
  updateIFB();

  if(failed)
    return false;
  
  DragAcceptFiles(hWindow, TRUE);
  
  return TRUE;  
}

void OpenGLDisplay::clear()
{
}

void OpenGLDisplay::renderMenu()
{
  checkFullScreen();
  DrawMenuBar(hWindow);
}

void OpenGLDisplay::checkFullScreen()
{
  //  if(tripleBuffering)
  //    pOpenGL->FlipToGDISurface();
}

void OpenGLDisplay::render()
{
  int pitch = filterWidth * 4 + 4;
  u8 *data = pix + (sizeX+1)*4;
  
  if(filterFunction) {
    data = filterData;
    (*filterFunction)(pix+pitch,
                      pitch,
                      (u8*)delta,
                      (u8*)filterData,
                      filterWidth*4*2,
                      filterWidth,
                      filterHeight);
  }
  
  // Texturemap complete texture to surface so we have free scaling 
  // and antialiasing
  int mult = 1;
  if(filterFunction) {
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 2*sizeX);
    mult = 2;
  } else {
    glPixelStorei(GL_UNPACK_ROW_LENGTH, sizeX+1);
  }
  glTexSubImage2D( GL_TEXTURE_2D,0,
                   0,0, mult*sizeX,mult*sizeY,
                   GL_RGBA,GL_UNSIGNED_BYTE,data);

  if(glType == 0) {
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0.0, 0.0); glVertex3i(0, 0, 0);
    glTexCoord2f(mult*sizeX/size, 0.0); glVertex3i(surfaceSizeX, 0, 0);
    glTexCoord2f(0.0, mult*sizeY/size); glVertex3i(0, surfaceSizeY, 0);
    glTexCoord2f(mult*sizeX/size, mult*sizeY/size); glVertex3i(surfaceSizeX, surfaceSizeY, 0);
    glEnd();
  } else {
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex3i(0, 0, 0);
    glTexCoord2f(mult*sizeX/size, 0.0); glVertex3i(surfaceSizeX, 0, 0);
    glTexCoord2f(mult*sizeX/size, mult*sizeY/size); glVertex3i(surfaceSizeX, surfaceSizeY, 0);
    glTexCoord2f(0.0, mult*sizeY/size); glVertex3i(0, surfaceSizeY, 0);
    glEnd();
  }
  if(screenMessage) {
    if(((GetTickCount() - screenMessageTime) < 3000) &&
       !disableStatusMessage) {
      SetTextColor(hDC, RGB(255,0,0));
      SetBkMode(hDC,TRANSPARENT);
      TextOut(hDC, 10, surfaceSizeY - 20, screenMessageBuffer,
              strlen(screenMessageBuffer));
    } else {
      screenMessage = false;
    }
  }  
  
  SwapBuffers(hDC);
}

void OpenGLDisplay::resize(int w, int h)
{
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho(0.0, (GLdouble)(w), (GLdouble)(h),
          0.0, 0.0,1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void OpenGLDisplay::updateFiltering(int value)
{
  switch(value) {
  case 0:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    break;
  case 1:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    break;
  }
}

bool OpenGLDisplay::initializeTexture(int w, int h)
{
  int mySize = 256;
  size = 256.0f;
  if(w > 255 || h > 255) {
    size = 512.0f;
    mySize = 512;
  }
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  int filter = regQueryDwordValue("glFilter", 0);
  if(filter < 0 || filter > 1)
    filter = 0;
  updateFiltering(filter);
  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mySize, mySize, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL );
  width = w;
  height = h;

  return true;
}

bool OpenGLDisplay::changeRenderSize(int w, int h)
{
  if(width != w || height != h) {
    if(texture != 0) {
      glDeleteTextures(1, &texture);
      texture = 0;
    }
    if(!initializeTexture(w, h)) {
      failed = true;
      return false;
    }
  }
  return true;
}

void OpenGLDisplay::setOption(const char *option, int value)
{
  if(!strcmp(option, "glFilter"))
    updateFiltering(value);
}

int OpenGLDisplay::selectFullScreenMode(GUID **)
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

IDisplay *newOpenGLDisplay()
{
  return new OpenGLDisplay();
}

