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
#ifndef VBA_WND_H
#define VBA_WND_H

#pragma warning(disable:4786)

#include "stdafx.h"
#include <map>

class Wnd;

typedef std::map<HWND,Wnd *> MapHWND;

extern void WndAssertFailed(char *, int, char *);
extern void WndApiFailure(char *, int, char *);

#ifdef _DEBUG
#define ASSERT(f) \
 {\
   if(!(f)) WndAssertFailed(__FILE__, __LINE__, #f);\
 }

#define ASSERT_API(a) \
  { \
    if(!(a)) {\
     WndApiFailure(__FILE__, __LINE__, #a); \
    }\
  }
#else
#define ASSERT(a)

#define ASSERT_API(a) (a)
#endif

#define WF_MODALLOOP     1
#define WF_CONTINUEMODAL 2

typedef void (Wnd::*WND_PMSG)(void);

enum WndSig {
  WndSig_none,
  WndSig_vv, // void (void)
  WndSig_bv, // bool (void)
  WndSig_vw, // void (WPARAM)
  WndSig_iLPCREATESTRUCT, // int (LPCREATESTRUCT)
  WndSig_vwii, // void (UINT, int, int)
  WndSig_vh, // void (HWND)
  WndSig_bh, // BOOL (HDC)
  WndSig_vwSIZING, // void (UINT, LPRECT)
  WndSig_lwl, // LRESULT (WPARAM, LPARAM)
  WndSig_bhww, // BOOL (HWND, UINT, UINT)
  WndSig_vwww, // void (UINT, UINT, UINT)
  WndSig_wv, // UINT (void)
  WndSig_vws, // void (UINT, LPCTSTR)
  WndSig_vwwh, // void (UINT, UINT, HWND)
  WndSig_vwLPDRAWITEMSTRUCT, // void (int, LPDRAWITEMSTRUCT)
  WndSig_vNMHDRpl, // void (NMHDR *, LRESULT *)
  WndSig_vwNMHDRpl, // void (UINT, NMHDR *, LRESULT *)
};

struct WndMapEntry {
  UINT msg;
  UINT notification;
  UINT firstId;
  UINT lastId;
  WndSig sig;
  union {
    WND_PMSG pfn;
    bool (Wnd::*pfn_bv)(void);
    void (Wnd::*pfn_vw)(WPARAM);
    int (Wnd::*pfn_iLPCREATESTRUCT)(LPCREATESTRUCT);
    BOOL (Wnd::*pfn_bh)(HANDLE);
    void (Wnd::*pfn_vwii)(UINT, int, int);
    void (Wnd::*pfn_vwSIZING)(UINT, LPRECT);
    LRESULT (Wnd::*pfn_lwl)(WPARAM, LPARAM);
    BOOL (Wnd::*pfn_bhww)(HWND, UINT, UINT);
    void (Wnd::*pfn_vwww)(UINT, UINT, UINT);
    UINT (Wnd::*pfn_wv)(void);
    void (Wnd::*pfn_vws)(UINT, LPCTSTR);
    void (Wnd::*pfn_vwwh)(UINT, UINT, HWND);
    void (Wnd::*pfn_vwLPDRAWITEMSTRUCT)(int, LPDRAWITEMSTRUCT);
    void (Wnd::*pfn_vh)(HWND);
    void (Wnd::*pfn_vNMHDRpl)(NMHDR *, LRESULT *);
    void (Wnd::*pfn_vwNMHDRpl)(UINT, NMHDR *, LRESULT *);
  };
};

struct WndMap {
  const WndMap *pBaseMap;
  const WndMapEntry *lpEntries;
};

#define DECLARE_MESSAGE_MAP() \
private:\
  static const WndMapEntry _messageEntries[];\
protected:\
  static const WndMap messageMap;\
  virtual const WndMap *GetMessageMap() const;\

#define BEGIN_MESSAGE_MAP(theClass, baseClass)\
        const WndMap* theClass::GetMessageMap() const \
                { return &theClass::messageMap; } \
        const WndMap theClass::messageMap = \
        { &baseClass::messageMap, &theClass::_messageEntries[0] }; \
        const WndMapEntry theClass::_messageEntries[] = \
        { \

#define END_MESSAGE_MAP() \
  { 0, 0, 0, 0, WndSig_none, (WND_PMSG)0 }\
};

#define ON_BN_CLICKED(id, memberFxn) \
  { WM_COMMAND, BN_CLICKED, id, id, WndSig_vv, (WND_PMSG)&memberFxn },

#define ON_CONTROL(notify, id, memberFxn) \
  { WM_COMMAND, notify, id, id, WndSig_vv, (WND_PMSG)&memberFxn },

#define ON_CONTROL_RANGE(notify, id, idLast, memberFxn) \
  { WM_COMMAND, notify, id, idLast, WndSig_vw, (WND_PMSG)(void (Wnd::*)(UINT))&memberFxn },

#define ON_COMMAND(id, memberFxn) \
  { WM_COMMAND, BN_CLICKED, id, id, WndSig_vv, (WND_PMSG)&memberFxn },

#define ON_COMMAND_RANGE(id, idLast, memberFxn) \
  { WM_COMMAND, BN_CLICKED, id, idLast, WndSig_vw, (WND_PMSG)(void (Wnd::*)(UINT))&memberFxn },

#define ON_WM_CREATE() \
  { WM_CREATE, 0, 0, 0, WndSig_iLPCREATESTRUCT, (WND_PMSG)(int (Wnd::*)(LPCREATESTRUCT))&OnCreate },

#define ON_WM_DESTROY() \
  { WM_DESTROY, 0, 0, 0, WndSig_vv, (WND_PMSG)&OnDestroy },

#define ON_WM_SIZE() \
  { WM_SIZE, 0, 0, 0, WndSig_vwii, (WND_PMSG)(void (Wnd::*)(UINT, int, int))&OnSize },

#define ON_WM_SETFOCUS() \
  { WM_SETFOCUS, 0, 0, 0, WndSig_vh, (WND_PMSG)(void (Wnd::*)(HWND))&OnSetFocus },

#define ON_WM_KILLFOCUS() \
  { WM_KILLFOCUS, 0, 0, 0, WndSig_vh, (WND_PMSG)(void (Wnd::*)(HWND))&OnKillFocus },

#define ON_WM_CLOSE() \
  { WM_CLOSE, 0, 0, 0, WndSig_vv, (WND_PMSG)&OnClose },

#define ON_WM_ERASEBKGND() \
  { WM_ERASEBKGND, 0, 0, 0, WndSig_bh, (WND_PMSG)(BOOL (Wnd::*)(HDC))&OnEraseBkgnd },

#define ON_WM_LBUTTONDOWN() \
  { WM_LBUTTONDOWN, 0, 0, 0, WndSig_vwii, (WND_PMSG)(void (Wnd::*)(UINT, int, int))&OnLButtonDown },

#define ON_WM_LBUTTONUP() \
  { WM_LBUTTONUP, 0, 0, 0, WndSig_vwii, (WND_PMSG)(void (Wnd::*)(UINT, int, int))&OnLButtonUp },

#define ON_WM_NCCREATE() \
  { WM_NCCREATE, 0, 0, 0, WndSig_iLPCREATESTRUCT, (WND_PMSG)(BOOL (Wnd::*)(LPCREATESTRUCT))&OnNcCreate },

#define ON_WM_NCDESTROY() \
  { WM_NCDESTROY, 0, 0, 0, WndSig_vv, (WND_PMSG)&OnNcDestroy },

#define ON_WM_PAINT() \
  { WM_PAINT, 0, 0, 0, WndSig_vv, (WND_PMSG)&OnPaint },

#define ON_WM_SIZING() \
  { WM_SIZING, 0, 0, 0, WndSig_vwSIZING, (WND_PMSG)(void (Wnd::*)(UINT,LPRECT))&OnSizing },

#define ON_WM_HSCROLL() \
  { WM_HSCROLL, 0, 0, 0, WndSig_vwwh, (WND_PMSG)(void (Wnd::*)(UINT, UINT, HWND))&OnHScroll },
    
#define ON_WM_VSCROLL() \
  { WM_VSCROLL, 0, 0, 0, WndSig_vwwh, (WND_PMSG)(void (Wnd::*)(UINT, UINT, HWND))&OnVScroll },

#define ON_WM_MOUSEMOVE() \
  { WM_MOUSEMOVE, 0, 0, 0, WndSig_vwii, (WND_PMSG)(void (Wnd::*)(UINT, int, int))&OnMouseMove },

#define ON_WM_SETCURSOR() \
  { WM_SETCURSOR, 0, 0, 0, WndSig_bhww, (WND_PMSG)(BOOL (Wnd::*)(HWND, UINT, UINT))&OnSetCursor },

#define ON_WM_CHAR() \
  { WM_CHAR, 0, 0, 0, WndSig_vwww, (WND_PMSG)(void (Wnd::*)(UINT, UINT, UINT))&OnChar },

#define ON_WM_GETDLGCODE() \
  { WM_GETDLGCODE, 0, 0, 0, WndSig_wv, (WND_PMSG)(UINT (Wnd::*)())&OnGetDlgCode },

#define ON_WM_SETTINGCHANGE() \
  { WM_SETTINGCHANGE, 0, 0, 0, WndSig_vws, (WND_PMSG)(void (Wnd::*)(UINT, LPCTSTR))&OnSettingChange },

#define ON_WM_DRAWITEM() \
  { WM_DRAWITEM, 0, 0, 0, WndSig_vwLPDRAWITEMSTRUCT, (WND_PMSG)(void (Wnd::*)(int, LPDRAWITEMSTRUCT))&OnDrawItem },

#define ON_WM_TIMER() \
  { WM_TIMER, 0, 0, 0, WndSig_vw, (WND_PMSG)(void (Wnd::*)(UINT))&OnTimer },

#define ON_WM_KEYDOWN() \
  { WM_KEYDOWN, 0, 0, 0, WndSig_vwww, (WND_PMSG)(void (Wnd::*)(UINT, UINT, UINT))&OnKeyDown },

#define ON_WM_KEYUP() \
  { WM_KEYUP, 0, 0, 0, WndSig_vwww, (WND_PMSG)(void (Wnd::*)(UINT, UINT, UINT))&OnKeyUp },

#define ON_NOTIFY(code, id, memberFxn) \
  { WM_NOTIFY, (WORD)(int)code, (WORD)id, (WORD)id, WndSig_vNMHDRpl, (WND_PMSG)(void (Wnd::*)(NMHDR *, LRESULT *))&memberFxn },

#define ON_NOTIFY_RANGE(code, id, idLast, memberFxn) \
  { WM_NOTIFY, (WORD)(int)code, (WORD)id, (WORD)idLast, WndSig_vwNMHDRpl, (WND_PMSG)(void (Wnd::*)(UINT, NMHDR *, LRESULT *))&memberFxn },
  
#define ON_MESSAGE(message, memberFxn) \
  { message, 0, 0, 0, WndSig_lwl, (WND_PMSG)(LRESULT (Wnd::*)(WPARAM, LPARAM))&memberFxn },  
  
class Wnd {
  DECLARE_MESSAGE_MAP();
 private:
  static MapHWND map;
  
 protected:
  HWND hWnd;
  WNDPROC oldWndProc;
  bool autoDelete;
  int flags;
  int modalResult;

 public:
  static BOOL WalkPreTranslateTree(HWND stop, MSG *msg);
  virtual BOOL PreTranslateMessage(MSG *msg);
  Wnd();
  virtual ~Wnd();
  
  operator HWND() const { return this == NULL ? NULL : hWnd; }

  HWND getHandle() { return hWnd; }
  void setAutoDelete(bool a) { autoDelete = a; }
  void trace(char *, ...);
  
  void Attach(HWND h);
  void Detach();
  void SetOldWndProc(WNDPROC old);
  void SubClassWindow(HWND h);
  void SubClassDlgItem(UINT id, Wnd *parent);
  BOOL InvalidateRect(CONST RECT *rect, BOOL erase);
  BOOL Invalidate(CONST RECT *rect = NULL, BOOL erase = TRUE)
  {
    return InvalidateRect(rect, erase);
  }
    
  HWND GetDlgItem(UINT id);
  HDC GetDC();
  int ReleaseDC(HDC);
  BOOL DestroyWindow();
  BOOL SetWindowText(const char *);
  LONG SetWindowLong(int, LONG);
  LONG GetWindowLong(int);
  DWORD GetStyle() const
    { ASSERT(::IsWindow(hWnd)); return (DWORD)::GetWindowLong(hWnd, GWL_STYLE);
    }
  BOOL ShowWindow(int nCmdShow)
    { ASSERT(::IsWindow(hWnd)); return ::ShowWindow(hWnd, nCmdShow); }
  void UpdateWindow()
    { ASSERT(::IsWindow(hWnd)); ::UpdateWindow(hWnd); }  

  int RunModalLoop();
  BOOL ContinueModal();
  void EndModalLoop(int);
  BOOL PumpMessage(MSG&);
  
  static void Map(HWND h, Wnd *pWnd);
  static void UnMap(HWND h);
  static Wnd *FromMap(HWND h);

  virtual void OnChar(UINT, UINT, UINT);
  virtual void OnClose();
  virtual BOOL OnCommand(WPARAM, LPARAM);
  virtual int OnCreate(LPCREATESTRUCT lpCreateStruct);  
  virtual void OnDestroy();
  virtual void OnDrawItem(int, LPDRAWITEMSTRUCT);
  virtual BOOL OnEraseBkgnd(HDC);
  virtual UINT OnGetDlgCode();
  virtual void OnHScroll(UINT, UINT, HWND);
  virtual void OnKeyDown(UINT, UINT, UINT);
  virtual void OnKeyUp(UINT, UINT, UINT);
  virtual void OnKillFocus(HWND);
  virtual void OnLButtonDown(UINT, int, int);
  virtual void OnLButtonUp(UINT, int, int);
  virtual void OnMouseMove(UINT, int, int);
  virtual BOOL OnMsg(UINT, WPARAM, LPARAM, LRESULT&);  
  virtual BOOL OnNcCreate(LPCREATESTRUCT);
  virtual void OnNcDestroy();
  virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult);
  virtual void OnPaint();
  virtual BOOL OnSetCursor(HWND, UINT, UINT);
  virtual void OnSetFocus(HWND);
  virtual void OnSettingChange(UINT, LPCTSTR);
  virtual void OnSize(UINT, int, int);
  virtual void OnSizing(UINT, RECT *);
  virtual void OnTimer(UINT);
  virtual void OnVScroll(UINT, UINT, HWND);  

  virtual void PreSubclassWindow();
  
  LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};

class Dlg : public Wnd {
 protected:
  DECLARE_MESSAGE_MAP()
 public:
  virtual BOOL PreTranslateMessage(MSG *msg);
  Dlg();
  virtual ~Dlg();

  void DoCheckbox(bool get, int id, int& value);
  void DoRadio(bool get, int id, int& value);
  int DoModal(UINT resource, HWND parent);
  int DoModal(UINT resource, HWND parent, LPARAM param);
  void MakeDialog(HINSTANCE hInstance, UINT resource, HWND parent);
  void EndDialog(int);
  virtual BOOL OnInitDialog(LPARAM);
  virtual void OnClose();
};

#endif
