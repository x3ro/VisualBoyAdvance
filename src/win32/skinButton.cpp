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
#include "skinButton.h"

extern bool winAccelGetID(const char *command, WORD& id);
extern int skinButtons;
extern HMENU menu;
extern HWND hWindow;

BEGIN_MESSAGE_MAP(SkinButton, Wnd)
  ON_WM_ERASEBKGND()
  ON_WM_CREATE()
  ON_WM_PAINT()
  ON_MESSAGE(WM_KILLFOCUS, OnKillFocus)
  ON_MESSAGE(WM_LBUTTONUP, OnLButtonUp)
  ON_MESSAGE(WM_LBUTTONDOWN, OnLButtonDown)
  ON_MESSAGE(WM_MOUSEMOVE, OnMouseMove)
  ON_MESSAGE(WM_CAPTURECHANGED, OnCaptureChanged)
  ON_MESSAGE(WM_CONTEXTMENU, OnRButtonDown)
  ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

SkinButton::SkinButton()
  : Wnd()
{
  normalBmp = NULL;
  downBmp = NULL;
  overBmp = NULL;
  mouseOver = false;
  id = "";
  idCommand = 0;
  region = NULL;
  buttonMask = 0;
  menu = -1;
}

SkinButton::~SkinButton()
{
  DestroyWindow();
  if(normalBmp) {
    DeleteObject(normalBmp);
    normalBmp = NULL;
  }
  if(downBmp) {
    DeleteObject(downBmp);
    downBmp = NULL;
  }
  if(overBmp) {
    DeleteObject(overBmp);
    overBmp = NULL;
  }
  if(region) {
    DeleteObject(region);
    region = NULL;
  }
}

void SkinButton::SetNormalBitmap(HBITMAP bmp)
{
  normalBmp = bmp;
}

void SkinButton::SetDownBitmap(HBITMAP bmp)
{
  downBmp = bmp;
}

void SkinButton::SetOverBitmap(HBITMAP bmp)
{
  overBmp = bmp;
}

void SkinButton::SetRect(const RECT& r)
{
  rect = r;
}

void SkinButton::SetId(const char *id)
{
  this->id = id;
  if(!winAccelGetID(id, idCommand)) {
    if(!strcmp(id, "A"))
      buttonMask = 1;
    else if(!strcmp("B", id))
      buttonMask = 2;
    else if(!strcmp("SEL", id))
      buttonMask = 4;
    else if(!strcmp("START", id))
      buttonMask = 8;
    else if(!strcmp("R", id))
      buttonMask = 16;
    else if(!strcmp("L", id))
      buttonMask = 32;
    else if(!strcmp("U", id))
      buttonMask = 64;
    else if(!strcmp("D", id))
      buttonMask = 128;
    else if(!strcmp("BR", id))
      buttonMask = 256;
    else if(!strcmp("BL", id))
      buttonMask = 512;
    else if(!strcmp("SPEED", id))
      buttonMask = 1024;
    else if(!strcmp("CAPTURE", id))
      buttonMask = 2048;
    else if(!strcmp("GS", id))
      buttonMask = 4096;
    else if(!strcmp("UR", id))
      buttonMask = 64+16;
    else if(!strcmp("UL", id))
      buttonMask = 64+32;
    else if(!strcmp("DR", id))
      buttonMask = 128+16;
    else if(!strcmp("DL", id))
      buttonMask = 128+32;
    else if(!strcmp("MENUFILE", id))
      menu = 0;
    else if(!strcmp("MENUOPTIONS", id))
      menu = 1;
    else if(!strcmp("MENUCHEATS", id))
      menu = 2;
    else if(!strcmp("MENUTOOLS", id))
      menu = 3;
    else if(!strcmp("MENUHELP", id))
      menu = 4;
  }
}

void SkinButton::SetRegion(HRGN rgn)
{
  region = rgn;
}

void SkinButton::GetRect(RECT& r)
{
  r = rect;
}

BOOL SkinButton::Create(const char *name, DWORD style, const RECT& r, 
			HWND parent, UINT id)
{
  return Wnd::Create("BUTTON",
		     name,
		     style|WS_CHILDWINDOW,
		     r,
		     parent,
		     id);
}

BOOL SkinButton::OnEraseBkgnd(HDC)
{
  return TRUE;
}

void SkinButton::OnPaint()
{
  PAINTSTRUCT ps;
  HDC hDC = BeginPaint(hWnd, &ps);
  HDC memDC = ::CreateCompatibleDC(hDC);
  UINT state = ::SendMessage(hWnd, BM_GETSTATE, 0, 0);
  
  if(state & BST_PUSHED)
    SelectObject(memDC, downBmp);
  else if(mouseOver && overBmp != NULL)
    SelectObject(memDC, overBmp);
  else
    SelectObject(memDC, normalBmp);
  SelectClipRgn(hDC, region);
  BitBlt(hDC, 0, 0, rect.right - rect.left,
	 rect.bottom - rect.top, memDC, 0, 0, SRCCOPY);
  SelectClipRgn(hDC, NULL);
  DeleteDC(memDC);

  EndPaint(hWnd, &ps);
}

LRESULT SkinButton::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
  if(idCommand != 0)
    return Wnd::Default(WM_LBUTTONDOWN, wParam, lParam);

  POINT pt;
  pt.x = GET_X_LPARAM(lParam);
  pt.y = GET_Y_LPARAM(lParam);
  RECT r;
  ::GetClientRect(getHandle(), &r);
  BOOL inside = PtInRect(&r, pt);
  if(region != NULL)
    inside &= PtInRegion(region, pt.x, pt.y);
  if(inside) {
    if(buttonMask)
      skinButtons = buttonMask;
    return Wnd::Default(WM_LBUTTONDOWN, wParam, lParam);
  }
  return SendMessage(GetParent(getHandle()), WM_LBUTTONDOWN, wParam, lParam);
}

LRESULT SkinButton::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
  POINT pt;
  pt.x = GET_X_LPARAM(lParam);
  pt.y = GET_Y_LPARAM(lParam);
  RECT r;
  ::GetClientRect(getHandle(), &r);
  BOOL inside = PtInRect(&r, pt);
  if(region != NULL)
    inside &= PtInRegion(region, pt.x, pt.y);
  if(inside) {
    if(idCommand != 0)
      ::SendMessage(::GetParent(getHandle()), WM_COMMAND, idCommand, 0);
    else if(buttonMask)
      skinButtons = 0;
    else if(menu != -1) {
      HMENU m = GetSubMenu(::menu, menu);
      pt.x = r.left;
      pt.y = r.bottom;
      ClientToScreen(getHandle(), &pt);
      SendMessage(hWindow, WM_INITMENUPOPUP, (WPARAM)m, menu);
      TrackPopupMenu(m, 0, pt.x, pt.y, 0, hWindow, NULL);
    }

    return Wnd::Default(WM_LBUTTONUP, wParam, lParam);
  }
  return SendMessage(::GetParent(getHandle()), WM_LBUTTONUP, wParam, lParam);
}

LRESULT SkinButton::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
  if(wParam * MK_LBUTTON && !mouseOver)
    return Wnd::Default(WM_MOUSEMOVE, wParam, lParam);

  if(GetCapture() != getHandle()) {
    SetCapture(getHandle());
  }
  POINT pt;
  pt.x = GET_X_LPARAM(lParam);
  pt.y = GET_Y_LPARAM(lParam);
  //  ClientToScreen(getHandle(), &p);
  RECT r;
  ::GetClientRect(getHandle(), &r);
  BOOL inside = PtInRect(&r, pt);
  if(region != NULL)
    inside &= PtInRegion(region, pt.x, pt.y);

  if(!inside) {
    //  HWND h = WindowFromPoint(p);
    //  if(h != getHandle()) {
    if(mouseOver) {
      mouseOver = false;
      Invalidate();
    }
    if(!(wParam & MK_LBUTTON))
      ReleaseCapture();
  } else {
    if(!mouseOver) {
      mouseOver = true;
      Invalidate();
    }
  }
  return Wnd::Default(WM_MOUSEMOVE, wParam, lParam);
}

LRESULT SkinButton::OnKillFocus(WPARAM wParam, LPARAM lParam)
{
  mouseOver = false;
  Invalidate();
  return Wnd::Default(WM_KILLFOCUS, wParam, lParam);
}

LRESULT SkinButton::OnCaptureChanged(WPARAM wParam, LPARAM lParam)
{
  if(mouseOver) {
    ReleaseCapture();
    Invalidate();
  }
  return Wnd::Default(WM_CAPTURECHANGED, wParam, lParam);
}

LRESULT SkinButton::OnRButtonDown(WPARAM wParam, LPARAM lParam)
{
  return 0;
}

LRESULT SkinButton::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
  if(mouseOver) {
    ReleaseCapture();
    mouseOver = false;
    Invalidate();
  }

  return Wnd::Default(WM_MOUSELEAVE, wParam, lParam);
}
