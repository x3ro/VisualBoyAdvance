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

/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <stdio.h>
#include "../Globals.h"
#include "../Cheats.h"

static bool initialized = false;

void GBACheatSearch::init()
{
  updateValues = false;
  sizeType = 0;
  numberType = 2;
  searchType = 0;
  valueType = 0;
  
  results->clear();
  results->setSorting(-1);
  results->removeColumn(0);
  results->addColumn(tr("Address"));
  results->addColumn(tr("Old Value"));
  results->addColumn(tr("New Value"));

  if(!initialized) {
    search->setEnabled(false);
    add->setEnabled(false);
  }

  switch(valueType) {
  case 1:
    specific->setChecked(true);
    break;
  default:
    old->setChecked(true);
    break;
  }
  
  switch(sizeType) {
  case 1:
    size16->setChecked(true);
    break;
  case 2:
    size32->setChecked(true);
    break;
  default:
    size8->setChecked(true);
    break;
  }

  switch(searchType) {
  case 1:
    notEqual->setChecked(true);
    break;
  case 2:
    lessThan->setChecked(true);
    break;
  case 3:
    lessOrEqual->setChecked(true);
    break;
  case 4:
    greaterThan->setChecked(true);
    break;
  case 5:
    greaterOrEqual->setChecked(true);
    break;
  default:
    equal->setChecked(true);
    break;
  }

  switch(numberType) {
  case 0:
    typeSigned->setChecked(true);
    break;
  case 1:
    typeUnsigned->setChecked(true);
    break;
  default:
    typeHex->setChecked(true);
    break;
  }
  
  value->setEnabled(updateValues);
  
  if(initialized)
      addChanges(false);
}


void GBACheatSearch::ok_clicked()
{
  accept();
}


void GBACheatSearch::old_clicked()
{
  valueType = 0;
  value->setEnabled(false);
}


void GBACheatSearch::specific_clicked()
{
  valueType = 1;
  value->setEnabled(true);
}


void GBACheatSearch::size8_released()
{
  sizeType = 0;
  if(initialized)
      addChanges(false);
}


void GBACheatSearch::size16_clicked()
{
  sizeType = 1;
  if(initialized)
      addChanges(false);
}


void GBACheatSearch::size32_clicked()
{
  sizeType = 2;
  if(initialized)
      addChanges(false);
}


void GBACheatSearch::equal_clicked()
{
  searchType = 0;
}


void GBACheatSearch::notEqual_clicked()
{
  searchType = 1;
}


void GBACheatSearch::lessThan_clicked()
{
  searchType = 2;
}


void GBACheatSearch::lessOrEqual_clicked()
{
  searchType = 3;
}


void GBACheatSearch::greaterThan_clicked()
{
  searchType = 4;
}


void GBACheatSearch::greaterOrEqual_clicked()
{
  searchType = 5;
}


void GBACheatSearch::typeSigned_clicked()
{
  numberType = 0;
  if(initialized)
      addChanges(false);
}


void GBACheatSearch::typeUnsigned_clicked()
{
  numberType = 1;
  if(initialized)
      addChanges(false);
}


void GBACheatSearch::typeHex_clicked()
{
  numberType = 2;
  if(initialized)
      addChanges(false);
}


void GBACheatSearch::update_toggled( bool  on)
{
  updateValues = on;
}


void GBACheatSearch::start_clicked()
{
  cheatsReset();
  search->setEnabled(true);
  initialized = true; 
}


void GBACheatSearch::search_clicked()
{
  char buffer[128];
  if(valueType == 0)
    cheatsSearchChange(searchType,
                       sizeType,
                       numberType == 0);
  else {
    strcpy(buffer,(const char *)value->text());
    if(strlen(buffer) == 0) {
      systemMessage(0, (char *)((const char *)tr("Number cannot be empty")));
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
  
  addChanges(true);

  if(updateValues)
    cheatsUpdateValues();

}


void GBACheatSearch::addChanges( bool  showMsgs)
{
  int count = cheatsGetCount(sizeType);
  
  results->clear();

  if(count > 1000) {
    if(showMsgs)
      systemMessage(0,
                    (char *)((const char *)tr("Search produced %d results. Please refine better")),
                    count);
    return;
  }

  if(count == 0) {
    if(showMsgs)
      systemMessage(0,
                    (char *)((const char *)tr("Search produced no results.")));
    return;
  }
  
  int inc = 1;
  switch(sizeType) {
  case 1:
    inc = 2;
    break;
  case 2:
    inc = 4;
    break;
  }
  int i;
  int index = 0;
  if(numberType == 0) {
    for(i = 0; i < 0x40000; i += inc) {
      if(TEST_BIT(cheatSearch.wBITS, i))
        addChange(index++,
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
        addChange(index++,
                  0x3000000 | i,
                  SIGNED_DATA(sizeType,
                              cheatSearch.iRAM,
                              i),
                  SIGNED_DATA(sizeType,
                              internalRAM,
                              i));
    }    
  } else {
    for(i = 0; i < 0x40000; i += inc) {
      if(TEST_BIT(cheatSearch.wBITS, i))
        addChange(index++,
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
        addChange(index++,
                  0x3000000 | i,
                  UNSIGNED_DATA(sizeType,
                                cheatSearch.iRAM,
                                i),
                  UNSIGNED_DATA(sizeType,
                                internalRAM,
                                i));
    }    
  }
}

void GBACheatSearch::addChange( int index, u32 address, u32 oldValue, u32 newValue)
{
  QListViewItem *item = new QListViewItem(results, results->lastItem(), 
					  QString().sprintf("%08x",address));
  switch(numberType) {
  case 0:
      item->setText(1, QString().sprintf("%d", oldValue));
      item->setText(2, QString().sprintf("%d",newValue));
    break;        
  case 1:
    item->setText(1, QString().sprintf("%u", oldValue));
    item->setText(2, QString().sprintf("%u", newValue));
    break;    
  case 2:
    switch(sizeType) {
    case 0:
      item->setText(1, QString().sprintf("%02x", oldValue));
      item->setText(2, QString().sprintf("%02x", newValue));
      break;
    case 1:
      item->setText(1, QString().sprintf("%04x", oldValue));
      item->setText(2, QString().sprintf("%04x", newValue));
      break;
    case 2:
      item->setText(1, QString().sprintf("%08x", oldValue));
      item->setText(2, QString().sprintf("%08x", newValue));
      break;
    }
  }
  item->setText(3, QString().setNum(index));
}
