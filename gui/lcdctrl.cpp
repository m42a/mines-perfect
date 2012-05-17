// Mines-Perfect: a minesweeper clone
// Copyright (C) 1995-2003  Christian Czepluch
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <stdio.h>

#include "../core/api.h"
#include "lcdctrl.h"

using namespace MinesPerfect;


Bitmap* LcdCtrl::s_minus_bmp      = 0;
Bitmap* LcdCtrl::s_digit_bmps[10] = { 0 };

//******************************************************************************
LcdCtrl::LcdCtrl(Ctrl* parent)
//------------------------------------------------------------------------------
: Ctrl(parent), m_val(1000)
{
  // nocht nicht initialisiert?
  if (s_minus_bmp == 0)
  {
    char      buf[20];
    unsigned  i;  // visual studio

    // initialisieren
    s_minus_bmp = CreateBitmap ("lcd", "-");

    for (i = 0; i < sizeof (s_digit_bmps) / sizeof (*s_digit_bmps); i++)
    {
      sprintf (buf, "%i", i);
      s_digit_bmps[i] = CreateBitmap ("lcd", buf);
    }

    // check size
    for (i = 0; i < sizeof (s_digit_bmps) / sizeof (*s_digit_bmps); i++)
      ASSERT (s_minus_bmp->getSize() == s_digit_bmps[i]->getSize());
  }
  
  setSize(Point(3 * s_minus_bmp->getSize().x, s_minus_bmp->getSize().y));
}

//******************************************************************************
void LcdCtrl::setVal (int val)
//------------------------------------------------------------------------------
{
  if      (val < -99)  val = -99;
  else if (val > 999)  val = 999;

  setElem (m_val, val);
}

//******************************************************************************
void LcdCtrl::draw()
//------------------------------------------------------------------------------
{
  Point pos = getAbsPos();

  // 1. Ziffer
  if (m_val < 0)
    WinDrawBitmap (s_minus_bmp, pos);
  else
    WinDrawBitmap (s_digit_bmps[m_val / 100], pos);

  // 2. Ziffer
  WinDrawBitmap (s_digit_bmps[(abs(m_val) / 10) % 10],
                 Point (pos.x + getSize().x / 3, pos.y));

  // 3. Ziffer
  WinDrawBitmap (s_digit_bmps[abs(m_val) % 10],
                 Point (pos.x + 2 * getSize().x / 3, pos.y));
}



