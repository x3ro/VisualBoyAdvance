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
////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1998 by Thierry Maurel
// All rights reserved
//
// Distribute freely, except: don't remove my name from the source or
// documentation (don't take credit for my work), mark your changes (don't
// get me blamed for your possible bugs), don't alter or remove this
// notice.
// No warrantee of any kind, express or implied, is included with this
// software; use at your own risk, responsibility for damages (if any) to
// anyone resulting from the use of this software rests entirely with the
// user.
//
// Send bug reports, bug fixes, enhancements, requests, flames, etc., and
// I'll try to keep a version up to date.  I can be reached as follows:
//    tmaurel@caramail.com   (or tmaurel@hol.fr)
//
////////////////////////////////////////////////////////////////////////////////
// File    : KeyboardEdit.h
// Project : AccelsEditor
////////////////////////////////////////////////////////////////////////////////
// Version : 1.0                       * Authors : A.Lebatard + T.Maurel
// Date    : 17.08.98
//
// Remarks : 
//
////////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_KEYBOARDEDIT_H__88E35AB0_2E23_11D2_BA24_0060B0B5E151__INCLUDED_)
#define AFX_KEYBOARDEDIT_H__88E35AB0_2E23_11D2_BA24_0060B0B5E151__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// KeyboardEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CKeyboardEdit window

class CKeyboardEdit : public Wnd
{
// Construction
public:
        CKeyboardEdit();

// Attributes
public:
        bool m_bKeyDefined;

        WORD m_wVirtKey;
        bool m_bCtrlPressed;
        bool m_bAltPressed;
        bool m_bShiftPressed;

// Operations
public:
        bool GetAccelKey(WORD& wVirtKey, bool& bCtrl, bool& bAlt, bool& bShift);
        void ResetKey ();

protected:
        void DisplayKeyboardString ();

        public:
        virtual BOOL PreTranslateMessage(MSG* pMsg);

// Implementation
public:
        virtual ~CKeyboardEdit();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KEYBOARDEDIT_H__88E35AB0_2E23_11D2_BA24_0060B0B5E151__INCLUDED_)
