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
#include "resource.h"
#include "../System.h"
#include "../GBA.h"
#include "../Globals.h"

#include <winsock.h>

#define SOCKET_MESSAGE WM_APP+1

extern HWND hWindow;
extern void winCenterWindow(HWND);
extern BOOL useBiosFile;

static bool initialized = false;

class GDBPortDlg : public Dlg {
  int port;
  SOCKET sock;
protected:
  DECLARE_MESSAGE_MAP()
public:
  GDBPortDlg();

  int getPort() { return port; }
  SOCKET getSocket() { return sock; }

  virtual BOOL OnInitDialog(LPARAM);

  void OnOk();
  void OnCancel();

  virtual void OnClose();
};

class GDBWaitingDlg : public Dlg {
  int port;
  SOCKET listenSocket;
  SOCKET sock;
protected:
  DECLARE_MESSAGE_MAP()
public:
  GDBWaitingDlg(SOCKET s);

  SOCKET getListenSocket() { return listenSocket; }
  SOCKET getSocket() { return sock; }
  
  virtual BOOL OnInitDialog(LPARAM);
  virtual LRESULT OnSocketAccept(WPARAM, LPARAM);

  void OnCancel();

  virtual void OnClose();
};

BEGIN_MESSAGE_MAP(GDBPortDlg, Dlg)
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(ID_CANCEL, OnCancel)
  ON_WM_CLOSE()
END_MESSAGE_MAP()

GDBPortDlg::GDBPortDlg()  :
  Dlg()
{
  port = 55555;
  sock = INVALID_SOCKET;
  
  if(!initialized) {
    WSADATA wsaData;

    int error = WSAStartup(MAKEWORD(1,1), &wsaData);

    initialized = true;
  }
}

BOOL GDBPortDlg::OnInitDialog(LPARAM)
{
  char buffer[16];

  sprintf(buffer, "%d", port);

  ::SetWindowText(GetDlgItem(IDC_PORT), buffer);

  winCenterWindow(getHandle());

  return TRUE;
}

void GDBPortDlg::OnOk()
{
  char buffer[16];

  ::GetWindowText(GetDlgItem(IDC_PORT), buffer, 16);
  
  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr("0.0.0.0");
  address.sin_port = htons(atoi(buffer));
  port = ntohs(address.sin_port);

  SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

  if(s != INVALID_SOCKET) {
    int error = bind(s, (sockaddr *)&address, sizeof(address));

    if(error) {
      systemMessage(IDS_ERROR_BINDING, "Error binding socket. Port probably in use.");
      error = closesocket(s);
      EndDialog(FALSE);
    } else {
      error = listen(s, 1);
      if(!error) {
        sock = s;
        EndDialog(TRUE);
      } else {
        systemMessage(IDS_ERROR_LISTENING, "Error listening on socket.");
        closesocket(s);
        EndDialog(FALSE);
      }
    }
  } else {
    systemMessage(IDS_ERROR_CREATING_SOCKET, "Error creating socket.");
    EndDialog(FALSE);
  }
}

void GDBPortDlg::OnCancel()
{
  OnClose();
}

void GDBPortDlg::OnClose()
{
  EndDialog(FALSE);
}

BEGIN_MESSAGE_MAP(GDBWaitingDlg, Dlg)
  ON_BN_CLICKED(ID_CANCEL, OnCancel)
  ON_WM_CLOSE()
  ON_MESSAGE(SOCKET_MESSAGE, OnSocketAccept)
END_MESSAGE_MAP()

GDBWaitingDlg::GDBWaitingDlg(SOCKET s)
  : Dlg()
{
  port = 55555;
  listenSocket = s;
}

BOOL GDBWaitingDlg::OnInitDialog(LPARAM l)
{
  char buffer[16];
  port = (l & 65535);

  sprintf(buffer, "%d", port);

  ::SetWindowText(GetDlgItem(IDC_PORT), buffer);

  winCenterWindow(getHandle());

  int error = WSAAsyncSelect(listenSocket,
                             getHandle(),
                             SOCKET_MESSAGE,
                             FD_ACCEPT);

  return TRUE;
}

LRESULT GDBWaitingDlg::OnSocketAccept(WPARAM wParam, LPARAM lParam)
{
  if(LOWORD(lParam) == FD_ACCEPT) {
    WSAAsyncSelect(listenSocket, getHandle(), 0, 0);
    
    int flag = 0;    
    ioctlsocket(listenSocket, FIONBIO, (unsigned long *)&flag);
    
    SOCKET s = accept(listenSocket, NULL, NULL);
    if(s != INVALID_SOCKET) {
      char dummy;
      recv(s, &dummy, 1, 0);
      if(dummy != '+') {
        systemMessage(IDS_ACK_NOT_RECEIVED, "ACK not received from GDB.");
        OnClose(); // calls EndDialog
      } else {
        sock = s;
        EndDialog(TRUE);
      }
    }
  }

  return TRUE;
}

void GDBWaitingDlg::OnCancel()
{
  OnClose();
}

void GDBWaitingDlg::OnClose()
{
  if(sock != INVALID_SOCKET) {
    closesocket(sock);
    sock = INVALID_SOCKET;
  }
  if(listenSocket != INVALID_SOCKET) {
    closesocket(listenSocket);
    listenSocket = INVALID_SOCKET;
  }
  EndDialog(FALSE);
}

extern void remoteSetSockets(SOCKET, SOCKET);

extern bool debugger;
extern int emulating;
extern int cartridgeType;
extern char filename[2048];
extern char biosFileName[2048];

extern bool (*emuWriteState)(char *);
extern bool (*emuReadState)(char *);
extern bool (*emuWriteBattery)(char *);
extern bool (*emuReadBattery)(char *);
extern void (*emuReset)();
extern void (*emuCleanUp)();
extern bool (*emuWritePNG)(char *);
extern bool (*emuWriteBMP)(char *);
extern void (*emuMain)(int);
extern int emuCount;

void toolsDebugGDB()
{
  GDBPortDlg dlg;

  if(dlg.DoModal(IDD_GDB_PORT, hWindow)) {
    GDBWaitingDlg wait(dlg.getSocket());
    if(wait.DoModal(IDD_GDB_WAITING, hWindow, dlg.getPort())) {
      remoteSetSockets(wait.getListenSocket(), wait.getSocket());
      debugger = true;
      emulating = 1;
      cartridgeType = 0;
      strcpy(filename, "\\gnu_stub");
      rom = (u8 *)malloc(0x2000000);
      workRAM = (u8 *)calloc(1, 0x40000);
      bios = (u8 *)calloc(1,0x4000);
      internalRAM = (u8 *)calloc(1,0x8000);
      paletteRAM = (u8 *)calloc(1,0x400);
      vram = (u8 *)calloc(1, 0x20000);
      oam = (u8 *)calloc(1, 0x400);
      pix = (u8 *)calloc(1, 4 * 240 * 160);
      ioMem = (u8 *)calloc(1, 0x400);
      
      emuWriteState = CPUWriteState;
      emuReadState = CPUReadState;
      emuWriteBattery = CPUWriteBatteryFile;
      emuReadBattery = CPUReadBatteryFile;
      emuReset = CPUReset;
      emuCleanUp = CPUCleanUp;
      emuWritePNG = CPUWritePNGFile;
      emuWriteBMP = CPUWriteBMPFile;
      emuMain = CPULoop;
      //      emuUpdateCPSR = CPUUpdateCPSR;
      //      emuHasDebugger = true;
      emuCount = 50000;    
      
      CPUInit(biosFileName, useBiosFile);
      CPUReset();    
    }
  }
}

extern BOOL fileOpenSelect();
extern BOOL fileOpen();
extern void fileClose();

void toolsDebugGDBLoad()
{
  if(fileOpenSelect()) {
    if(fileOpen()) {
      if(cartridgeType != 0) {
        systemMessage(IDS_ERROR_NOT_GBA_IMAGE, "Error: not a GBA image");
        fileClose();
        return;
      }
      GDBPortDlg dlg;

      if(dlg.DoModal(IDD_GDB_PORT, hWindow)) {
        GDBWaitingDlg wait(dlg.getSocket());
        if(wait.DoModal(IDD_GDB_WAITING, hWindow, dlg.getPort())) {
          remoteSetSockets(wait.getListenSocket(), wait.getSocket());
          debugger = true;
          emulating = 1;
        }
      }
    }
  }
}
