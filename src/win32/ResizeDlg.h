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

#ifndef _INC_TCHAR
        #include <tchar.h>
#endif  //      _INC_TCHAR

//
//      Predefined sizing information
#define DS_MoveX                1
#define DS_MoveY                2
#define DS_SizeX                4
#define DS_SizeY                8

typedef struct DialogSizerSizingItem    //      sdi
{
        UINT uControlID;
        UINT uSizeInfo;
} DialogSizerSizingItem;

#define DIALOG_SIZER_START( name )      DialogSizerSizingItem name[] = {
#define DIALOG_SIZER_ENTRY( controlID, flags )  { controlID, flags },
#define DIALOG_SIZER_END()      { 0xFFFFFFFF, 0xFFFFFFFF } };

class ResizeDlg : public Dlg {
  void *dd;  
public:
  ResizeDlg();

  void *AddDialogData();
  BOOL SetData(const DialogSizerSizingItem *psd,
               BOOL bShowSizingGrip,
               HKEY hkRootSave,
               LPCTSTR pcszName,
               SIZE *psizeMax);
  void UpdateWindowSize(const int cx, const int cy, HWND);

  virtual BOOL OnMsg(UINT, WPARAM, LPARAM, LRESULT &);
};
