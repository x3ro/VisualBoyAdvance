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

#include "../System.h"
#include "../Cheats.h"

#include <stdlib.h>

void AddGBACode::okButton_clicked()
{
  char buffer[1000];
  strcpy(buffer, code->text());
  
  char *s = strtok(buffer, "\n\r");
  while(s) {
    if(strlen(s) > 17)
      s[17] = 0;
    cheatsAddCheatCode(s, (char *)((const char *)descr->text()));
    s = strtok(NULL, "\n\r");
  }
  accept();
}


void AddGBACode::cancelButton_clicked()
{
  reject();
}
