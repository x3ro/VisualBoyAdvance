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
#ifndef VBA_WIN32_CONTROLS_H
#define VBA_WIN32_CONTROLS_H

#include "../System.h"
#include "ScrollWnd.h"

#define WM_COLINFO WM_APP+100
#define WM_MAPINFO WM_APP+101

class ColorControl : public Wnd {
  u16 color;
  static bool isRegistered;
 protected:
  DECLARE_MESSAGE_MAP()
public:
  ColorControl();

  void setColor(u16);
  
  virtual BOOL OnEraseBkgnd(HDC);
  virtual void OnPaint();

  static void registerClass();
};

class ColorButton : public Wnd {
  u16 color;
  static bool isRegistered;
public:
  ColorButton();

  void setColor(u16);
  
  virtual void OnDrawItem(int, LPDRAWITEMSTRUCT);

  virtual void PreSubclassWindow();
  
  static void registerClass();
};

class ZoomControl : public Wnd {
  u8 colors[3 * 64];
  static bool isRegistered;
  int selected;
 protected:
  DECLARE_MESSAGE_MAP()
public:
  ZoomControl();

  static void registerClass();
  void setColors(u8 *colors);

  virtual void OnPaint();
  virtual BOOL OnEraseBkgnd(HDC);
  virtual void OnLButtonDown(UINT, int, int);
};

class BitmapControl : public ScrollWnd {
  int w;
  int h;
  u8 *data;
  BITMAPINFO *bmpInfo;
  static bool isRegistered;
  u8 colors[3 * 64];
  bool stretch;
 protected:
  DECLARE_MESSAGE_MAP()  
public:
  void setStretch(bool b);
  bool getStretch()
  {
    return stretch;
  }
  BitmapControl();
  
  void setBmpInfo(BITMAPINFO *);
  void setData(u8 *);
  void setSize(int,int);
  void refresh();
  
  virtual void DoPaint(HDC);
  virtual BOOL OnEraseBkgnd(HDC);
  virtual void OnLButtonDown(UINT, int, int);
  virtual void OnSize(UINT, int, int);

  static void registerClass();
};

class HyperLink : public Wnd {
protected:
  DECLARE_MESSAGE_MAP()
    
public:
  LPTSTR m_lpstrLabel;
  LPTSTR m_lpstrHyperLink;
  HCURSOR m_hCursor;
  HFONT m_hFont;
  RECT m_rcLink;
  bool m_bPaintLabel;
//  CToolTipCtrl m_tip;
  
  bool m_bVisited;
  COLORREF m_clrLink;
  COLORREF m_clrVisited;
  
  HyperLink();
  ~HyperLink();
  bool GetLabel(LPTSTR lpstrBuffer, int nLength) const;
  bool SetLabel(LPCTSTR lpstrLabel);
  bool GetHyperLink(LPTSTR lpstrBuffer, int nLength) const;
  bool SetHyperLink(LPCTSTR lpstrLink);
  void SubClassWindow(HWND hWnd);
  bool Navigate();

  virtual int OnCreate(LPCREATESTRUCT);
  virtual BOOL OnEraseBkgnd(HDC);
  virtual void OnPaint();
  virtual void OnSetFocus(HWND);
  virtual void OnKillFocus(HWND);
  virtual void OnMouseMove(UINT, int, int);
  virtual void OnLButtonDown(UINT, int, int);
  virtual void OnLButtonUp(UINT, int, int);
  virtual BOOL OnSetCursor(HWND, UINT, UINT);
  virtual void OnChar(UINT, UINT, UINT);
  virtual UINT OnGetDlgCode();

  void Init();
  static COLORREF _ParseColorString(LPTSTR lpstr);
  bool CalcLabelRect();
  void DoPaint(HDC);
};
#endif
