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

#include <qapplication.h>
#include <qmenubar.h>
#include <qmainwindow.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qaction.h>
#include <qsettings.h>

class qtGUI : public QMainWindow {
  Q_OBJECT
public:
  qtGUI();
  ~qtGUI();

  void drawScreen();
protected:
  virtual void windowActivationChange(bool);
  virtual void resizeEvent(QResizeEvent *);
  
private slots:
  void fileOpen();
  void fileLoadState(int id);
  void fileSaveState(int id);
  void filePause(); 
  void fileReset();
  void fileScreenCapture();
  void fileClose();
  void fileExit();
  void optionsFrameskip(int id);
  void optionsVideo1x();
  void optionsVideo2x();
  void optionsVideo3x();
  void optionsVideo4x();
  void optionsVideoLayers(int);
  void optionsEmulatorDirectories();
  void cheatsSearch();
  void cheatsCheatList();
  void toolsLogging();
  void doIdle();
  void updateFileMenu();
  void updateLoadGameMenu();
  void updateSaveGameMenu();
  void updateFrameskipMenu();
  void updateVideoMenu();
  void updateLayersMenu();
  void updateCheatsMenu();

private:
  virtual void keyPressEvent(QKeyEvent *);
  virtual void keyReleaseEvent(QKeyEvent *);

  void readBattery();
  void writeBattery();
  void readSettings();
  
  QTimer *timer;
  int drawY;
  bool paused;
  int videoOption;
  int captureFormat;
  QString filename;

  QRect destRect;
  
  QPopupMenu *fileMenu;
  QPopupMenu *saveStateMenu;
  QPopupMenu *loadStateMenu;
  QPopupMenu *frameskipMenu;
  QPopupMenu *videoMenu;
  QPopupMenu *layersMenu;
  QPopupMenu *emulatorMenu;
  QPopupMenu *cheatsMenu;
};
