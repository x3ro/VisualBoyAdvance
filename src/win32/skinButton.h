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
#include "StdString.h"

class SkinButton : public Wnd {
  DECLARE_MESSAGE_MAP()
private:
  HBITMAP normalBmp;
  HBITMAP downBmp;
  HBITMAP overBmp;
  RECT rect;
  bool mouseOver;
  CStdString id;
  HRGN region;
  WORD idCommand;
  int buttonMask;
  int menu;
public:
  SkinButton();
  virtual ~SkinButton();

  BOOL Create(const char *, DWORD, const RECT&, HWND, UINT);

  void SetNormalBitmap(HBITMAP);
  void SetDownBitmap(HBITMAP);
  void SetOverBitmap(HBITMAP);
  void SetRect(const RECT &);
  void GetRect(RECT& r);
  void SetId(const char *);
  void SetRegion(HRGN);

  virtual BOOL OnEraseBkgnd(HDC);
  virtual void OnPaint();
  virtual LRESULT OnLButtonUp(WPARAM, LPARAM);
  virtual LRESULT OnLButtonDown(WPARAM, LPARAM);
  virtual LRESULT OnMouseMove(WPARAM, LPARAM);
  virtual LRESULT OnKillFocus(WPARAM, LPARAM);
  virtual LRESULT OnCaptureChanged(WPARAM, LPARAM);
  virtual LRESULT OnRButtonDown(WPARAM, LPARAM);
  virtual LRESULT OnMouseLeave(WPARAM, LPARAM);
};
