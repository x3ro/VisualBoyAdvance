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
#include "Reg.h"
#include "../GBA.h"
#include "../Cheats.h"
#include "../gb/gbCheats.h"
#include "../NLS.h"
#include "resource.h"
#include "WinResUtil.h"

extern void winCenterWindow(HWND);
extern HWND hWindow;

struct WinGbCheatsData {
  int  bank;  
  u16  addr;
  char address[9];
  char oldValue[12];
  char newValue[12];
};

class GBCheatSearchDlg : public Dlg {
  static bool initialized;
  int numberType;
  int sizeType;
  int valueType;
  int searchType;
  bool updateValues;
  WinGbCheatsData *data;
protected:
  DECLARE_MESSAGE_MAP()
public:
  GBCheatSearchDlg();
  ~GBCheatSearchDlg();

  int getBank(u16 addr, int j);
  void addChange(HWND lv,
                 int index,
                 int bank,
                 u16 address,
                 int offset,
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
  void OnNumberType(UINT);
  void OnSizeType(UINT);
  void OnUpdate();
  void OnSearchType(UINT);
};

class GBCheatListDlg : public Dlg {
  bool duringRefresh;
protected:
  DECLARE_MESSAGE_MAP()
public:
  GBCheatListDlg();

  void refresh(HWND lv);
  
  virtual BOOL OnInitDialog(LPARAM);
  void OnItemChanged(NMHDR *, LRESULT *);
  void OnOk();
  void OnAddGGCheat();
  void OnAddGSCheat();
  void OnRemoveAll();
  void OnRemove();
  void OnEnable();
};

class AddGBCheatDlg : public Dlg {
  int numberType;
  int sizeType;
protected:
  DECLARE_MESSAGE_MAP()
public:
  AddGBCheatDlg();

  BOOL addCheat();
  
  virtual BOOL OnInitDialog(LPARAM);
  void OnOk();
  void OnCancel();
  void OnNumberType(UINT);
  void OnSizeType(UINT);
};

class AddGBCodeDlg : public Dlg {
  bool (*addVerify)(char *, char*);
  int addLength;
  char *addTitle;
protected:
  DECLARE_MESSAGE_MAP()
public:
  AddGBCodeDlg(bool (*verify)(char *, char*),int, char*);

  virtual BOOL OnInitDialog(LPARAM);
  void OnOk();
  void OnCancel();
};

bool GBCheatSearchDlg::initialized = false;

void winGbCheatsDialog()
{
  GBCheatSearchDlg dlg;
  dlg.DoModal(IDD_CHEATS, hWindow, 0);
}

void winGbCheatsAddDialog(HWND parent, u32 address)
{
  AddGBCheatDlg dlg;
  dlg.DoModal(IDD_ADD_CHEAT,
              parent,
              address);
}

void winGbCheatsListDialog()
{
  GBCheatListDlg dlg;
  dlg.DoModal(IDD_GB_CHEAT_LIST,
              hWindow, 0);
}

bool winGbCheatAddVerifyGs(char *code, char *desc)
{
  gbAddGsCheat(code, desc);
  return true;
}

bool winGbCheatAddVerifyGg(char *code, char *desc)
{
  gbAddGgCheat(code, desc);
  return true;
}

/**************************************************************************
 * Add GB Cheat after search
 **************************************************************************/

BEGIN_MESSAGE_MAP(AddGBCheatDlg, Dlg)
  ON_BN_CLICKED(ID_CANCEL, OnCancel)
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_CONTROL_RANGE(BN_CLICKED, IDC_SIGNED, IDC_HEXADECIMAL, OnNumberType)
  ON_CONTROL_RANGE(BN_CLICKED, IDC_SIZE_8, IDC_SIZE_32, OnSizeType)
END_MESSAGE_MAP()

AddGBCheatDlg::AddGBCheatDlg()
  : Dlg()
{
}

BOOL AddGBCheatDlg::addCheat()
{
  char buffer[128];
  char code[20];

  u32 address = (u32)::GetWindowLong(GetDlgItem(IDC_ADDRESS),
                                   GWL_USERDATA);
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

  int bank = (address >> 16);
  address &= 0xFFFF;

  if(address >= 0xd000)
    bank += 0x90;
  else
    bank = 0x01;
  
  switch(sizeType) {
  case 0:
    sprintf(code, "%02x%02x%02x%02x", bank, value, address&0xFF, address>>8);
    gbAddGsCheat(code, buffer);
    break;
  case 1:
    sprintf(code, "%02x%02x%02x%02x", bank, value&0xFF, address&0xFF,
            address>>8);
    gbAddGsCheat(code, buffer);
    address++;
    sprintf(code, "%02x%02x%02x%02x", bank, value>>8, address&0xFF,
            address>>8);
    gbAddGsCheat(code, buffer);    
    break;
  case 2:
    sprintf(code, "%02x%02x%02x%02x", bank, value&0xFF, address&0xFF,
            address>>8);
    gbAddGsCheat(code, buffer);
    address++;
    sprintf(code, "%02x%02x%02x%02x", bank, (value>>8) & 0xFF, address&0xFF,
            address>>8);
    gbAddGsCheat(code, buffer);
    address++;
    sprintf(code, "%02x%02x%02x%02x", bank, (value>>16)&0xFF, address&0xFF,
            address>>8);
    gbAddGsCheat(code, buffer);
    address++;
    sprintf(code, "%02x%02x%02x%02x", bank, value>>24, address&0xFF,
            address>>8);
    gbAddGsCheat(code, buffer);    
    break;
  }
  
  return TRUE;
}

BOOL AddGBCheatDlg::OnInitDialog(LPARAM lParam)
{
  u32 address = (u32)lParam;
  
  char buffer[16];
  sprintf(buffer,"%02x:%08x", (address>>16), address&0xFFFF);
  ::SetWindowText(GetDlgItem(IDC_ADDRESS),
                  buffer);
  EnableWindow(GetDlgItem(IDC_ADDRESS),
               FALSE);
  ::SetWindowLong(GetDlgItem(IDC_ADDRESS),
                GWL_USERDATA,
                address);
  
  numberType = regQueryDwordValue("gbCheatsNumberType", 2);
  if(numberType < 0 || numberType > 2)
    numberType = 2;
  DoRadio(false, IDC_SIGNED, numberType);
  
  sizeType = regQueryDwordValue("gbCheatsSizeType", 0);
  if(sizeType < 0 || sizeType > 2)
    sizeType = 0;
  DoRadio(false, IDC_SIZE_8, sizeType);
  
  EnableWindow(GetDlgItem(IDC_FREEZE),
               FALSE);
  
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

void AddGBCheatDlg::OnOk()
{
  // add cheat
  if(addCheat()) {
    EndDialog(TRUE);
  }
}

void AddGBCheatDlg::OnCancel()
{
  EndDialog(FALSE);
}

void AddGBCheatDlg::OnNumberType(UINT id)
{
  switch(id) {
  case IDC_SIGNED:
    numberType = 0;
    regSetDwordValue("gbCheatsNumberType", 0);
    break;
  case IDC_UNSIGNED:
    numberType = 1;
    regSetDwordValue("gbCheatsNumberType", 1);
    break;
  case IDC_HEXADECIMAL:
    numberType = 2;
    regSetDwordValue("gbCheatsNumberType", 2);
    break;
  }
}

void AddGBCheatDlg::OnSizeType(UINT id)
{
  switch(id) {
  case IDC_SIZE_8:
    sizeType = SIZE_8;
    regSetDwordValue("gbCheatsSizeType", 0);
    break;
  case IDC_SIZE_16:
    sizeType = SIZE_16;
    regSetDwordValue("gbCheatsSizeType", 1);
    break;
  case IDC_SIZE_32:
    sizeType = SIZE_32;
    regSetDwordValue("gbCheatsSizeType", 2);
    break;
  }
}

/**************************************************************************
 * Search for GB Cheats
 **************************************************************************/

BEGIN_MESSAGE_MAP(GBCheatSearchDlg, Dlg)
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

GBCheatSearchDlg::GBCheatSearchDlg()
  : Dlg()
{
  data = NULL;
}

GBCheatSearchDlg::~GBCheatSearchDlg()
{
  if(data)
    free(data);
}

int GBCheatSearchDlg::getBank(u16 addr, int j)
{
  switch(addr >> 12) {
  case 0x0a:
    return j / 0x2000;
  case 0x0d:
    return j / 0x1000;
  }
  return 0;
}

void GBCheatSearchDlg::addChange(HWND lv,
                                 int index,
                                 int bank,
                                 u16 address,
                                 int offset,
                                 u32 oldValue,
                                 u32 newValue)
{
  data[index].bank = bank;
  if(bank) {
    if(address == 0xa000)
      address |= offset & 0x1fff;
    else
      address |= offset & 0xfff;
  } else
    address |= offset;
  data[index].addr = address;
  sprintf(data[index].address, "%02x:%04x",bank,address);
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

void GBCheatSearchDlg::addChanges(HWND lv, bool showMsg)
{
  int count = gbCheatsGetCount(sizeType);
    
  ListView_DeleteAllItems(lv);

  if(count > 1000) {
    if(showMsg)
      systemMessage(IDS_SEARCH_PRODUCED_TOO_MANY,
                    "Search produced %d results. Please refine better",
                    count);
    return;
  }  

  if(count == 0) {
    if(showMsg)
      systemMessage(IDS_SEARCH_PRODUCED_NO_RESULTS,
                    "Search produced no results");
    return;
  }
  
  ListView_SetItemCount(lv, count);  
  if(data)
    free(data);

  data = (WinGbCheatsData *)malloc(count *
                                   sizeof(WinGbCheatsData));
  
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
    for(int i = 0; i < gbCheatsSearchCount; i++) {
      int j;
      int end = gbCheatsSearchMap[i].mask + 1;
      end -= (inc - 1);
      for(j = 0; j < end; j++) {
        if(TEST_BIT(gbCheatsSearchMap[i].bits, j))
          addChange(lv,
                    index++,
                    getBank(gbCheatsSearchMap[i].
                            address, j),
                    gbCheatsSearchMap[i].address,
                    j,
                    SIGNED_DATA(sizeType,
                                gbCheatsSearchMap[i].data,
                                j),
                    SIGNED_DATA(sizeType,
                                gbCheatsSearchMap[i].memory,
                                j));
      }
    }
  } else {
    for(int i = 0; i < gbCheatsSearchCount; i++) {
      int j;
      int end = gbCheatsSearchMap[i].mask + 1;
      end -= (inc - 1);
      for(j = 0; j < end; j++) {
        if(TEST_BIT(gbCheatsSearchMap[i].bits, j))
          addChange(lv,
                    index++,
                    getBank(gbCheatsSearchMap[i].
                            address, j),
                    gbCheatsSearchMap[i].address,
                    j,
                    UNSIGNED_DATA(sizeType,
                                  gbCheatsSearchMap[i].data,
                                  j),
                    UNSIGNED_DATA(sizeType,
                                  gbCheatsSearchMap[i].memory,
                                  j));
      }
    }
  }

  for(int i = 0; i < count; i++) {
    LVITEM item;

    item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    item.iItem = i;
    item.iSubItem = 0;
    item.lParam = data[i].addr|
      (data[i].bank << 16);
    item.state = 0;
    item.stateMask = 0;
    item.pszText = LPSTR_TEXTCALLBACK;
    ListView_InsertItem(lv, &item);

    ListView_SetItemText(lv, i, 1, LPSTR_TEXTCALLBACK);
    ListView_SetItemText(lv, i, 2, LPSTR_TEXTCALLBACK);
  }  
}

void GBCheatSearchDlg::search()
{
  char buffer[128];
  if(valueType == 0)
    gbCheatsSearchChange(searchType,
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
    gbCheatsSearchValue(searchType,
                        sizeType,
                        numberType == 0,
                        value);
  }
  
  HWND lv = GetDlgItem(IDC_CHEAT_LIST);

  addChanges(lv, true);

  if(updateValues)
    gbCheatsUpdateValues();
}

BOOL GBCheatSearchDlg::OnInitDialog(LPARAM)
{
  HWND h = GetDlgItem(IDC_CHEAT_LIST);
  LVCOLUMN col;
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

  valueType = regQueryDwordValue("gbCheatsValueType", 0);
  if(valueType < 0 || valueType > 1)
    valueType = 2;
  DoRadio(false, IDC_OLD_VALUE, valueType);
  
  searchType = regQueryDwordValue("gbCheatsSearchType",
                                  GBA_EQUAL);
  if(searchType < 0 || searchType > 5)
    searchType = 0;
  DoRadio(false, IDC_EQ, searchType);
  
  numberType = regQueryDwordValue("gbCheatsNumberType", 2);
  if(numberType < 0 || numberType > 2)
    numberType = 2;
  DoRadio(false, IDC_SIGNED, numberType);
  
  sizeType = regQueryDwordValue("gbCheatsSizeType", 0);
  if(sizeType < 0 || sizeType > 2)
    sizeType = 0;
  DoRadio(false, IDC_SIZE_8, sizeType);
  
  updateValues = regQueryDwordValue("gbCheatsUpdate", 0) ?
    true : false;
  int u = (int)updateValues;

  DoCheckbox(false, IDC_UPDATE, u);
  
  if(valueType == 0)
    EnableWindow(GetDlgItem(IDC_VALUE), FALSE);
  
  winCenterWindow(getHandle());

  if(initialized) {
    addChanges(h, false);
  }
  return TRUE;  
}

void GBCheatSearchDlg::OnGetDispInfo(NMHDR *hdr, LRESULT *pResult)
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

void GBCheatSearchDlg::OnItemChanged(NMHDR *hdr, LRESULT *pResult)
{
  if(ListView_GetSelectionMark(GetDlgItem(IDC_CHEAT_LIST)) != -1)
    EnableWindow(GetDlgItem(IDC_ADD_CHEAT),
                 TRUE);
  else
    EnableWindow(GetDlgItem(IDC_ADD_CHEAT),
                 FALSE);
}

void GBCheatSearchDlg::OnOk()
{
  if(data)
    free(data);
  data = NULL;
  EndDialog(TRUE);
}

void GBCheatSearchDlg::OnStart()
{
  gbCheatsInitialize();
  EnableWindow(GetDlgItem(IDC_SEARCH), TRUE);
  initialized = true;
}

void GBCheatSearchDlg::OnAddCheat()
{
  int mark = ListView_GetSelectionMark(GetDlgItem(IDC_CHEAT_LIST));
  
  if(mark != -1) {
    LVITEM item;
    memset(&item,0, sizeof(item));
    item.mask = LVIF_PARAM;
    item.iItem = mark;
    if(ListView_GetItem(GetDlgItem(IDC_CHEAT_LIST),
                        &item)) {
      winGbCheatsAddDialog(getHandle(), (u32)item.lParam);
    }
  }
}

void GBCheatSearchDlg::OnValueType(UINT id)
{
  switch(id) {
  case IDC_OLD_VALUE:
    valueType = 0;
    EnableWindow(GetDlgItem(IDC_VALUE), FALSE);
    regSetDwordValue("gbCheatsValueType", 0);
    break;
  case IDC_SPECIFIC_VALUE:
    valueType = 1;
    EnableWindow(GetDlgItem(IDC_VALUE), TRUE);
    regSetDwordValue("gbCheatsValueType", 1);   
    break;
  }
}

void GBCheatSearchDlg::OnSearchType(UINT id)
{
  switch(id) {
  case IDC_EQ:
    searchType = GBA_EQUAL;
    regSetDwordValue("gbCheatsSearchType", 0);
    break;
  case IDC_NE:
    searchType = GBA_NOT_EQUAL;
    regSetDwordValue("gbCheatsSearchType", 1);
    break;
  case IDC_LT:
    searchType = GBA_LESS_THAN;
    regSetDwordValue("gbCheatsSearchType", 2);
    break;
  case IDC_LE:
    searchType = GBA_LESS_EQUAL;
    regSetDwordValue("gbCheatsSearchType", 3);
    break;
  case IDC_GT:
    searchType = GBA_GREATER_THAN;
    regSetDwordValue("gbCheatsSearchType", 4);
    break;
  case IDC_GE:
    searchType = GBA_GREATER_EQUAL;
    regSetDwordValue("gbCheatsSearchType", 5);
    break;
  }
}

void GBCheatSearchDlg::OnNumberType(UINT id)
{
  HWND h;
  switch(id) {
  case IDC_SIGNED:
    numberType = 0;
    regSetDwordValue("gbCheatsNumberType", 0);
    h = GetDlgItem(IDC_CHEAT_LIST);
    if(ListView_GetItemCount(h)) {
      addChanges(h, false);
    }
    break;
  case IDC_UNSIGNED:
    numberType = 1;
    regSetDwordValue("gbCheatsNumberType", 1);
    h = GetDlgItem(IDC_CHEAT_LIST);
    if(ListView_GetItemCount(h)) {
      addChanges(h, false);
    }
    break;
  case IDC_HEXADECIMAL:
    numberType = 2;
    regSetDwordValue("gbCheatsNumberType", 2);
    h = GetDlgItem(IDC_CHEAT_LIST);
    if(ListView_GetItemCount(h)) {
      addChanges(h, false);
    }
    break;
  }
}

void GBCheatSearchDlg::OnSizeType(UINT id)
{
  HWND h;
  switch(id) {
  case IDC_SIZE_8:
    sizeType = SIZE_8;
    regSetDwordValue("gbCheatsSizeType", 0);
    h = GetDlgItem(IDC_CHEAT_LIST);
    if(ListView_GetItemCount(h)) {
      addChanges(h, false);
    }
    break;
  case IDC_SIZE_16:
    sizeType = SIZE_16;
    regSetDwordValue("gbCheatsSizeType", 1);
    h = GetDlgItem(IDC_CHEAT_LIST);
    if(ListView_GetItemCount(h)) {
      addChanges(h, false);
    }
    break;
  case IDC_SIZE_32:
    sizeType = SIZE_32;
    regSetDwordValue("gbCheatsSizeType", 2);
    h = GetDlgItem(IDC_CHEAT_LIST);
    if(ListView_GetItemCount(h)) {
      addChanges(h, false);
    }
    break;
  }
}

void GBCheatSearchDlg::OnUpdate()
{
  if(SendMessage(GetDlgItem(IDC_UPDATE),
                 BM_GETCHECK,
                 0,
                 0) & BST_CHECKED)
    updateValues = true;
  else
    updateValues = false;
  regSetDwordValue("gbCheatsUpdate", updateValues);
}

/**************************************************************************
 * GB Cheat List
 **************************************************************************/
BEGIN_MESSAGE_MAP(GBCheatListDlg, Dlg)
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(IDC_ADD_GG_CHEAT, OnAddGGCheat)
  ON_BN_CLICKED(IDC_ADD_GS_CHEAT, OnAddGSCheat)
  ON_BN_CLICKED(IDC_REMOVE_ALL, OnRemoveAll)
  ON_BN_CLICKED(IDC_REMOVE, OnRemove)
  ON_BN_CLICKED(IDC_ENABLE, OnEnable)
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_CHEAT_LIST, OnItemChanged)
END_MESSAGE_MAP()

GBCheatListDlg::GBCheatListDlg()
  : Dlg()
{
  duringRefresh = false;
}

BOOL GBCheatListDlg::OnInitDialog(LPARAM)
{
  LVCOLUMN col;      
  HWND h = GetDlgItem(IDC_CHEAT_LIST);
  col.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
  col.fmt = LVCFMT_LEFT;
  col.cx = 120;
  col.pszText = (char *)winResLoadString(IDS_CODE);
  col.iSubItem = 0;
  ListView_InsertColumn(h, 0, &col);
  col.pszText = (char *)winResLoadString(IDS_DESCRIPTION);
  col.iSubItem = 1;
  col.cx = 200;
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
  
  refresh(h);
  EnableWindow(GetDlgItem(IDC_REMOVE), FALSE);
  EnableWindow(GetDlgItem(IDC_ENABLE), FALSE);
  winCenterWindow(getHandle());
  return TRUE;
}

void GBCheatListDlg::OnItemChanged(NMHDR *hdr, LRESULT *pResult)
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
          gbCheatEnable(l->lParam);
        else
          gbCheatDisable(l->lParam);
        refresh(GetDlgItem(IDC_CHEAT_LIST));
      }
    }
  }    
}

void GBCheatListDlg::OnOk()
{
  EndDialog(TRUE);
}

void GBCheatListDlg::OnAddGGCheat()
{
  AddGBCodeDlg dlg(winGbCheatAddVerifyGg, 11,
                   (char *)winResLoadString(IDS_ADD_GG_CODE));
  dlg.DoModal(IDD_ADD_CHEAT_DLG, getHandle(), 0);
  refresh(GetDlgItem(IDC_CHEAT_LIST));
}

void GBCheatListDlg::OnAddGSCheat()
{
  AddGBCodeDlg dlg(winGbCheatAddVerifyGs, 8,
                   (char *)winResLoadString(IDS_ADD_GS_CODE));
  dlg.DoModal(IDD_ADD_CHEAT_DLG,
              getHandle(), 0);
  refresh(GetDlgItem(IDC_CHEAT_LIST));
}

void GBCheatListDlg::OnRemoveAll()
{
  gbCheatRemoveAll();
  refresh(GetDlgItem(IDC_CHEAT_LIST));    
}

void GBCheatListDlg::OnRemove()
{
  HWND h = GetDlgItem(IDC_CHEAT_LIST);
  int mark = ListView_GetSelectionMark(h);
  
  if(mark != -1) {
    LVITEM item;
    memset(&item,0, sizeof(item));
    item.mask = LVIF_PARAM;
    item.iItem = mark;
    if(ListView_GetItem(h,
                        &item)) {
      gbCheatRemove(item.lParam);
      refresh(h);
    }       
  }
}

void GBCheatListDlg::OnEnable()
{
  HWND h = GetDlgItem(IDC_CHEAT_LIST);
  int mark = ListView_GetSelectionMark(h);
  
  if(mark != -1) {
    LVITEM item;
    memset(&item,0, sizeof(item));
    item.mask = LVIF_PARAM;
    item.iItem = mark;
    if(ListView_GetItem(h,
                        &item)) {
      if(gbCheatList[item.lParam].enabled)
        gbCheatDisable(item.lParam);
      else
        gbCheatEnable(item.lParam);
      refresh(h);
    }       
  }
}

void GBCheatListDlg::refresh(HWND lv)
{
  duringRefresh = true;
  
  ListView_DeleteAllItems(lv);
  
  char buffer[2];
  
  for(int i = 0; i < gbCheatNumber; i++) {
    LVITEM item;
    
    item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    item.iItem = i;
    item.iSubItem = 0;
    item.lParam = i;
    item.state = 0;
    item.stateMask = 0;
    item.pszText = gbCheatList[i].cheatCode;
    ListView_InsertItem(lv, &item);

    ListView_SetCheckState(lv, i, (gbCheatList[i].enabled ? TRUE : FALSE));
    
    ListView_SetItemText(lv, i, 1, gbCheatList[i].cheatDesc);
    
    buffer[0] = (gbCheatList[i].enabled) ? 'E' : 'D';    
    buffer[1] = 0;
    ListView_SetItemText(lv, i, 2, buffer);
  }
  duringRefresh = false;
}

/**************************************************************************
 * Add a GB cheat code
 **************************************************************************/

BEGIN_MESSAGE_MAP(AddGBCodeDlg, Dlg)
  ON_BN_CLICKED(ID_OK, OnOk)
  ON_BN_CLICKED(ID_CANCEL, OnCancel)
END_MESSAGE_MAP()

AddGBCodeDlg::AddGBCodeDlg(bool (*verify)(char *,char*), int len, char *title)
  : Dlg()
{
  addVerify = verify;
  addLength = len;
  addTitle = title;
}

BOOL AddGBCodeDlg::OnInitDialog(LPARAM)
{
  SendMessage(GetDlgItem(IDC_CODE),
              EM_LIMITTEXT,
              1024,
              0);
  SendMessage(GetDlgItem(IDC_DESC),
              EM_LIMITTEXT,
              32,
              0);    
  SetWindowText(addTitle);
  winCenterWindow(getHandle());
  return TRUE;
}

void AddGBCodeDlg::OnOk()
{
  char buffer[1024];
  char desc[32];
  
  GetWindowText(GetDlgItem(IDC_CODE), buffer, 1024);
  GetWindowText(GetDlgItem(IDC_DESC), desc, 32);
  char *s = strtok(buffer, "\n\r");
  while(s) {
    if((int)strlen(s) > addLength)
      s[addLength] = 0;
    addVerify(s, desc);
    s = strtok(NULL, "\n\r");
  }
  EndDialog(TRUE);
}

void AddGBCodeDlg::OnCancel()
{
  EndDialog(FALSE);
}
