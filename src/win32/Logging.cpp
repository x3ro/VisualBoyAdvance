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
#include "ResizeDlg.h"
#include "CommDlg.h"
#include "Reg.h"
#include "WinResUtil.h"
#include "resource.h"
#include "../System.h"

extern HWND hWindow;
extern HINSTANCE hInstance;

class Logging : public ResizeDlg {
protected:
  DECLARE_MESSAGE_MAP()
public:
  Logging();

  virtual BOOL OnInitDialog(LPARAM);
  virtual void OnClose();
  void OnOk();
  void OnClear();
  void OnSave();

  void OnSWI();
  void OnUnaligned();
  void OnIllegalWrite();
  void OnIllegalRead();
  void OnDma0();
  void OnDma1();
  void OnDma2();
  void OnDma3();
  void OnUndefined();
  void OnAgbPrint();

  void OnErrSpace();
  void OnMaxText();
  
  void log(char *s);
};

BEGIN_MESSAGE_MAP(Logging, ResizeDlg)
  ON_WM_CLOSE()
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(IDC_CLEAR, OnClear)
  ON_BN_CLICKED(IDC_VERBOSE_SWI, OnSWI)
  ON_BN_CLICKED(IDC_VERBOSE_UNALIGNED_ACCESS, OnUnaligned)
  ON_BN_CLICKED(IDC_VERBOSE_ILLEGAL_WRITE, OnIllegalWrite)
  ON_BN_CLICKED(IDC_VERBOSE_ILLEGAL_READ, OnIllegalRead)
  ON_BN_CLICKED(IDC_VERBOSE_DMA0, OnDma0)
  ON_BN_CLICKED(IDC_VERBOSE_DMA1, OnDma1)
  ON_BN_CLICKED(IDC_VERBOSE_DMA2, OnDma2)
  ON_BN_CLICKED(IDC_VERBOSE_DMA3, OnDma3)
  ON_BN_CLICKED(IDC_VERBOSE_UNDEFINED, OnUndefined)
  ON_BN_CLICKED(IDC_VERBOSE_AGBPRINT, OnAgbPrint)
  ON_BN_CLICKED(IDC_SAVE, OnSave)
  ON_CONTROL(EN_ERRSPACE, IDC_LOG, OnErrSpace)
  ON_CONTROL(EN_MAXTEXT, IDC_LOG, OnMaxText)
END_MESSAGE_MAP()

static Logging *instance = NULL;
  
Logging::Logging()
  : ResizeDlg()
{
}

BOOL Logging::OnInitDialog(LPARAM)
{
  DIALOG_SIZER_START( sz )
    DIALOG_SIZER_ENTRY( IDC_LOG, DS_SizeY|DS_SizeX)
    DIALOG_SIZER_ENTRY( ID_OK, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_CLEAR, DS_MoveY)
    DIALOG_SIZER_ENTRY( IDC_SAVE, DS_MoveY)
  DIALOG_SIZER_END()
  SetData(sz,
          TRUE,
          HKEY_CURRENT_USER,
          "Software\\Emulators\\VisualBoyAdvance\\Viewer\\LogView",
          NULL);
  int value = (systemVerbose & 1);
  DoCheckbox(false, IDC_VERBOSE_SWI, value);
  value = (systemVerbose & 2) >> 1;
  DoCheckbox(false, IDC_VERBOSE_UNALIGNED_ACCESS, value);
  value = (systemVerbose & 4) >> 2;
  DoCheckbox(false, IDC_VERBOSE_ILLEGAL_WRITE, value);
  value = (systemVerbose & 8) >> 3;
  DoCheckbox(false, IDC_VERBOSE_ILLEGAL_READ, value);
  value = (systemVerbose & 16) >> 4;
  DoCheckbox(false, IDC_VERBOSE_DMA0, value);
  value = (systemVerbose & 32) >> 5;
  DoCheckbox(false, IDC_VERBOSE_DMA1, value);
  value = (systemVerbose & 64) >> 6;
  DoCheckbox(false, IDC_VERBOSE_DMA2, value);
  value = (systemVerbose & 128) >> 7;
  DoCheckbox(false, IDC_VERBOSE_DMA3, value);
  value = (systemVerbose & 256) >> 8;
  DoCheckbox(false, IDC_VERBOSE_UNDEFINED, value);
  value = (systemVerbose & 256) >> 9;
  DoCheckbox(false, IDC_VERBOSE_AGBPRINT, value);

  ::SendMessage(GetDlgItem(IDC_LOG), EM_SETLIMITTEXT, (WPARAM)-1, (LPARAM)-1);
  
  return TRUE;
}

void Logging::log(char *s)
{
  HWND h = GetDlgItem(IDC_LOG);

  int size = ::SendMessage(h, WM_GETTEXTLENGTH, 0, 0);
  ::SendMessage(h, EM_SETSEL, size, size);
  ::SendMessage(h, EM_REPLACESEL, FALSE, (LPARAM)s);
}

void Logging::OnOk()
{
  EndDialog(TRUE);

  instance = NULL;
}

void Logging::OnClose()
{
  EndDialog(FALSE);

  instance = NULL;
}

void Logging::OnClear()
{
  HWND h = GetDlgItem(IDC_LOG);

  ::SetWindowText(h, "");
}

void Logging::OnSave()
{
  HWND h = GetDlgItem(IDC_LOG);
  
  int len = GetWindowTextLength(h);

  char *mem = (char *)malloc(len);

  if(mem) {
    char *exts[] = { ".txt" };
    ::GetWindowText(h, mem, len);
    char capture[2048];
    capture[0] = 0;
    FileDlg dlg(getHandle(), capture, 2048, "All Files\0*.*\0\0",0,
                NULL, exts, NULL, "Save output", true);

    if(dlg.DoModal()) {
      FILE *f = fopen(capture, "w");
      if(f) {
        fwrite(mem, 1, len, f);
        fclose(f);
      }
    }
  }

  free(mem);
}

void Logging::OnSWI()
{
  systemVerbose ^= 1;
}

void Logging::OnUnaligned()
{
  systemVerbose ^= 2;
}

void Logging::OnIllegalWrite()
{
  systemVerbose ^= 4;
}

void Logging::OnIllegalRead()
{
  systemVerbose ^= 8;
}

void Logging::OnDma0()
{
  systemVerbose ^= 16;
}

void Logging::OnDma1()
{
  systemVerbose ^= 32;
}

void Logging::OnDma2()
{
  systemVerbose ^= 64;
}

void Logging::OnDma3()
{
  systemVerbose ^= 128;
}

void Logging::OnUndefined()
{
  systemVerbose ^= 256;
}

void Logging::OnAgbPrint()
{
  systemVerbose ^= 512;
}

void Logging::OnErrSpace()
{
  systemMessage(0, "Error allocating space");
}

void Logging::OnMaxText()
{
  systemMessage(0, "Max text length reached %d", ::SendMessage(GetDlgItem(IDC_LOG), EM_GETLIMITTEXT, 0, 0));
}

void toolsLogging()
{
  if(instance == NULL) {
    instance = new Logging();
    instance->setAutoDelete(true);
    instance->MakeDialog(hInstance,
                         IDD_LOGGING,
                         hWindow);
  } else {
    ::SetForegroundWindow(instance->getHandle());
  }
}

void toolsLog(char *s)
{
  if(instance != NULL) {
    instance->log(s);
  }
}
