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
#include "ExportGSASnapshot.h"

#include "../GBA.h"
#include "../NLS.h"

extern void winCenterWindow(HWND h);
extern int emulating;
extern int cartridgeType;

BEGIN_MESSAGE_MAP(ExportGSASnapshot, Dlg)
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(ID_CANCEL, OnCancel)
END_MESSAGE_MAP()  

ExportGSASnapshot::ExportGSASnapshot(char *f, char *t)
  : Dlg()
{
  filename = f;
  title = t;
}

BOOL ExportGSASnapshot::OnInitDialog(LPARAM)
{
  ::SetWindowText(GetDlgItem(IDC_TITLE), title);
  char date[100];
  char time[100];
  char desc[100];

  GetDateFormat(LOCALE_USER_DEFAULT,
                DATE_SHORTDATE,
                NULL,
                NULL,
                date,
                100);
  GetTimeFormat(LOCALE_USER_DEFAULT,
                0,
                NULL,
                NULL,
                time,
                100);
  sprintf(desc, "%s %s", date, time);
  ::SetWindowText(GetDlgItem(IDC_DESC), desc);

  SendMessage(GetDlgItem(IDC_TITLE), EM_LIMITTEXT, 100, 0);
  SendMessage(GetDlgItem(IDC_DESC), EM_LIMITTEXT, 100, 0);
  SendMessage(GetDlgItem(IDC_NOTES), EM_LIMITTEXT, 512, 0);
  
  winCenterWindow(hWnd);
  
  return TRUE;
}

void ExportGSASnapshot::OnOk()
{
  char title[100];
  char desc[100];
  char notes[512];

  GetWindowText(GetDlgItem(IDC_TITLE), title, 100);
  GetWindowText(GetDlgItem(IDC_DESC), desc, 100);
  GetWindowText(GetDlgItem(IDC_NOTES), notes, 512);
  
  bool result = CPUWriteGSASnapshot(filename, title, desc, notes);
  
  if(!result)
    systemMessage(MSG_ERROR_CREATING_FILE, "Error creating file %s",
                  filename);
  
  EndDialog(TRUE);
}

void ExportGSASnapshot::OnCancel()
{
  EndDialog(FALSE);
}

extern HWND hWindow;

void fileExportSPSSnapshot(char *filename, char *title)
{

  bool result = false;

  ExportGSASnapshot dlg(filename, title);

  dlg.DoModal(IDD_EXPORT_SPS, hWindow);
}
