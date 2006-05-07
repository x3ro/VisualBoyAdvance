/*
    VisualBoyAdvance - a Game Boy & Game Boy Advance emulator

	Copyright (C) 1999 - 2003 Forgotten
    Copyright (C) 2003 - 2004 Forgotten and the VBA development team
    Copyright (C) 2005 - 2006 VBA development team


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#include "stdafx.h"
#include "VBA.h"

// Link with Direct3D9
#pragma comment(lib, "D3d9.lib")
#define DIRECT3D_VERSION 0x0900
#ifdef _DEBUG
# define D3D_DEBUG_INFO
#endif
#include <D3d9.h>

// Link with DirectDraw7
#pragma comment(lib, "ddraw.lib")
#define DIRECTDRAW_VERSION 0x0700
#include <ddraw.h>


// UniVideoModeDlg-Dialogfeld

class UniVideoModeDlg : public CDialog
{
	DECLARE_DYNAMIC(UniVideoModeDlg)

public:
	UniVideoModeDlg(CWnd* pParent = NULL, int *width=0, int *height=0, int *BPP=0, int *freq=0, int *adapt=0);   // Standardkonstruktor
	virtual ~UniVideoModeDlg();

// Dialogfelddaten
	enum { IDD = IDD_UNIVIDMODE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	DECLARE_MESSAGE_MAP()

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnBnClickedOk();
	BOOL OnInitDialog();
	afx_msg void OnBnClickedCancel();

private:
	IDirect3D9   *pD3D;
	IDirectDraw7 *pDirectDraw;

public:
	// Generic
	UINT        nAdapters;
	DWORD       *WidthList, *HeightList, *BPPList, *FreqList;
	DWORD       iDisplayDevice;
	int         *SelectedWidth;
	int         *SelectedHeight;
	int         *SelectedBPP;
	int         *SelectedFreq;
	int         *SelectedAdapter;
	// DirectDraw
	int         nDisplayModes;
	bool        bEnumerateDisplayModeCount;
	GUID        guidDisplayDevice;
	bool        useStandardDisplayDevice;
	int         iCurrentMode;

public:
	afx_msg void OnStnClickedDisplaydevice();
	afx_msg void OnBnClickedCheckStretchtofit();
	CStatic apiname;
	CButton devicename;
	CListBox listmodes;
};

BOOL WINAPI DDEnumCallbackEx_UniVideoModeDlg(
  GUID FAR *lpGUID,
  LPSTR     lpDriverDescription, 
  LPSTR     lpDriverName,
  LPVOID    lpContext,
  HMONITOR  hm
);

HRESULT WINAPI EnumModesCallback2(
  LPDDSURFACEDESC2 lpDDSurfaceDesc,
  LPVOID lpContext
);