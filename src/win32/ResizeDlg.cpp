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
/*----------------------------------------------------------------------
Copyright (c)  Gipsysoft. All Rights Reserved.
File:   DialogSizer_Set.cpp
Web site: http://gipsysoft.com

This software is provided 'as-is', without any express or implied warranty.

In no event will the author be held liable for any damages arising from the
use of this software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject
to the following restrictions: 

1) The origin of this software must not be misrepresented; you must not claim
   that you wrote the original software. If you use this software in a product,
         an acknowledgment in the product documentation is requested but not required. 
2) Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software. Altered source is encouraged
         to be submitted back to the original author so it can be shared with the
         community. Please share your changes.
3) This notice may not be removed or altered from any source distribution.

Owner:  russf@gipsysoft.com
Purpose:        Main functionality for sizeable dialogs

        Store a local copy of the user settings
        Subclass the window
        Respond to various messages withinn the subclassed window.

----------------------------------------------------------------------*/
#include "ResizeDlg.h"
#undef ASSERT
#include "WinHelper.h"

// moved functions to this file to reduce number of files

struct RegistryData
{
  WINDOWPLACEMENT       m_wpl;
};


struct DialogData       //      dd
{
  HKEY hkRootSave;
  LPCTSTR pcszName;

  //
  //    The number of items contained in the psd member.
  //    Used in the DeferWindowPos structure and in allocating memory
  int nItemCount;
  DialogSizerSizingItem *psd;

  //
  //    We need the smallest to respond to the WM_GETMINMAXINFO message
  POINT m_ptSmallest;

  //
  //    We don't strictly speaking need to say how big the biggest can be but
  POINT m_ptLargest;
  bool m_bLargestSet;

  //
  //    we need this to decide how much the window has changed size when we get a WM_SIZE message
  SIZE m_sizeClient;

  //
  //    Draw the sizing grip...or not
  bool m_bMaximised;
  BOOL m_bShowSizingGrip;

  WinHelper::CRect m_rcGrip;
};

long FASTCALL RegQueryValueExRecursive( HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData )
{
  TCHAR szBuffer[ 256 ];
  ASSERT( lstrlen( lpValueName ) < countof( szBuffer ) );
  (void)lstrcpy( szBuffer, lpValueName );
  
  LPTSTR pszBuffer = szBuffer;
  LPTSTR pszLast = szBuffer;
  while( *pszBuffer )
    {
      if( *pszBuffer == _T('\\') || *pszBuffer == _T('/') )
        {
          pszLast = pszBuffer;
          lpValueName = pszLast + 1;
        }
      pszBuffer++;
    }
  
  bool m_bNeedToCloseKey = false;
  if( pszLast != szBuffer )
    {
      *pszLast = _T('\000');
      HKEY hkeyTemp;
      long lRet = RegOpenKey( hKey, szBuffer, &hkeyTemp );
      if( lRet != ERROR_SUCCESS )
        {
          return lRet;
        }
      hKey = hkeyTemp;
      m_bNeedToCloseKey = true;
    }
  
  long lRet = RegQueryValueEx( hKey, lpValueName, lpReserved, lpType, lpData, lpcbData );
  if( m_bNeedToCloseKey )
    {
      VERIFY( RegCloseKey( hKey ) == ERROR_SUCCESS );
    }
  return lRet;
}

long FASTCALL RegSetValueExRecursive( HKEY hKey, LPCTSTR lpValueName, DWORD Reserved, DWORD dwType, CONST BYTE* lpData, DWORD cbData )
{
  TCHAR szBuffer[ 256 ];
  ASSERT( lstrlen( lpValueName ) < countof( szBuffer ) );
  (void)lstrcpy( szBuffer, lpValueName );
  
  LPTSTR pszBuffer = szBuffer;
  LPTSTR pszLast = szBuffer;
  while( *pszBuffer )
    {
      if( *pszBuffer == _T('\\') || *pszBuffer == _T('/') )
        {
          pszLast = pszBuffer;
          lpValueName = pszLast + 1;
        }
      pszBuffer++;
    }
  
  bool m_bNeedToCloseKey = false;
  if( pszLast != szBuffer )
    {
      *pszLast = _T('\000');
      HKEY hkeyTemp;
      long lRet = RegOpenKey( hKey, szBuffer, &hkeyTemp );
      if( lRet != ERROR_SUCCESS )
        {
          lRet = RegCreateKey( hKey, szBuffer, &hkeyTemp );
          if( lRet != ERROR_SUCCESS )
            return lRet;
        }
      hKey = hkeyTemp;
      m_bNeedToCloseKey = true;
    }
  
  long lRet = RegSetValueEx( hKey, lpValueName, Reserved, dwType, lpData, cbData );
  if( m_bNeedToCloseKey )
    {
      VERIFY( RegCloseKey( hKey ) == ERROR_SUCCESS );
    }
  return lRet;
}


int ResizeDlgGetItemCount(const DialogSizerSizingItem *psd)
{
  ASSERT( psd );
  int nCount = 0;
  while( psd->uSizeInfo != 0xFFFFFFFF )
    {
      nCount++;
      psd++;
    }
  return nCount;
}

void ResizeDlgUpdateGripperRect( const int cx, const int cy, WinHelper::CRect &rcGrip )
{
  const int nGripWidth = GetSystemMetrics( SM_CYVSCROLL );
  const int nGripHeight = GetSystemMetrics( SM_CXVSCROLL );
  rcGrip.left = cx - nGripWidth;
  rcGrip.top = cy - nGripHeight;
  rcGrip.right = cx;
  rcGrip.bottom = cy;
}

void ResizeDlgUpdateGripper( HWND hwnd, DialogData *pdd )
{
  if( pdd->m_bShowSizingGrip )
    {
      WinHelper::CRect rcOld( pdd->m_rcGrip );
      
      ResizeDlgUpdateGripperRect( pdd->m_sizeClient.cx, pdd->m_sizeClient.cy, pdd->m_rcGrip );
      
      //
      //        We also need to invalidate the combined area of the old and new rectangles
      //        otherwise we would have trail of grippers when we sized the dialog larger
      //        in any axis
      (void)UnionRect( &rcOld, &rcOld, &pdd->m_rcGrip );
      (void)InvalidateRect( hwnd, &rcOld, TRUE );
    }
}

void ResizeDlgCopyItems( DialogSizerSizingItem *psdDest, const DialogSizerSizingItem *psdSource )
  //
  //    Will copy all of the items in psdSource into psdDest.
{
  //
  //    Loop til we reach the end
  while( psdSource->uSizeInfo != 0xFFFFFFFF )
    {
      *psdDest = *psdSource;
      psdDest++;
      psdSource++;
    }
  //    And when we do copy the last item
  *psdDest = *psdSource;
}


ResizeDlg::ResizeDlg()
  : Dlg()
{
  dd = NULL;
}

void *ResizeDlg::AddDialogData()
  //
  //    Firstly determine if the data already exists, if it does then return that, if not then we will
  //    create and initialise a brand new structure.
{
  DialogData *pdd = (DialogData*)dd;
  if( !pdd ) {
    pdd = (DialogData *)calloc(1, sizeof(DialogData));
  }
  
  if( pdd ) {
    //
    //  Store some sizes etc. for later.
    WinHelper::CRect rc;
    VAPI( GetWindowRect( getHandle(), rc ) );
    pdd->m_ptSmallest.x = rc.Width();
    pdd->m_ptSmallest.y = rc.Height();
    
    
    VAPI( GetClientRect( getHandle(), rc ) );
    pdd->m_sizeClient = rc.Size();
    dd = pdd;
    ResizeDlgUpdateGripperRect( pdd->m_sizeClient.cx, pdd->m_sizeClient.cy, pdd->m_rcGrip );
  }
  return pdd;
}

BOOL ResizeDlg::SetData(const DialogSizerSizingItem *psd,
                        BOOL bShowSizingGrip,
                        HKEY hkRootSave,
                        LPCTSTR pcszName,
                        SIZE *psizeMax)
  //
  //    Setting a dialog sizeable involves subclassing the window and handling it's
  //    WM_SIZE messages, if we have a hkRootSave and pcszName then we will also be loading/saving
  //    the size and position of the window from the registry. We load from the registry when we 
  //    subclass the window and we save to the registry when we get a WM_DESTROY.
  //
  //    It will return non-zero for success and zero if it fails
{
  ASSERT( psd );
  ASSERT( ( hkRootSave != NULL && pcszName != NULL )
          || ( hkRootSave == NULL && pcszName == NULL ) );
  //
  //    Make sure all of the parameters are valid.
  if( ::IsWindow( getHandle() )
      && psd
      && ( ( hkRootSave != NULL && pcszName != NULL &&
             !IsBadStringPtr( pcszName, 0xFFFF ) ) ||
           ( hkRootSave == NULL && pcszName == NULL ) )
      && ( psizeMax == NULL || !IsBadReadPtr( psizeMax, sizeof( SIZE ) ) )
      ) {
    DialogData *pdd = (DialogData *)AddDialogData();
    if( pdd ) {
      pdd->hkRootSave = hkRootSave;
      pdd->pcszName = pcszName;
      pdd->m_bShowSizingGrip = bShowSizingGrip;
      pdd->nItemCount = ResizeDlgGetItemCount( psd ) + 1;
      pdd->psd = (DialogSizerSizingItem *)
        calloc(pdd->nItemCount,
               sizeof(DialogSizerSizingItem ));
      if( pdd->psd ) {
        //
        //      Copy all of the user controls etc. for later, this way the user can quite happily
        //      let the structure go out of scope.
        ResizeDlgCopyItems( pdd->psd, psd );
        if( psizeMax ) {
          pdd->m_ptLargest.x = psizeMax->cx;
          pdd->m_ptLargest.y = psizeMax->cy;
          pdd->m_bLargestSet = true;
        }
        
        //
        //      If the there was save info passed in then we need to make damn good use of it
        //      by attempting to load the RegistryData structure 
        if( hkRootSave && pcszName ) {
          RegistryData rd;
          DWORD dwSize = sizeof( RegistryData );
          DWORD dwType = REG_BINARY;
          if( RegQueryValueExRecursive( hkRootSave, pcszName, NULL, &dwType, reinterpret_cast<LPBYTE>( &rd ), &dwSize ) == ERROR_SUCCESS && dwSize == sizeof( rd ) ) {
            if( !(GetWindowLong( GWL_STYLE ) & WS_VISIBLE) )
              rd.m_wpl.showCmd = SW_HIDE;
            
            VAPI( SetWindowPlacement( getHandle(), &rd.m_wpl ) );
          }
        }
        return TRUE;
      } else {
        free(pdd);
      }
    }
  }
  return FALSE;
}

void ResizeDlg::UpdateWindowSize(const int cx, const int cy, HWND hwnd)
{
  DialogData *pdd = (DialogData*)dd;
  if( pdd ) {
    const int nDeltaX = cx - pdd->m_sizeClient.cx;
    const int nDeltaY = cy - pdd->m_sizeClient.cy;
    WinHelper::CDeferWindowPos def( pdd->nItemCount );
    WinHelper::CRect rc;
    const DialogSizerSizingItem *psd = pdd->psd;
    while( psd->uSizeInfo != 0xFFFFFFFF ) {
      HWND hwndChild = GetDlgItem( psd->uControlID );
      if( ::IsWindow( hwndChild ) ) {
        VAPI( ::GetWindowRect( hwndChild, rc ) );
        (void)MapWindowPoints( GetDesktopWindow(),  hwnd,
                               (LPPOINT)&rc, 2 );
        
        //
        //      Adjust the window horizontally
        if( psd->uSizeInfo & DS_MoveX ) {
          rc.left += nDeltaX;
          rc.right += nDeltaX;
        }
        
        //
        //      Adjust the window vertically
        if( psd->uSizeInfo & DS_MoveY ) {
          rc.top += nDeltaY;
          rc.bottom += nDeltaY;
        }
        
        //
        //      Size the window horizontally
        if( psd->uSizeInfo & DS_SizeX ) {
          rc.right += nDeltaX;
        }
        
        //
        //      Size the window vertically
        if( psd->uSizeInfo & DS_SizeY ) {
          rc.bottom += nDeltaY;
        }
        
        (void)def.DeferWindowPos( hwndChild, NULL, rc,
                                  SWP_NOACTIVATE | SWP_NOZORDER );
      }
      psd++;
    }
    
    pdd->m_sizeClient.cx = cx;
    pdd->m_sizeClient.cy = cy;
    
    //
    //  If we have a sizing grip enabled then adjust it's position
    ResizeDlgUpdateGripper( hwnd, pdd );
  }
}

BOOL ResizeDlg::OnMsg(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT &res )
  //    Actual window procedure that will handle saving window size/position and moving
  //    the controls whilst the window sizes.
{
  if(dd == NULL) {
    return Dlg::OnMsg(msg, wParam, lParam, res);
  }
  switch( msg ) {
  case WM_ERASEBKGND:
    {
      BOOL r = Dlg::OnMsg(msg, wParam, lParam, res);
      DialogData *pdd = (DialogData*)dd;
      if( pdd && pdd->m_bShowSizingGrip && !pdd->m_bMaximised ) {
        VAPI( ::DrawFrameControl( reinterpret_cast<HDC>( wParam ),
                                  pdd->m_rcGrip,
                                  DFC_SCROLL, DFCS_SCROLLSIZEGRIP ) );
      }
      return r;
    }
  case WM_SIZE:
    {
      DialogData *pdd = (DialogData*)dd;
      if( pdd && wParam != SIZE_MINIMIZED ) {
        pdd->m_bMaximised = ( wParam == SIZE_MAXIMIZED ? true : false );
        UpdateWindowSize( LOWORD( lParam ), HIWORD( lParam ), getHandle());
      }
    }
    break;
  case WM_NCHITTEST:
    {
      //
      //        If the gripper is enabled then perform a simple hit test on our gripper area.
      DialogData *pdd = (DialogData*)dd;
      if( pdd && pdd->m_bShowSizingGrip ) {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        (void)ScreenToClient( getHandle(), &pt );
        if( PtInRect( pdd->m_rcGrip, pt ) )
          return (BOOL)HTBOTTOMRIGHT;
      }
    }
    break;
  case WM_GETMINMAXINFO:
    {
      //
      //        Our opportunity to say that we do not want the dialog to grow or shrink any more.
      DialogData *pdd = (DialogData*)dd;
      LPMINMAXINFO lpmmi = reinterpret_cast<LPMINMAXINFO>( lParam );
      lpmmi->ptMinTrackSize = pdd->m_ptSmallest;
      if( pdd->m_bLargestSet ) {
        lpmmi->ptMaxTrackSize = pdd->m_ptLargest;
      }
    }
    return (BOOL)0;
  case WM_NOTIFY:
    {
      if( reinterpret_cast<LPNMHDR>(lParam)->code == PSN_SETACTIVE ) {
        WinHelper::CRect rc;
        VAPI( GetClientRect( GetParent( getHandle() ), &rc ) );
        UpdateWindowSize( rc.Width(), rc.Height(), GetParent( getHandle() ) );
      }
    }
    break;
  case WM_DESTROY:
    {
      //
      //        Our opportunty for cleanup.
      //        Simply acquire all of our objects, free the appropriate memory and remove the 
      //        properties from the window. If we do not remove the properties then they will constitute
      //        a resource leak.
      DialogData *pdd = (DialogData*)dd;
      if( pdd ) {
        RegistryData rd;
        rd.m_wpl.length = sizeof( rd.m_wpl );
        VAPI( GetWindowPlacement( getHandle(), &rd.m_wpl ) );
        
        if( pdd->hkRootSave && pdd->pcszName ) {
          (void)RegSetValueExRecursive( pdd->hkRootSave, pdd->pcszName,
                                        NULL, REG_BINARY,
                                        reinterpret_cast<LPBYTE>( &rd ),
                                        sizeof( rd ) );
        }
        
        if( pdd->psd ) {
          free(pdd->psd);
        }
        free(pdd);
      }
      
    }
    break;
  }
  return Dlg::OnMsg(msg, wParam, lParam, res);
}
