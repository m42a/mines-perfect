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

//---------------------------------------------------------------------------

#include "bitmapctrl.h"

//******************************************************************************
void BitmapCtrl::draw()
//------------------------------------------------------------------------------
{
  WinDrawBitmap (m_bmp, getAbsPos());
}

//******************************************************************************
void BitmapCtrl::setBitmap (const Bitmap* bmp) 
//------------------------------------------------------------------------------
{ 
  Ctrl::setElem (m_bmp, bmp);
  setSize(bmp == 0 ? Point(0,0) : m_bmp->getSize());
}
