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

#include "screenarea.h"

#include <string.h>

namespace VBA
{

ScreenArea::ScreenArea(int _iWidth, int _iHeight, int _iScale) :
  m_puiPixels(NULL),
  m_puiDelta(NULL),
  m_vFilter2x(NULL)
{
  g_assert(_iWidth >= 1 && _iHeight >= 1 && _iScale >= 1);

  m_iWidth  = _iWidth;
  m_iHeight = _iHeight;
  m_iScale  = _iScale;
  vUpdateSize();

  set_events(Gdk::EXPOSURE_MASK);
}

ScreenArea::~ScreenArea()
{
  if (m_puiPixels != NULL)
  {
    delete[] m_puiPixels;
  }

  if (m_puiDelta != NULL)
  {
    delete[] m_puiDelta;
  }
}

void ScreenArea::vSetSize(int _iWidth, int _iHeight)
{
  g_return_if_fail(_iWidth >= 1 && _iHeight >= 1);

  if (_iWidth != m_iWidth || _iHeight != m_iHeight)
  {
    m_iWidth  = _iWidth;
    m_iHeight = _iHeight;
    vUpdateSize();
  }
}

void ScreenArea::vSetScale(int _iScale)
{
  g_return_if_fail(_iScale >= 1);

  if (_iScale != m_iScale)
  {
    m_iScale = _iScale;
    vUpdateSize();
  }
}

void ScreenArea::vSetFilter2x(Filter2x _vFilter2x)
{
  m_vFilter2x = _vFilter2x;
}

void ScreenArea::vDrawPixels(const u8 * _puiData)
{
  if (m_iScale == 1)
  {
    u32 * puiSrc = (u32 *)_puiData + m_iWidth + 1;
    u32 * puiPixel = m_puiPixels;
    for (int y = 0; y < m_iHeight; y++)
    {
      for (int x = 0; x < m_iWidth; x++)
      {
        *puiPixel++ = *puiSrc++;
      }
      puiSrc++;
    }
  }
  else if (m_iScale == 2 && m_vFilter2x != NULL)
  {
    m_vFilter2x(const_cast<u8 *>(_puiData) + m_iAreaWidth * 2 + 4,
                m_iAreaWidth * 2 + 4,
                m_puiDelta,
                (u8 *)m_puiPixels,
                m_iRowStride,
                m_iWidth,
                m_iHeight);
  }
  else
  {
    u32 * puiSrc = (u32 *)_puiData + m_iWidth + 1;
    u32 * puiSrc2;
    u32 * puiPixel = m_puiPixels;
    for (int y = 0; y < m_iHeight; y++)
    {
      for (int j = 0; j < m_iScale; j++)
      {
        puiSrc2 = puiSrc;
        for (int x = 0; x < m_iWidth; x++)
        {
          for (int i = 0; i < m_iScale; i++)
          {
            *puiPixel++ = *puiSrc2;
          }
          puiSrc2++;
        }
      }
      puiSrc = puiSrc2 + 1;
    }
  }

  queue_draw_area(0, 0, m_iAreaWidth, m_iAreaHeight);
}

void ScreenArea::vDrawColor(u32 _uiColor)
{
  _uiColor = GUINT32_TO_BE(_uiColor) << 8;

  u32 * puiPixel = m_puiPixels;
  u32 * puiEnd   = m_puiPixels + m_iAreaWidth * m_iAreaHeight;
  while (puiPixel != puiEnd)
  {
    *puiPixel++ = _uiColor;
  }

  queue_draw_area(0, 0, m_iAreaWidth, m_iAreaHeight);
}

bool ScreenArea::on_expose_event(GdkEventExpose * _pstEvent)
{
  if (_pstEvent->area.x + _pstEvent->area.width > m_iAreaWidth
      || _pstEvent->area.y + _pstEvent->area.height > m_iAreaHeight)
  {
    return false;
  }

  guchar * puiAreaPixels = (guchar *)m_puiPixels;

  if (_pstEvent->area.x != 0)
  {
    puiAreaPixels += _pstEvent->area.x << 2;
  }

  if (_pstEvent->area.y != 0)
  {
    puiAreaPixels += _pstEvent->area.y * m_iRowStride;
  }

  get_window()->draw_rgb_32_image(get_style()->get_fg_gc(get_state()),
                                  _pstEvent->area.x,
                                  _pstEvent->area.y,
                                  _pstEvent->area.width,
                                  _pstEvent->area.height,
                                  Gdk::RGB_DITHER_MAX,
                                  puiAreaPixels,
                                  m_iRowStride);
  return true;
}

void ScreenArea::vUpdateSize()
{
  if (m_puiPixels != NULL)
  {
    delete[] m_puiPixels;
  }

  if (m_puiDelta != NULL)
  {
    delete[] m_puiDelta;
  }

  m_iAreaWidth  = m_iScale * m_iWidth;
  m_iAreaHeight = m_iScale * m_iHeight;
  m_iRowStride  = m_iAreaWidth * 4;

  m_puiPixels = new u32[m_iAreaWidth * m_iAreaHeight];

  m_puiDelta = new u8[(m_iWidth + 2) * (m_iHeight + 2) * 4];
  memset(m_puiDelta, 255, (m_iWidth + 2) * (m_iHeight + 2) * 4);

  set_size_request(m_iAreaWidth, m_iAreaHeight);
}

} // namespace VBA
