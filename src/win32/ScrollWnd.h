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
#ifndef VBA_WIN32_SCROLLWND_H
#define VBA_WIN32_SCROLLWND_H

#include "Stdafx.h"
#include "Wnd.h"

// Scroll extended styles
#define SCRL_SCROLLCHILDREN     0x00000001
#define SCRL_ERASEBACKGROUND    0x00000002
#define SCRL_NOTHUMBTRACKING    0x00000004
#if (WINVER >= 0x0500)
#define SCRL_SMOOTHSCROLL       0x00000008
#endif //(WINVER >= 0x0500)

class ScrollWnd : public Wnd {
 protected:
  DECLARE_MESSAGE_MAP()
    
 public:
  enum { uSCROLL_FLAGS = SW_INVALIDATE };
  
  POINT m_ptOffset;
  SIZE m_sizeAll;
  SIZE m_sizeLine;
  SIZE m_sizePage;
  SIZE m_sizeClient;
  int m_zDelta;                 // current wheel value
  int m_nWheelLines;            // number of lines to scroll on wheel
#if !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))
  UINT m_uMsgMouseWheel;                // MSH_MOUSEWHEEL
  // Note that this message must be forwarded from a top level window
#endif //!((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))
  UINT m_uScrollFlags;
  DWORD m_dwExtendedStyle;      // scroll specific extended styles
  
  ScrollWnd();
  
  DWORD GetScrollExtendedStyle() const
    {
      return m_dwExtendedStyle;
    }

  DWORD SetScrollExtendedStyle(DWORD dwExtendedStyle, DWORD dwMask = 0);
  void SetScrollOffset(int x, int y, BOOL bRedraw = TRUE);
  void SetScrollOffset(POINT ptOffset, BOOL bRedraw = TRUE);
  void GetScrollOffset(POINT& ptOffset) const;
  void SetScrollSize(int cx, int cy, BOOL bRedraw = TRUE);
  void SetScrollSize(SIZE size, BOOL bRedraw = TRUE);
  void GetScrollSize(SIZE& sizeWnd) const;
  void SetScrollLine(int cxLine, int cyLine);
  void SetScrollLine(SIZE sizeLine);
  void GetScrollLine(SIZE& sizeLine) const;
  void SetScrollPage(int cxPage, int cyPage);
  void SetScrollPage(SIZE sizePage);
  void GetScrollPage(SIZE& sizePage) const;

  virtual int OnCreate(LPCREATESTRUCT);
  virtual void OnVScroll(UINT, UINT, HWND);
  virtual void OnHScroll(UINT, UINT, HWND);
  virtual void OnSettingChange(UINT, LPCTSTR);
  virtual void OnSize(UINT, int, int);
  virtual void OnPaint();
  virtual void DoPaint(HDC);
  
  int CalcLineOrPage(int nVal, int nMax, int nDiv);
  void GetSystemSettings();
  void DoScroll(int nType, int nScrollCode, int& cxyOffset, int cxySizeAll, int cxySizePage, int cxySizeLine);
  
  bool IsScrollingChildren() const
    {
      return (m_dwExtendedStyle & SCRL_SCROLLCHILDREN) != 0;
    }
  
  bool IsErasingBackground() const
    {
      return (m_dwExtendedStyle & SCRL_ERASEBACKGROUND) != 0;
    }
  
  bool IsNoThumbTracking() const
    {
      return (m_dwExtendedStyle & SCRL_NOTHUMBTRACKING) != 0;
    }
  
#if (WINVER >= 0x0500)
  bool IsSmoothScroll() const
    {
      return (m_dwExtendedStyle & SCRL_SMOOTHSCROLL) != 0;
    }
#endif //(WINVER >= 0x0500)  
};
#endif
