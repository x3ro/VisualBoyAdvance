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
// AccelEditor.cpp: implementation of the AccelEditor class.
//
//////////////////////////////////////////////////////////////////////

#include "Wnd.h"
#include "AccelEditor.h"
#include "KeyboardEdit.h"
#include "../System.h"
#include "resource.h"

extern CAcceleratorManager winAccelMgr;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(AccelEditor, ResizeDlg)
  ON_BN_CLICKED(IDC_RESET, Reset)
  ON_BN_CLICKED(IDC_ASSIGN, Assign)
  ON_BN_CLICKED(IDC_REMOVE, Remove)
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(ID_CANCEL, OnCancel)
  ON_CONTROL(LBN_SELCHANGE, IDC_COMMANDS, SelChangeCommands)
END_MESSAGE_MAP()

AccelEditor::AccelEditor()
{
  mgr = winAccelMgr;
}

AccelEditor::~AccelEditor()
{

}

BOOL AccelEditor::OnInitDialog(LPARAM)
{
  DIALOG_SIZER_START( sz )
    DIALOG_SIZER_ENTRY( IDC_STATIC1, DS_MoveX)
    DIALOG_SIZER_ENTRY( IDC_STATIC2, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_STATIC3, DS_MoveX | DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_ALREADY_AFFECTED, DS_MoveY)
    DIALOG_SIZER_ENTRY( ID_OK, DS_MoveX)
    DIALOG_SIZER_ENTRY( ID_CANCEL, DS_MoveX)
    DIALOG_SIZER_ENTRY( IDC_ASSIGN, DS_MoveX)
    DIALOG_SIZER_ENTRY( IDC_REMOVE, DS_MoveX)
    DIALOG_SIZER_ENTRY( IDC_RESET, DS_MoveX)
    DIALOG_SIZER_ENTRY( IDC_CLOSE, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_COMMANDS, DS_SizeX | DS_SizeY)
    DIALOG_SIZER_ENTRY( IDC_CURRENTS, DS_MoveX | DS_SizeY)
    DIALOG_SIZER_ENTRY( IDC_EDIT_KEY, DS_MoveX | DS_MoveY)
  DIALOG_SIZER_END()
  SetData(sz,
    TRUE,
    HKEY_CURRENT_USER,
    "Software\\Emulators\\VisualBoyAdvance\\Viewer\\AccelEditor",
    NULL);

  m_Key.SubClassDlgItem(IDC_EDIT_KEY, this);
  InitCommands();

  return TRUE;
}

void AccelEditor::InitCommands()
{

  HWND h = GetDlgItem(IDC_COMMANDS);

  SendMessage(h, LB_RESETCONTENT, 0, 0);
  ::SetWindowText(GetDlgItem(IDC_ALREADY_AFFECTED), "");

  CMapStringToWord::iterator it = mgr.m_mapAccelString.begin();

  while(it != mgr.m_mapAccelString.end()) {
    CStdString str = it->first;
    int index = SendMessage(h, LB_ADDSTRING, 0, (LPARAM)((LPCSTR)str));
    SendMessage(h, LB_SETITEMDATA, index, MAKELONG(it->second,0));
    it++;
  }

  // Update the currents accels associated with the selected command
  if (::SendMessage(h, LB_SETCURSEL, 0, 0) != LB_ERR)
    SelChangeCommands();
}

void AccelEditor::OnCancel()
{
  EndDialog(FALSE);
}

void AccelEditor::OnOk()
{
  EndDialog(TRUE);
}

void AccelEditor::SelChangeCommands()
{
  HWND h = GetDlgItem(IDC_COMMANDS);
  // Check if some commands exist.
  int index = SendMessage(h, LB_GETCURSEL, 0, 0);
  if (index == LB_ERR)
    return;

  WORD wIDCommand = LOWORD(SendMessage(h, LB_GETITEMDATA, index, 0));
  h = GetDlgItem(IDC_CURRENTS);
  SendMessage(h, LB_RESETCONTENT, 0, 0);

  CCmdAccelOb* pCmdAccel;
  CMapWordToCCmdAccelOb::iterator it = mgr.m_mapAccelTable.find(wIDCommand);
  if (it != mgr.m_mapAccelTable.end()) {
    CAccelsOb* pAccel;
    CStdString szBuffer;
    pCmdAccel = it->second;
    it++;
    std::list<CAccelsOb*>::iterator i = pCmdAccel->m_Accels.begin();
    // Add the keys to the 'currents keys' listbox.
    while (i != pCmdAccel->m_Accels.end()) {
      pAccel = *i;
      i++;
      pAccel->GetString(szBuffer);
      index = SendMessage(h, LB_ADDSTRING, 0, (LPARAM)((LPCSTR)szBuffer));
      // and a pointer to the accel object.
      SendMessage(h, LB_SETITEMDATA, index, (LPARAM)pAccel);
    }
  }
  // Init the key editor
//  m_pKey->ResetKey();

}

void AccelEditor::Reset()
{
  mgr.Default();
  InitCommands(); // update the listboxes.
}

void AccelEditor::Assign()
{
  // Control if it's not already affected
  CCmdAccelOb* pCmdAccel;
  CAccelsOb* pAccel;
  WORD wIDCommand;
  
  WORD wKey;
  bool bCtrl, bAlt, bShift;

  if (!m_Key.GetAccelKey(wKey, bCtrl, bAlt, bShift))
    return; // no valid key, abort

  HWND h = GetDlgItem(IDC_COMMANDS);

  int count = ::SendMessage(h, LB_GETCOUNT, 0, 0);
  for (int index = 0; index < count; index++) {

    wIDCommand = LOWORD(::SendMessage(h, LB_GETITEMDATA, index, 0));
    CMapWordToCCmdAccelOb::iterator it = mgr.m_mapAccelTable.find(wIDCommand);
    ASSERT(it != mgr.m_mapAccelTable.end());

    pCmdAccel = it->second;

    std::list<CAccelsOb*>::iterator it2 = pCmdAccel->m_Accels.begin();
    while (it2 != pCmdAccel->m_Accels.end()) {
      pAccel = *it2;
      it2++;
      if (pAccel->IsEqual(wKey, bCtrl, bAlt, bShift)) {
        // the key is already affected (in the same or other command)
        ::SetWindowText(GetDlgItem(IDC_ALREADY_AFFECTED), pCmdAccel->m_szCommand);
        ::SendMessage(m_Key, EM_SETSEL, 0, -1);
        return; // abort
      }
    }
  }

  // OK, we can add the accel key in the currently selected group
  index = ::SendMessage(h, LB_GETCURSEL, 0, 0);
  if (index == LB_ERR)
    return;

  // Get the object who manage the accels list, associated to the command.
  wIDCommand = LOWORD(::SendMessage(h, LB_GETITEMDATA, index, 0));

  CMapWordToCCmdAccelOb::iterator it = mgr.m_mapAccelTable.find(wIDCommand);

  if (it == mgr.m_mapAccelTable.end())
    return;

  pCmdAccel = it->second;
  
  BYTE cVirt = 0;
  if (bCtrl)
    cVirt |= FCONTROL;
  if (bAlt)
    cVirt |= FALT;
  if (bShift)
    cVirt |= FSHIFT;

  cVirt |= FVIRTKEY;

  // Create the new key...
  pAccel = new CAccelsOb(cVirt, wKey, false);
  ASSERT(pAccel != NULL);
  // ...and add in the list.
  pCmdAccel->m_Accels.push_back(pAccel);

  // Update the listbox.
  CStdString szBuffer;
  pAccel->GetString(szBuffer);
  h = GetDlgItem(IDC_CURRENTS);

  index = ::SendMessage(h, LB_ADDSTRING, 0, (LPARAM)((LPCSTR)szBuffer));
  ::SendMessage(h, LB_SETITEMDATA, index, (LPARAM)pAccel);

  // Reset the key editor.
  m_Key.ResetKey();

}

void AccelEditor::Remove()
{
  HWND h = GetDlgItem(IDC_CURRENTS);
  HWND h2 = GetDlgItem(IDC_COMMANDS);
  // Some controls
  int indexCurrent = ::SendMessage(h, LB_GETCURSEL, 0, 0);
  if (indexCurrent == LB_ERR)
    return;
  
  // 2nd part.
  int indexCmd = ::SendMessage(h2, LB_GETCURSEL, 0, 0);
  if (indexCmd == LB_ERR)
    return;

  // Ref to the ID command
  WORD wIDCommand = LOWORD(::SendMessage(h2, LB_GETITEMDATA, indexCmd, 0));

  // Run through the accels,and control if it can be deleted.
  CCmdAccelOb* pCmdAccel;
  CMapWordToCCmdAccelOb::iterator it = mgr.m_mapAccelTable.find(wIDCommand);

  if (it != mgr.m_mapAccelTable.end()) {
    pCmdAccel = it->second;
    CAccelsOb* pAccel;
    CAccelsOb* pAccelCurrent = (CAccelsOb*)(::SendMessage(h, LB_GETITEMDATA, indexCurrent, 0));
    CStdString szBuffer;
    std::list<CAccelsOb*>::iterator it2 = pCmdAccel->m_Accels.begin();
    while (it2 != pCmdAccel->m_Accels.end()) {
      pAccel = *it2;
      if (pAccel == pAccelCurrent) {
        if (!pAccel->m_bLocked) {
          // not locked, so we delete the key
          pCmdAccel->m_Accels.erase(it2);
          delete pAccel;
          // and update the listboxes/key editor/static text
          ::SendMessage(h, LB_DELETESTRING, indexCurrent, 0);
          m_Key.ResetKey();
          ::SetWindowText(GetDlgItem(IDC_ALREADY_AFFECTED), "");
          return;
        } else {
          systemMessage(0,"Unable to remove this\naccelerator (Locked)");
          return;
        }
      }
      it2++;
    }
    systemMessage(0,"internal error (CAccelDlgHelper::Remove : pAccel unavailable)");
    return;
  }
  systemMessage(0,"internal error (CAccelDlgHelper::Remove : Lookup failed)");


}

extern HWND hWindow;
extern HINSTANCE hInstance;

void toolsCustomize()
{
  AccelEditor dlg;

  if(dlg.DoModal(IDD_ACCEL_EDITOR, hWindow)) {
    winAccelMgr = dlg.mgr;
    winAccelMgr.UpdateWndTable();
    winAccelMgr.Write();
  }
}
