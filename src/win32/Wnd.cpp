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

Wnd *wndInit = NULL;
HHOOK wndOldHook = NULL;
extern HWND hWindow;

MapHWND Wnd::map;

void WndAssertFailed(char *file, int line, char *exp)
{
  char buffer[1024];

  sprintf(buffer, "File %s\nLine %d\nExpression %s\nPress Retry to debug",
          file, line, exp);
  int res = MessageBox(hWindow, buffer, "Assertion failed!",
                       MB_ICONHAND | MB_SETFOREGROUND | MB_TASKMODAL |
                       MB_ABORTRETRYIGNORE);

  if(res == IDRETRY) {
    __asm int 3;
  } else if(res == IDABORT)
    SendMessage(hWindow, WM_QUIT, 0, 0);
}

#ifndef countof
        #define countof( t )    (sizeof( (t) ) / sizeof( (t)[0] ) )
#endif  //      countof

void WndApiFailure(char *pcszFilename, int nLine, char *pcszExpression )
{
  const DWORD dwLastError = ::GetLastError();
  LPCTSTR lpMsgBuf;
  (void)::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                         FORMAT_MESSAGE_FROM_SYSTEM |
                         FORMAT_MESSAGE_IGNORE_INSERTS,
                         NULL, dwLastError,
                         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                         (LPTSTR) &lpMsgBuf, 0, NULL );
  
  char szExeName[ MAX_PATH ];
  
  if( !GetModuleFileName( NULL, szExeName, countof( szExeName ) ) )
    strcpy( szExeName, "<No Program Name>" );
  
  
  char szMessage[ 1024 ];
  _snprintf( szMessage, countof( szMessage )
             , "API VERIFY Failure!"
             "\nProgram: %s"
             "\n"
             "\nFile %s"
             "\nLine %d"
             "\n"
             "\nExpression %s"
             "\n"
             "\nLast Error %d"
             "\n           %s"
             "\n\nPress Retry to debug the application"
             , szExeName
             , pcszFilename
             , nLine
             , pcszExpression
             , dwLastError
             , lpMsgBuf
             );
  
  (void)LocalFree( (LPVOID)lpMsgBuf );
  HWND hwndParent = ::GetActiveWindow();
  hwndParent = ::GetLastActivePopup( hwndParent );
  int nCode = ::MessageBoxA( hwndParent,
                             szMessage,
                             "Debug Helper",
                             MB_TASKMODAL | MB_ICONHAND | MB_ABORTRETRYIGNORE |
                             MB_SETFOREGROUND );
  if(nCode == IDABORT) {
    ::SendMessage(hWindow, WM_QUIT, 0, 0);
  } else if(nCode == IDRETRY)
    __asm int 3;
}

LONG WndInitCommonControls(LPINITCOMMONCONTROLSEX lpInitCtrls,LONG fToRegister)
{
  ASSERT(fToRegister != 0);
  
  HINSTANCE hInstBefore = ::GetModuleHandleA("COMCTL32.DLL");
  // load the COMCTL32.DLL library because it may not be loaded yet (delayload)
  HINSTANCE hInst = ::LoadLibraryA("COMCTL32.DLL");
  if (hInst == NULL)
    return 0;

  LONG lResult = 0;
  // attempt to get/call InitCommonControlsEx
  BOOL (STDAPICALLTYPE* pfnInit)(LPINITCOMMONCONTROLSEX lpInitCtrls) = NULL;
  (FARPROC&)pfnInit = ::GetProcAddress(hInst, "InitCommonControlsEx");
  if (pfnInit == NULL) {
    // not there, so call InitCommonControls if possible
    InitCommonControls();
    lResult = ICC_WIN95_CLASSES;
  } else if ((*pfnInit)(lpInitCtrls)) {
    // InitCommonControlsEx was successful so return the full mask
    lResult = fToRegister;
    if (hInstBefore == NULL) {
      // In the case that we are statically linked and COMCTL32.DLL
      // was not loaded before we loaded it with LoadLibrary in this
      // function, that indicates that the calling module is linked
      // with /delayload:comctl32.dll, and in this case we have to do
      // something to cause COMCTL32.DLL to stay loaded.  The only thing
      // we can do is to call a COMCTL32.DLL API which will cause the
      // CRT's delay load helpers to get called and will cause the DLL
      // to get loaded.  We choose to call InitCommonControls because
      // it exists in the original COMCTL32.DLL and it doesn't really
      // do any harm to call it, except for the time it takes to
      // register the set of original Windows 95 classes.
      // If this isn't done our FreeLibrary call below will cause
      // COMCTL32.DLL to go away, undoing the registration.
      InitCommonControls();
      lResult |= ICC_WIN95_CLASSES;
    }
  }
  
  // free the library reference and return the result
  FreeLibrary(hInst);
  return lResult;
}

BOOL WndInitCommonControls(LONG fToRegister)
{
  if (fToRegister == 0)
    return TRUE;

  LONG fRegisteredClasses = 0;

  INITCOMMONCONTROLSEX init;
  init.dwSize = sizeof(init);

  if (fToRegister & ICC_WIN95_CLASSES) {
    // this flag is compatible with the old InitCommonControls() API
    init.dwICC = ICC_WIN95_CLASSES;
    fRegisteredClasses |= WndInitCommonControls(&init, ICC_WIN95_CLASSES);
    fToRegister &= ~ICC_WIN95_CLASSES;
  }
  if (fToRegister & ICC_UPDOWN_CLASS) {
    init.dwICC = ICC_UPDOWN_CLASS;
    fRegisteredClasses |= WndInitCommonControls(&init,
                                                ICC_UPDOWN_CLASS);
  }
  if (fToRegister & ICC_LISTVIEW_CLASSES) {
    init.dwICC = ICC_LISTVIEW_CLASSES;
    fRegisteredClasses |= WndInitCommonControls(&init,
                                                ICC_LISTVIEW_CLASSES);
  }
  // must have registered at least as mamy classes as requested
  return (fToRegister & fRegisteredClasses) == fToRegister;
}

LRESULT CALLBACK WndProc(HWND h, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  Wnd *p = Wnd::FromMap(h);

  if(p != NULL) {
    return p->WindowProc(uMsg, wParam, lParam);
  }

  return DefWindowProc(h, uMsg, wParam, lParam);
}

LRESULT CALLBACK WndCbtFilterHook(int code, WPARAM wParam, LPARAM lParam)
{
  if(code != HCBT_CREATEWND)
    return CallNextHookEx(wndOldHook, code, wParam, lParam);

  ASSERT(lParam != NULL);
  LPCREATESTRUCT lpcs = ((LPCBT_CREATEWND)lParam)->lpcs;
  ASSERT(lpcs != NULL);

  if(wndInit != NULL || (!(lpcs->style & WS_CHILD))) {
    ASSERT(wParam != NULL);
    ASSERT(wndInit != NULL);

    
    HWND hWnd = (HWND)wParam;

    ASSERT(Wnd::FromMap(hWnd) == NULL);
    
    Wnd::Map(hWnd, wndInit);
    wndInit->Attach(hWnd);

    wndInit->PreSubclassWindow();

    if(!((WNDPROC)GetWindowLong(hWnd, GWL_WNDPROC) == WndProc)) {
      WNDPROC oldWndProc = (WNDPROC)SetWindowLong(hWnd,
                                                  GWL_WNDPROC,
                                                  (DWORD)WndProc);
      ASSERT(oldWndProc != NULL);
      
      wndInit->SetOldWndProc(oldWndProc);
    }
  }
  LRESULT r = CallNextHookEx(wndOldHook, code, wParam, lParam);
  UnhookWindowsHookEx(wndOldHook);
  wndOldHook = NULL;
  wndInit = NULL;
  return r;
}

void WndHookCreate(Wnd *pWnd)
{
  if(wndOldHook == NULL) {
    wndOldHook = SetWindowsHookEx(WH_CBT,
                                  WndCbtFilterHook,
                                  NULL,
                                  GetCurrentThreadId());
    ASSERT(wndOldHook != NULL);
    ASSERT(pWnd != NULL);
    ASSERT(pWnd->getHandle() == NULL);
    ASSERT(wndInit == NULL);
    
    wndInit = pWnd;
  }
}

const WndMap Wnd::messageMap =
{
        NULL,
        &Wnd::_messageEntries[0]
};

const WndMap* Wnd::GetMessageMap() const
{
        return &Wnd::messageMap;
}

const WndMapEntry Wnd::_messageEntries[] =
{
  ON_WM_NCDESTROY()
  { 0, 0, 0, 0 }     // nothing here
};

Wnd::Wnd()
{
  hWnd = NULL;
  oldWndProc = NULL;
  autoDelete = false;
}

Wnd::~Wnd()
{
}

void Wnd::Attach(HWND h)
{
  ASSERT(hWnd == NULL);
  
  hWnd = h;
  Map(h, this);
}

void Wnd::Detach()
{
  ASSERT(hWnd != NULL);
  
  hWnd = NULL;
}

void Wnd::SetOldWndProc(WNDPROC old)
{
  ASSERT(oldWndProc == NULL);
  
  oldWndProc = old;
}

LONG Wnd::SetWindowLong(int id, LONG l)
{
  return ::SetWindowLong(getHandle(), id, l);
}

LONG Wnd::GetWindowLong(int id)
{
  return ::GetWindowLong(getHandle(), id);
}

void Wnd::SubClassWindow(HWND hWnd)
{
  Attach(hWnd);

  PreSubclassWindow();
  
  WNDPROC proc = (WNDPROC)GetWindowLong(GWL_WNDPROC);
  if(proc != (WNDPROC)WndProc) {
    WNDPROC old = (WNDPROC)SetWindowLong(GWL_WNDPROC,
                                         (DWORD)WndProc);
    SetOldWndProc(old);
  }
}

void Wnd::SubClassDlgItem(UINT id, Wnd *parent)
{
  ASSERT(parent != NULL);
  
  HWND h = ::GetDlgItem(parent->hWnd, id);
  if (h != NULL)
    SubClassWindow(h);
}

BOOL Wnd::InvalidateRect(CONST RECT *rect, BOOL erase)
{
  return ::InvalidateRect(getHandle(), rect, erase);
}

HWND Wnd::GetDlgItem(UINT id)
{
  return ::GetDlgItem(getHandle(), id);
}

HDC Wnd::GetDC()
{
  return ::GetDC(getHandle());
}

int Wnd::ReleaseDC(HDC dc)
{
  return ::ReleaseDC(getHandle(), dc);
}

BOOL Wnd::DestroyWindow()
{
  return ::DestroyWindow(getHandle());
}

BOOL Wnd::SetWindowText(const char *s)
{
  return ::SetWindowText(getHandle(), s);
}

BOOL Wnd::WalkPreTranslateTree(HWND hWndStop, MSG *pMsg)
{
  ASSERT(hWndStop == NULL || ::IsWindow(hWndStop));
  ASSERT(pMsg != NULL);
  
  // walk from the target window up to the hWndStop window checking
  //  if any window wants to translate this message
  
  for (HWND hWnd = pMsg->hwnd; hWnd != NULL; hWnd = ::GetParent(hWnd))
    {
      Wnd* pWnd = Wnd::FromMap(hWnd);
      if (pWnd != NULL)
        {
          // target window is a C++ window
          if (pWnd->PreTranslateMessage(pMsg))
            return TRUE; // trapped by target window (eg: accelerators)
        }
      
      // got to hWndStop window without interest
      if (hWnd == hWndStop)
        break;
    }
  return FALSE;       // no special processing
}

BOOL Wnd::PreTranslateMessage(MSG *msg)
{
  return FALSE;
}

BOOL Wnd::PumpMessage(MSG& m_msgCur)
{
  ASSERT(this);
  
  if (!::GetMessage(&m_msgCur, NULL, NULL, NULL)) {
    return FALSE;
  }

  // process this message
  extern HWND hWindow;
  if (!WalkPreTranslateTree(hWindow, &m_msgCur)) {
    ::TranslateMessage(&m_msgCur);
    ::DispatchMessage(&m_msgCur);
  }
  return TRUE;
}

int Wnd::RunModalLoop()
{
  ASSERT(::IsWindow(hWnd)); // window must be created
  ASSERT(!(flags & WF_MODALLOOP)); // window must not already be in modal state

  BOOL bIdle = TRUE;
  BOOL bShowIdle = !(GetStyle() & WS_VISIBLE);
  HWND hWndParent = ::GetParent(hWnd);
  flags |= (WF_MODALLOOP|WF_CONTINUEMODAL);
  MSG msg;
  MSG *pMsg = &msg;

  for(;;) {
    while (bIdle &&
           !::PeekMessage(pMsg, NULL, NULL, NULL, PM_NOREMOVE)) {
      ASSERT(ContinueModal());
      
      // show the dialog when the message queue goes idle
      if (bShowIdle) {
        ShowWindow(SW_SHOWNORMAL);
        UpdateWindow();
        bShowIdle = FALSE;
      }
      // stop idle processing next time
      bIdle = FALSE;
    }
    
    do {
      ASSERT(ContinueModal());
      
      // pump message, but quit on WM_QUIT
      if (!PumpMessage(msg)) {
        return -1;
      }
      
      // show the window when certain special messages rec'd
      if (bShowIdle &&
          (pMsg->message == 0x118 || pMsg->message == WM_SYSKEYDOWN)) {
        ShowWindow(SW_SHOWNORMAL);
        UpdateWindow();
        bShowIdle = FALSE;
      }
      
      if (!ContinueModal())
        goto ExitModal;
      
    } while (::PeekMessage(pMsg, NULL, NULL, NULL, PM_NOREMOVE));
  }
  
ExitModal:
  flags &= ~(WF_MODALLOOP|WF_CONTINUEMODAL);
  return modalResult;
}

BOOL Wnd::ContinueModal()
{
  return flags & WF_CONTINUEMODAL;
}

void Wnd::EndModalLoop(int nResult)
{
  ASSERT(::IsWindow(hWnd));

  // this result will be returned from CWnd::RunModalLoop
  modalResult = nResult;
  
  // make sure a message goes through to exit the modal loop
  if (flags & WF_CONTINUEMODAL) {
    flags &= ~WF_CONTINUEMODAL;
    PostMessage(hWnd, WM_NULL, 0, 0);
  }
}

BOOL Wnd::OnCommand(WPARAM wParam, LPARAM lParam)
{
  UINT id = LOWORD(wParam);
  UINT notification = HIWORD(wParam);

  for(const WndMap *pMessageMap = GetMessageMap(); pMessageMap != NULL;
      pMessageMap = pMessageMap->pBaseMap) {
    ASSERT(pMessageMap != pMessageMap->pBaseMap);

    const WndMapEntry *entries = pMessageMap->lpEntries;

    while(entries->sig) {
      if(entries->msg == WM_COMMAND && (id >= entries->firstId) &&
         (id <= entries->lastId) && (notification == entries->notification)) {
        switch(entries->sig) {
        case WndSig_vv:
          (this->*entries->pfn)();
          return TRUE;
        case WndSig_vw:
          (this->*entries->pfn_vw)(id);
          return TRUE;
        default:
          ASSERT(FALSE);
        }
      }
      entries++;
    }
  }
  return FALSE;
}

void Wnd::OnChar(UINT, UINT, UINT)
{
}

void Wnd::OnClose()
{
}

int Wnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  return 0;
}

void Wnd::OnDestroy()
{
}

void Wnd::OnDrawItem(int, LPDRAWITEMSTRUCT)
{
}

BOOL Wnd::OnEraseBkgnd(HDC)
{
  return FALSE;
}

UINT Wnd::OnGetDlgCode()
{
  return 0;
}

void Wnd::OnHScroll(UINT, UINT, HWND)
{
}

void Wnd::OnKeyDown(UINT, UINT, UINT)
{
}

void Wnd::OnKeyUp(UINT, UINT, UINT)
{
}

void Wnd::OnKillFocus(HWND)
{
}

void Wnd::OnLButtonDown(UINT, int, int)
{
}

void Wnd::OnLButtonUp(UINT, int, int)
{
}

void Wnd::OnMouseMove(UINT, int, int)
{
}

void Wnd::OnPaint()
{
}

BOOL Wnd::OnNcCreate(CREATESTRUCT *)
{
  return FALSE;
}

void Wnd::OnNcDestroy()
{
  UnMap(hWnd);
  
  if(autoDelete) {
    this->~Wnd();
    delete this;
  }
}

BOOL Wnd::OnSetCursor(HWND, UINT, UINT)
{
  return FALSE;
}

void Wnd::OnSetFocus(HWND)
{
}

void Wnd::OnSettingChange(UINT, LPCTSTR)
{
}

void Wnd::OnSize(UINT, int, int)
{
}

void Wnd::OnSizing(UINT wParam, RECT *rect)
{
}

void Wnd::OnTimer(UINT)
{
}

void Wnd::OnVScroll(UINT, UINT, HWND)
{
}

BOOL Wnd::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
  ASSERT(pResult != NULL);
  NMHDR *pNMHDR = (NMHDR *)lParam;
  HWND hWndCtrl = pNMHDR->hwndFrom;

  UINT id = ((UINT)(WORD)::GetDlgCtrlID(hWndCtrl));
  
  int nCode = (UINT)(WORD)pNMHDR->code;

  ASSERT(hWndCtrl != NULL);
  ASSERT(::IsWindow(hWndCtrl));

  for(const WndMap *pMessageMap = GetMessageMap(); pMessageMap != NULL;
      pMessageMap = pMessageMap->pBaseMap) {
    ASSERT(pMessageMap != pMessageMap->pBaseMap);

    const WndMapEntry *entries = pMessageMap->lpEntries;

    while(entries->sig) {
      if(entries->msg == WM_NOTIFY && (id >= entries->firstId) &&
         (id <= entries->lastId) && (entries->notification == nCode)) {
        switch(entries->sig) {
        case WndSig_vNMHDRpl:
          (this->*entries->pfn_vNMHDRpl)(pNMHDR, pResult);
          return TRUE;
        case WndSig_vwNMHDRpl:
          (this->*entries->pfn_vwNMHDRpl)(id, pNMHDR, pResult);
          return TRUE;
        default:
          ASSERT(FALSE);
        }
      }
      entries++;
    }
  }
  
  return FALSE;
}

BOOL Wnd::OnMsg(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT &res)
{
  if(msg == WM_COMMAND) {
    if(OnCommand(wParam, lParam)) {
      res = 1;
      return TRUE;
    }
    return FALSE;
  }

  if(msg == WM_NOTIFY) {
    NMHDR *pNMHDR = (NMHDR *)lParam;
    if(pNMHDR->hwndFrom != NULL && OnNotify(wParam, lParam, &res))
      return TRUE;
    return FALSE;
  }
  
  for(const WndMap *pMessageMap = GetMessageMap(); pMessageMap != NULL;
      pMessageMap = pMessageMap->pBaseMap) {
    ASSERT(pMessageMap != pMessageMap->pBaseMap);

    const WndMapEntry *entries = pMessageMap->lpEntries;

    while(entries->sig) {
      if(entries->msg == msg) {
        switch(entries->sig) {
        case WndSig_vv:
          (this->*entries->pfn)();
          break;
        case WndSig_iLPCREATESTRUCT:
          res = (this->*entries->pfn_iLPCREATESTRUCT)((CREATESTRUCT *)lParam);
          break;
        case WndSig_bh:
          res = (this->*entries->pfn_bh)((HANDLE)wParam);
          break;
        case WndSig_vwii:
          (this->*entries->pfn_vwii)(wParam, LOWORD(lParam), HIWORD(lParam));
          break;
        case WndSig_vwSIZING:
          (this->*entries->pfn_vwSIZING)(wParam, (RECT *)lParam);
          res = TRUE;
          break;
        case WndSig_wv:
          res = (this->*entries->pfn_wv)();
          break;
        case WndSig_bhww:
          res = (this->*entries->pfn_bhww)((HWND)wParam, LOWORD(lParam), HIWORD(lParam));
          break;
        case WndSig_vh:
          (this->*entries->pfn_vh)((HWND)wParam);
          break;
        case WndSig_vwww:
          (this->*entries->pfn_vwww)(wParam, LOWORD(lParam), HIWORD(lParam));
          break;
        case WndSig_lwl:
          res = (this->*entries->pfn_lwl)(wParam, lParam);
          break;
        case WndSig_vwwh:
          (this->*entries->pfn_vwwh)(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
          break;
        case WndSig_vw:
          (this->*entries->pfn_vw)(wParam);
          break;
        case WndSig_vws:
          (this->*entries->pfn_vws)(wParam, (LPCTSTR)lParam);
          break;
        case WndSig_vwLPDRAWITEMSTRUCT:
          (this->*entries->pfn_vwLPDRAWITEMSTRUCT)(wParam, (LPDRAWITEMSTRUCT)lParam);
          break;
        default:
          ASSERT(FALSE);
          break;
        }
        return TRUE;
      }
      entries++;
    }
  }

  return FALSE;
}

void Wnd::PreSubclassWindow()
{
}

LRESULT Wnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  LRESULT l = 0;
  HWND handle= getHandle();
  WNDPROC old = oldWndProc;
  
  if(!OnMsg(message, wParam, lParam, l)) {
    if(old != NULL && handle != NULL)
      return CallWindowProc(old, handle, message, wParam, lParam);
    if(handle != NULL)
       return DefWindowProc(handle, message, wParam, lParam);
  }
  return l;
}

void Wnd::Map(HWND h, Wnd *pWnd)
{
  ASSERT(h != NULL);
  ASSERT(pWnd != NULL);
  
  MapHWND::iterator it = map.find(h);

  if(it == map.end())
    map.insert(MapHWND::value_type(h, pWnd));
}

void Wnd::UnMap(HWND h)
{
  ASSERT(h != NULL);
  
  map.erase(h);
}

Wnd *Wnd::FromMap(HWND h)
{
  ASSERT(h != NULL);
  
  MapHWND::iterator it = map.find(h);

  if(it != map.end())
    return it->second;

  return NULL;
}

void Wnd::trace(char *s, ...)
{
  char buffer[2048];
  va_list valist;
  
  va_start(valist, s);
  vsprintf(buffer, s, valist);

  OutputDebugString(buffer);
  
  va_end(valist);  
}

BOOL CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  Wnd *p = Wnd::FromMap(hWnd);

  if(p != NULL) {
    if(uMsg == WM_INITDIALOG)
      return ((Dlg *)p)->OnInitDialog(lParam);
  }
    
  return FALSE;
}

Dlg::Dlg()
  : Wnd()
{
}

Dlg::~Dlg()
{
}

extern UCHAR *winResGetResource(LPCTSTR, LPCTSTR);
extern HINSTANCE hInstance;

BEGIN_MESSAGE_MAP(Dlg, Wnd)
  ON_WM_CLOSE()
END_MESSAGE_MAP()

BOOL Dlg::OnInitDialog(LPARAM)
{
  return FALSE;
}

void Dlg::EndDialog(int res)
{
  if(flags & (WF_MODALLOOP | WF_CONTINUEMODAL))
    EndModalLoop(res);
  
  ::EndDialog(getHandle(), res);
}

void Dlg::OnClose()
{
  EndDialog(FALSE);
}

void Dlg::DoCheckbox(bool get, int id, int& value)
{
  HWND h = GetDlgItem(id);

  ASSERT(::IsWindow(h));
  
  if(get) {
    value = (int)::SendMessage(h, BM_GETCHECK, 0, 0L);
    ASSERT(value >= 0 && value <= 2);
  } else {
    if (value < 0 || value > 2) {
      value = 0;
    }
    ::SendMessage(h, BM_SETCHECK, (WPARAM)value, 0L);
  }  
}

void Dlg::DoRadio(bool get, int id, int& value)
{
  HWND h = GetDlgItem(id);

  ASSERT(::IsWindow(h));
  
  ASSERT(::GetWindowLong(h, GWL_STYLE) & WS_GROUP);
  ASSERT(::SendMessage(h, WM_GETDLGCODE, 0, 0L) & DLGC_RADIOBUTTON);

  if(get)
    value = -1;
  
  int button = 0;
  do {
    if(::SendMessage(h, WM_GETDLGCODE, 0, 0L) & DLGC_RADIOBUTTON) {
      if(get) {
        if (::SendMessage(h, BM_GETCHECK, 0, 0L) != 0) {
          ASSERT(value == -1);
          value = button;
        }
      } else {
        ::SendMessage(h, BM_SETCHECK, (button == value), 0L);
      }
      button++;
    }
    h = ::GetWindow(h, GW_HWNDNEXT);
  } while (h != NULL &&
           !(::GetWindowLong(h, GWL_STYLE) & WS_GROUP));
}

int Dlg::DoModal(UINT resource, HWND parent)
{
  return DoModal(resource, parent, 0);
}

int Dlg::DoModal(UINT resource, HWND parent, LPARAM l)
{
  UCHAR * b = winResGetResource(RT_DIALOG, MAKEINTRESOURCE(resource));

  if(b != NULL) {
    ASSERT_API(WndInitCommonControls(ICC_WIN95_CLASSES));
    ASSERT_API(WndInitCommonControls(ICC_UPDOWN_CLASS));
    ASSERT_API(WndInitCommonControls(ICC_LISTVIEW_CLASSES));

    BOOL enableParent = FALSE;
    if(parent != NULL && ::IsWindowEnabled(parent)) {
      ::EnableWindow(parent, FALSE);
      enableParent = TRUE;
    }
    
    WndHookCreate(this);
    if(CreateDialogIndirectParam(hInstance,
                                 (LPCDLGTEMPLATE)b,
                                 parent,
                                 DlgProc,
                                 l)) {
      flags |= WF_CONTINUEMODAL;
      modalResult = -1;

      int res = RunModalLoop();
      
      ASSERT(res == modalResult);

      if(hWnd != NULL)
        ::SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_HIDEWINDOW|
                       SWP_NOSIZE |SWP_NOMOVE|SWP_NOACTIVATE|
                       SWP_NOZORDER);
    }
    if(enableParent)
      ::EnableWindow(parent, TRUE);
    if(parent != NULL && ::GetActiveWindow() == hWnd)
      ::SetActiveWindow(parent);
    DestroyWindow();
    return modalResult;
  }
  return -1;
}

void Dlg::MakeDialog(HINSTANCE hInstance,
                       UINT resource,
                       HWND hParent)
{
  UCHAR * b = winResGetResource(RT_DIALOG, MAKEINTRESOURCE(resource));

  if(b != NULL) {
    ASSERT_API(WndInitCommonControls(ICC_WIN95_CLASSES));
    ASSERT_API(WndInitCommonControls(ICC_UPDOWN_CLASS));
    ASSERT_API(WndInitCommonControls(ICC_LISTVIEW_CLASSES));
    
    WndHookCreate(this);
    CreateDialogIndirect(hInstance,
                         (LPCDLGTEMPLATE)b,
                         hParent,
                         DlgProc);
  }
}

BOOL Dlg::PreTranslateMessage(MSG *msg)
{
  if((msg->message < WM_KEYFIRST || msg->message>WM_KEYLAST) &&
     (msg->message < WM_MOUSEFIRST || msg->message > WM_MOUSELAST))
    return FALSE;
  
  return ::IsDialogMessage(getHandle(), msg);
}

