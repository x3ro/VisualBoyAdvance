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
#include <commctrl.h>
#include "../GBA.h"
#include "../Globals.h"
#include "../Cheats.h"
#include "Reg.h"
#include "WinResUtil.h"
#include "resource.h"

struct WinCheatsData {
  u32  addr;
  char address[9];
  char oldValue[12];
  char newValue[12];
};

class GBACheatSearchDlg : public Dlg {
  static bool initialized;
  WinCheatsData *data;
  int valueType;
  int searchType;
  int numberType;
  int sizeType;
  bool updateValues;
protected:
  DECLARE_MESSAGE_MAP()
public:
  GBACheatSearchDlg();
  ~GBACheatSearchDlg();
  void addChange(HWND lv,
                 int index,
                 u32 address,
                 u32 oldValue,
                 u32 newValue);
  void addChanges(HWND lv, bool);
  void search();
  
  virtual BOOL OnInitDialog(LPARAM);

  void OnGetDispInfo(NMHDR *, LRESULT *);
  void OnItemChanged(NMHDR *, LRESULT *);
  void OnOk();
  void OnStart();
  void OnAddCheat();
  void OnValueType(UINT);
  void OnSearchType(UINT);
  void OnNumberType(UINT);
  void OnSizeType(UINT);
  void OnUpdate();
};

class GBACheatListDlg : public Dlg {
  bool restoreValues;
  int numberType;
  bool duringRefresh;
protected:
  DECLARE_MESSAGE_MAP()
public:
  GBACheatListDlg();

  void refresh(HWND);
  
  virtual BOOL OnInitDialog(LPARAM);

  void OnItemChanged(NMHDR *, LRESULT *);
  void OnOk();
  void OnAddCheat();
  void OnAddCode();
  void OnAddGameshark();
  void OnAddCodeBreaker();
  void OnRemoveAll();
  void OnRemove();
  void OnEnable();
  void OnNumberType(UINT);
  void OnRestore();
};

class AddGSACodeDlg : public Dlg {
protected:
  DECLARE_MESSAGE_MAP()
public:
  AddGSACodeDlg();

  virtual BOOL OnInitDialog(LPARAM);

  void OnOk();
  void OnCancel();
};

class AddCBACodeDlg : public Dlg {
protected:
  DECLARE_MESSAGE_MAP()
public:
  AddCBACodeDlg();

  virtual BOOL OnInitDialog(LPARAM);

  void OnOk();
  void OnCancel();
};

class AddCheatCodeDlg : public Dlg {
protected:
  DECLARE_MESSAGE_MAP()
public:
  AddCheatCodeDlg();

  virtual BOOL OnInitDialog(LPARAM);

  void OnOk();
  void OnCancel();
};

class AddCheatDlg : public Dlg {
  bool freeze;
  int sizeType;
  int numberType;
protected:
  DECLARE_MESSAGE_MAP()
public:
  AddCheatDlg();

  BOOL addCheat();
  
  virtual BOOL OnInitDialog(LPARAM);

  void OnOk();
  void OnCancel();
  void OnNumberType(UINT);
  void OnSizeType(UINT);
  void OnFreeze();
};

void winCheatsAddGSACodeDialog(HWND);
void winCheatsAddCBACodeDialog(HWND);
void winCheatsAddCheatCodeDialog(HWND);
void winCheatsAddDialog(HWND parent, u32 address);
extern void winCenterWindow(HWND);
extern HWND hWindow;

bool GBACheatSearchDlg::initialized = false;

/**************************************************************************
 * GBA Cheat search
 **************************************************************************/

BEGIN_MESSAGE_MAP(GBACheatSearchDlg, Dlg)
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(IDC_START, OnStart)
  ON_BN_CLICKED(IDC_SEARCH, search)
  ON_BN_CLICKED(IDC_ADD_CHEAT, OnAddCheat)
  ON_CONTROL_RANGE(BN_CLICKED, IDC_OLD_VALUE, IDC_SPECIFIC_VALUE, OnValueType)
  ON_CONTROL_RANGE(BN_CLICKED, IDC_EQ, IDC_GE, OnSearchType)
  ON_CONTROL_RANGE(BN_CLICKED, IDC_SIGNED, IDC_HEXADECIMAL, OnNumberType)
  ON_CONTROL_RANGE(BN_CLICKED, IDC_SIZE_8, IDC_SIZE_32, OnSizeType)
  ON_BN_CLICKED(IDC_UPDATE, OnUpdate)
  ON_NOTIFY(LVN_GETDISPINFO, IDC_CHEAT_LIST, OnGetDispInfo)
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_CHEAT_LIST, OnItemChanged)
END_MESSAGE_MAP()

GBACheatSearchDlg::GBACheatSearchDlg()
  : Dlg()
{
  data = NULL;
}

GBACheatSearchDlg::~GBACheatSearchDlg()
{
  if(data)
    free(data);
}

void GBACheatSearchDlg::addChange(HWND lv,
                                  int index,
                                  u32 address,
                                  u32 oldValue,
                                  u32 newValue)
{
  data[index].addr = address;
  sprintf(data[index].address, "%08x",address);
  switch(numberType) {
  case 0:
    sprintf(data[index].oldValue, "%d", oldValue);
    sprintf(data[index].newValue, "%d", newValue);
    break;        
  case 1:
    sprintf(data[index].oldValue, "%u", oldValue);
    sprintf(data[index].newValue, "%u", newValue);
    break;    
  case 2:
    switch(sizeType) {
    case 0:
      sprintf(data[index].oldValue, "%02x", oldValue);
      sprintf(data[index].newValue, "%02x", newValue);      
      break;
    case 1:
      sprintf(data[index].oldValue, "%04x", oldValue);
      sprintf(data[index].newValue, "%04x", newValue);      
      break;
    case 2:
      sprintf(data[index].oldValue, "%08x", oldValue);
      sprintf(data[index].newValue, "%08x", newValue);      
      break;
    }
  }
}

void GBACheatSearchDlg::addChanges(HWND lv, bool showMsgs)
{
  int count = cheatsGetCount(sizeType);
  
  ListView_DeleteAllItems(lv);

  if(count > 1000) {
    if(showMsgs)
      systemMessage(IDS_SEARCH_PRODUCED_TOO_MANY,
                    "Search produced %d results. Please refine better",
                    count);
    return;
  }

  if(count == 0) {
    if(showMsgs)
      systemMessage(IDS_SEARCH_PRODUCED_NO_RESULTS,
                    "Search produced no results.");
    return;
  }
  
  ListView_SetItemCount(lv, count);  
  if(data)
    free(data);
  
  data = (WinCheatsData *)malloc(count * sizeof(WinCheatsData));
  
  int inc = 1;
  switch(sizeType) {
  case 1:
    inc = 2;
    break;
  case 2:
    inc = 4;
    break;
  }
  
  int index = 0;
  if(numberType == 0) {
    for(int i = 0; i < 0x40000; i += inc) {
      if(TEST_BIT(cheatSearch.wBITS, i))
        addChange(lv,
                  index++,
                  0x2000000 | i,
                  SIGNED_DATA(sizeType,
                              cheatSearch.wRAM,
                              i),
                  SIGNED_DATA(sizeType,
                              workRAM,
                              i));
    }
    
    for(i = 0 ; i < 0x8000; i += inc) {
      if(TEST_BIT(cheatSearch.iBITS, i))
        addChange(lv,
                  index++,
                  0x3000000 | i,
                  SIGNED_DATA(sizeType,
                              cheatSearch.iRAM,
                              i),
                  SIGNED_DATA(sizeType,
                              internalRAM,
                              i));
    }    
  } else {
    for(int i = 0; i < 0x40000; i += inc) {
      if(TEST_BIT(cheatSearch.wBITS, i))
        addChange(lv,
                  index++,
                  0x2000000 | i,
                  UNSIGNED_DATA(sizeType,
                                cheatSearch.wRAM,
                                i),
                  UNSIGNED_DATA(sizeType,
                                workRAM,
                                i));
    }
    
    for(i = 0 ; i < 0x8000; i += inc) {
      if(TEST_BIT(cheatSearch.iBITS, i))
        addChange(lv,
                  index++,
                  0x3000000 | i,
                  UNSIGNED_DATA(sizeType,
                                cheatSearch.iRAM,
                                i),
                  UNSIGNED_DATA(sizeType,
                                internalRAM,
                                i));
    }    
  }

  for(int i = 0; i < count; i++) {
    LVITEM item;

    item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    item.iItem = i;
    item.iSubItem = 0;
    item.lParam = data[i].addr;
    item.state = 0;
    item.stateMask = 0;
    item.pszText = LPSTR_TEXTCALLBACK;
    ListView_InsertItem(lv, &item);

    ListView_SetItemText(lv, i, 1, LPSTR_TEXTCALLBACK);
    ListView_SetItemText(lv, i, 2, LPSTR_TEXTCALLBACK);
  }  
}

void GBACheatSearchDlg::search()
{
  char buffer[128];
  if(valueType == 0)
    cheatsSearchChange(searchType,
                       sizeType,
                       numberType == 0);
  else {
    GetWindowText(GetDlgItem(IDC_VALUE), buffer, 127);
    if(strlen(buffer) == 0) {
      systemMessage(IDS_NUMBER_CANNOT_BE_EMPTY, "Number cannot be empty");
      return;
    }
    int value = 0;
    switch(numberType) {
    case 0:
      sscanf(buffer, "%d", &value);
      break;
    case 1:
      sscanf(buffer, "%u", &value);
      break;
    default:
      sscanf(buffer, "%x", &value);
    }
    cheatsSearchValue(searchType,
                      sizeType,
                      numberType == 0,
                      value);
  }
  
  HWND lv = GetDlgItem(IDC_CHEAT_LIST);

  addChanges(lv, true);

  if(updateValues)
    cheatsUpdateValues();
}

BOOL GBACheatSearchDlg::OnInitDialog(LPARAM)
{
  LVCOLUMN col;
  
  HWND h = GetDlgItem(IDC_CHEAT_LIST);
  col.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
  col.fmt = LVCFMT_CENTER;
  col.cx = 125;
  col.pszText = (char *)winResLoadString(IDS_ADDRESS);
  col.iSubItem = 0;
  ListView_InsertColumn(h, 0, &col);
  col.pszText = (char *)winResLoadString(IDS_OLD_VALUE);
  col.iSubItem = 1;
  ListView_InsertColumn(h, 1, &col);
  col.iSubItem = 2;
  col.pszText = (char *)winResLoadString(IDS_NEW_VALUE);
  ListView_InsertColumn(h, 2, &col);
  
  SendMessage(h,
              WM_SETFONT,
              (WPARAM)GetStockObject(SYSTEM_FIXED_FONT),
              TRUE);

  ListView_SetExtendedListViewStyle(h, LVS_EX_FULLROWSELECT);    
  
  if(!initialized) {
    EnableWindow(GetDlgItem(IDC_SEARCH), FALSE);
    EnableWindow(GetDlgItem(IDC_ADD_CHEAT), FALSE);
  }
  
  valueType = regQueryDwordValue("cheatsValueType", 0);
  if(valueType < 0 || valueType > 1)
    valueType = 0;
  DoRadio(false, IDC_OLD_VALUE, valueType);

  searchType = regQueryDwordValue("cheatsSearchType", GBA_EQUAL);
  if(searchType > 5 || searchType < 0)
    searchType = 0;
  DoRadio(false, IDC_EQ, searchType);
  
  numberType = regQueryDwordValue("cheatsNumberType", 2);
  if(numberType < 0 || numberType > 2)
    numberType = 2;
  DoRadio(false, IDC_SIGNED, numberType);
  
  sizeType = regQueryDwordValue("cheatsSizeType", 0);
  if(sizeType < 0 || sizeType > 2)
    sizeType = 0;
  DoRadio(false, IDC_SIZE_8, sizeType);
  
  updateValues = regQueryDwordValue("cheatsUpdate", 0) ?
    true : false;

  int upd = (int)updateValues;
  
  DoCheckbox(false, IDC_UPDATE, upd);
  
  if(valueType == 0)
    EnableWindow(GetDlgItem(IDC_VALUE), FALSE);
  winCenterWindow(getHandle());

  if(initialized) {
    addChanges(h, false);
  }
  return TRUE;  
}

void GBACheatSearchDlg::OnOk()
{
  EndDialog(TRUE);
}

void GBACheatSearchDlg::OnStart()
{
  cheatsReset();
  EnableWindow(GetDlgItem(IDC_SEARCH), TRUE);
  initialized = true;
}

void GBACheatSearchDlg::OnAddCheat()
{
  int mark = ListView_GetSelectionMark(GetDlgItem(IDC_CHEAT_LIST));
  
  if(mark != -1) {
    LVITEM item;
    memset(&item,0, sizeof(item));
    item.mask = LVIF_PARAM;
    item.iItem = mark;
    if(ListView_GetItem(GetDlgItem(IDC_CHEAT_LIST),
                        &item)) {
      winCheatsAddDialog(getHandle(), (u32)item.lParam);
    }
  }
}

void GBACheatSearchDlg::OnValueType(UINT id)
{
  switch(id) {
  case IDC_OLD_VALUE:
    valueType = 0;
    EnableWindow(GetDlgItem(IDC_VALUE), FALSE);
    regSetDwordValue("cheatsValueType", 0);
    break;
  case IDC_SPECIFIC_VALUE:
    valueType = 1;
    EnableWindow(GetDlgItem(IDC_VALUE), TRUE);
    regSetDwordValue("cheatsValueType", 1);     
    break;
  }
}

void GBACheatSearchDlg::OnSearchType(UINT id)
{
  switch(id) {
  case IDC_EQ:
    searchType = GBA_EQUAL;
    regSetDwordValue("cheatsSearchType", 0);
    break;
  case IDC_NE:
    searchType = GBA_NOT_EQUAL;
    regSetDwordValue("cheatsSearchType", 1);
    break;
  case IDC_LT:
    searchType = GBA_LESS_THAN;
    regSetDwordValue("cheatsSearchType", 2);
    break;
  case IDC_LE:
    searchType = GBA_LESS_EQUAL;
    regSetDwordValue("cheatsSearchType", 3);
    break;
  case IDC_GT:
    searchType = GBA_GREATER_THAN;
    regSetDwordValue("cheatsSearchType", 4);
    break;
  case IDC_GE:
    searchType = GBA_GREATER_EQUAL;
    regSetDwordValue("cheatsSearchType", 5);
    break;
  }
}

void GBACheatSearchDlg::OnNumberType(UINT id)
{
  HWND h;
  switch(id) {
  case IDC_SIGNED:
    numberType = 0;
    regSetDwordValue("cheatsNumberType", 0);
    h = GetDlgItem(IDC_CHEAT_LIST);
    if(ListView_GetItemCount(h)) {
      addChanges(h, false);
    }
    break;
  case IDC_UNSIGNED:
    numberType = 1;
    regSetDwordValue("cheatsNumberType", 1);
    h = GetDlgItem(IDC_CHEAT_LIST);
    if(ListView_GetItemCount(h)) {
      addChanges(h, false);
    }
    break;
  case IDC_HEXADECIMAL:
    numberType = 2;
    regSetDwordValue("cheatsNumberType", 2);
    h = GetDlgItem(IDC_CHEAT_LIST);
    if(ListView_GetItemCount(h)) {
      addChanges(h, false);
    }
    break;
  }
}

void GBACheatSearchDlg::OnSizeType(UINT id)
{
  HWND h;
  switch(id) {
  case IDC_SIZE_8:
    sizeType = SIZE_8;
    regSetDwordValue("cheatsSizeType", 0);
    h = GetDlgItem(IDC_CHEAT_LIST);
    if(ListView_GetItemCount(h)) {
      addChanges(h, false);
    }
    break;
  case IDC_SIZE_16:
    sizeType = SIZE_16;
    regSetDwordValue("cheatsSizeType", 1);
    h = GetDlgItem(IDC_CHEAT_LIST);
    if(ListView_GetItemCount(h)) {
      addChanges(h, false);
    }
    break;
  case IDC_SIZE_32:
    sizeType = SIZE_32;
    regSetDwordValue("cheatsSizeType", 2);
    h = GetDlgItem(IDC_CHEAT_LIST);
    if(ListView_GetItemCount(h)) {
      addChanges(h, false);
    }
    break;
  }
}

void GBACheatSearchDlg::OnUpdate()
{
  if(SendMessage(GetDlgItem(IDC_UPDATE),
                 BM_GETCHECK,
                 0,
                 0) & BST_CHECKED)
    updateValues = true;
  else
    updateValues = false;
  regSetDwordValue("cheatsUpdate", updateValues);
}

void GBACheatSearchDlg::OnGetDispInfo(NMHDR *hdr, LRESULT *pResult)
{
  NMLVDISPINFO *info = (NMLVDISPINFO*)hdr;
  if(info->item.mask & LVIF_TEXT) {
    int index = info->item.iItem;
    int col = info->item.iSubItem;
    
    switch(col) {
    case 0:
      strcpy(info->item.pszText, data[index].address);
      break;
    case 1:
      strcpy(info->item.pszText, data[index].oldValue);
      break;
    case 2:
      strcpy(info->item.pszText, data[index].newValue);
      break;
    }
  }
  *pResult = TRUE;
}

void GBACheatSearchDlg::OnItemChanged(NMHDR *hdr, LRESULT *pResult)
{
  if(ListView_GetSelectionMark(GetDlgItem(IDC_CHEAT_LIST)) != -1)
    EnableWindow(GetDlgItem(IDC_ADD_CHEAT),
                 TRUE);
  else
    EnableWindow(GetDlgItem(IDC_ADD_CHEAT),
                 FALSE);
  *pResult = TRUE;
}

/**************************************************************************
 * GBA Cheat list
 **************************************************************************/

BEGIN_MESSAGE_MAP(GBACheatListDlg, Dlg)
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(IDC_ADD_CHEAT, OnAddCheat)
  ON_BN_CLICKED(IDC_ADD_CODE, OnAddCode)
  ON_BN_CLICKED(IDC_ADD_GAMESHARK, OnAddGameshark)
  ON_BN_CLICKED(IDC_ADD_CODEBREAKER, OnAddCodeBreaker)
  ON_BN_CLICKED(IDC_REMOVE_ALL, OnRemoveAll)
  ON_BN_CLICKED(IDC_REMOVE, OnRemove)
  ON_BN_CLICKED(IDC_ENABLE, OnEnable)
  ON_CONTROL_RANGE(BN_CLICKED, IDC_SIGNED, IDC_HEXADECIMAL, OnNumberType)
  ON_BN_CLICKED(IDC_RESTORE, OnRestore)
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_CHEAT_LIST, OnItemChanged)
END_MESSAGE_MAP()

GBACheatListDlg::GBACheatListDlg()
  : Dlg()
{
  duringRefresh = false;
}

void GBACheatListDlg::refresh(HWND lv)
{
  duringRefresh = true;
  ListView_DeleteAllItems(lv);
  
  char buffer[100];
  
  for(int i = 0; i < cheatsNumber; i++) {
    LVITEM item;

    item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    item.iItem = i;
    item.iSubItem = 0;
    item.lParam = i;
    item.state = 0;
    item.stateMask = 0;
    item.pszText = cheatsList[i].codestring;
    ListView_InsertItem(lv, &item);

    ListView_SetCheckState(lv, i, (cheatsList[i].enabled) ? TRUE : FALSE);

    ListView_SetItemText(lv, i, 1, cheatsList[i].desc);

    buffer[0] = (cheatsList[i].status & 2) ? 'E' : 'D';    
    buffer[1] = (cheatsList[i].status & 1) ? 'F' : ' ';
    buffer[2] = 0;
    ListView_SetItemText(lv, i, 2, buffer);
  }
  duringRefresh = false;
}

BOOL GBACheatListDlg::OnInitDialog(LPARAM)
{
  LVCOLUMN col;      
  HWND h = GetDlgItem(IDC_CHEAT_LIST);
  col.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
  col.fmt = LVCFMT_LEFT;
  col.cx = 170;
  col.pszText = (char *)winResLoadString(IDS_CODE);
  col.iSubItem = 0;
  ListView_InsertColumn(h, 0, &col);
  col.pszText = (char *)winResLoadString(IDS_DESCRIPTION);
  col.iSubItem = 1;
  col.cx = 150;
  ListView_InsertColumn(h, 1, &col);
  col.iSubItem = 2;
  col.pszText = (char *)winResLoadString(IDS_STATUS);
  col.cx = 80;
  ListView_InsertColumn(h, 2, &col);
  
  SendMessage(h,
              WM_SETFONT,
              (WPARAM)GetStockObject(SYSTEM_FIXED_FONT),
              TRUE);

  ListView_SetExtendedListViewStyle(h, LVS_EX_CHECKBOXES |
                                    LVS_EX_FULLROWSELECT);  
  
  numberType = regQueryDwordValue("cheatsNumberType", 2);
  if(numberType < 0 || numberType > 2)
    numberType = 2;
  DoRadio(false, IDC_SIGNED, numberType);
  
  restoreValues = regQueryDwordValue("cheatsRestore", 0) ?
    true : false;
  int rest = (int)restoreValues;
  DoCheckbox(false, IDC_RESTORE, rest);
  
  refresh(h);
  EnableWindow(GetDlgItem(IDC_REMOVE), FALSE);
  EnableWindow(GetDlgItem(IDC_ENABLE), FALSE);
  winCenterWindow(getHandle());
  return TRUE;
}

void GBACheatListDlg::OnItemChanged(NMHDR *hdr, LRESULT *pResult)
{
  if(ListView_GetSelectionMark(GetDlgItem(IDC_CHEAT_LIST)) != -1) {
    EnableWindow(GetDlgItem(IDC_REMOVE),
                 TRUE);
    EnableWindow(GetDlgItem(IDC_ENABLE),
                 TRUE);
  } else {
    EnableWindow(GetDlgItem(IDC_REMOVE),
                 FALSE);
    EnableWindow(GetDlgItem(IDC_ENABLE),
                 FALSE);
  }
  
  if(!duringRefresh) {
    LPNMLISTVIEW l = (LPNMLISTVIEW)hdr;
    if(l->uChanged & LVIF_STATE) {
      if(((l->uOldState & LVIS_STATEIMAGEMASK)>>12) !=
         (((l->uNewState & LVIS_STATEIMAGEMASK)>>12))) {
        if(ListView_GetCheckState(GetDlgItem(IDC_CHEAT_LIST), l->iItem))
          cheatsEnable(l->lParam);
        else
          cheatsDisable(l->lParam);
        refresh(GetDlgItem(IDC_CHEAT_LIST));
      }
    }
  }
}

void GBACheatListDlg::OnOk()
{
  EndDialog(TRUE);
}

void GBACheatListDlg::OnAddCheat()
{
  winCheatsAddDialog(getHandle(), 0);
  refresh(GetDlgItem(IDC_CHEAT_LIST));
}

void GBACheatListDlg::OnAddCode()
{
  winCheatsAddCheatCodeDialog(getHandle());
  refresh(GetDlgItem(IDC_CHEAT_LIST));    
}

void GBACheatListDlg::OnAddGameshark()
{
  winCheatsAddGSACodeDialog(getHandle());
  refresh(GetDlgItem(IDC_CHEAT_LIST));
}

void GBACheatListDlg::OnAddCodeBreaker()
{
  winCheatsAddCBACodeDialog(getHandle());
  refresh(GetDlgItem(IDC_CHEAT_LIST));
}

void GBACheatListDlg::OnRemoveAll()
{
  cheatsDeleteAll(restoreValues);
  refresh(GetDlgItem(IDC_CHEAT_LIST));    
}

void GBACheatListDlg::OnRemove()
{
  HWND h = GetDlgItem(IDC_CHEAT_LIST);
  int mark = ListView_GetSelectionMark(h);
  int count = ListView_GetItemCount(h);
  
  if(mark != -1) {
    for(int i = count - 1; i >= 0; i--) {
      LVITEM item;
      memset(&item,0, sizeof(item));
      item.mask = LVIF_PARAM|LVIF_STATE;
      item.iItem = i;
      item.stateMask = LVIS_SELECTED;
      if(ListView_GetItem(h,
                          &item)) {
        if(item.state & LVIS_SELECTED) {
          cheatsDelete(item.lParam, restoreValues);
        }
      }
    }
    refresh(h);
  }         
}

void GBACheatListDlg::OnEnable()
{
  HWND h = GetDlgItem(IDC_CHEAT_LIST);
  int mark = ListView_GetSelectionMark(h);
  int count = ListView_GetItemCount(h);
  
  if(mark != -1) {
    LVITEM item;
    for(int i = 0; i < count; i++) {
      memset(&item,0, sizeof(item));
      item.mask = LVIF_PARAM|LVIF_STATE;
      item.stateMask = LVIS_SELECTED;
      item.iItem = i;
      if(ListView_GetItem(h,
                          &item)) {
        if(item.state & LVIS_SELECTED) {
          if(cheatsList[item.lParam].status & 2)
            cheatsDisable(item.lParam);
          else
            cheatsEnable(item.lParam);
        }
      }
    }
    refresh(h);
  }
}

void GBACheatListDlg::OnNumberType(UINT id)
{
  switch(id) {
  case IDC_SIGNED:
    numberType = 0;
    regSetDwordValue("cheatsNumberType", 0);
    refresh(GetDlgItem(IDC_CHEAT_LIST));
    break;
  case IDC_UNSIGNED:
    numberType = 1;
    regSetDwordValue("cheatsNumberType", 1);
    refresh(GetDlgItem(IDC_CHEAT_LIST));
    break;
  case IDC_HEXADECIMAL:
    numberType = 2;
    regSetDwordValue("cheatsNumberType", 2);
    refresh(GetDlgItem(IDC_CHEAT_LIST));
    break;
  }
}

void GBACheatListDlg::OnRestore()
{
  restoreValues = !restoreValues;
  regSetDwordValue("cheatsRestore", restoreValues);
}

/**************************************************************************
 * Add GSA Cheat
 **************************************************************************/

BEGIN_MESSAGE_MAP(AddGSACodeDlg, Dlg)
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(ID_CANCEL, OnCancel)
END_MESSAGE_MAP()

AddGSACodeDlg::AddGSACodeDlg()
  : Dlg()
{  
}

BOOL AddGSACodeDlg::OnInitDialog(LPARAM)
{
  SendMessage(GetDlgItem(IDC_CODE),
              EM_LIMITTEXT,
              1024,
              0);
  SendMessage(GetDlgItem(IDC_DESC),
              EM_LIMITTEXT,
              32,
              0);
  SetWindowText(winResLoadString(IDS_ADD_GSA_CODE));
  winCenterWindow(getHandle());
  return TRUE;
}

void AddGSACodeDlg::OnOk()
{  
  char desc[100];
  char buffer[1024];
  GetWindowText(GetDlgItem(IDC_CODE), buffer, 1024);
  GetWindowText(GetDlgItem(IDC_DESC), desc, 100);
  
  char *s = strtok(buffer, "\n\r");
  while(s) {
    s[16] = 0;
    cheatsAddGSACode(s, desc);
    s = strtok(NULL, "\n\r");
  }
  EndDialog(TRUE);
}

void AddGSACodeDlg::OnCancel()
{
  EndDialog(FALSE);
}

/**************************************************************************
 * Add CBA Cheat
 **************************************************************************/
BEGIN_MESSAGE_MAP(AddCBACodeDlg, Dlg)
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(ID_CANCEL, OnCancel)
END_MESSAGE_MAP()

AddCBACodeDlg::AddCBACodeDlg()
  : Dlg()
{  
}

BOOL AddCBACodeDlg::OnInitDialog(LPARAM)
{
  SendMessage(GetDlgItem(IDC_CODE),
              EM_LIMITTEXT,
              1024,
              0);
  SendMessage(GetDlgItem(IDC_DESC),
              EM_LIMITTEXT,
              32,
              0);
  SetWindowText(winResLoadString(IDS_ADD_CBA_CODE));
  winCenterWindow(getHandle());
  return TRUE;
}

void AddCBACodeDlg::OnOk()
{  
  char desc[100];
  char buffer[1024];
  GetWindowText(GetDlgItem(IDC_CODE), buffer, 1024);
  GetWindowText(GetDlgItem(IDC_DESC), desc, 100);
  
  char *s = strtok(buffer, "\n\r");
  while(s) {
    s[13] = 0;
    cheatsAddCBACode(s, desc);
    s = strtok(NULL, "\n\r");
  }
  EndDialog(TRUE);
}

void AddCBACodeDlg::OnCancel()
{
  EndDialog(FALSE);
}

/**************************************************************************
 * Add GBA Cheat
 **************************************************************************/
BEGIN_MESSAGE_MAP(AddCheatCodeDlg, Dlg)
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(ID_CANCEL, OnCancel)
END_MESSAGE_MAP()

AddCheatCodeDlg::AddCheatCodeDlg()
  : Dlg()
{  
}

BOOL AddCheatCodeDlg::OnInitDialog(LPARAM)
{
  SendMessage(GetDlgItem(IDC_CODE),
              EM_LIMITTEXT,
              1024,
              0);
  SendMessage(GetDlgItem(IDC_DESC),
              EM_LIMITTEXT,
              32,
              0);
  SetWindowText(winResLoadString(IDS_ADD_CHEAT_CODE));
  winCenterWindow(getHandle());
  return TRUE;
}

void AddCheatCodeDlg::OnOk()
{  
  char desc[100];
  char buffer[1024];
  GetWindowText(GetDlgItem(IDC_CODE), buffer, 1024);
  GetWindowText(GetDlgItem(IDC_DESC), desc, 100);
  
  char *s = strtok(buffer, "\n\r");
  while(s) {
    if(strlen(s) > 17)
      s[17] = 0;
    cheatsAddCheatCode(s, desc);
    s = strtok(NULL, "\n\r");
  }
  EndDialog(TRUE);
}

void AddCheatCodeDlg::OnCancel()
{
  EndDialog(FALSE);
}

/**************************************************************************
 * Add GBA Cheat
 **************************************************************************/
BEGIN_MESSAGE_MAP(AddCheatDlg, Dlg)
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(ID_CANCEL, OnCancel)
  ON_CONTROL_RANGE(BN_CLICKED, IDC_SIGNED, IDC_HEXADECIMAL, OnNumberType)
  ON_CONTROL_RANGE(BN_CLICKED, IDC_SIZE_8, IDC_SIZE_32, OnSizeType)
  ON_BN_CLICKED(IDC_FREEZE, OnFreeze)
END_MESSAGE_MAP()

AddCheatDlg::AddCheatDlg()
  : Dlg()
{
}

BOOL AddCheatDlg::addCheat()
{
  char buffer[128];
  char code[20];

  GetWindowText(GetDlgItem(IDC_ADDRESS),
                buffer,
                127);
  buffer[127] = 0;
  u32 address = 0;
  sscanf(buffer, "%x", &address);
  if((address >= 0x02000000 && address < 0x02040000) ||
     (address >= 0x03000000 && address < 0x03008000)) {
  } else {
    systemMessage(IDS_INVALID_ADDRESS, "Invalid address: %08x", address);
    return FALSE;
  }
  if(sizeType != 0) {
    if(sizeType == 1 && address & 1) {
      systemMessage(IDS_MISALIGNED_HALFWORD,
                    "Misaligned half-word address: %08x", address);
      return FALSE;
    }
    if(sizeType == 2 && address & 3) {
      systemMessage(IDS_MISALIGNED_WORD,
                    "Misaligned word address: %08x", address);
      return FALSE;
    }    
  }
  u32 value;
  GetWindowText(GetDlgItem(IDC_VALUE),
                buffer,
                127);
  buffer[127] = 0;
  
  if(buffer[0] == 0) {
    systemMessage(IDS_VALUE_CANNOT_BE_EMPTY, "Value cannot be empty");
    return FALSE;
  }
  
  switch(numberType) {
  case 0:
    sscanf(buffer, "%d", &value);
    break;
  case 1:
    sscanf(buffer, "%u", &value);
    break;
  default:
    sscanf(buffer, "%x", &value);
  }

  GetWindowText(GetDlgItem(IDC_DESC), buffer, 32);

  switch(sizeType) {
  case 0:
    sprintf(code, "%08x:%02x", address, value);
    break;
  case 1:
    sprintf(code, "%08x:%04x", address, value);
    break;
  case 2:
    sprintf(code, "%08x:%08x", address, value);
    break;
  }
  
  cheatsAdd(code, buffer,address, value,-1, sizeType, freeze);
  return TRUE;
}

BOOL AddCheatDlg::OnInitDialog(LPARAM lParam)
{
  u32 address = (u32)lParam;
  
  if(address != 0) {
    char buffer[16];
    sprintf(buffer,"%08x", address);
    ::SetWindowText(GetDlgItem(IDC_ADDRESS),
                    buffer);
    EnableWindow(GetDlgItem(IDC_ADDRESS),
                 FALSE);
  }
  
  numberType = regQueryDwordValue("cheatsNumberType", 2);
  if(numberType < 0 || numberType > 2)
    numberType = 2;
  DoRadio(false, IDC_SIGNED, numberType);
  
  sizeType = regQueryDwordValue("cheatsSizeType", 0);
  if(sizeType < 0 || sizeType > 2)
    sizeType = 0;
  DoRadio(false, IDC_SIZE_8, sizeType);
  
  freeze = regQueryDwordValue("cheatsFreeze", 0) ?
    true : false;
  int fr = (int)freeze;
  DoCheckbox(false, IDC_FREEZE, fr);

  SendMessage(GetDlgItem(IDC_DESC),
              EM_LIMITTEXT,
              32,
              0);
  if(address != 0) {
    EnableWindow(GetDlgItem(IDC_SIZE_8),
                 FALSE);
    EnableWindow(GetDlgItem(IDC_SIZE_16),
                 FALSE);
    EnableWindow(GetDlgItem(IDC_SIZE_32),
                 FALSE);
    EnableWindow(GetDlgItem(IDC_HEXADECIMAL),
                 FALSE);
    EnableWindow(GetDlgItem(IDC_UNSIGNED),
                 FALSE);
    EnableWindow(GetDlgItem(IDC_SIGNED),
                 FALSE);        
  }

  winCenterWindow(getHandle());
  return TRUE;
}

void AddCheatDlg::OnOk()
{
  // add cheat
  if(addCheat()) {
    EndDialog(TRUE);
  }
}

void AddCheatDlg::OnCancel()
{
  EndDialog(FALSE);
}

void AddCheatDlg::OnNumberType(UINT id)
{
  switch(id) {
  case IDC_SIGNED:
    numberType = 0;
    regSetDwordValue("cheatsNumberType", 0);
    break;
  case IDC_UNSIGNED:
    numberType = 1;
    regSetDwordValue("cheatsNumberType", 1);
    break;
  case IDC_HEXADECIMAL:
    numberType = 2;
    regSetDwordValue("cheatsNumberType", 2);
    break;
  }
}

void AddCheatDlg::OnSizeType(UINT id)
{
  switch(id) {
  case IDC_SIZE_8:
    sizeType = SIZE_8;
    regSetDwordValue("cheatsSizeType", 0);
    break;
  case IDC_SIZE_16:
    sizeType = SIZE_16;
    regSetDwordValue("cheatsSizeType", 1);
    break;
  case IDC_SIZE_32:
    sizeType = SIZE_32;
    regSetDwordValue("cheatsSizeType", 2);
    break;
  }
}

void AddCheatDlg::OnFreeze()
{
  if(SendMessage(GetDlgItem(IDC_FREEZE),
                 BM_GETCHECK,
                 0,
                 0) & BST_CHECKED)
    freeze = true;
  else
    freeze = false;
  regSetDwordValue("cheatsFreeze", freeze);
}

void winCheatsListDialog()
{
  GBACheatListDlg dlg;
  dlg.DoModal(IDD_CHEAT_LIST,
              hWindow,
              0);
}

void winCheatsAddCBACodeDialog(HWND parent)
{
  AddCBACodeDlg dlg;
  dlg.DoModal(IDD_ADD_CHEAT_DLG,
              parent,
              0);
}

void winCheatsAddGSACodeDialog(HWND parent)
{
  AddGSACodeDlg dlg;
  dlg.DoModal(IDD_ADD_CHEAT_DLG,
              parent,
              0);
}

void winCheatsAddCheatCodeDialog(HWND parent)
{
  AddCheatCodeDlg dlg;
  dlg.DoModal(IDD_ADD_CHEAT_DLG,
              parent,
              0);
}

void winCheatsAddDialog(HWND parent, u32 address)
{
  AddCheatDlg dlg;
  dlg.DoModal(IDD_ADD_CHEAT,
              parent,
              address);
}

void winCheatsDialog()
{
  GBACheatSearchDlg dlg;
  dlg.DoModal(IDD_CHEATS,
              hWindow,
              0);
}
