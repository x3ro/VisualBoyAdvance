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
// AccelEditor.h: interface for the AccelEditor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ACCELEDITOR_H__E9025177_3B01_409A_9714_7E6C9BD3C281__INCLUDED_)
#define AFX_ACCELEDITOR_H__E9025177_3B01_409A_9714_7E6C9BD3C281__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Wnd.h"
#include "ResizeDlg.h"
#include "AcceleratorManager.h" // Added by ClassView
#include "KeyboardEdit.h"       // Added by ClassView

class AccelEditor : public ResizeDlg  
{
 protected:
  DECLARE_MESSAGE_MAP()
    public:
  void Remove();
  void Assign();
  void Reset();
  CKeyboardEdit m_Key;
  void SelChangeCommands();
  CAcceleratorManager mgr;
  void InitCommands();
  virtual BOOL OnInitDialog(LPARAM);
  AccelEditor();
  virtual ~AccelEditor();
  void OnOk();
  void OnCancel();
};

#endif // !defined(AFX_ACCELEDITOR_H__E9025177_3B01_409A_9714_7E6C9BD3C281__INCLUDED_)
