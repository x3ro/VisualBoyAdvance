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
// FileDlg.h: interface for the FileDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMMDLG_H__7800E1B3_BAD5_4C2B_87E1_5A33B415F56D__INCLUDED_)
#define AFX_COMMDLG_H__7800E1B3_BAD5_4C2B_87E1_5A33B415F56D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Wnd.h"

class FileDlg : public Dlg  
{
 public:
  virtual int getFilterIndex();
  virtual void OnTypeChange();
  virtual BOOL OnNotify(WPARAM, LPARAM, LRESULT *);
  virtual BOOL DoModal();
  virtual BOOL OnInitDialog(LPARAM);
  FileDlg(HWND parent, char *file, int maxFile, char *filter,
	  int filterIndex, char *ext, char **exts, char *initialDir, 
	  char *title, bool save);
  virtual ~FileDlg();
  
 protected:
  bool isSave;
  OPENFILENAME ofn;
  char **extensions;
};

class ColorDlg : public Dlg {
  CHOOSECOLOR cc;
  static COLORREF customColors[16];
 public:
  ColorDlg(COLORREF clrInit, DWORD dwFlags, Wnd *parent);

  virtual int DoModal();

  COLORREF GetColor() const;
};

#endif // !defined(AFX_COMMDLG_H__7800E1B3_BAD5_4C2B_87E1_5A33B415F56D__INCLUDED_)
