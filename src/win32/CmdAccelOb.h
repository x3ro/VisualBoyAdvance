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
// File    : CmdAccelOb.h
// Project : AccelsEditor
////////////////////////////////////////////////////////////////////////////////
// Version : 1.0                       * Author : T.Maurel
// Date    : 17.08.98
//
// Remarks : 
//
////////////////////////////////////////////////////////////////////////////////
#ifndef __CMDACCEL_OB_INCLUDE
#define __CMDACCEL_OB_INCLUDE

#pragma warning(disable:4786)
#include <list>

#include "StdString.h"

////////////////////////////////////////////////////////////////////////
//
//
typedef struct tagMAPVIRTKEYS {
        WORD wKey;
        TCHAR szKey[15];
} MAPVIRTKEYS, *PMAPVIRTKEYS;


////////////////////////////////////////////////////////////////////////
//
//
#define sizetable(table) (sizeof(table)/sizeof(table[0]))


////////////////////////////////////////////////////////////////////////
//
//
class CAccelsOb {
public:
        CAccelsOb();
        CAccelsOb(CAccelsOb* pFrom);
        CAccelsOb(BYTE cVirt, WORD wKey, bool bLocked = false);
        CAccelsOb(LPACCEL pACCEL);

public:
        CAccelsOb& operator=(const CAccelsOb& from);

        void GetString(CStdString& szBuffer);
        bool IsEqual(WORD wKey, bool bCtrl, bool bAlt, bool bShift);
        DWORD GetData();
        bool SetData(DWORD dwDatas);

public:
        BYTE m_cVirt;
        WORD m_wKey;
        bool m_bLocked;
};


//////////////////////////////////////////////////////////////////////
//
//
class CCmdAccelOb {
public:
        CCmdAccelOb();
        CCmdAccelOb(WORD wIDCommand, LPCTSTR szCommand);
        CCmdAccelOb(BYTE cVirt, WORD wIDCommand, WORD wKey, LPCTSTR szCommand, bool bLocked = false);
        ~CCmdAccelOb();

public:
        void Add(CAccelsOb* pAccel);
        void Add(BYTE cVirt, WORD wKey, bool bLocked = false);
        void Reset();
        void DeleteUserAccels();

        CCmdAccelOb& operator=(const CCmdAccelOb& from);


public:
        WORD m_wIDCommand;
        CStdString m_szCommand;

        std::list<CAccelsOb *> m_Accels;
};


////////////////////////////////////////////////////////////////////////
#endif // __CMDACCEL_OB_INCLUDE


