// -*- C++ -*-
// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2004 Forgotten and the VBA development team

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or(at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef __VBA_INPUT_H__
#define __VBA_INPUT_H__

#include <glib.h>

namespace VBA
{

enum EKey
{
  KEY_NONE,
  // GBA keys
  KEY_A,
  KEY_B,
  KEY_SELECT,
  KEY_START,
  KEY_RIGHT,
  KEY_LEFT,
  KEY_UP,
  KEY_DOWN,
  KEY_R,
  KEY_L,
  // VBA extension
  KEY_SPEED,
  KEY_CAPTURE
};

enum EKeyFlag
{
  // GBA keys
  KEYFLAG_A       = 1 << 0,
  KEYFLAG_B       = 1 << 1,
  KEYFLAG_SELECT  = 1 << 2,
  KEYFLAG_START   = 1 << 3,
  KEYFLAG_RIGHT   = 1 << 4,
  KEYFLAG_LEFT    = 1 << 5,
  KEYFLAG_UP      = 1 << 6,
  KEYFLAG_DOWN    = 1 << 7,
  KEYFLAG_R       = 1 << 8,
  KEYFLAG_L       = 1 << 9,
  // VBA extension
  KEYFLAG_SPEED   = 1 << 10,
  KEYFLAG_CAPTURE = 1 << 11,
};

class Keymap
{
 public:
  Keymap();
  ~Keymap();

  void vRegister(guint _uiVal, EKey _eKey);
  void vClear();
  inline EKey eGetKey(guint _uiVal);

 private:
  GHashTable * m_pstTable;

  // noncopyable
  Keymap(const Keymap &);
  Keymap & operator=(const Keymap &);
};

inline EKey Keymap::eGetKey(guint _uiVal)
{
  return (EKey)GPOINTER_TO_UINT(g_hash_table_lookup(m_pstTable,
                                                    GUINT_TO_POINTER(_uiVal)));
}

} // namespace VBA


#endif // __VBA_INPUT_H__
