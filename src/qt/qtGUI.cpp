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

#include <stdio.h>
#include <stdarg.h>

#include "qtGUI.h"
#include <qmessagebox.h>
#include <qimage.h>
#include <qpainter.h>
#include <qsettings.h>
#include <qfiledialog.h>

#include "../GBA.h"
#include "../Font.h"

#include "loggingdlg.h"
#include "gbacheats.h"
#include "gbacheatsearch.h"
#include "directories.h"

extern void toolsLogging(QWidget *);
extern void toolsLog(char *);
extern int Init_2xSaI(u32);
extern void Pixelate32(u8*,u32,u8*,u8*,u32,int,int);
extern void MotionBlur32(u8*,u32,u8*,u8*,u32,int,int);
extern void TVMode32(u8*,u32,u8*,u8*,u32,int,int);
extern void _2xSaI32(u8*,u32,u8*,u8*,u32,int,int);
extern void Super2xSaI32(u8*,u32,u8*,u8*,u32,int,int);
extern void SuperEagle32(u8*,u32,u8*,u8*,u32,int,int);
extern void AdMame2x32(u8*,u32,u8*,u8*,u32,int,int);
extern void Simple2x32(u8*,u32,u8*,u8*,u32,int,int);
extern void Bilinear32(u8*,u32,u8*,u8*,u32,int,int);
extern void BilinearPlus32(u8*,u32,u8*,u8*,u32,int,int);

QImage image(240, 160, 32);
QImage filterImage(480, 320, 32);

u8 *delta[257*244*4];

qtGUI *gui = NULL;
QSettings *settings = NULL;
bool screenMessage = false;
char screenMessageBuffer[21];
u32  screenMessageTime = 0;

enum {
  KEY_LEFT, KEY_RIGHT,
  KEY_UP, KEY_DOWN,
  KEY_BUTTON_A, KEY_BUTTON_B,
  KEY_BUTTON_START, KEY_BUTTON_SELECT,
  KEY_BUTTON_L, KEY_BUTTON_R,
  KEY_BUTTON_SPEED, KEY_BUTTON_CAPTURE
};

u16 joypad[12] = {
  QKeyEvent::Key_Left, QKeyEvent::Key_Right,
  QKeyEvent::Key_Up, QKeyEvent::Key_Down,
  QKeyEvent::Key_Z, QKeyEvent::Key_X,
  QKeyEvent::Key_Return, QKeyEvent::Key_Backspace,
  QKeyEvent::Key_A, QKeyEvent::Key_S,
  QKeyEvent::Key_Space, QKeyEvent::Key_F12
};

u16 motion[4] = {
  QKeyEvent::Key_Delete, QKeyEvent::Key_PageDown,
  QKeyEvent::Key_Home, QKeyEvent::Key_End
};

bool buttons[12] = { false, false, false, false, false, false, 
                     false, false, false, false, false, false };
bool motionButtons[4] = { false, false, false, false };

int systemRedShift = 19;
int systemGreenShift = 11;
int systemBlueShift = 3;
int systemColorDepth = 32;
int systemDebug = 0;
int systemVerbose = 0;
bool systemSoundOn = false;

int RGB_LOW_BITS_MASK = 0;

int emulating = 0;

u32 systemColorMap32[0x10000];
u16 systemColorMap16[0x10000];
u16 systemGbPalette[24];

qtGUI::qtGUI()
  : QMainWindow(0, 0, WDestructiveClose)
{
  gui = this;
  paused = false;
  videoOption = 0;
  captureFormat = 0;
  recentFreeze = false;
  filterType = 0;
  filterFunction = NULL;

  readSettings();

  Init_2xSaI(32);

  // File menu
  fileMenu = new QPopupMenu(this);
  connectMenu(fileMenu);
  fileMenu->setCheckable(true);
  connect(fileMenu, SIGNAL(aboutToShow()), this, SLOT(updateFileMenu()));  
  menuBar()->insertItem(tr("&File"), fileMenu);
  fileMenu->insertItem(tr("&Open..."), this, SLOT(fileOpen()), QKeySequence(tr("Ctrl+O","File|Open")));
  fileMenu->insertItem(tr("Open &Gameboy..."), this, SLOT(fileOpenGB()), 0);
  fileMenu->insertSeparator();

  fileMenu->insertItem(tr("&Load..."), this, SLOT(fileLoad()), QKeySequence(tr("Ctrl+L", "File|Load")), 4);
  fileMenu->insertItem(tr("&Save..."), this, SLOT(fileSave()), QKeySequence(tr("Ctrl+S", "File|Save")), 5);

  // Load Game menu
  loadStateMenu = new QPopupMenu(this);
  fileMenu->insertItem(tr("Loa&d Game"), loadStateMenu);
  connect(loadStateMenu, SIGNAL(aboutToShow()), this, SLOT(updateLoadGameMenu()));
  loadStateMenu->insertItem(tr("Slot #1"), this, SLOT(fileLoadState(int)), QKeySequence(tr("F1", "File|Load Game|Slot #1")), 0);
  loadStateMenu->insertItem(tr("Slot #2"), this, SLOT(fileLoadState(int)), QKeySequence(tr("F2", "File|Load Game|Slot #2")), 1);
  loadStateMenu->insertItem(tr("Slot #3"), this, SLOT(fileLoadState(int)), QKeySequence(tr("F3", "File|Load Game|Slot #3")), 2);
  loadStateMenu->insertItem(tr("Slot #4"), this, SLOT(fileLoadState(int)), QKeySequence(tr("F4", "File|Load Game|Slot #4")), 3);
  loadStateMenu->insertItem(tr("Slot #5"), this, SLOT(fileLoadState(int)), QKeySequence(tr("F5", "File|Load Game|Slot #5")), 4);
  loadStateMenu->insertItem(tr("Slot #6"), this, SLOT(fileLoadState(int)), QKeySequence(tr("F6", "File|Load Game|Slot #6")), 5);
  loadStateMenu->insertItem(tr("Slot #7"), this, SLOT(fileLoadState(int)), QKeySequence(tr("F7", "File|Load Game|Slot #7")), 6);
  loadStateMenu->insertItem(tr("Slot #8"), this, SLOT(fileLoadState(int)), QKeySequence(tr("F8", "File|Load Game|Slot #8")), 7);
  loadStateMenu->insertItem(tr("Slot #9"), this, SLOT(fileLoadState(int)), QKeySequence(tr("F9", "File|Load Game|Slot #9")), 8);
  loadStateMenu->insertItem(tr("Slot #10"), this, SLOT(fileLoadState(int)), QKeySequence(tr("F10", "File|Load Game|Slot #10")), 9);  

  // Save Game menu
  saveStateMenu = new QPopupMenu(this);
  fileMenu->insertItem(tr("S&ave Game"), saveStateMenu);
  connect(saveStateMenu, SIGNAL(aboutToShow()), this, SLOT(updateSaveGameMenu()));
  saveStateMenu->insertItem(tr("Slot #1"), this, SLOT(fileSaveState(int)), QKeySequence(tr("Ctrl+F1", "File|Save Game|Slot #1")), 0);
  saveStateMenu->insertItem(tr("Slot #2"), this, SLOT(fileSaveState(int)), QKeySequence(tr("Ctrl+F2", "File|Save Game|Slot #2")), 1);
  saveStateMenu->insertItem(tr("Slot #3"), this, SLOT(fileSaveState(int)), QKeySequence(tr("Ctrl+F3", "File|Save Game|Slot #3")), 2);
  saveStateMenu->insertItem(tr("Slot #4"), this, SLOT(fileSaveState(int)), QKeySequence(tr("Ctrl+F4", "File|Save Game|Slot #4")), 3);
  saveStateMenu->insertItem(tr("Slot #5"), this, SLOT(fileSaveState(int)), QKeySequence(tr("Ctrl+F5", "File|Save Game|Slot #5")), 4);
  saveStateMenu->insertItem(tr("Slot #6"), this, SLOT(fileSaveState(int)), QKeySequence(tr("Ctrl+F6", "File|Save Game|Slot #6")), 5);
  saveStateMenu->insertItem(tr("Slot #7"), this, SLOT(fileSaveState(int)), QKeySequence(tr("Ctrl+F7", "File|Save Game|Slot #7")), 6);
  saveStateMenu->insertItem(tr("Slot #8"), this, SLOT(fileSaveState(int)), QKeySequence(tr("Ctrl+F8", "File|Save Game|Slot #8")), 7);
  saveStateMenu->insertItem(tr("Slot #9"), this, SLOT(fileSaveState(int)), QKeySequence(tr("Ctrl+F9", "File|Save Game|Slot #9")), 8);
  saveStateMenu->insertItem(tr("Slot #10"), this, SLOT(fileSaveState(int)), QKeySequence(tr("Ctrl+F10", "File|Save Game|Slot #10")), 9);  

  fileMenu->insertSeparator();
  fileMenu->insertItem(tr("&Reset"), this, SLOT(fileReset()), QKeySequence(tr("Ctrl+R", "File|Reset")), 0);
  fileMenu->insertItem(tr("&Pause"), this, SLOT(filePause()), QKeySequence(tr("Ctrl+P", "File|Pause")), 1);
  fileMenu->insertSeparator();

  // Recent menu
  recentMenu = new QPopupMenu(this);
  fileMenu->insertItem(tr("Re&cent"), recentMenu);

  recentMenu->insertItem(tr("&Reset"), this, SLOT(fileRecentReset()), QKeySequence());
  recentMenu->insertItem(tr("&Freeze"), this, SLOT(fileRecentFreeze()), QKeySequence(), 10);
  recentMenu->insertSeparator();
  connect(recentMenu, SIGNAL(aboutToShow()), this, SLOT(updateRecentMenu()));
  fileMenu->insertSeparator();
  fileMenu->insertItem(tr("Scr&een capture..."), this, SLOT(fileScreenCapture()), QKeySequence(), 3);  
  fileMenu->insertSeparator();
  fileMenu->insertItem(tr("&Close"), this, SLOT(fileClose()), QKeySequence(), 2);
  fileMenu->insertSeparator();
  fileMenu->insertItem(tr("E&xit...") , this, SLOT(fileExit()), QKeySequence(tr("Ctrl+X","File|Exit")));

  // Options menu
  QPopupMenu *optionsMenu = new QPopupMenu(this);
  connectMenu(optionsMenu);
  menuBar()->insertItem(tr("&Options"), optionsMenu);

  // Frameskip menu
  frameskipMenu = new QPopupMenu(this);
  frameskipMenu->setCheckable(true);

  connect(frameskipMenu, SIGNAL(aboutToShow()), this, SLOT(updateFrameskipMenu()));

  optionsMenu->insertItem(tr("&Frameskip"), frameskipMenu);

  frameskipMenu->insertItem(tr("&0"), this, SLOT(optionsFrameskip(int)), 0, 0);
  frameskipMenu->insertItem(tr("&1"), this, SLOT(optionsFrameskip(int)), 0, 1);
  frameskipMenu->insertItem(tr("&2"), this, SLOT(optionsFrameskip(int)), 0, 2);
  frameskipMenu->insertItem(tr("&3"), this, SLOT(optionsFrameskip(int)), 0, 3);
  frameskipMenu->insertItem(tr("&4"), this, SLOT(optionsFrameskip(int)), 0, 4);
  frameskipMenu->insertItem(tr("&5"), this, SLOT(optionsFrameskip(int)), 0, 5);
  frameskipMenu->insertItem(tr("&6"), this, SLOT(optionsFrameskip(int)), 0, 6);
  frameskipMenu->insertItem(tr("&7"), this, SLOT(optionsFrameskip(int)), 0, 7);
  frameskipMenu->insertItem(tr("&8"), this, SLOT(optionsFrameskip(int)), 0, 8);
  frameskipMenu->insertItem(tr("&9"), this, SLOT(optionsFrameskip(int)), 0, 9);

  // Video menu
  videoMenu = new QPopupMenu(this);

  optionsMenu->insertItem(tr("&Video"), videoMenu);
  connect(videoMenu, SIGNAL(aboutToShow()), this, SLOT(updateVideoMenu()));  
  
  videoMenu->setCheckable(true);

  videoMenu->insertItem(tr("&1x"), this, SLOT(optionsVideo1x()), 0, 0);
  videoMenu->insertItem(tr("&2x"), this, SLOT(optionsVideo2x()), 0, 1);
  videoMenu->insertItem(tr("&3x"), this, SLOT(optionsVideo3x()), 0, 2);
  videoMenu->insertItem(tr("&4x"), this, SLOT(optionsVideo4x()), 0, 3);

  videoMenu->insertSeparator();

  // Layers menu
  layersMenu = new QPopupMenu(this);
  videoMenu->insertItem(tr("&Layers"), layersMenu);
  connect(layersMenu, SIGNAL(aboutToShow()), this, SLOT(updateLayersMenu()));

  layersMenu->insertItem(tr("BG &0"),
                         this,
                         SLOT(optionsVideoLayers(int)),
                         QKeySequence(tr("Ctrl+1","Options|Video|Layers|BG 0")),
                         0);
  layersMenu->insertItem(tr("BG &1"),
                         this,
                         SLOT(optionsVideoLayers(int)),
                         QKeySequence(tr("Ctrl+2","Options|Video|Layers|BG 1")),
                         1);
  layersMenu->insertItem(tr("BG &2"),
                         this,
                         SLOT(optionsVideoLayers(int)),
                         QKeySequence(tr("Ctrl+3","Options|Video|Layers|BG 2")),
                         2);
  layersMenu->insertItem(tr("BG &3"),
                         this,
                         SLOT(optionsVideoLayers(int)),
                         QKeySequence(tr("Ctrl+4","Options|Video|Layers|BG 3")),
                         3);
  layersMenu->insertItem(tr("&OBJ"),
                         this,
                         SLOT(optionsVideoLayers(int)),
                         QKeySequence(tr("Ctrl+5","Options|Video|Layers|OBJ")),
                         4);
  layersMenu->insertItem(tr("&WIN 0"),
                         this,
                         SLOT(optionsVideoLayers(int)),
                         QKeySequence(tr("Ctrl+6","Options|Video|Layers|WIN 0")),
    5);
  layersMenu->insertItem(tr("W&IN 1"),
                         this,
                         SLOT(optionsVideoLayers(int)),
                         QKeySequence(tr("Ctrl+7","Options|Video|Layers|WIN 1")),
                         6);
  layersMenu->insertItem(tr("O&BJ WIN"),
                         this,
                         SLOT(optionsVideoLayers(int)),
                         QKeySequence(tr("Ctrl+8","Options|Video|Layers|OBJ WIN")),
                         7);

  // Emulator menu
  emulatorMenu = new QPopupMenu(this);
  optionsMenu->insertItem(tr("&Emulator"), emulatorMenu);

  emulatorMenu->insertItem(tr("&Directories..."),
                           this,
                           SLOT(optionsEmulatorDirectories()));

  // Filter menu
  filterMenu = new QPopupMenu(this);
  optionsMenu->insertItem(tr("&Filter"), filterMenu);
  connect(filterMenu, SIGNAL(aboutToShow()), this, SLOT(updateFilterMenu()));
  filterMenu->setCheckable(true);  

  filterMenu->insertItem(tr("&Normal"),
                         this,
                         SLOT(optionsFilter(int)),
                         0,
                         0);
  filterMenu->insertItem(tr("&TV Mode"),
                         this,
                         SLOT(optionsFilter(int)),
                         0,
                         1);
  filterMenu->insertItem(tr("&2xSaI"),
                         this,
                         SLOT(optionsFilter(int)),
                         0,
                         2);
  filterMenu->insertItem(tr("&Super 2xSaI"),
                         this,
                         SLOT(optionsFilter(int)),
                         0,
                         3);
  filterMenu->insertItem(tr("Super &Eagle"),
                         this,
                         SLOT(optionsFilter(int)),
                         0,
                         4);
  filterMenu->insertItem(tr("&Pixelate"),
                         this,
                         SLOT(optionsFilter(int)),
                         0,
                         5);
  filterMenu->insertItem(tr("&Motion Blur"),
                         this,
                         SLOT(optionsFilter(int)),
                         0,
                         6);
  filterMenu->insertItem(tr("&AdvanceMAME Scale2x"),
                         this,
                         SLOT(optionsFilter(int)),
                         0,
                         7);
  filterMenu->insertItem(tr("S&imple 2x"),
                         this,
                         SLOT(optionsFilter(int)),
                         0,
                         8);
  filterMenu->insertItem(tr("&Bilinear"),
                         this,
                         SLOT(optionsFilter(int)),
                         0,
                         9);

  filterMenu->insertItem(tr("Bilinear &Plus"),
                         this,
                         SLOT(optionsFilter(int)),
                         0,
                         10);  

  // Cheats menu
  cheatsMenu = new QPopupMenu(this);
  connectMenu(cheatsMenu);
  menuBar()->insertItem(tr("&Cheats"), cheatsMenu);
  connect(cheatsMenu, SIGNAL(aboutToShow()), this, SLOT(updateCheatsMenu()));
  
  cheatsMenu->insertItem(tr("Search for cheats..."),
                         this,
                         SLOT(cheatsSearch()),
                         QKeySequence(tr("Ctrl+C", "Cheats|Search for cheats")),
                         0);
  cheatsMenu->insertItem(tr("Cheat List..."),
                         this,
                         SLOT(cheatsCheatList()),
                         0,
                         1);

  // Tools menu
  QPopupMenu *toolsMenu = new QPopupMenu(this);
  connectMenu(toolsMenu);
  menuBar()->insertItem(tr("&Tools"), toolsMenu);

  toolsMenu->insertItem(tr("&Logging..."),
                        this,
                        SLOT(toolsLogging()),
                        0);
  
  drawY = menuBar()->heightForWidth(240);
  resize(240*(videoOption+1), 160*(videoOption+1)+drawY);
  destRect.setRect(0, drawY, 240, 160);

  for(int i = 0; i < 0x10000; i++) {
    systemColorMap32[i] =
      ((i & 0x1f) << systemRedShift) |
      (((i & 0x3e0) >> 5) << systemGreenShift) |
      (((i & 0x7c00) >> 10) << systemBlueShift);  
  }

  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(doIdle()));
}

qtGUI::~qtGUI()
{
  fileClose();
  if(settings != NULL) {
    delete settings;
    settings = NULL;
  }
}

void qtGUI::connectMenu(QPopupMenu *menu)
{
  connect(menu, SIGNAL(aboutToShow()), this, SLOT(menuAboutToShow()));
  connect(menu, SIGNAL(aboutToHide()), this, SLOT(menuAboutToHide()));
}

void qtGUI::menuAboutToShow()
{
  if(emulating && !paused)
    timer->stop();
}

void qtGUI::menuAboutToHide()
{
  if(emulating && !paused)
    timer->start(0);
}

void qtGUI::readSettings()
{
  settings = new QSettings();
  settings->insertSearchPath(QSettings::Windows, "/Emulators");

  frameSkip = settings->readNumEntry("/VisualBoyAdvance/frameSkip", 2);
  if(frameSkip < 0 || frameSkip > 9)
    frameSkip = 2;

  videoOption = settings->readNumEntry("/VisualBoyAdvance/video", 0);
  if(videoOption < 0 || videoOption > 3)
    videoOption = 0;

  filterType = settings->readNumEntry("/VisualBoyAdvance/filter", 0);
  if(filterType < 0 || filterType > 10)
    filterType = 0;

  recentFreeze = settings->readBoolEntry("/VisualBoyAdvance/recentFreeze",
                                         false);

  int i;
  QString buffer;
  for(i = 0; i < 10; i++) {
    buffer.sprintf("/VisualBoyAdvance/recent%d", i);
    recentFiles[i] = settings->readEntry(buffer);
  }

  updateFilter();
}

void qtGUI::windowActivationChange(bool /* oldActive */)
{
  if(emulating && !paused) {
    if(isActiveWindow())
      timer->start(0);
    else
      timer->stop();
  }
}

void qtGUI::resizeEvent(QResizeEvent *e)
{
  QSize size = e->size();

  drawY = menuBar()->heightForWidth(size.width());

  destRect.setRect(0, drawY, size.width(), size.height()-drawY);
}

bool qtGUI::fileRun(QString fileName)
{
  bool failed = !CPULoadRom((char *)((const char *)fileName));
  
  if(!failed) {
    filename = (const char *)fileName;
    fileinfo.setFile(fileName);
    int index = filename.findRev('.');
    if(index != -1)
      filename.truncate(index);
    
    addRecentFile(fileName);
    
    CPUInit(NULL, useBios);
    CPUReset();
    
    readBattery();
    
    emulating = 1;
    timer->start(0);
    return true;
  } else {
    QMessageBox::warning(this, tr("VisualBoyQt"), tr("Failed to load image"));
  }
  return false;
}

void qtGUI::fileOpen()
{
  QFileDialog dlg(this);
  
  dlg.addFilter(tr("ROMs (*.gba;*.zip)"));
  QString romdir = settings->readEntry("/VisualBoyAdvance/romdir");
  
  if(romdir.length() != 0)
    dlg.setDir(romdir);
  
  if(dlg.exec() == QDialog::Accepted) {
    QString selected = dlg.selectedFile();

    if(fileRun(selected))
      settings->writeEntry("/VisualBoyAdvance/romdir", dlg.dirPath());
  }
}

void qtGUI::fileOpenGB()
{
  QFileDialog dlg(this);
  
  dlg.addFilter(tr("ROMs (*.gba;*.zip)"));
  QString romdir = settings->readEntry("/VisualBoyAdvance/gbromdir");
  
  if(romdir.length() != 0)
    dlg.setDir(romdir);
  
  if(dlg.exec() == QDialog::Accepted) {
    QString selected = dlg.selectedFile();
    
    if(fileRun(selected)) {
      settings->writeEntry("/VisualBoyAdvance/gbromdir", dlg.dirPath());
    }
  }
}

void qtGUI::fileLoad()
{
  QString buffer;
  QString captureDir = settings->readEntry("/VisualBoyAdvance/saveDir");

  if(captureDir.length() == 0)
    captureDir = fileinfo.dirPath();
  
  buffer.sprintf("%s/%s.sgm", (const char *)captureDir,
                 (const char *)fileinfo.baseName(TRUE));
  
  QString filter = tr("Save Game (*.sgm)");
  
  QFileDialog dlg(this);
  dlg.setCaption(tr("Select save name"));
  dlg.addFilter(filter);
  dlg.setDir(captureDir);
  dlg.setSelection(buffer);
  dlg.setMode(QFileDialog::AnyFile);

  if(dlg.exec() != QDialog::Accepted) {
    return;
  }
  
  CPUReadState((char *)((const char *)dlg.selectedFile()));

  systemScreenMessage((char *)((const char *)tr("Loaded state")));  
}

void qtGUI::fileSave()
{
  QString buffer;
  QString captureDir = settings->readEntry("/VisualBoyAdvance/saveDir");

  if(captureDir.length() == 0)
    captureDir = fileinfo.dirPath();
  
  buffer.sprintf("%s/%s.sgm", (const char *)captureDir,
                 (const char *)fileinfo.baseName(TRUE));
  
  QString filter = tr("Save Game (*.sgm)");
  
  QFileDialog dlg(this);
  dlg.setCaption(tr("Select save name"));
  dlg.addFilter(filter);
  dlg.setDir(captureDir);
  dlg.setSelection(buffer);
  dlg.setMode(QFileDialog::AnyFile);

  if(dlg.exec() != QDialog::Accepted) {
    return;
  }
  
  CPUWriteState((char *)((const char *)dlg.selectedFile()));

  systemScreenMessage((char *)((const char *)tr("Wrote state")));  
}

void qtGUI::fileLoadState(int n)
{
  QString buffer;
  QString captureDir = settings->readEntry("/VisualBoyAdvance/saveDir");

  if(captureDir.length() == 0)
    captureDir = fileinfo.dirPath();  

  buffer.sprintf("%s/%s%d.sgm",
                 (const char *)captureDir,
                 (const char *)fileinfo.baseName(TRUE),
                 n+1);
  CPUReadState((char *)((const char *)buffer));

  buffer.sprintf("Loaded state %d",n+1);
  systemScreenMessage((char *)((const char *)buffer));
}

void qtGUI::fileSaveState(int n)
{
  QString buffer;
  QString captureDir = settings->readEntry("/VisualBoyAdvance/saveDir");

  if(captureDir.length() == 0)
    captureDir = fileinfo.dirPath();  

  buffer.sprintf("%s/%s%d.sgm",
                 (const char *)captureDir,
                 (const char *)fileinfo.baseName(TRUE),
                 n+1);
  CPUWriteState((char *)((const char *)buffer));

  buffer.sprintf("Wrote state %d",n+1);
  systemScreenMessage((char *)((const char *)buffer));
}

void qtGUI::fileRecentReset()
{
  int i = 0;
  for(i = 0; i < 10; i++)
    recentFiles[i] = QString::null;
  
  QString buffer;

  for(i = 0; i < 10; i++) {
    buffer.sprintf("recent%d", i);
    settings->removeEntry(buffer);
  }
}

void qtGUI::fileRecentFreeze()
{
  recentFreeze = !recentFreeze;
  settings->writeEntry("/VisualBoyAdvance/recentFreeze", recentFreeze);
}

void qtGUI::addRecentFile(const char *s)
{
  // Do not change recent list if frozen
  if(recentFreeze)
    return;
  
  int i = 0;
  QString buffer;
  for(i = 0; i < 10; i++) {
    if(recentFiles[i].length() == 0)
      break;
    
    if(recentFiles[i] ==  s) {
      if(i == 0)
        return;
      QString p = recentFiles[i];
      for(int j = i; j > 0; j--) {
        recentFiles[j] = recentFiles[j-1];
      }
      recentFiles[0] = p;
      for(i = 0; i < 10; i++) {
        if(recentFiles[i].length() == 0)
          break;
        buffer.sprintf("/VisualBoyAdvance/recent%d",i);
        settings->writeEntry(buffer, recentFiles[i]);
      }
      return;
    }
  }
  int num = 0;
  for(i = 0; i < 10; i++) {
    if(recentFiles[i].length() != 0)
      num++;
  }
  if(num == 10) {
    num--;
  }

  for(i = num; i >= 1; i--) {
    recentFiles[i] = recentFiles[i-1];
  }
  recentFiles[0] = s;
  for(i = 0; i < 10; i++) {
    if(recentFiles[i].length() == 0)
      break;
    buffer.sprintf("/VisualBoyAdvance/recent%d",i);
    settings->writeEntry(buffer, recentFiles[i]);
  }  
}

void qtGUI::fileRecent(int id)
{
  if(id >= 0 && id < 10) {
    fileRun(recentFiles[id]);
  }
}

void qtGUI::filePause()
{
  paused = !paused;  
  if(emulating) {
    if(paused)
      timer->stop();
    else
      timer->start(0);
  }
}

void qtGUI::fileReset()
{
  if(emulating)
    CPUReset();
}

void qtGUI::fileScreenCapture()
{
  QString buffer;
  QString captureDir = settings->readEntry("/VisualBoyAdvance/captureDir");

  if(captureDir.length() == 0)
    captureDir = fileinfo.dirPath();
  char *ext = "png";
  if(captureFormat != 0)
    ext = "bmp";
  
  buffer.sprintf("%s/%s.%s", (const char *)captureDir,
                 (const char *)fileinfo.baseName(TRUE),
                 ext);
  
  QString pngFilter = tr("PNG Image (*.png)");
  
  QFileDialog dlg(this);
  dlg.setCaption(tr("Select capture name"));
  dlg.addFilter(pngFilter);
  dlg.addFilter(tr("BMP Image (*.bmp)"));
  dlg.setDir(captureDir);
  dlg.setSelectedFilter(1);
  dlg.setSelection(buffer);
  dlg.setMode(QFileDialog::AnyFile);

  if(dlg.exec() != QDialog::Accepted) {
    return;
  }
  
  if(dlg.selectedFilter() != pngFilter)
    CPUWriteBMPFile((char *)((const char *)dlg.selectedFile()));
  else
    CPUWritePNGFile((char *)((const char *)dlg.selectedFile()));

  systemScreenMessage((char *)((const char *)tr("Screen capture")));
}

void qtGUI::fileClose()
{
  if(emulating) {
    writeBattery();
    CPUCleanUp();
  }
  if(timer != NULL)
    timer->stop();
  emulating = 0;
}

void qtGUI::fileExit()
{
  fileClose();

  if(timer != NULL)
    timer->stop();
  if(settings) {
    delete settings;
    settings = NULL;
  }
  qApp->exit(0);
}

void qtGUI::optionsFrameskip(int id)
{
  settings->writeEntry("/VisualBoyAdvance/frameSkip", id);
  frameskipMenu->setItemChecked(frameSkip, false);
  frameSkip = id;
}

void qtGUI::optionsVideo1x()
{
  videoMenu->setItemChecked(videoOption, false);
  videoOption = 0;
  resize(240 * (videoOption+1), drawY + 160 * (videoOption+1));
  destRect.setRect(0, drawY, 240, 160);
  settings->writeEntry("/VisualBoyAdvance/video", videoOption);
}

void qtGUI::optionsVideo2x()
{
  videoMenu->setItemChecked(videoOption, false);
  videoOption = 1;
  resize(240 * (videoOption+1), drawY + 160 * (videoOption+1));
  destRect.setRect(0, drawY, 240*2, 160*2);
  settings->writeEntry("/VisualBoyAdvance/video", videoOption);  
}

void qtGUI::optionsVideo3x()
{
  videoMenu->setItemChecked(videoOption, false);
  videoOption = 2;
  resize(240 * (videoOption+1), drawY + 160 * (videoOption+1));
  destRect.setRect(0, drawY, 240*3, 160*3);
  settings->writeEntry("/VisualBoyAdvance/video", videoOption);  
}

void qtGUI::optionsVideo4x()
{
  videoMenu->setItemChecked(videoOption, false);
  videoOption = 3;
  resize(240 * (videoOption+1), drawY + 160 * (videoOption+1));
  destRect.setRect(0, drawY, 240*4, 160*4);
  settings->writeEntry("/VisualBoyAdvance/video", videoOption);  
}

void qtGUI::optionsVideoLayers(int id)
{
  int mask = (0x0100 << id);
  layerSettings ^= mask;
  layerEnable = DISPCNT & layerSettings;  
}

void qtGUI::optionsEmulatorDirectories()
{
  Directories dlg(this);

  dlg.exec();
}

void qtGUI::optionsFilter(int id)
{
  filterMenu->setItemChecked(filterType, false);
  filterType = id;
  if(filterType < 0 || filterType > 10)
    filterType = 0;
  settings->writeEntry("/VisualBoyAdvance/filter", filterType);
  updateFilter();
}

void qtGUI::cheatsSearch()
{
  GBACheatSearch dlg(this);

  dlg.exec();
}

void qtGUI::cheatsCheatList()
{
  GBACheatList dlg(this);

  dlg.exec();
}

void qtGUI::toolsLogging()
{
  ::toolsLogging(this);
}

void qtGUI::updateFilter()
{
  switch(filterType) {
  default:
  case 0:
    filterFunction = NULL;
    break;
  case 1:
    filterFunction = TVMode32;
    break;
  case 2:
    filterFunction = _2xSaI32;
    break;
  case 3:
    filterFunction = Super2xSaI32;
    break;
  case 4:
    filterFunction = SuperEagle32;
    break;        
  case 5:
    filterFunction = Pixelate32;
    break;
  case 6:
    filterFunction = MotionBlur32;
    break;
  case 7:
    filterFunction = AdMame2x32;
    break;
  case 8:
    filterFunction = Simple2x32;
    break;
  case 9:
    filterFunction = Bilinear32;
    break;
  case 10:
    filterFunction = BilinearPlus32;
    break;
  }
}

void qtGUI::updateFileMenu()
{
  fileMenu->setItemEnabled(0, emulating);
  fileMenu->setItemChecked(1, paused);
  fileMenu->setItemEnabled(2, emulating);
  fileMenu->setItemEnabled(3, emulating);
  fileMenu->setItemEnabled(4, emulating);
  fileMenu->setItemEnabled(5, emulating);
}

void qtGUI::updateLoadGameMenu()
{
  for(int i = 0; i < 10; i++)
    loadStateMenu->setItemEnabled(i, emulating);
}

void qtGUI::updateSaveGameMenu()
{
  for(int i = 0; i < 10; i++)
    saveStateMenu->setItemEnabled(i, emulating);
}

void qtGUI::updateRecentMenu()
{
  int i;

  recentMenu->setItemChecked(10, recentFreeze);
  
  for(i = 0; i < 10; i++) {
    if(recentMenu->indexOf(i) != -1)
      recentMenu->removeItem(i);
  }

  for(i = 0; i < 10; i++) {
    if(recentFiles[i].length() == 0)
      break;

    QFileInfo f(recentFiles[i]);
    QString buffer;
    buffer.sprintf("Ctrl+F%d", i+1);
    recentMenu->insertItem(f.fileName(), this, SLOT(fileRecent(int)),
                           QKeySequence(buffer), i);
  }
}

void qtGUI::updateFrameskipMenu()
{
  frameskipMenu->setItemChecked(frameSkip,  true);
}

void qtGUI::updateVideoMenu()
{
  videoMenu->setItemChecked(videoOption, true);
}

void qtGUI::updateLayersMenu()
{
  for(int i = 0; i < 8; i++)
    layersMenu->setItemChecked(i, (layerSettings & (0x0100 << i)) ?
                               true : false);
}

void qtGUI::updateFilterMenu()
{
  filterMenu->setItemChecked(filterType, true);
}

void qtGUI::updateCheatsMenu()
{
  cheatsMenu->setItemEnabled(0, emulating);
  cheatsMenu->setItemEnabled(1, emulating);
}

void qtGUI::doIdle()
{
  if(emulating) {
    CPULoop(50000);
  }
}

void qtGUI::drawScreen()
{
  u32 *p = (u32 *)pix + 241;
  QPainter painter(this);  

  if(filterFunction) {
    (*filterFunction)(pix+241*4,
                      241*4,
                      (u8 *)delta,
                      filterImage.bits(),
                      filterImage.bytesPerLine(),
                      240,
                      160);
    painter.drawImage(destRect, filterImage);
  } else {
    for(int y = 0; y < 160; y++) {
      u32 *line = (u32 *)image.scanLine(y);
      
      for(int x = 0; x < 240; x++) {
        *line++ = *p++;
      }
      p++;
    }

    painter.drawImage(destRect, image);
  }
}

void qtGUI::keyPressEvent(QKeyEvent *e)
{
  int i;
  for(i = 0; i < 12; i++) {
    if(e->key() == joypad[i])
      buttons[i] = true;
  }
  for(i = 0; i < 4; i++) {
    if(e->key() == motion[i])    
      motionButtons[i] = true;
  }
  QMainWindow::keyPressEvent(e);
}

void qtGUI::keyReleaseEvent(QKeyEvent *e)
{
  int i;
  for(i = 0; i < 12; i++) {
    if(e->key() == joypad[i])
      buttons[i] = false;
  }
  for(i = 0; i < 4; i++) {
    if(e->key() == motion[i])    
      motionButtons[i] = false;
  }
  QMainWindow::keyReleaseEvent(e);
}

void qtGUI::readBattery()
{
  QString batteryDir = settings->readEntry("/VisualBoyAdvance/batteryDir");
  QString buffer;
  
  if(batteryDir.length() != 0)
    buffer.sprintf("%s/%s.sav",
                   (const char *)batteryDir,
                   (const char *)fileinfo.baseName(TRUE));
  else 
    buffer.sprintf("%s.sav", (const char *)filename);
  
  bool res = false;

  res = CPUReadBatteryFile((char *)((const char *)buffer));

  if(res)
    systemScreenMessage((char *)((const char *)tr("Loaded battery")));
}

void qtGUI::writeBattery()
{
  QString batteryDir = settings->readEntry("/VisualBoyAdvance/batteryDir");
  QString buffer;
  
  if(batteryDir.length() != 0)
    buffer.sprintf("%s/%s.sav",
                   (const char *)batteryDir,
                   (const char *)fileinfo.baseName(TRUE));
  else 
    buffer.sprintf("%s.sav", (const char *)filename);
  
  bool res = false;

  res = CPUWriteBatteryFile((char *)((const char *)buffer));

  if(res)
    systemScreenMessage((char *)((const char *)tr("Wrote  battery")));
}

void systemMessage(int /* num */, char *msg, ...)
{
  char buffer[2048];
  va_list valist;
  
  va_start(valist, msg);
  vsprintf(buffer, msg, valist);
  
  va_end(valist);

  QMessageBox::warning(gui, "VisualBoyAdvance", buffer);
}

void systemDrawScreen()
{
  if(screenMessage) {
    //    if(cartridgeType == 1 && gbBorderOn) {
    //      gbSgbRenderBorder();
    //    }
    if(((systemGetClock() - screenMessageTime) < 3000)) {
      //       !disableStatusMessages) {
      fontDisplayString(pix, 240*4, 10, 160 - 20,
                        screenMessageBuffer); 
    } else {
      screenMessage = false;
    }
  }
  
  gui->drawScreen();
}

u32 systemReadJoypad()
{
  u32 res = 0;
  
  if(buttons[KEY_BUTTON_A])
    res |= 1;
  if(buttons[KEY_BUTTON_B])
    res |= 2;
  if(buttons[KEY_BUTTON_SELECT])
    res |= 4;
  if(buttons[KEY_BUTTON_START])
    res |= 8;
  if(buttons[KEY_RIGHT])
    res |= 16;
  if(buttons[KEY_LEFT])
    res |= 32;
  if(buttons[KEY_UP])
    res |= 64;
  if(buttons[KEY_DOWN])
    res |= 128;
  if(buttons[KEY_BUTTON_R])
    res |= 256;
  if(buttons[KEY_BUTTON_L])
    res |= 512;

  return res;
}

void systemSetTitle(char *title)
{
  gui->setCaption(title);
}

void systemScreenCapture(int /* a */)
{
}

u32 systemReadJoypadExtended()
{
  int res = 0;

  if(buttons[KEY_BUTTON_SPEED])
    res |= 1;
  if(buttons[KEY_BUTTON_CAPTURE])
    res |= 2;

  return res;
}

void systemWriteDataToSoundBuffer()
{
}

bool systemSoundInit()
{
  return true;
}

void systemSoundShutdown()
{
}

void systemSoundPause()
{
}

void systemSoundResume()
{
}

void systemSoundReset()
{
}

u32 systemGetClock()
{
  QTime time = QTime::currentTime();

  return time.hour() * 60000 + time.second() * 1000 + time.msec();
}

void systemUpdateMotionSensor()
{
}

int systemGetSensorX()
{
  return 0;
}

int systemGetSensorY()
{
  return 0;
}

void systemGbPrint(u8 * /* data */,int /*  pages */,int /* feed */,int /* palette */, int /* contrast */)
{
}

void systemScreenMessage(char *msg)
{
  screenMessage = true;
  screenMessageTime = systemGetClock();
  if(strlen(msg) > 20) {
    strncpy(screenMessageBuffer, msg, 20);
    screenMessageBuffer[20] = 0;
  } else
    strcpy(screenMessageBuffer, msg);    
}

bool systemCanChangeSoundQuality()
{
  return false;
}

bool systemPauseOnFrame()
{
  return false;
}

void log(char *defaultMsg, ...)
{
  char buffer[2048];
  va_list valist;
  
  va_start(valist, defaultMsg);
  vsprintf(buffer, defaultMsg, valist);

  toolsLog(buffer);
  
  va_end(valist);  
}
