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
#include "Reg.h"
#include "resource.h"

extern void checkKeys();
extern void checkJoypads();
extern void checkKeyboard();
extern void setDeviceFirst();
extern void winSaveKeys();
extern void winCenterWindow(HWND);
extern char *getKeyName(int);
extern HWND hWindow;

extern USHORT joypad[4][13];
extern USHORT motion[4];

#define JOYCONFIG_MESSAGE (WM_USER + 1000)

enum {
  KEY_LEFT, KEY_RIGHT,
  KEY_UP, KEY_DOWN,
  KEY_BUTTON_A, KEY_BUTTON_B,
  KEY_BUTTON_START, KEY_BUTTON_SELECT,
  KEY_BUTTON_L, KEY_BUTTON_R,
  KEY_BUTTON_SPEED, KEY_BUTTON_CAPTURE,
  KEY_BUTTON_GS
};

class JoypadEditControl : public Wnd {
  Wnd *parent;
protected:
  DECLARE_MESSAGE_MAP()
    
public:
  JoypadEditControl();
  void setParent(Wnd *);

  virtual LRESULT OnJoyConfig(WPARAM, LPARAM);
};

class JoypadConfigDlg : public Dlg {
  JoypadEditControl up;
  JoypadEditControl down;
  JoypadEditControl left;
  JoypadEditControl right;
  JoypadEditControl buttonL;
  JoypadEditControl buttonR;
  JoypadEditControl buttonA;
  JoypadEditControl buttonB;  
  JoypadEditControl buttonSelect;
  JoypadEditControl buttonStart;
  JoypadEditControl speed;
  JoypadEditControl capture;
  JoypadEditControl buttonGS;
  UINT timerId;
  int which;
protected:
  DECLARE_MESSAGE_MAP()
public:
  JoypadConfigDlg(int);

  void assignKey(int, int);
  void assignKeys();
  
  virtual BOOL OnInitDialog(LPARAM);
  virtual void OnDestroy();
  virtual void OnTimer(UINT);
  void OnOk();
  void OnCancel();
  LRESULT OnCtlColorStatic(WPARAM, LPARAM);
};

class MotionConfigDlg : public Dlg {
  JoypadEditControl left;
  JoypadEditControl down;
  JoypadEditControl right;
  JoypadEditControl up;
  UINT timerId;
protected:
  DECLARE_MESSAGE_MAP()
public:
  MotionConfigDlg();
  
  void assignKey(int, int);
  void assignKeys();

  virtual BOOL OnInitDialog(LPARAM);
  virtual void OnDestroy();
  virtual void OnTimer(UINT);
  void OnOk();
  void OnCancel();
  LRESULT OnCtlColorStatic(WPARAM, LPARAM);
};

void configurePad(int which)
{
  setDeviceFirst();
  checkKeys();
  JoypadConfigDlg dlg(which);
  dlg.DoModal(IDD_CONFIG,
              hWindow);
}

void motionConfigurePad()
{
  setDeviceFirst();  
  checkKeys();
  MotionConfigDlg dlg;

  dlg.DoModal(IDD_MOTION_CONFIG,
              hWindow);
}

BEGIN_MESSAGE_MAP(JoypadEditControl, Wnd)
  ON_MESSAGE(JOYCONFIG_MESSAGE, OnJoyConfig)
  ON_WM_CHAR()
END_MESSAGE_MAP()

JoypadEditControl::JoypadEditControl()
  : Wnd()
{
  parent = NULL;
}

void JoypadEditControl::setParent(Wnd *p)
{
  parent = p;
}

LRESULT JoypadEditControl::OnJoyConfig(WPARAM wParam, LPARAM lParam)
{
  SetWindowLong(GWL_USERDATA,((wParam<<8)|lParam));
  SetWindowText(getKeyName((wParam<<8)|lParam));
  HWND h = GetNextDlgTabItem(parent->getHandle(), getHandle(), FALSE);
  SetFocus(h);
  return TRUE;
}

BEGIN_MESSAGE_MAP(JoypadConfigDlg, Dlg)
  //  ON_MESSAGE(WM_CTLCOLORSTATIC, OnCtlColorStatic)
  ON_WM_CHAR()
  ON_WM_DESTROY()
  ON_WM_TIMER()
  ON_WM_KEYDOWN()
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(ID_CANCEL, OnCancel)
END_MESSAGE_MAP()

JoypadConfigDlg::JoypadConfigDlg(int w)
  : Dlg()
{
  timerId = 0;
  which = w;
  if(which < 0 || which > 3)
    which = 0;
}

void JoypadConfigDlg::assignKey(int id,int key)
{
  switch(id) {
  case IDC_EDIT_LEFT:
    joypad[which][KEY_LEFT] = key;
    break;
  case IDC_EDIT_RIGHT:
    joypad[which][KEY_RIGHT] = key;
    break;
  case IDC_EDIT_UP:
    joypad[which][KEY_UP] = key;
    break;
  case IDC_EDIT_SPEED:
    joypad[which][KEY_BUTTON_SPEED] = key;
    break;
  case IDC_EDIT_CAPTURE:
    joypad[which][KEY_BUTTON_CAPTURE] = key;
    break;    
  case IDC_EDIT_DOWN:
    joypad[which][KEY_DOWN] = key;
    break;
  case IDC_EDIT_BUTTON_A:
    joypad[which][KEY_BUTTON_A] = key;
    break;
  case IDC_EDIT_BUTTON_B:
    joypad[which][KEY_BUTTON_B] = key;
    break;
  case IDC_EDIT_BUTTON_L:
    joypad[which][KEY_BUTTON_L] = key;
    break;
  case IDC_EDIT_BUTTON_R:
    joypad[which][KEY_BUTTON_R] = key;
    break;
  case IDC_EDIT_BUTTON_START:
    joypad[which][KEY_BUTTON_START] = key;
    break;
  case IDC_EDIT_BUTTON_SELECT:
    joypad[which][KEY_BUTTON_SELECT] = key;
    break;
  case IDC_EDIT_BUTTON_GS:
    joypad[which][KEY_BUTTON_GS] = key;
    break;
  }
}

void JoypadConfigDlg::assignKeys()
{
  int id;

  id = IDC_EDIT_UP;
  assignKey(id, up.GetWindowLong(GWL_USERDATA));

  id = IDC_EDIT_DOWN;
  assignKey(id, down.GetWindowLong(GWL_USERDATA));

  id = IDC_EDIT_LEFT;
  assignKey(id, left.GetWindowLong(GWL_USERDATA));

  id = IDC_EDIT_RIGHT;
  assignKey(id, right.GetWindowLong(GWL_USERDATA));

  id = IDC_EDIT_BUTTON_A;
  assignKey(id, buttonA.GetWindowLong(GWL_USERDATA));

  id = IDC_EDIT_BUTTON_B;
  assignKey(id, buttonB.GetWindowLong(GWL_USERDATA));

  id = IDC_EDIT_BUTTON_L;
  assignKey(id, buttonL.GetWindowLong(GWL_USERDATA));

  id = IDC_EDIT_BUTTON_R;
  assignKey(id, buttonR.GetWindowLong(GWL_USERDATA));
  
  id = IDC_EDIT_BUTTON_SELECT;
  assignKey(id, buttonSelect.GetWindowLong(GWL_USERDATA));

  id = IDC_EDIT_BUTTON_START;
  assignKey(id, buttonStart.GetWindowLong(GWL_USERDATA));

  id = IDC_EDIT_SPEED;
  assignKey(id, speed.GetWindowLong(GWL_USERDATA));

  id = IDC_EDIT_CAPTURE;
  assignKey(id, capture.GetWindowLong(GWL_USERDATA));

  id = IDC_EDIT_BUTTON_GS;
  assignKey(id, buttonGS.GetWindowLong(GWL_USERDATA));

  winSaveKeys();
}

BOOL JoypadConfigDlg::OnInitDialog(LPARAM)
{
  timerId = SetTimer(getHandle(),0,200,NULL);
  
  up.SubClassWindow(GetDlgItem(IDC_EDIT_UP));
  up.SetWindowLong(GWL_USERDATA,joypad[which][KEY_UP]);
  up.SetWindowText(getKeyName(joypad[which][KEY_UP]));
  up.setParent(this);
  
  down.SubClassWindow(GetDlgItem(IDC_EDIT_DOWN));
  down.SetWindowLong(GWL_USERDATA,joypad[which][KEY_DOWN]);
  down.SetWindowText(getKeyName(joypad[which][KEY_DOWN]));
  down.setParent(this);

  left.SubClassWindow(GetDlgItem(IDC_EDIT_LEFT));
  left.SetWindowLong(GWL_USERDATA,joypad[which][KEY_LEFT]);
  left.SetWindowText(getKeyName(joypad[which][KEY_LEFT]));
  left.setParent(this);

  right.SubClassWindow(GetDlgItem(IDC_EDIT_RIGHT));
  right.SetWindowLong(GWL_USERDATA,joypad[which][KEY_RIGHT]);
  right.SetWindowText(getKeyName(joypad[which][KEY_RIGHT]));
  right.setParent(this);

  buttonA.SubClassWindow(GetDlgItem(IDC_EDIT_BUTTON_A));
  buttonA.SetWindowLong(GWL_USERDATA,joypad[which][KEY_BUTTON_A]);
  buttonA.SetWindowText(getKeyName(joypad[which][KEY_BUTTON_A]));
  buttonA.setParent(this);

  buttonB.SubClassWindow(GetDlgItem(IDC_EDIT_BUTTON_B));
  buttonB.SetWindowLong(GWL_USERDATA,joypad[which][KEY_BUTTON_B]);
  buttonB.SetWindowText(getKeyName(joypad[which][KEY_BUTTON_B]));
  buttonB.setParent(this);
  
  buttonL.SubClassWindow(GetDlgItem(IDC_EDIT_BUTTON_L));
  buttonL.SetWindowLong(GWL_USERDATA,joypad[which][KEY_BUTTON_L]);
  buttonL.SetWindowText(getKeyName(joypad[which][KEY_BUTTON_L]));
  buttonL.setParent(this);

  buttonR.SubClassWindow(GetDlgItem(IDC_EDIT_BUTTON_R));
  buttonR.SetWindowLong(GWL_USERDATA,joypad[which][KEY_BUTTON_R]);
  buttonR.SetWindowText(getKeyName(joypad[which][KEY_BUTTON_R]));
  buttonR.setParent(this);
  
  buttonSelect.SubClassWindow(GetDlgItem(IDC_EDIT_BUTTON_SELECT));
  buttonSelect.SetWindowLong(GWL_USERDATA,joypad[which][KEY_BUTTON_SELECT]);
  buttonSelect.SetWindowText(getKeyName(joypad[which][KEY_BUTTON_SELECT]));
  buttonSelect.setParent(this);

  buttonStart.SubClassWindow(GetDlgItem(IDC_EDIT_BUTTON_START));
  buttonStart.SetWindowLong(GWL_USERDATA,joypad[which][KEY_BUTTON_START]);
  buttonStart.SetWindowText(getKeyName(joypad[which][KEY_BUTTON_START]));
  buttonStart.setParent(this);

  speed.SubClassWindow(GetDlgItem(IDC_EDIT_SPEED));
  speed.SetWindowLong(GWL_USERDATA,joypad[which][KEY_BUTTON_SPEED]);
  speed.SetWindowText(getKeyName(joypad[which][KEY_BUTTON_SPEED]));
  speed.setParent(this);
  
  capture.SubClassWindow(GetDlgItem(IDC_EDIT_CAPTURE));
  capture.SetWindowLong(GWL_USERDATA,joypad[which][KEY_BUTTON_CAPTURE]);
  capture.SetWindowText(getKeyName(joypad[which][KEY_BUTTON_CAPTURE]));
  capture.setParent(this);

  buttonGS.SubClassWindow(GetDlgItem(IDC_EDIT_BUTTON_GS));
  buttonGS.SetWindowLong(GWL_USERDATA,joypad[which][KEY_BUTTON_GS]);
  buttonGS.SetWindowText(getKeyName(joypad[which][KEY_BUTTON_GS]));
  buttonGS.setParent(this);
  
  winCenterWindow(getHandle());

  return TRUE;
}

void JoypadConfigDlg::OnOk()
{
  assignKeys();
  checkKeys();
  EndDialog(TRUE);
}

void JoypadConfigDlg::OnCancel()
{
  EndDialog(FALSE);
}

void JoypadConfigDlg::OnDestroy()
{
  KillTimer(hWnd, timerId);
}

void JoypadConfigDlg::OnTimer(UINT)
{
  checkJoypads();
  checkKeyboard();
}

LRESULT JoypadConfigDlg::OnCtlColorStatic(WPARAM wParam, LPARAM lParam)
{
  HDC hDC = (HDC)wParam;

  SetBkMode(hDC, 2);
  SetBkColor(hDC, RGB(192,192,192));
  return FALSE;
}

BEGIN_MESSAGE_MAP(MotionConfigDlg, Dlg)
  //  ON_MESSAGE(WM_CTLCOLORSTATIC, OnCtlColorStatic)
  ON_WM_CHAR()
  ON_WM_DESTROY()
  ON_WM_TIMER()
  ON_WM_KEYDOWN()
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(ID_CANCEL, OnCancel)
END_MESSAGE_MAP()

MotionConfigDlg::MotionConfigDlg()
  : Dlg()
{
  timerId = 0;
}

void MotionConfigDlg::assignKey(int id,int key)
{
  switch(id) {
  case IDC_EDIT_LEFT:
    motion[KEY_LEFT] = key;
    break;
  case IDC_EDIT_RIGHT:
    motion[KEY_RIGHT] = key;
    break;
  case IDC_EDIT_UP:
    motion[KEY_UP] = key;
    break;
  case IDC_EDIT_DOWN:
    motion[KEY_DOWN] = key;
    break;
  }
}

void MotionConfigDlg::assignKeys()
{
  int id;

  id = IDC_EDIT_UP;
  assignKey(id, up.GetWindowLong(GWL_USERDATA));

  id = IDC_EDIT_DOWN;
  assignKey(id, down.GetWindowLong(GWL_USERDATA));

  id = IDC_EDIT_LEFT;
  assignKey(id, left.GetWindowLong(GWL_USERDATA));

  id = IDC_EDIT_RIGHT;
  assignKey(id, right.GetWindowLong(GWL_USERDATA));

  regSetDwordValue("Motion_Left",
                   motion[KEY_LEFT]);
  regSetDwordValue("Motion_Right",
                   motion[KEY_RIGHT]);
  regSetDwordValue("Motion_Up",
                   motion[KEY_UP]);
  regSetDwordValue("Motion_Down",
                   motion[KEY_DOWN]);
}

BOOL MotionConfigDlg::OnInitDialog(LPARAM)
{
  timerId = SetTimer(getHandle(),0,200,NULL);
  
  up.SubClassWindow(GetDlgItem(IDC_EDIT_UP));
  up.SetWindowLong(GWL_USERDATA,motion[KEY_UP]);
  up.SetWindowText(getKeyName(motion[KEY_UP]));
  up.setParent(this);
  
  down.SubClassWindow(GetDlgItem(IDC_EDIT_DOWN));
  down.SetWindowLong(GWL_USERDATA,motion[KEY_DOWN]);
  down.SetWindowText(getKeyName(motion[KEY_DOWN]));
  down.setParent(this);

  left.SubClassWindow(GetDlgItem(IDC_EDIT_LEFT));
  left.SetWindowLong(GWL_USERDATA,motion[KEY_LEFT]);
  left.SetWindowText(getKeyName(motion[KEY_LEFT]));
  left.setParent(this);

  right.SubClassWindow(GetDlgItem(IDC_EDIT_RIGHT));
  right.SetWindowLong(GWL_USERDATA,motion[KEY_RIGHT]);
  right.SetWindowText(getKeyName(motion[KEY_RIGHT]));
  right.setParent(this);

  winCenterWindow(getHandle());

  return TRUE;
}

void MotionConfigDlg::OnOk()
{
  assignKeys();
  checkKeys();
  EndDialog( TRUE);
}

void MotionConfigDlg::OnCancel()
{
  EndDialog(FALSE);
}

void MotionConfigDlg::OnTimer(UINT)
{
  checkJoypads();
  checkKeyboard();
}

void MotionConfigDlg::OnDestroy()
{
  KillTimer(getHandle(), timerId);
}

LRESULT MotionConfigDlg::OnCtlColorStatic(WPARAM wParam, LPARAM)
{
  HDC hdc = (HDC)wParam;
  SetBkMode(hdc, 2);
  SetBkColor(hdc, RGB(192,192,192));
  return FALSE;
}

