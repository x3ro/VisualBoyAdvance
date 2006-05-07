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

#include "univideomodedlg.h"

// UniVideoModeDlg-Dialogfeld

IMPLEMENT_DYNAMIC(UniVideoModeDlg, CDialog)
UniVideoModeDlg::UniVideoModeDlg(CWnd* pParent /*=NULL*/, int *width, int *height, int *BPP, int *freq, int *adapt)
	: CDialog(UniVideoModeDlg::IDD, pParent)
{
	// initialize COM
	if (FAILED(CoInitialize(NULL))) {
		systemMessage(0, "COM initialization failed");
	}

	WidthList = 0;
	HeightList = 0;
	BPPList = 0;
	FreqList = 0;
	iDisplayDevice = 0;
	SelectedWidth = width;
	SelectedHeight = height;
	SelectedBPP = BPP;
	SelectedFreq = freq;
	SelectedAdapter = adapt;
	pD3D = NULL;
	nAdapters = 0;
	nDisplayModes = 0;
	bEnumerateDisplayModeCount = false;
	iCurrentMode = 0;
}

UniVideoModeDlg::~UniVideoModeDlg()
{
	if (WidthList) delete [] WidthList;
	if (HeightList) delete [] HeightList;
	if (BPPList) delete [] BPPList;
	if (FreqList) delete [] FreqList;
	
	// uninizialize COM
	CoUninitialize(); 
}

void UniVideoModeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_APINAME, apiname);
	DDX_Control(pDX, IDC_DISPLAYDEVICE, devicename);
	DDX_Control(pDX, IDC_LISTMODES, listmodes);
}


BEGIN_MESSAGE_MAP(UniVideoModeDlg, CDialog)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_STN_CLICKED(IDC_DISPLAYDEVICE, OnStnClickedDisplaydevice)
	ON_BN_CLICKED(IDC_CHECK_STRETCHTOFIT, OnBnClickedCheckStretchtofit)
END_MESSAGE_MAP()


// UniVideoModeDlg-Meldungshandler

int UniVideoModeDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Fügen Sie Ihren spezialisierten Erstellcode hier ein.

	return 0;
}


BOOL UniVideoModeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CButton* check_stretchtofit = (CButton*)GetDlgItem(IDC_CHECK_STRETCHTOFIT);
	check_stretchtofit->SetCheck(theApp.fullScreenStretch ? BST_CHECKED : BST_UNCHECKED);

	CString temp;
	DWORD w, h, b, f;
	UINT nModes16, nModes32;
	HRESULT hret;

	switch(theApp.display->getType())
	{
	case GDI:
	case OPENGL:
		if( GDI == theApp.display->getType() ) {
			apiname.SetWindowText("Windows GDI");
		}
		if( OPENGL == theApp.display->getType() ) {
			apiname.SetWindowText("OpenGL");
		}
		DISPLAY_DEVICE dd;
		dd.cb = sizeof(dd);
		EnumDisplayDevices(NULL, iDisplayDevice, &dd, 0);
		temp.Format( "(#%d) %s", iDisplayDevice, dd.DeviceString );
		devicename.SetWindowText( temp );
		DEVMODE dm;
		dm.dmSize = sizeof(DEVMODE);
		dm.dmDriverExtra = 0;
		DWORD maxMode, i;
		for (i=0; 0 != EnumDisplaySettings(dd.DeviceName, i, &dm); i++) {}
		maxMode = i-1;
		listmodes.InitStorage(i, 25);
		if (WidthList!=0) delete [] WidthList;
		if (HeightList!=0) delete [] HeightList;
		if (BPPList!=0) delete [] BPPList;
		if (FreqList!=0) delete [] FreqList;
		WidthList = new DWORD[i];
		HeightList = new DWORD[i];
		BPPList = new DWORD[i];
		FreqList = new DWORD[i];
		listmodes.ResetContent();
		for (i=0; i<maxMode+1; i++)
		{
			EnumDisplaySettings(dd.DeviceName, i, &dm);
			w = dm.dmPelsWidth;
			h = dm.dmPelsHeight;
			b = dm.dmBitsPerPel;
			f = dm.dmDisplayFrequency;

			temp.Format("%dx%dx%d @%dHz", w, h, b, f);
			listmodes.AddString(temp);
			WidthList[i] = w;
			HeightList[i] = h;
			BPPList[i] = b;
			FreqList[i] = f;
		}
		break;


	case DIRECT_DRAW:
		apiname.SetWindowText( "DirectDraw7" );
		// Enumerate Adapters
		nAdapters = 0;
		DirectDrawEnumerateEx(
			DDEnumCallbackEx_UniVideoModeDlg,
			this,
			DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_DETACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES);
		// Create DirectDraw
		hret = CoCreateInstance( CLSID_DirectDraw, NULL, CLSCTX_ALL, IID_IDirectDraw7, (LPVOID*)&pDirectDraw );
		if( FAILED(hret) ) {
			systemMessage(0, "CoCreateInstance failed: %d", hret);
		} else {
			hret = IDirectDraw7_Initialize( pDirectDraw, useStandardDisplayDevice ? NULL : &guidDisplayDevice );
			if( FAILED( hret ) ) {
				systemMessage(0, "DirectDraw initialization failed: %d", hret);
			}
		}
		// Enumerate number of display modes
		nDisplayModes = 0;
		bEnumerateDisplayModeCount = true;
		pDirectDraw->EnumDisplayModes(
			DDEDM_REFRESHRATES,
			NULL,
			this,
			EnumModesCallback2);
		bEnumerateDisplayModeCount = false;
		// Allocate required memory
		if (WidthList!=0) delete [] WidthList;
		if (HeightList!=0) delete [] HeightList;
		if (BPPList!=0) delete [] BPPList;
		if (FreqList!=0) delete [] FreqList;
		WidthList = new DWORD[nDisplayModes];
		HeightList = new DWORD[nDisplayModes];
		BPPList = new DWORD[nDisplayModes];
		FreqList = new DWORD[nDisplayModes];
		listmodes.InitStorage(nDisplayModes, 25);
		listmodes.ResetContent();
		// Enumerate dimensions of all video modes
		iCurrentMode = 0;
		pDirectDraw->EnumDisplayModes(
			DDEDM_REFRESHRATES,
			NULL,
			this,
			EnumModesCallback2);
		//Destroy DirectDraw
		pDirectDraw->Release();
		pDirectDraw = NULL;
		break;


	case DIRECT_3D:
		apiname.SetWindowText("Direct3D9");
		pD3D = Direct3DCreate9(D3D_SDK_VERSION);
		nAdapters = pD3D->GetAdapterCount();
		D3DADAPTER_IDENTIFIER9 id;
		pD3D->GetAdapterIdentifier(iDisplayDevice, 0, &id);
		temp.Format( "(#%d) %s", iDisplayDevice, id.Description );
		devicename.SetWindowText( temp );

		D3DDISPLAYMODE d3ddm;

		nModes16 = pD3D->GetAdapterModeCount(iDisplayDevice, D3DFMT_R5G6B5);
		nModes32 = pD3D->GetAdapterModeCount(iDisplayDevice, D3DFMT_X8R8G8B8);

		listmodes.InitStorage(nModes16+nModes32, 25);
		listmodes.ResetContent();

		if (WidthList!=0) delete [] WidthList;
		if (HeightList!=0) delete [] HeightList;
		if (BPPList!=0) delete [] BPPList;
		if (FreqList!=0) delete [] FreqList;
		WidthList = new DWORD[nModes16+nModes32];
		HeightList = new DWORD[nModes16+nModes32];
		BPPList = new DWORD[nModes16+nModes32];
		FreqList = new DWORD[nModes16+nModes32];

		b = 16;
		for (UINT i = 0; i < nModes16; i++)
		{
			pD3D->EnumAdapterModes(iDisplayDevice, D3DFMT_R5G6B5, i, &d3ddm);
			w = d3ddm.Width;
			h = d3ddm.Height;
			f = d3ddm.RefreshRate;

			temp.Format("%dx%dx%d @%dHz", w, h, b, f);
			listmodes.AddString(temp);

			WidthList[i] = w;
			HeightList[i] = h;
			BPPList[i] = b;
			FreqList[i] = f;
		}
		b = 32;
		for (UINT i = 0; i < nModes32; i++)
		{
			pD3D->EnumAdapterModes(iDisplayDevice, D3DFMT_X8R8G8B8, i, &d3ddm);
			w = d3ddm.Width;
			h = d3ddm.Height;
			f = d3ddm.RefreshRate;

			temp.Format("%dx%dx%d @%dHz", w, h, b, f);
			listmodes.AddString(temp);

			WidthList[i+nModes16] = w;
			HeightList[i+nModes16] = h;
			BPPList[i+nModes16] = b;
			FreqList[i+nModes16] = f;
		}

		// Clean up		
		pD3D->Release();
		pD3D = NULL;
		break;
	}
	
	return TRUE;
}


void UniVideoModeDlg::OnBnClickedOk()
{
	CListBox* listmodes = (CListBox*)GetDlgItem(IDC_LISTMODES);
	int selection = listmodes->GetCurSel();
	if (selection == LB_ERR)
	{
		MessageBox("No mode selected!", "Error", MB_OK | MB_ICONWARNING);
		return;
	}
	*SelectedWidth = WidthList[selection];
	*SelectedHeight = HeightList[selection];
	*SelectedBPP = BPPList[selection];
	*SelectedFreq = FreqList[selection];
	*SelectedAdapter = iDisplayDevice;

	EndDialog(0);
}

void UniVideoModeDlg::OnBnClickedCancel()
{
	EndDialog(-1);
}
void UniVideoModeDlg::OnStnClickedDisplaydevice()
{
	DWORD old = iDisplayDevice;
	switch(theApp.display->getType())
	{
	case GDI:
		DISPLAY_DEVICE dd;
		dd.cb = sizeof(dd);
		if (0 == EnumDisplayDevices(NULL, ++iDisplayDevice, &dd, 0))
			iDisplayDevice = 0;
		break;

	case DIRECT_DRAW:
		iDisplayDevice++;
		if (iDisplayDevice == nAdapters) iDisplayDevice = 0;
		break;

	case DIRECT_3D:
		iDisplayDevice++;
		if (iDisplayDevice == nAdapters) iDisplayDevice = 0;
		break;
	}

	if (iDisplayDevice != old)
		OnInitDialog();
}


void UniVideoModeDlg::OnBnClickedCheckStretchtofit()
{
	CButton *checkBox = (CButton*)GetDlgItem( IDC_CHECK_STRETCHTOFIT );
	theApp.fullScreenStretch = ((checkBox->GetState()&0x0003) == BST_CHECKED) ? true : false;
	theApp.d3dKeepAspectRatio = !theApp.fullScreenStretch;
	theApp.updateWindowSize( theApp.videoOption );
	if( theApp.display ) {
		if( emulating ) {
			theApp.display->clear( );
		}
		theApp.display->setOption( _T("d3dKeepAspectRatio") );
	}
	this->SetFocus();
}


BOOL WINAPI DDEnumCallbackEx_UniVideoModeDlg(
  GUID FAR *lpGUID,
  LPSTR     lpDriverDescription,
  LPSTR     lpDriverName,
  LPVOID    lpContext,
  HMONITOR  hm
)
{
	CString temp;
	UniVideoModeDlg *dlg = (UniVideoModeDlg*)lpContext;
	if( dlg->nAdapters == dlg->iDisplayDevice ) {
		temp.Format( "(#%d) %s", dlg->iDisplayDevice, lpDriverDescription );
		dlg->devicename.SetWindowText( temp );
		if( lpGUID == NULL ) {
			dlg->useStandardDisplayDevice = true;
		} else {
			dlg->useStandardDisplayDevice = false;
			dlg->guidDisplayDevice = *lpGUID;
		}
	}
	dlg->nAdapters++;
	return DDENUMRET_OK;
}

HRESULT WINAPI EnumModesCallback2(
  LPDDSURFACEDESC2 lpDDSurfaceDesc,
  LPVOID lpContext
)
{
	UniVideoModeDlg *dlg = (UniVideoModeDlg*)lpContext;
	if( dlg->bEnumerateDisplayModeCount ) {
		dlg->nDisplayModes++;
	} else {
		DWORD w, h, b, f;
		w = dlg->WidthList[dlg->iCurrentMode] = lpDDSurfaceDesc->dwWidth;
		h = dlg->HeightList[dlg->iCurrentMode] = lpDDSurfaceDesc->dwHeight;
		b = dlg->BPPList[dlg->iCurrentMode] = lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;
		f = dlg->FreqList[dlg->iCurrentMode] = lpDDSurfaceDesc->dwRefreshRate;
		CString temp;
		temp.Format("%dx%dx%d @%dHz", w, h, b, f);
		dlg->listmodes.AddString(temp);
		dlg->iCurrentMode++;
	}
	return DDENUMRET_OK;
}