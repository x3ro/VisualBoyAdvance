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
// File    : AcceleratorManager.cpp
// Project : AccelsEditor
////////////////////////////////////////////////////////////////////////////////
// Version : 1.0                       * Author : T.Maurel
// Date    : 17.08.98
//
// Remarks : implementation of the CAcceleratorManager class.
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "resource.h"
#include "../System.h"

#include "Wnd.h"
#include "AcceleratorManager.h"
#include "Reg.h"

#include <vector>

extern HACCEL hAccel;
extern HWND hWindow;

//////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////
//
//
CAcceleratorManager::CAcceleratorManager()
{
  m_hRegKey = HKEY_CURRENT_USER;
  m_szRegKey = "";
  m_bAutoSave = FALSE;
  hWndConnected = NULL;

  m_bDefaultTable = false;
}


//////////////////////////////////////////////////////////////////////
//
//
CAcceleratorManager::~CAcceleratorManager()
{
  if ((m_bAutoSave == true) && (m_szRegKey.IsEmpty() != false)) {
    //          bool bRet = Write();
    //          if (!bRet)
    //                  systemMessage(0, "CAcceleratorManager::~CAcceleratorManager\nError in CAcceleratorManager::Write...");
  }

  Reset();
}


//////////////////////////////////////////////////////////////////////
// Internal fcts
//////////////////////////////////////////////////////////////////////
//
//
void CAcceleratorManager::Reset()
{
  //CCmdAccelOb* pCmdAccel;
  //WORD wKey;
  CMapWordToCCmdAccelOb::iterator it = m_mapAccelTable.begin();

  while (it != m_mapAccelTable.end()) {
    delete it->second;
    it++;
  }
  m_mapAccelTable.clear();
  m_mapAccelString.clear();

  it = m_mapAccelTableSaved.begin();
  while (it != m_mapAccelTableSaved.end()) {
    delete it->second;
    it++;
  }
  m_mapAccelTableSaved.clear();
}


//////////////////////////////////////////////////////////////////////
//
//
bool CAcceleratorManager::AddAccel(BYTE cVirt, WORD wIDCommand, WORD wKey, LPCTSTR szCommand, bool bLocked)
{
  ASSERT(szCommand != NULL);

  //    WORD wIDCmd;
  CMapStringToWord::iterator it;

  it = m_mapAccelString.find((char *)szCommand);

  if (it != m_mapAccelString.end()) {
    if (it->second != wIDCommand)
      return false;
  }

  CCmdAccelOb* pCmdAccel = NULL;
  CMapWordToCCmdAccelOb::iterator it2;

  it2 = m_mapAccelTable.find(wIDCommand);

  if(it2 != m_mapAccelTable.end()) {
    pCmdAccel = it2->second;
    if(strcmp(pCmdAccel->m_szCommand, szCommand)) {
      return false;
    }
    CAccelsOb* pAccel;

    std::list<CAccelsOb *>::iterator it3 = pCmdAccel->m_Accels.begin();
    while (it3 != pCmdAccel->m_Accels.end()) {
      pAccel = *it3;
      if (pAccel->m_cVirt == cVirt &&
          pAccel->m_wKey == wKey)
        return FALSE;
      it3++;
    }
    // Adding the accelerator
    pCmdAccel->Add(cVirt, wKey, bLocked);

  } else {
    pCmdAccel = new CCmdAccelOb(cVirt, wIDCommand, wKey, szCommand, bLocked);
    ASSERT(pCmdAccel != NULL);
    m_mapAccelTable.insert(CMapWordToCCmdAccelOb::value_type(wIDCommand, pCmdAccel));
  }
  // 2nd table
  m_mapAccelString.insert(CMapStringToWord::value_type((char *)szCommand, wIDCommand));
  return true;
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
//
//
void CAcceleratorManager::Connect(HWND pWnd, bool bAutoSave)
{
  ASSERT(hWndConnected == NULL);
  hWndConnected = pWnd;
  m_bAutoSave = bAutoSave;
}


//////////////////////////////////////////////////////////////////////
//
//
bool CAcceleratorManager::GetRegKey(HKEY& hRegKey, CStdString &szRegKey)
{
  if (m_szRegKey.IsEmpty())
    return false;

  hRegKey = m_hRegKey;
  szRegKey = m_szRegKey;
  return true;
}


//////////////////////////////////////////////////////////////////////
//
//
bool CAcceleratorManager::SetRegKey(HKEY hRegKey, LPCTSTR szRegKey)
{
  ASSERT(hRegKey != NULL);
  ASSERT(szRegKey != NULL);

  m_szRegKey = szRegKey;
  m_hRegKey = hRegKey;
  return true;
}


//////////////////////////////////////////////////////////////////////
// Update the application's ACCELs table
//////////////////////////////////////////////////////////////////////
//
//
bool CAcceleratorManager::UpdateWndTable()
{
  int iLoop = 0;
  std::vector<LPACCEL> arrayACCEL;

  CCmdAccelOb* pCmdAccel;
  WORD wKey;
  LPACCEL pACCEL;
  CAccelsOb* pAccelOb;
  CMapWordToCCmdAccelOb::iterator it = m_mapAccelTable.begin();
  while (it != m_mapAccelTable.end()) {
    wKey = it->first;
    pCmdAccel = it->second;
    std::list<CAccelsOb*>::iterator it2 = pCmdAccel->m_Accels.begin();

    while (it2 != pCmdAccel->m_Accels.end()) {
      pAccelOb = *it2;
      it2++;

      pACCEL = new ACCEL;
      ASSERT(pACCEL != NULL);
      pACCEL->fVirt = pAccelOb->m_cVirt;
      pACCEL->key = pAccelOb->m_wKey;
      pACCEL->cmd = pCmdAccel->m_wIDCommand;
      arrayACCEL.push_back(pACCEL);
    }

    it++;
  }
        
  int nAccel = arrayACCEL.size();
  LPACCEL lpAccel = (LPACCEL)LocalAlloc(LPTR, nAccel * sizeof(ACCEL));
  if (!lpAccel) {
    for (iLoop = 0; iLoop < nAccel; iLoop++)
      delete arrayACCEL.at(iLoop);
    arrayACCEL.clear();

    return false;
  }

  for (iLoop = 0; iLoop < nAccel; iLoop++) {
                
    pACCEL = arrayACCEL.at(iLoop);
    lpAccel[iLoop].fVirt = pACCEL->fVirt;
    lpAccel[iLoop].key = pACCEL->key;
    lpAccel[iLoop].cmd = pACCEL->cmd;

    delete pACCEL;
  }
  arrayACCEL.clear();

  HACCEL hNewTable = CreateAcceleratorTable(lpAccel, nAccel);
  if (!hNewTable) {
    ::LocalFree(lpAccel);
    return false;
  }
  HACCEL hOldTable = hAccel;
  if (!::DestroyAcceleratorTable(hOldTable)) {
    ::LocalFree(lpAccel);
    return false;
  }
  hAccel = hNewTable;
  ::LocalFree(lpAccel);

  UpdateMenu(GetMenu(hWindow));

  return true;
}


//////////////////////////////////////////////////////////////////////
// Create/Destroy accelerators
//////////////////////////////////////////////////////////////////////
//
//
bool CAcceleratorManager::DeleteAccel(BYTE cVirt, WORD wIDCommand, WORD wKey)
{
  CCmdAccelOb* pCmdAccel = NULL;
  CMapWordToCCmdAccelOb::iterator it = m_mapAccelTable.find(wIDCommand);

  if(it != m_mapAccelTable.end()) {
    pCmdAccel = it->second;
    std::list<CAccelsOb*>::iterator it2 = pCmdAccel->m_Accels.begin();
    CAccelsOb* pAccel = NULL;
    while (it2 != pCmdAccel->m_Accels.end()) {
      pAccel = *it2;
      if (pAccel->m_bLocked == true)
        return false;

      if (pAccel->m_cVirt == cVirt && pAccel->m_wKey == wKey) {
        pCmdAccel->m_Accels.erase(it2);
        delete pAccel;
        return true;
      }
      it2++;
    }
  }
  return false;
}


//////////////////////////////////////////////////////////////////////
//
//
bool CAcceleratorManager::DeleteEntry(WORD wIDCommand)
{
  CCmdAccelOb* pCmdAccel = NULL;
  CMapWordToCCmdAccelOb::iterator it = m_mapAccelTable.find(wIDCommand);

  ASSERT(it != m_mapAccelTable.end());

  CAccelsOb* pAccel;

  std::list<CAccelsOb *>::iterator it2 = pCmdAccel->m_Accels.begin();
  while (it2 != pCmdAccel->m_Accels.end() ) {
    pAccel = *it2;
    if (pAccel->m_bLocked == true)
      return false;
    it2++;
  }
  m_mapAccelString.erase(pCmdAccel->m_szCommand);
  m_mapAccelTable.erase(wIDCommand);
  delete pCmdAccel;

  return true;
}


//////////////////////////////////////////////////////////////////////
//
//
bool CAcceleratorManager::DeleteEntry(LPCTSTR szCommand)
{
  ASSERT(szCommand != NULL);

  //    WORD wIDCommand;
  CMapStringToWord::iterator it = m_mapAccelString.find((char *)szCommand);
  if (it != m_mapAccelString.end()) {
    return DeleteEntry(it->second);
  }
  return true;
}


//////////////////////////////////////////////////////////////////////
//
//
bool CAcceleratorManager::SetAccel(BYTE cVirt, WORD wIDCommand, WORD wKey, LPCTSTR szCommand, bool bLocked)
{
  ASSERT(szCommand != NULL);

  return AddAccel(cVirt, wIDCommand, wKey, szCommand, bLocked);
}


//////////////////////////////////////////////////////////////////////
//
//
bool CAcceleratorManager::AddCommandAccel(WORD wIDCommand, LPCTSTR szCommand, bool bLocked)
{
  ASSERT(szCommand != NULL);

  ASSERT(hWndConnected != NULL);
  HACCEL hOriginalTable = hAccel;
  ASSERT(hAccel != NULL);

  int nAccel = ::CopyAcceleratorTable(hOriginalTable, NULL, 0);
  LPACCEL lpAccel = (LPACCEL)LocalAlloc(LPTR, (nAccel) * sizeof(ACCEL));
  if (!lpAccel)
    return false;
  ::CopyAcceleratorTable(hOriginalTable, lpAccel, nAccel);

  bool bRet = false;
  for (int i = 0; i < nAccel; i++) {
    if (lpAccel[i].cmd == wIDCommand)
      bRet = AddAccel(lpAccel[i].fVirt, wIDCommand, lpAccel[i].key, szCommand, bLocked);
  }
  ::LocalFree(lpAccel);
  return bRet;
}


//////////////////////////////////////////////////////////////////////
//
//
bool CAcceleratorManager::CreateEntry(WORD wIDCommand, LPCTSTR szCommand)
{
  ASSERT(szCommand != NULL);

  //    WORD wIDDummy;
  CMapStringToWord::iterator it = m_mapAccelString.find((char *)szCommand);

  if ( it != m_mapAccelString.end())
    return false;

  CCmdAccelOb* pCmdAccel = new CCmdAccelOb(wIDCommand, szCommand);
  ASSERT(pCmdAccel != NULL);
  m_mapAccelTable.insert(CMapWordToCCmdAccelOb::value_type(wIDCommand, pCmdAccel));
  m_mapAccelString.insert(CMapStringToWord::value_type((char *)szCommand, wIDCommand));

  return false;
}


//////////////////////////////////////////////////////////////////////
// Get a string from the ACCEL definition
//////////////////////////////////////////////////////////////////////
//
//
bool CAcceleratorManager::GetStringFromACCEL(ACCEL* pACCEL, CStdString& szAccel)
{
  ASSERT(pACCEL != NULL);
        
  CAccelsOb accel(pACCEL);
  accel.GetString(szAccel);

  if (szAccel.IsEmpty())
    return false;
  else
    return true;
}


//////////////////////////////////////////////////////////////////////
//
//
bool CAcceleratorManager::GetStringFromACCEL(BYTE cVirt, WORD nCode, CStdString& szAccel)
{
  CAccelsOb accel(cVirt, nCode);
  accel.GetString(szAccel);

  if (szAccel.IsEmpty())
    return false;
  else
    return true;
}


//////////////////////////////////////////////////////////////////////
// Copy function
//
CAcceleratorManager& CAcceleratorManager::operator=(const CAcceleratorManager& accelmgr)
{
  Reset();

  CCmdAccelOb* pCmdAccel;
  CCmdAccelOb* pNewCmdAccel;
  WORD wKey;
  // Copy the 2 tables : normal accel table...
  CMapWordToCCmdAccelOb::const_iterator it = accelmgr.m_mapAccelTable.begin();

  while (it != accelmgr.m_mapAccelTable.end()) {
    wKey = it->first;
    pCmdAccel = it->second;
    pNewCmdAccel = new CCmdAccelOb;
    ASSERT(pNewCmdAccel != NULL);
    *pNewCmdAccel = *pCmdAccel;
    m_mapAccelTable.insert(CMapWordToCCmdAccelOb::value_type(wKey, pNewCmdAccel));
    it++;
  }
  // ... and saved accel table.
  it = accelmgr.m_mapAccelTableSaved.begin();
  while (it != accelmgr.m_mapAccelTableSaved.end()) {
    wKey = it->first;
    pCmdAccel = it->second;
    pNewCmdAccel = new CCmdAccelOb;
    ASSERT(pNewCmdAccel != NULL);
    *pNewCmdAccel = *pCmdAccel;
    m_mapAccelTableSaved.insert(CMapWordToCCmdAccelOb::value_type(wKey, pNewCmdAccel));
    it++;
  }

  // The Strings-ID table
  CStdString szKey;
  CMapStringToWord::const_iterator it2 = accelmgr.m_mapAccelString.begin();
  while (it2 != accelmgr.m_mapAccelString.end()) {
    szKey = it2->first;
    wKey = it2->second;
    m_mapAccelString.insert(CMapStringToWord::value_type(szKey, wKey));
    it2++;
  }
  m_bDefaultTable = accelmgr.m_bDefaultTable;

  return *this;
}

void CAcceleratorManager::UpdateMenu(HMENU menu)
{
  int count = GetMenuItemCount(menu);

  MENUITEMINFO info;
  wchar_t ss[128];
  ZeroMemory(&info, sizeof(info));
  info.cbSize = sizeof(info);
  info.fMask = MIIM_ID | MIIM_SUBMENU;
  for(int i = 0; i < count; i++) {
    GetMenuItemInfo(menu, i, TRUE, &info);

    if(info.hSubMenu != NULL) {
      UpdateMenu(info.hSubMenu);
    } else {
      if(info.wID != -1) {
        MENUITEMINFOW info2;
        ZeroMemory(&info2, sizeof(info2));
        info2.cbSize = sizeof(info2);
        info2.fMask = MIIM_STRING;
        info2.dwTypeData = ss;
        info2.cch = 128;
        GetMenuItemInfoW(menu, i, MF_BYPOSITION, &info2);
        CStdStringW str = ss;
        int index = str.Find('\t');
        if(index != -1)
          str = str.Left(index);

        CMapWordToCCmdAccelOb::iterator it = m_mapAccelTable.find(info.wID);

        if(it != m_mapAccelTable.end()) {
          CCmdAccelOb *o = it->second;
          if(o->m_Accels.begin() != o->m_Accels.end()) {
            std::list<CAccelsOb*>::iterator j = o->m_Accels.begin();

            CAccelsOb *accel = *j;

            CStdString s;
            accel->GetString(s);
            str += "\t";
            str += s;
          }
        }
        if(str != ss)
          ModifyMenuW(menu, i, MF_BYPOSITION | MF_STRING, info.wID, str);
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////
// In/Out to the registry
//////////////////////////////////////////////////////////////////////
//
//
bool CAcceleratorManager::Load(HKEY hRegKey, LPCTSTR szRegKey)
{
  ASSERT(szRegKey != NULL);
  HKEY hKey;

  m_hRegKey = hRegKey;
  m_szRegKey = szRegKey;

  DWORD data[2048/sizeof(DWORD)];

  DWORD len = sizeof(data);
  if(regQueryBinaryValue("keyboard", (char *)data, len)) {
    int count = len/sizeof(DWORD);

    CCmdAccelOb* pCmdAccel;
    CAccelsOb* pAccel;
    DWORD dwIDAccelData, dwAccelData;
    
    int iIndex = 0;
    if(count) {
      while(iIndex < count) {
        dwIDAccelData = data[iIndex++];
        
        WORD wIDCommand = LOWORD(dwIDAccelData);
        CMapWordToCCmdAccelOb::iterator it = m_mapAccelTable.find(wIDCommand);
        bool bExistID = (it != m_mapAccelTable.end());
        
        if (bExistID) {
          pCmdAccel = it->second;
          pCmdAccel->DeleteUserAccels();
        }
        for (int j = 0; j < HIWORD(dwIDAccelData) && iIndex < count; j++) {
          dwAccelData = data[iIndex++];
          if (bExistID) {
            pAccel = new CAccelsOb;
            ASSERT(pAccel != NULL);
            pAccel->SetData(dwAccelData);
            pCmdAccel->Add(pAccel);
          }
        }
      }
    }
    UpdateWndTable();
    return true;
  }
  return false;
}


//////////////////////////////////////////////////////////////////////
//
//
bool CAcceleratorManager::Load()
{
  BOOL bRet = FALSE;
  if (!m_szRegKey.IsEmpty())
    bRet = Load(m_hRegKey, m_szRegKey);

  if (bRet == TRUE)
    return true;
  else
    return false;
}


//////////////////////////////////////////////////////////////////////
//
//
bool CAcceleratorManager::Write()
{
  std::vector<DWORD> AccelsDatasArray;
  std::vector<DWORD> CmdDatasArray;

  int iCount = 0;
  CCmdAccelOb* pCmdAccel;
  CAccelsOb* pAccel;
  DWORD dwAccelData;

  WORD wKey;
  CMapWordToCCmdAccelOb::iterator it = m_mapAccelTable.begin();
  while (it != m_mapAccelTable.end()) {
    wKey = it->first;
    pCmdAccel = it->second;

    CmdDatasArray.clear();

    std::list<CAccelsOb*>::iterator it2 = pCmdAccel->m_Accels.begin();
    while (it2 != pCmdAccel->m_Accels.end()) {
      pAccel = *it2;
      //                        if (!pAccel->m_bLocked) {
      dwAccelData = pAccel->GetData();
      CmdDatasArray.push_back(dwAccelData);
      //}
      it2++;
    }
    if (CmdDatasArray.size() > 0) {
      CmdDatasArray.insert(CmdDatasArray.begin(), MAKELONG(pCmdAccel->m_wIDCommand, CmdDatasArray.size()));
                        
      AccelsDatasArray.insert(AccelsDatasArray.end(), CmdDatasArray.begin(), CmdDatasArray.end());
      iCount++;
    }
    it++;
  }
  //    AccelsDatasArray.insert(AccelsDataArray.begin(), MAKELONG(65535, iCount));
  
  int count = AccelsDatasArray.size();
  DWORD *data = (DWORD *)malloc(count * sizeof(DWORD));
  ASSERT(data != NULL);
  std::vector<DWORD>::iterator it3 = AccelsDatasArray.begin();
  int index = 0;
  while(it3 != AccelsDatasArray.end()) {
    data[index] = AccelsDatasArray[index];
    index++;
    it3++;
  }

  regSetBinaryValue("keyboard", (char *)data, count*sizeof(DWORD));

  AccelsDatasArray.clear();
  CmdDatasArray.clear();

  free(data);

  return true;
}


//////////////////////////////////////////////////////////////////////
// Defaults values management.
//////////////////////////////////////////////////////////////////////
//
//
bool CAcceleratorManager::CreateDefaultTable()
{
  if (m_bDefaultTable)
    return false;
        
  CCmdAccelOb* pCmdAccel;
  CCmdAccelOb* pNewCmdAccel;

  CAccelsOb* pAccel;
  CAccelsOb* pNewAccel;

  WORD wKey;
  CMapWordToCCmdAccelOb::iterator it = m_mapAccelTable.begin();
  while (it != m_mapAccelTable.end()) {
    wKey = it->first;
    pCmdAccel = it->second;
    it++;
    pNewCmdAccel = new CCmdAccelOb;
    ASSERT(pNewCmdAccel != NULL);

    std::list<CAccelsOb*>::iterator it2 = pCmdAccel->m_Accels.begin();
    while (it2 != pCmdAccel->m_Accels.end()) {
      pAccel = *it2;
      it2++;
      if (!pAccel->m_bLocked) {
        pNewAccel = new CAccelsOb;
        ASSERT(pNewAccel != NULL);

        *pNewAccel = *pAccel;
        pNewCmdAccel->m_Accels.push_back(pNewAccel);
      }
    }
    if (pNewCmdAccel->m_Accels.size() != 0) {
      pNewCmdAccel->m_wIDCommand = pCmdAccel->m_wIDCommand;
      pNewCmdAccel->m_szCommand = pCmdAccel->m_szCommand;

      m_mapAccelTableSaved.insert(CMapWordToCCmdAccelOb::value_type(wKey, pNewCmdAccel));
    } else 
      delete pNewCmdAccel;
  }

  m_bDefaultTable = true;
  return true;
}


//////////////////////////////////////////////////////////////////////
//
//
bool CAcceleratorManager::Default()
{
  CCmdAccelOb* pCmdAccel;
  CCmdAccelOb* pSavedCmdAccel;

  CAccelsOb* pAccel;
  CAccelsOb* pSavedAccel;

  WORD wKey;

  CMapWordToCCmdAccelOb::iterator it = m_mapAccelTable.begin();

  while(it != m_mapAccelTable.end()) {
    wKey = it->first;
    pCmdAccel = it->second;
    pCmdAccel->DeleteUserAccels();
    it++;
  }

// @ Add the saved Accelerators
  it = m_mapAccelTableSaved.begin();
        while (it != m_mapAccelTableSaved.end()) {
    wKey = it->first;
    pSavedCmdAccel = it->second;
    it++;

    pCmdAccel = m_mapAccelTable.find(wKey)->second;

                // @ Removed by kukac  pCmdAccel->DeleteUserAccels();
    std::list<CAccelsOb*>::iterator it = pSavedCmdAccel->m_Accels.begin();
                while (it != pSavedCmdAccel->m_Accels.end()) {
                        pSavedAccel = *it;
      it++;
                        pAccel = new CAccelsOb(pSavedAccel);
                        ASSERT(pAccel != NULL);
                        pCmdAccel->m_Accels.push_back(pAccel);
                }
        }

  return true;
}
