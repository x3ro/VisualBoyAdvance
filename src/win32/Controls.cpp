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
#include "Wnd.h"
#include "Controls.h"

extern LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
extern HINSTANCE hInstance;

bool ColorButton::isRegistered = false;
bool ColorControl::isRegistered = false;
bool BitmapControl::isRegistered = false;
bool ZoomControl::isRegistered = false;

/**************************************************************************
 * Color control
 **************************************************************************/
BEGIN_MESSAGE_MAP(ColorControl, Wnd)
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
END_MESSAGE_MAP()
  
ColorControl::ColorControl()
  : Wnd()
{
  color = 0;
}

BOOL ColorControl::OnEraseBkgnd(HDC dc)
{
  int r = (color & 0x1f) << 3;
  int g = (color & 0x3e0) >> 2;
  int b = (color & 0x7c00) >> 7;

  HBRUSH br = CreateSolidBrush(RGB(r,g,b));
  RECT rect;
  GetClientRect(getHandle(), &rect);
  FillRect(dc, &rect,br);
  DrawEdge(dc, &rect, EDGE_SUNKEN, BF_RECT);
  DeleteObject(br);
  return TRUE;
}

void ColorControl::OnPaint()
{
  PAINTSTRUCT ps;
  BeginPaint(hWnd, &ps);
  EndPaint(hWnd, &ps);
}

void ColorControl::setColor(u16 c)
{
  color = c;
  Invalidate();
}

void ColorControl::registerClass()
{
  if(!isRegistered) {
    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH )GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "VbaColorControl";
    RegisterClass(&wc);
    isRegistered = true;
  }
}

/**************************************************************************
 * Color button
 **************************************************************************/

ColorButton::ColorButton()
  : Wnd()
{
  color = 0;
}

void ColorButton::PreSubclassWindow()
{
  SetWindowLong(GWL_STYLE, GetStyle() | BS_OWNERDRAW);
  Wnd::PreSubclassWindow();
}

void ColorButton::OnDrawItem(int, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
  ASSERT(lpDrawItemStruct);
  
  int r = (color & 0x1f) << 3;
  int g = (color & 0x3e0) >> 2;
  int b = (color & 0x7c00) >> 7;

  HDC dc = lpDrawItemStruct->hDC;
  UINT state = lpDrawItemStruct->itemState;
  RECT rect = lpDrawItemStruct->rcItem;

  SIZE margins;
  margins.cx = ::GetSystemMetrics(SM_CXEDGE);
  margins.cy = ::GetSystemMetrics(SM_CYEDGE);

  if(::SendMessage(hWnd, BM_GETSTATE, 0, 0) & BST_PUSHED)
    DrawEdge(dc, &rect, EDGE_SUNKEN, BF_RECT);
  else
    DrawEdge(dc, &rect, EDGE_RAISED, BF_RECT);

  InflateRect(&rect, -margins.cx, -margins.cy);
  
  HBRUSH br = CreateSolidBrush((state & ODS_DISABLED) ? 
                               ::GetSysColor(COLOR_3DFACE) : RGB(r,g,b));

  FillRect(dc, &rect, br);

  if(state & ODS_FOCUS) {
    InflateRect(&rect, -1, -1);
    DrawFocusRect(dc, &rect);
  }
  
  DeleteObject(br);
}

void ColorButton::setColor(u16 c)
{
  color = c;
  Invalidate();
}

void ColorButton::registerClass()
{
  if(!isRegistered) {
    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH )GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "VbaColorButton";
    RegisterClass(&wc);
    isRegistered = true;
  }
}

/**************************************************************************
 * Zoom control
 **************************************************************************/

BEGIN_MESSAGE_MAP(ZoomControl, Wnd)
  ON_WM_PAINT()
  ON_WM_ERASEBKGND()
  ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

ZoomControl::ZoomControl()
{
  ZeroMemory(colors, 3*64);
  selected = -1;
}

void ZoomControl::registerClass()
{
  if(!isRegistered) {
    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH )GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "VbaZoomControl";
    RegisterClass(&wc);
    isRegistered = true;    
  }
}

void ZoomControl::setColors(u8 *c)
{
  memcpy(colors, c, 3*64);
  selected = -1;
  Invalidate();
}

void ZoomControl::OnPaint()
{
  RECT rect;
  GetClientRect(getHandle(), &rect);
  PAINTSTRUCT ps;
  HDC dc = BeginPaint(getHandle(), &ps);

  int w = rect.right - rect.left;
  int h = rect.bottom - rect.top;
  
  HDC memDC = CreateCompatibleDC(dc);
  HBITMAP bitmap = CreateCompatibleBitmap(dc, w, h);

  SelectObject(memDC, bitmap);
  
  int multX = w / 8;
  int multY = h / 8;

  int i;
  for(i = 0; i < 64; i++) {
    HBRUSH b = CreateSolidBrush(RGB(colors[i*3+2], colors[i*3+1], colors[i*3]));
    RECT r;
    int x = i & 7;
    int y = i / 8;
    r.top = y*multY;
    r.left = x*multX;
    r.bottom = r.top + multY;
    r.right = r.left + multX;
    FillRect(memDC, &r, b);
    DeleteObject(b);
  }

  HPEN pen = CreatePen(PS_SOLID, 1, RGB(192,192,192));
  HPEN old = (HPEN)SelectObject(memDC, pen);

  for(i = 0; i < 8; i++) {
    MoveToEx(memDC, 0, i * multY, NULL);
    LineTo(memDC, w, i * multY);
    MoveToEx(memDC, i * multX, 0, NULL);
    LineTo(memDC, i * multX, h);
  }

  if(selected != -1) {
    HPEN pen2 = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
    HPEN old2 = (HPEN)SelectObject(memDC, pen2);

    int startX = (selected & 7)*multX+1;
    int startY = (selected / 8)*multY+1;
    int endX = startX + multX-2;
    int endY = startY + multY-2;
    
    MoveToEx(memDC, startX, startY, NULL);
    LineTo(memDC, endX, startY);
    LineTo(memDC, endX, endY);
    LineTo(memDC, startX, endY);
    LineTo(memDC, startX, startY-1);

    SelectObject(memDC, old2);
    DeleteObject(pen2);
  }
  
  SelectObject(memDC, old);
  DeleteObject(pen);

  BitBlt(dc, 0,0,w,h,
         memDC,0,0, SRCCOPY);

  DeleteObject(bitmap);
  DeleteDC(memDC);
  
  EndPaint(getHandle(), &ps);
}

BOOL ZoomControl::OnEraseBkgnd(HDC)
{
  return TRUE;
}

void ZoomControl::OnLButtonDown(UINT, int x, int y)
{
  RECT rect;
  GetClientRect(getHandle(), &rect);
  
  int height = rect.bottom - rect.top;
  int width = rect.right - rect.left;

  int multX = width / 8;
  int multY = height / 8;

  selected = x / multX + 8 * (y / multY);
  
  int c = x / multX + 8 * (y/multY);
  u16 color = colors[c*3] << 7 |
    colors[c*3+1] << 2 |
    (colors[c*3+2] >> 3);
  
  PostMessage(GetParent(getHandle()),
              WM_COLINFO,
              color,
              0);

  Invalidate();
}

/**************************************************************************
 * Bitmap control
 **************************************************************************/

BEGIN_MESSAGE_MAP(BitmapControl, ScrollWnd)
  ON_WM_ERASEBKGND()
  ON_WM_LBUTTONDOWN()
  ON_WM_SIZE()
END_MESSAGE_MAP()

BitmapControl::BitmapControl()
  : ScrollWnd()
{
  w = 0;
  h = 0;
  data = NULL;
  bmpInfo = NULL;
  stretch = false;
}

void BitmapControl::setBmpInfo(BITMAPINFO *info)
{
  bmpInfo = info;
}

void BitmapControl::setData(u8 *d)
{
  data = d;
}

void BitmapControl::setSize(int w1, int h1)
{
  if(w != w1 || h != h1) {
    w = w1;
    h = h1;
    SetScrollSize(w1, h1, FALSE);
  }
}

void BitmapControl::refresh()
{
  Invalidate();
}

void BitmapControl::OnSize(UINT flag, int cx, int cy)
{
  if(!stretch)
    ScrollWnd::OnSize(flag, cx, cy);
}

BOOL BitmapControl::OnEraseBkgnd(HDC dc)
{
  return TRUE;
}

void BitmapControl::DoPaint(HDC dc)
{
  RECT r;
  GetClientRect(hWnd, &r);
  int w1 = r.right - r.left;
  int h1 = r.bottom - r.top;
  HDC memDC = CreateCompatibleDC(dc);
  if(!stretch) {
    if(w > w1)
      w1 = w;
    if(h > h1)
      h1 = h;
  }
  HBITMAP bitmap = CreateCompatibleBitmap(dc, w1, h1);
  SelectObject(memDC, bitmap);
  if(stretch) {
    bmpInfo->bmiHeader.biWidth = w;
    bmpInfo->bmiHeader.biHeight = -h;
    
    StretchDIBits(memDC,
                  0,
                  0,
                  w1,
                  h1, 
                  0,
                  0,
                  w,
                  h,
                  data,
                  bmpInfo,
                  DIB_RGB_COLORS,
                  SRCCOPY);
  } else {
    FillRect(memDC, &r, GetSysColorBrush(COLOR_BTNFACE));
    
    bmpInfo->bmiHeader.biWidth = w;
    bmpInfo->bmiHeader.biHeight = -h;
    
    SetDIBitsToDevice(memDC,
                      0,
                      0,
                      w,
                      h,
                      0,
                      0,
                      0,
                      h,
                      data,
                      bmpInfo,
                            DIB_RGB_COLORS);
  }

  BitBlt(dc,0,0,w1,h1,
         memDC,0,0,SRCCOPY);

  DeleteObject(bitmap);
  DeleteDC(memDC);  
}

void BitmapControl::OnLButtonDown(UINT, int x, int y)
{
  if(!data)
    return;
  WPARAM point;
  
  if(stretch) {
    RECT rect;
    GetClientRect(getHandle(), &rect);
  
    int height = rect.bottom - rect.top;
    int width = rect.right - rect.left;
  
    int xx = (x * w) / width;
    int yy = (y * h) / height;

    point = xx | (yy<<16);

    int xxx = xx / 8;
    int yyy = yy / 8;

    for(int i = 0; i < 8; i++) {
      memcpy(&colors[i*3*8], &data[xxx * 8 * 3 +
                                 w * yyy * 8 * 3 +
                                   i * w * 3], 8 * 3);
    }
  } else {
    POINT p;
    GetScrollOffset(p);

    p.x += x;
    p.y += y;

    if(p.x >= w ||
      p.y >= h)
      return;

    point = p.x | (p.y<<16);
    
    int xxx = p.x / 8;
    int yyy = p.y / 8;

    for(int i = 0; i < 8; i++) {
      memcpy(&colors[i*3*8], &data[xxx * 8 * 3 +
                                 w * yyy * 8 * 3 +
                                   i * w * 3], 8 * 3);
    }
  }
  
  SendMessage(GetParent(getHandle()),
              WM_MAPINFO,
              point,
              (LPARAM)colors);
}

void BitmapControl::registerClass()
{
  if(!isRegistered) {
    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "VbaBitmapControl";
    RegisterClass(&wc);
    isRegistered = true;
  }
}

void BitmapControl::setStretch(bool b)
{
  if(b != stretch) {
    stretch = b;
    Invalidate();
  }
}

/**************************************************************************
 * HyperLink control
 **************************************************************************/

BEGIN_MESSAGE_MAP(HyperLink, Wnd)
  ON_WM_CREATE()
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
  ON_WM_SETFOCUS()
  ON_WM_KILLFOCUS()
  ON_WM_MOUSEMOVE()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_SETCURSOR()
  ON_WM_CHAR()
  ON_WM_GETDLGCODE()
END_MESSAGE_MAP()

__declspec(selectany) struct
{
  enum { cxWidth = 32, cyHeight = 32 };
  int xHotSpot;
  int yHotSpot;
  unsigned char arrANDPlane[cxWidth * cyHeight / 8];
  unsigned char arrXORPlane[cxWidth * cyHeight / 8];
} _AtlHyperLink_CursorData = 
  {
    5, 0, 
    {
      0xF9, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xFF, 
      0xF0, 0xFF, 0xFF, 0xFF, 0xF0, 0x3F, 0xFF, 0xFF, 0xF0, 0x07, 0xFF, 0xFF, 0xF0, 0x01, 0xFF, 0xFF, 
      0xF0, 0x00, 0xFF, 0xFF, 0x10, 0x00, 0x7F, 0xFF, 0x00, 0x00, 0x7F, 0xFF, 0x00, 0x00, 0x7F, 0xFF, 
      0x80, 0x00, 0x7F, 0xFF, 0xC0, 0x00, 0x7F, 0xFF, 0xC0, 0x00, 0x7F, 0xFF, 0xE0, 0x00, 0x7F, 0xFF, 
      0xE0, 0x00, 0xFF, 0xFF, 0xF0, 0x00, 0xFF, 0xFF, 0xF0, 0x00, 0xFF, 0xFF, 0xF8, 0x01, 0xFF, 0xFF, 
      0xF8, 0x01, 0xFF, 0xFF, 0xF8, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
    },
    {
      0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 
      0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0xC0, 0x00, 0x00, 0x06, 0xD8, 0x00, 0x00, 
      0x06, 0xDA, 0x00, 0x00, 0x06, 0xDB, 0x00, 0x00, 0x67, 0xFB, 0x00, 0x00, 0x77, 0xFF, 0x00, 0x00, 
      0x37, 0xFF, 0x00, 0x00, 0x17, 0xFF, 0x00, 0x00, 0x1F, 0xFF, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00, 
      0x0F, 0xFE, 0x00, 0x00, 0x07, 0xFE, 0x00, 0x00, 0x07, 0xFE, 0x00, 0x00, 0x03, 0xFC, 0x00, 0x00, 
      0x03, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }
  };

HyperLink::HyperLink():
  m_lpstrLabel(NULL), m_lpstrHyperLink(NULL),
  m_hCursor(NULL), m_hFont(NULL), m_bPaintLabel(true), m_bVisited(false),
  m_clrLink(RGB(0, 0, 255)), m_clrVisited(RGB(128, 0, 128)), Wnd()
  
{
  ::SetRectEmpty(&m_rcLink);
}

HyperLink::~HyperLink()
{
  free(m_lpstrLabel);
  free(m_lpstrHyperLink);
  if(m_hFont != NULL)
    ::DeleteObject(m_hFont);
  // It was created, not loaded, so we have to destroy it
  if(m_hCursor != NULL)
    ::DestroyCursor(m_hCursor);
}

// Attributes
bool HyperLink::GetLabel(LPTSTR lpstrBuffer, int nLength) const
{
  if(m_lpstrLabel == NULL)
    return false;
  ASSERT(lpstrBuffer != NULL);
  if(nLength > lstrlen(m_lpstrLabel) + 1) {
    lstrcpy(lpstrBuffer, m_lpstrLabel);
    return true;
  }
  return false;
}

bool HyperLink::SetLabel(LPCTSTR lpstrLabel)
{
  free(m_lpstrLabel);
  m_lpstrLabel = NULL;
  ASSERT_API(m_lpstrLabel = (LPTSTR)malloc((lstrlen(lpstrLabel) + 1) * sizeof(TCHAR)));
  if(m_lpstrLabel == NULL)
    return false;
  lstrcpy(m_lpstrLabel, lpstrLabel);
  CalcLabelRect();
  return true;
}

bool HyperLink::GetHyperLink(LPTSTR lpstrBuffer, int nLength) const
{
  if(m_lpstrHyperLink == NULL)
    return false;
  ASSERT(lpstrBuffer != NULL);
  if(nLength > lstrlen(m_lpstrHyperLink) + 1) {
    lstrcpy(lpstrBuffer, m_lpstrHyperLink);
    return true;
  }
  return false;
}

bool HyperLink::SetHyperLink(LPCTSTR lpstrLink)
{
  free(m_lpstrHyperLink);
  m_lpstrHyperLink = NULL;
  ASSERT_API(m_lpstrHyperLink = (LPTSTR)malloc((lstrlen(lpstrLink) + 1) * sizeof(TCHAR)));
  if(m_lpstrHyperLink == NULL)
    return false;
  lstrcpy(m_lpstrHyperLink, lpstrLink);
  if(m_lpstrLabel == NULL) {
    CalcLabelRect();
  }
  return true;
}

// Operations
void HyperLink::SubClassWindow(HWND hWnd)
{
  ASSERT(this->hWnd == NULL);
  ASSERT(::IsWindow(hWnd));
  Wnd::SubClassWindow(hWnd);
  Init();
}

bool HyperLink::Navigate()
{
  ASSERT(::IsWindow(hWnd));
  ASSERT(m_lpstrHyperLink != NULL);
  DWORD_PTR dwRet = (DWORD_PTR)
    ::ShellExecute(0, _T("open"), m_lpstrHyperLink, 0, 0, SW_SHOWNORMAL);
  ASSERT(dwRet > 32);
  if(dwRet > 32) {
    m_bVisited = true;
    Invalidate();
  }
  return (dwRet > 32);
}

int HyperLink::OnCreate(LPCREATESTRUCT)
{
  Init();
  return FALSE;
}

BOOL HyperLink::OnEraseBkgnd(HDC dc)
{
  if(m_bPaintLabel) {
    HBRUSH hBrush = (HBRUSH)::SendMessage(GetParent(hWnd),
                                          WM_CTLCOLORSTATIC,
                                          (WPARAM)dc,
                                          (LPARAM)hWnd);
    if(hBrush != NULL) {
      RECT rect;
      GetClientRect(hWnd, &rect);
      FillRect(dc, &rect, hBrush);
    }
  } else {
    return FALSE;
  }
  return TRUE;
}

void HyperLink::OnPaint()
{
  if(!m_bPaintLabel) {
    return;
  }

  PAINTSTRUCT ps;
  HDC dc = ::BeginPaint(hWnd, &ps);
  DoPaint(dc);
  ::EndPaint(hWnd, &ps);
}

void HyperLink::OnSetFocus(HWND)
{
  if(m_bPaintLabel)
    Invalidate();
}

void HyperLink::OnKillFocus(HWND)
{
  if(m_bPaintLabel)
    Invalidate();
}

void HyperLink::OnMouseMove(UINT, int x, int y)
{
  POINT pt = { x, y };
  if(m_lpstrHyperLink != NULL && ::PtInRect(&m_rcLink, pt))
    ::SetCursor(m_hCursor);
}

void HyperLink::OnLButtonDown(UINT, int x, int y)
{
  POINT pt = { x, y };
  if(::PtInRect(&m_rcLink, pt)) {
    SetFocus(hWnd);
    SetCapture(hWnd);
  }
}

void HyperLink::OnLButtonUp(UINT, int x, int y)
{
  if(GetCapture() == hWnd) {
    ReleaseCapture();
    POINT pt = { x, y };
    if(::PtInRect(&m_rcLink, pt)) {
      Navigate();
    }
  }
}

BOOL HyperLink::OnSetCursor(HWND, UINT, UINT)
{
  POINT pt;
  GetCursorPos(&pt);
  ScreenToClient(hWnd, &pt);
  if(m_lpstrHyperLink != NULL && ::PtInRect(&m_rcLink, pt)) {
    return TRUE;
  }
  return FALSE;
}

void HyperLink::OnChar(UINT c, UINT /* rep */, UINT /* flag */)
{
  if(c == VK_RETURN || c == VK_SPACE) {
    Navigate();
  }
}

UINT HyperLink::OnGetDlgCode()
{
  return DLGC_WANTCHARS;
}

// Implementation
void HyperLink::Init()
{
  ASSERT(::IsWindow(hWnd));

  // Check if we should paint a label
  TCHAR lpszBuffer[8];
  if(::GetClassName(hWnd, lpszBuffer, 8)) {
    if(lstrcmpi(lpszBuffer, _T("static")) == 0) {
      ::SetWindowLong(hWnd, GWL_STYLE, GetStyle() | SS_NOTIFY);
      DWORD dwStyle = GetStyle() & 0x000000FF;
      if(dwStyle == SS_ICON || dwStyle == SS_BLACKRECT ||
         dwStyle == SS_GRAYRECT || 
         dwStyle == SS_WHITERECT || dwStyle == SS_BLACKFRAME ||
         dwStyle == SS_GRAYFRAME || 
         dwStyle == SS_WHITEFRAME || dwStyle == SS_OWNERDRAW || 
         dwStyle == SS_BITMAP || dwStyle == SS_ENHMETAFILE)
        m_bPaintLabel = false;
    }
  }

  // create or load a cursor
  m_hCursor = ::CreateCursor(hInstance, _AtlHyperLink_CursorData.xHotSpot,
                             _AtlHyperLink_CursorData.yHotSpot,
                             _AtlHyperLink_CursorData.cxWidth,
                             _AtlHyperLink_CursorData.cyHeight,
                             _AtlHyperLink_CursorData.arrANDPlane,
                             _AtlHyperLink_CursorData.arrXORPlane);
  ASSERT(m_hCursor != NULL);
  
  // set font
  if(m_bPaintLabel) {
    HWND wnd = GetParent(hWnd);
    HFONT font = (HFONT)::SendMessage(wnd, WM_GETFONT, 0, 0);
    if(font != NULL) {
      LOGFONT lf;
      ::GetObject(font, sizeof(lf), &lf);
      lf.lfUnderline = TRUE;
      m_hFont = ::CreateFontIndirect(&lf);
    }
  }

  // set label (defaults to window text)
  if(m_lpstrLabel == NULL) {
    int nLen = GetWindowTextLength(hWnd);
    if(nLen > 0) {
      LPTSTR lpszText = (LPTSTR)malloc((nLen+1)*sizeof(TCHAR));
      if(GetWindowText(hWnd, lpszText, nLen+1))
        SetLabel(lpszText);
    }
  }

  // set hyperlink (defaults to label)
  if(m_lpstrHyperLink == NULL && m_lpstrLabel != NULL)
    SetHyperLink(m_lpstrLabel);

  CalcLabelRect();

  // create a tool tip
  //m_tip.Create(m_hWnd);
  //  ATLASSERT(m_tip.IsWindow());
  //m_tip.Activate(TRUE);
  //m_tip.AddTool(m_hWnd, m_lpstrHyperLink);
  
  // set link colors
  if(m_bPaintLabel) {
    HKEY rk = 0;
    LONG lRet = RegOpenKeyEx(HKEY_CURRENT_USER, 
      _T("Software\\Microsoft\\Internet Explorer\\Settings"), 0, KEY_READ, &rk);
    if(lRet == 0) {
      TCHAR szBuff[12];
      DWORD type;
      DWORD dwCount = 12 * sizeof(TCHAR);
      lRet = RegQueryValueEx(rk, _T("Anchor Color"), NULL, &type, (LPBYTE)szBuff, &dwCount);
      if(lRet == 0) {
        COLORREF clr = _ParseColorString(szBuff);
        ASSERT(clr != CLR_INVALID);
        if(clr != CLR_INVALID)
          m_clrLink = clr;
      }
      
      dwCount = 12 * sizeof(TCHAR);
      lRet = RegQueryValueEx(rk, _T("Anchor Color Visited"), NULL, &type, (LPBYTE)szBuff, &dwCount);
      if(lRet == 0) {
        COLORREF clr = _ParseColorString(szBuff);
        ASSERT(clr != CLR_INVALID);
        if(clr != CLR_INVALID)
          m_clrVisited = clr;
      }
      RegCloseKey(rk);
    }
  }
}

COLORREF HyperLink::_ParseColorString(LPTSTR lpstr)
{
  int c[3] = { -1, -1, -1 };
  LPTSTR p;
  for(int i = 0; i < 2; i++) {
    for(p = lpstr; *p != _T('\0'); p = ::CharNext(p)) {
      if(*p == _T(',')) {
        *p = _T('\0');
        c[i] = _ttoi(lpstr);
        lpstr = &p[1];
        break;
      }
    }
    if(c[i] == -1)
      return CLR_INVALID;
  }
  if(*lpstr == _T('\0'))
    return CLR_INVALID;
  c[2] = _ttoi(lpstr);
  
  return RGB(c[0], c[1], c[2]);
}

bool HyperLink::CalcLabelRect()
{
  if(!::IsWindow(hWnd))
    return false;
  if(m_lpstrLabel == NULL && m_lpstrHyperLink == NULL)
    return false;

  HDC dc = GetDC();
  RECT rect;
  GetClientRect(hWnd, &rect);
  m_rcLink = rect;
  if(m_bPaintLabel) {
    HFONT hOldFont = NULL;
    if(m_hFont != NULL)
      hOldFont = (HFONT)SelectObject(dc, m_hFont);
    LPTSTR lpstrText = (m_lpstrLabel != NULL) ? m_lpstrLabel :
      m_lpstrHyperLink;
    DWORD dwStyle = GetStyle();
    int nDrawStyle = DT_LEFT;
    if (dwStyle & SS_CENTER)
      nDrawStyle = DT_CENTER;
    else if (dwStyle & SS_RIGHT)
      nDrawStyle = DT_RIGHT;
    DrawText(dc, lpstrText, -1, &m_rcLink,
             nDrawStyle | DT_WORDBREAK | DT_CALCRECT);
    if(m_hFont != NULL)
      SelectObject(dc, hOldFont);
    if (dwStyle & SS_CENTER) {
      int dx = (rect.right - m_rcLink.right) / 2;
      ::OffsetRect(&m_rcLink, dx, 0);
    } else if (dwStyle & SS_RIGHT) {
      int dx = rect.right - m_rcLink.right;
      ::OffsetRect(&m_rcLink, dx, 0);
    }
  }
  ReleaseDC(dc);
  
  return true;
}

void HyperLink::DoPaint(HDC dc)
{
  SetBkMode(dc, TRANSPARENT);
  SetTextColor(dc, m_bVisited ? m_clrVisited : m_clrLink);
  if(m_hFont != NULL)
    SelectObject(dc, m_hFont);
  LPTSTR lpstrText = (m_lpstrLabel != NULL) ? m_lpstrLabel : m_lpstrHyperLink;
  DWORD dwStyle = GetStyle();
  int nDrawStyle = DT_LEFT;
  if (dwStyle & SS_CENTER)
    nDrawStyle = DT_CENTER;
  else if (dwStyle & SS_RIGHT)
    nDrawStyle = DT_RIGHT;
  DrawText(dc, lpstrText, -1, &m_rcLink, nDrawStyle | DT_WORDBREAK);
  if(GetFocus() == hWnd)
    DrawFocusRect(dc, &m_rcLink);
}
