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

#include <qsettings.h>
#include <qfiledialog.h>

extern QSettings *settings;

void Directories::resetRomDir_clicked()
{
  romDir->clear();
}


void Directories::resetGbRomDir_clicked()
{
  gbRomDir->clear();
}


void Directories::resetBatteryDir_clicked()
{
  batteryDir->clear();
}


void Directories::resetSaveDir_clicked()
{
  saveDir->clear();
}


void Directories::resetCaptureDir_clicked()
{
  captureDir->clear();
}


void Directories::cancel_clicked()
{
  reject();
}


void Directories::ok_clicked()
{
  settings->writeEntry("/VisualBoyAdvance/romdir", romDir->text());
  settings->writeEntry("/VisualBoyAdvance/gbromdir", gbRomDir->text());
  settings->writeEntry("/VisualBoyAdvance/batteryDir", batteryDir->text());
  settings->writeEntry("/VisualBoyAdvance/saveDir", saveDir->text());
  settings->writeEntry("/VisualBoyAdvance/captureDir", captureDir->text());  
  accept();
}


void Directories::init()
{
    QString s = settings->readEntry("/VisualBoyAdvance/romdir");
    romDir->setText(s);
    s = settings->readEntry("/VisualBoyAdvance/gbromdir");
    gbRomDir->setText(s);
    s = settings->readEntry("/VisualBoyAdvance/batteryDir");
    batteryDir->setText(s);
    s = settings->readEntry("/VisualBoyAdvance/saveDir");
    saveDir->setText(s);
    s = settings->readEntry("/VisualBoyAdvance/captureDir");
    captureDir->setText(s);
}


QString Directories::browseForDir(QString title, QString dir)
{
  QFileDialog dialog(this, title);
  dialog.setDir(dir);
  dialog.setMode(QFileDialog::DirectoryOnly);
  
  if(dialog.exec() == QDialog::Accepted) {
      return dialog.dirPath();
  }
  
  return QString::null;
}


void Directories::browseRomDir_clicked()
{
  QString s = browseForDir(tr("Select ROM directory"), romDir->text());
  
  if(s.length() != 0)
      romDir->setText(s);
}


void Directories::browseGbRomDir_clicked()
{
  QString s = browseForDir(tr("Select ROM directory"), gbRomDir->text());
  
  if(s.length() != 0)
      gbRomDir->setText(s);
}


void Directories::browseBatteryDir_clicked()
{
  QString s = browseForDir(tr("Select battery directory"), batteryDir->text());
  
  if(s.length() != 0)
      batteryDir->setText(s);
}


void Directories::browseSaveDir_clicked()
{
  QString s = browseForDir(tr("Select save directory"), saveDir->text());
  
  if(s.length() != 0)
      saveDir->setText(s);
}


void Directories::browseCaptureDir_clicked()
{
  QString s = browseForDir(tr("Select capture directory"), captureDir->text());
  
  if(s.length() != 0)
      captureDir->setText(s);
}
