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

#include "addcbacode.h"
#include "addgsacode.h"
#include "addgbacode.h"

#include "../System.h"
#include "../Cheats.h"

void GBACheatList::init()
{
  cheats->clear();
  cheats->setSorting(-1);
  cheats->removeColumn(0);
  cheats->addColumn(tr("Code"));
  cheats->addColumn(tr("Description"));
  cheats->addColumn(tr("Status"));

  cheats->setColumnWidth(0, 170);
  cheats->setColumnWidth(1, 150);
  cheats->setColumnWidth(2, 80);

  removeButton->setEnabled(false);
  enableButton->setEnabled(false);

  refresh();
}

void GBACheatList::codeBreakerButton_clicked()
{
  AddCBACode dlg(this);

  if(dlg.exec() == QDialog::Accepted)
    refresh();
}





void GBACheatList::okButton_clicked()
{
  accept();
}


void GBACheatList::cheats_currentChanged( QListViewItem *item )
{
  if(!duringRefresh) {
    QCheckListItem *check = (QCheckListItem *)item;
    int index = item->text(3).toInt();
    if(check->isOn())
      cheatsEnable(index);
    else
      cheatsDisable(index);
  }
}

void GBACheatList::cheats_selectionChanged()
{
  if(getSelectedCount() != 0) {
    removeButton->setEnabled(true);
    enableButton->setEnabled(true);
  } else {
    removeButton->setEnabled(false);
    enableButton->setEnabled(false);
  }
}


void GBACheatList::removeAllButton_clicked()
{
  cheatsDeleteAll(true);
  refresh();
}


void GBACheatList::removeButton_clicked()
{
  QListViewItem *item = cheats->firstChild();

  while(item) {
    if(item->isSelected()) {
      cheatsDelete(item->text(3).toInt(), true);
    }
    item = item->nextSibling();
  }
  refresh();
}


void GBACheatList::enableButton_clicked()
{
  QListViewItem *item = cheats->firstChild();

  while(item) {
    if(item->isSelected()) {
      QCheckListItem *check = (QCheckListItem *)item;
      if(check->isOn())
        cheatsDisable(item->text(3).toInt());
      else
        cheatsEnable(item->text(3).toInt());
    }
    item = item->nextSibling();
  }
  refresh();
}


int GBACheatList::getSelectedCount()
{
  int sel = 0;
  QListViewItem *item = cheats->firstChild();

  while(item) {
    if(item->isSelected())
      sel++;
    item = item->nextSibling();
  }

  return sel;
}


void GBACheatList::refresh()
{
  duringRefresh = true;

  cheats->clear();
  
  char buffer[100];
  QCheckListItem *last = NULL;
  for(int i = 0; i < cheatsNumber; i++) {
    QCheckListItem *item = new QCheckListItem(cheats,
                                              cheatsList[i].codestring,
                                              QCheckListItem::CheckBox);
    item->moveItem(last);
    item->setOn(cheatsList[i].enabled ? true : false);

    item->setText(1, cheatsList[i].desc);

    buffer[0] = (cheatsList[i].status & 2) ? 'E' : 'D';    
    buffer[1] = (cheatsList[i].status & 1) ? 'F' : ' ';
    buffer[2] = 0;
    item->setText(2, buffer);
    item->setText(3, QString().setNum(i));
    last = item;
  }
  duringRefresh = false;
}


void GBACheatList::gamesharkButton_clicked()
{
  AddGSACode dlg(this);

  if(dlg.exec() == QDialog::Accepted)
    refresh();
}


void GBACheatList::codeButton_clicked()
{
  AddGBACode dlg(this);

  if(dlg.exec() == QDialog::Accepted)
    refresh();
}
