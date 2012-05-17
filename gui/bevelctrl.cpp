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

#include "../core/api.h"
#include "bevelctrl.h"

//******************************************************************************
BevelCtrl::BevelCtrl (Ctrl* parent, int thickness, bool raised) : Ctrl(parent)
//------------------------------------------------------------------------------
{
  m_thickness = thickness;
  m_raised    = raised;
}

//******************************************************************************
Point BevelCtrl::getContPos() 
//------------------------------------------------------------------------------
{ 
  return  Point(m_thickness, m_thickness); 
}

//******************************************************************************
Point BevelCtrl::getContSize() 
//------------------------------------------------------------------------------
{ 
  return  getSize() - 2 * Point(m_thickness, m_thickness); 
}

//******************************************************************************
void BevelCtrl::setContSize (Point s) 
//------------------------------------------------------------------------------
{
  setSize (s + 2 * Point(m_thickness, m_thickness));
}

//******************************************************************************
void BevelCtrl::draw() 
//------------------------------------------------------------------------------
{
  Rect  rect (getAbsPos(), getSize());
  WinDrawBevel (rect, m_thickness, m_raised);
}

