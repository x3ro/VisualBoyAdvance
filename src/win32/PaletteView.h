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

class PaletteViewControl : public Wnd {
  int w;
  int h;
  int colors;
  u8 *data;
  BITMAPINFO bmpInfo;
  static bool isRegistered;
  int selected;
protected:
  u16 palette[256];
  int paletteAddress;  
  DECLARE_MESSAGE_MAP()
public:
  PaletteViewControl();
  ~PaletteViewControl();

  void init(int, int, int);

  bool saveAdobe(char *);
  bool saveMSPAL(char *);
  bool saveJASCPAL(char *);
  void setPaletteAddress(int);
  void setSelected(int);
  void render(u16 color, int x, int y);
  void refresh();

  virtual void updatePalette()=0;
  
  virtual void OnPaint();
  virtual BOOL OnEraseBkgnd(HDC);
  virtual void OnLButtonDown(UINT, int, int);

  static void registerClass();
};
