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
#include "ScrollWnd.h"
 
#if !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))
#include <zmouse.h>
#endif //!((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))

BEGIN_MESSAGE_MAP(ScrollWnd, Wnd)
  ON_WM_CREATE()
  ON_WM_SIZE()
  ON_WM_PAINT()
  ON_WM_HSCROLL()
  ON_WM_VSCROLL()
  ON_WM_SETTINGCHANGE()
END_MESSAGE_MAP()

ScrollWnd::ScrollWnd()
  : Wnd()
{
  m_zDelta = 0;
  m_nWheelLines = 3;
  m_uScrollFlags = 0;
  m_dwExtendedStyle = 0;

  m_ptOffset.x = 0;
  m_ptOffset.y = 0;
  m_sizeAll.cx = 0;
  m_sizeAll.cy = 0;
  m_sizePage.cx = 0;
  m_sizePage.cy = 0;
  m_sizeLine.cx = 0;
  m_sizeLine.cy = 0;
  m_sizeClient.cx = 0;
  m_sizeClient.cy = 0;
  
  SetScrollExtendedStyle(SCRL_SCROLLCHILDREN | SCRL_ERASEBACKGROUND);
}

DWORD ScrollWnd::SetScrollExtendedStyle(DWORD dwExtendedStyle,
                                        DWORD dwMask)
{
  DWORD dwPrevStyle = m_dwExtendedStyle;
  if(dwMask == 0)
    m_dwExtendedStyle = dwExtendedStyle;
  else
    m_dwExtendedStyle = (m_dwExtendedStyle & ~dwMask) | (dwExtendedStyle & dwMask);
  // cache scroll flags
  m_uScrollFlags = uSCROLL_FLAGS |
    (IsScrollingChildren() ? SW_SCROLLCHILDREN : 0) |
    (IsErasingBackground() ? SW_ERASE : 0);
#if (WINVER >= 0x0500)
  m_uScrollFlags |= (IsSmoothScroll() ? SW_SMOOTHSCROLL : 0);
#endif //(WINVER >= 0x0500)
  return dwPrevStyle;
}

// offset operations
void ScrollWnd::SetScrollOffset(int x, int y, BOOL bRedraw)
{
  ASSERT(::IsWindow(hWnd));

  m_ptOffset.x = x;
  m_ptOffset.y = y;
  
  SCROLLINFO si;
  si.cbSize = sizeof(si);
  si.fMask = SIF_POS;
  
  si.nPos = m_ptOffset.x;
  SetScrollInfo(hWnd, SB_HORZ, &si, bRedraw);
  
  si.nPos = m_ptOffset.y;
  SetScrollInfo(hWnd, SB_VERT, &si, bRedraw);
  
  if(bRedraw)
    InvalidateRect(NULL, TRUE);
}

void ScrollWnd::SetScrollOffset(POINT ptOffset, BOOL bRedraw)
{
  SetScrollOffset(ptOffset.x, ptOffset.y, bRedraw);
}

void ScrollWnd::GetScrollOffset(POINT& ptOffset) const
{
  ptOffset = m_ptOffset;
}

// size operations
void ScrollWnd::SetScrollSize(int cx, int cy, BOOL bRedraw)
{
  ASSERT(::IsWindow(hWnd));
  
  m_sizeAll.cx = cx;
  m_sizeAll.cy = cy;
  
  m_ptOffset.x = 0;
  m_ptOffset.y = 0;
  
  SCROLLINFO si;
  si.cbSize = sizeof(si);
  si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
  si.nMin = 0;
  
  si.nMax = m_sizeAll.cx - 1;
  si.nPage = m_sizeClient.cx;
  si.nPos = m_ptOffset.x;
  SetScrollInfo(hWnd, SB_HORZ, &si, bRedraw);
  
  si.nMax = m_sizeAll.cy - 1;
  si.nPage = m_sizeClient.cy;
  si.nPos = m_ptOffset.y;
  SetScrollInfo(hWnd, SB_VERT, &si, bRedraw);
  
  SetScrollLine(0, 0);
  SetScrollPage(0, 0);
  
  if(bRedraw)
    InvalidateRect(NULL, TRUE);
}

void ScrollWnd::SetScrollSize(SIZE size, BOOL bRedraw)
{
  SetScrollSize(size.cx, size.cy, bRedraw);
}

void ScrollWnd::GetScrollSize(SIZE& sizeWnd) const
{
  sizeWnd = m_sizeAll;
}

// line operations
void ScrollWnd::SetScrollLine(int cxLine, int cyLine)
{
  ASSERT(cxLine >= 0 && cyLine >= 0);
  ASSERT(m_sizeAll.cx != 0 && m_sizeAll.cy != 0);
  
  m_sizeLine.cx = CalcLineOrPage(cxLine, m_sizeAll.cx, 100);
  m_sizeLine.cy = CalcLineOrPage(cyLine, m_sizeAll.cy, 100);
}

void ScrollWnd::SetScrollLine(SIZE sizeLine)
{
  SetScrollLine(sizeLine.cx, sizeLine.cy);
}

void ScrollWnd::GetScrollLine(SIZE& sizeLine) const
{
  sizeLine = m_sizeLine;
}

// page operations
void ScrollWnd::SetScrollPage(int cxPage, int cyPage)
{
  ASSERT(cxPage >= 0 && cyPage >= 0);
  ASSERT(m_sizeAll.cx != 0 && m_sizeAll.cy != 0);
  
  m_sizePage.cx = CalcLineOrPage(cxPage, m_sizeAll.cx, 10);
  m_sizePage.cy = CalcLineOrPage(cyPage, m_sizeAll.cy, 10);
}

void ScrollWnd::SetScrollPage(SIZE sizePage)
{
  SetScrollPage(sizePage.cx, sizePage.cy);
}

void ScrollWnd::GetScrollPage(SIZE& sizePage) const
{
  sizePage = m_sizePage;
}

BOOL ScrollWnd::OnCreate(LPCREATESTRUCT s)
{
  GetSystemSettings();
  return Wnd::OnCreate(s);
}

void ScrollWnd::OnVScroll(UINT code, UINT pos, HWND)
{
  ASSERT(::IsWindow(hWnd));

  DoScroll(SB_VERT, (int)code, (int &)m_ptOffset.y,
           m_sizeAll.cy, m_sizePage.cy, m_sizeLine.cy);
}

void ScrollWnd::OnHScroll(UINT code, UINT pos, HWND)
{
  ASSERT(::IsWindow(hWnd));

  DoScroll(SB_HORZ, (int)(short)code, (int &)m_ptOffset.x,
           m_sizeAll.cx, m_sizePage.cx, m_sizeLine.cx);
}

void ScrollWnd::OnSettingChange(UINT, LPCTSTR)
{
  GetSystemSettings();
}

void ScrollWnd::OnSize(UINT, int cx, int cy)
{
  ASSERT(::IsWindow(hWnd));

  m_sizeClient.cx = cx;
  m_sizeClient.cy = cy;
  
  SCROLLINFO si;
  si.cbSize = sizeof(si);
  si.fMask = SIF_PAGE | SIF_POS;
  
  si.nPage = m_sizeClient.cx;
  si.nPos = m_ptOffset.x;
  SetScrollInfo(hWnd, SB_HORZ, &si, FALSE);

  si.nPage = m_sizeClient.cy;
  si.nPos = m_ptOffset.y;
  SetScrollInfo(hWnd, SB_VERT, &si, FALSE);

  bool bUpdate = false;
  int cxMax = m_sizeAll.cx - m_sizeClient.cx;
  int cyMax = m_sizeAll.cy - m_sizeClient.cy;
  int x = m_ptOffset.x;
  int y = m_ptOffset.y;
  if(m_ptOffset.x > cxMax) {
    bUpdate = true;
    x = (cxMax >= 0) ? cxMax : 0;
  }
  if(m_ptOffset.y > cyMax) {
    bUpdate = true;
    y = (cyMax >= 0) ? cyMax : 0;
  }
  if(bUpdate) {
    ScrollWindowEx(hWnd, m_ptOffset.x - x, m_ptOffset.y - y, NULL, NULL,
      NULL, NULL, m_uScrollFlags);
    SetScrollOffset(x, y, FALSE);
  }
}

void ScrollWnd::OnPaint()
{
  ASSERT(::IsWindow(hWnd));

  PAINTSTRUCT ps;

  HDC dc = ::BeginPaint(hWnd, &ps);
  ::SetViewportOrgEx(dc, -m_ptOffset.x, -m_ptOffset.y, NULL);
  
  DoPaint(dc);
  ::EndPaint(hWnd, &ps);
}

void ScrollWnd::DoPaint(HDC)
{
  ASSERT(FALSE);
}

void ScrollWnd::DoScroll(int nType, int nScrollCode, int& cxyOffset,
                         int cxySizeAll, int cxySizePage,
                         int cxySizeLine)
{
  RECT rect;
  GetClientRect(hWnd, &rect);
  int cxyClient = (nType == SB_VERT) ? rect.bottom : rect.right;
  int cxyMax = cxySizeAll - cxyClient;
  
  if(cxyMax < 0)                // can't scroll, client area is bigger
    return;
  
  BOOL bUpdate = TRUE;
  int cxyScroll = 0;
  
  switch(nScrollCode) {
  case SB_TOP:          // top or all left
    cxyScroll = cxyOffset;
    cxyOffset = 0;
    break;
  case SB_BOTTOM:               // bottom or all right
    cxyScroll = cxyOffset - cxyMax;
    cxyOffset = cxyMax;
    break;
  case SB_LINEUP:               // line up or line left
    if(cxyOffset >= cxySizeLine) {
      cxyScroll = cxySizeLine;
      cxyOffset -= cxySizeLine;
    } else {
      cxyScroll = cxyOffset;
      cxyOffset = 0;
    }
    break;
  case SB_LINEDOWN:     // line down or line right
    if(cxyOffset < cxyMax - cxySizeLine) {
      cxyScroll = -cxySizeLine;
      cxyOffset += cxySizeLine;
    } else {
      cxyScroll = cxyOffset - cxyMax;
      cxyOffset = cxyMax;
    }
    break;
  case SB_PAGEUP:               // page up or page left
    if(cxyOffset >= cxySizePage) {
      cxyScroll = cxySizePage;
      cxyOffset -= cxySizePage;
    } else {
      cxyScroll = cxyOffset;
      cxyOffset = 0;
    }
    break;
  case SB_PAGEDOWN:     // page down or page right
    if(cxyOffset < cxyMax - cxySizePage)
      {
        cxyScroll = -cxySizePage;
        cxyOffset += cxySizePage;
      } else {
        cxyScroll = cxyOffset - cxyMax;
        cxyOffset = cxyMax;
      }
    break;
  case SB_THUMBTRACK:
    if(IsNoThumbTracking())
      break;
    // else fall through
  case SB_THUMBPOSITION:
    {
      SCROLLINFO si;
      si.cbSize = sizeof(SCROLLINFO);
      si.fMask = SIF_TRACKPOS;
      if(GetScrollInfo(hWnd, nType, &si)) {
        cxyScroll = cxyOffset - si.nTrackPos;
        cxyOffset = si.nTrackPos;
      }
    }
    break;
  case SB_ENDSCROLL:
  default:
    bUpdate = FALSE;
    break;
  }
  
  if(bUpdate && cxyScroll != 0) {
    SetScrollPos(hWnd, nType, cxyOffset, TRUE);
    if(nType == SB_VERT)
      ScrollWindowEx(hWnd, 0, cxyScroll, NULL, NULL, NULL, NULL, m_uScrollFlags);
    else
      ScrollWindowEx(hWnd, cxyScroll, 0, NULL, NULL, NULL, NULL, m_uScrollFlags);
  }  
}

int ScrollWnd::CalcLineOrPage(int nVal, int nMax, int nDiv)
{
  if(nVal == 0) {
    nVal = nMax / nDiv;
    if(nVal < 1)
      nVal = 1;
  } else if(nVal > nMax)
    nVal = nMax;
  
  return nVal;
}

void ScrollWnd::GetSystemSettings()
{
#ifndef SPI_GETWHEELSCROLLLINES
  const UINT SPI_GETWHEELSCROLLLINES = 104;
#endif //!SPI_GETWHEELSCROLLLINES
  ::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &m_nWheelLines, 0);
  
#if !((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))
  if(m_uMsgMouseWheel != 0)
    m_uMsgMouseWheel = ::RegisterWindowMessage(MSH_MOUSEWHEEL);
  
  HWND hWndWheel = FindWindow(MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE);
  if(::IsWindow(hWndWheel)) {
    UINT uMsgScrollLines = ::RegisterWindowMessage(MSH_SCROLL_LINES);
    if(uMsgScrollLines != 0)
      m_nWheelLines = ::SendMessage(hWndWheel, uMsgScrollLines, 0, 0L);
  }
#endif //!((_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400))
}
