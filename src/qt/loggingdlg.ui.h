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

static Logging *logDlg = NULL;

void Logging::CheckBox1_stateChanged( int )
{
  systemVerbose ^= 1;
}


void Logging::CheckBox2_stateChanged( int )
{
  systemVerbose ^= 2;
}


void Logging::CheckBox3_stateChanged( int )
{
  systemVerbose ^= 4;
}


void Logging::CheckBox4_stateChanged( int )
{
  systemVerbose ^= 8;
}


void Logging::CheckBox5_stateChanged( int )
{
  systemVerbose ^= 16;
}


void Logging::CheckBox6_stateChanged( int )
{
  systemVerbose ^= 32;
}


void Logging::CheckBox7_stateChanged( int )
{
  systemVerbose ^= 64;
}


void Logging::CheckBox8_stateChanged( int )
{
  systemVerbose ^= 128;
}


void Logging::logMessage( char *msg )
{
  logWidget->insert(msg);
}


void Logging::clearButton_clicked()
{
  logWidget->clear();
}


void Logging::okButton_clicked()
{
  hide();
  delete logDlg;
  logDlg = NULL;
}

void toolsLogging(QWidget *parent)
{
  if(logDlg == NULL) {
    logDlg = new Logging(parent);
  }
  logDlg->show();
}

void toolsLog(char *msg)
{
  if(logDlg)
    logDlg->logMessage(msg);
}
