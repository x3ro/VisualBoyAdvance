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
#include "../System.h"

class IMemoryViewerDlg {
 public:
  virtual void setCurrentAddress(u32 address)=0;
};

class MemoryViewer : public Wnd {
  u32 address;
  int addressSize;
  int dataSize;
  bool hasCaret;
  int caretWidth;
  int caretHeight;
  HFONT font;
  SIZE fontSize;
  u32 editAddress;
  int editNibble;
  int maxNibble;
  int displayedLines;
  int beginAscii;
  int beginHex;
  bool editAscii;
  IMemoryViewerDlg *dlg;

  static bool isRegistered;
protected:
  DECLARE_MESSAGE_MAP()
public:
  void beep();
  void setDialog(IMemoryViewerDlg *);
  BOOL OnEditInput(UINT c);
  LRESULT OnWMChar(WPARAM wParam, LPARAM lParam);
  
  void setCaretPos();
  void destroyEditCaret();
  void createEditCaret(int w, int h);
  void moveAddress(s32 off, int nibble);
  MemoryViewer();

  static void registerClass();

  virtual void readData(u32,int,u8 *) = 0;
  virtual void editData(u32,int,int,u32)=0;
  void setAddress(u32);
  void setAddressSize(int s) { addressSize = s; }
  u32 getCurrentAddress() { return editAddress; }
  void setSize(int);
  int getSize() { return dataSize; }
  void updateScrollInfo(int lines);

  virtual BOOL OnEraseBkgnd(HDC);
  virtual void OnPaint();
  virtual void OnVScroll(UINT, UINT, HWND);
  virtual void OnKeyDown(UINT virtKey, UINT, UINT);
  virtual void OnKillFocus(HWND next);
  virtual void OnSetFocus(HWND old);
  virtual void OnLButtonDown(UINT flags, int x, int y);
  virtual UINT OnGetDlgCode();
};

class MemoryViewerAddressSize : public Dlg {
  u32 address;
  int size;
protected:
  DECLARE_MESSAGE_MAP()
public:
  MemoryViewerAddressSize();
  
  void setAddress(u32 a) { address = a; }
  void setSize(int s) { size = s; }
  u32 getAddress() { return address; }
  int getSize() { return size; }

  virtual BOOL OnInitDialog(LPARAM);  
  void OnOk();
  void OnCancel();
};
