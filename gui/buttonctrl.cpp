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

#include "buttonctrl.h"

Bitmap* ButtonCtrl::s_button_up_bmp   = 0;
Bitmap* ButtonCtrl::s_button_down_bmp = 0;

//******************************************************************************
ButtonCtrl::ButtonCtrl(Ctrl* parent) : Ctrl(parent)
//------------------------------------------------------------------------------
{
  m_is_pushed  = false;
  
  // statische Elemente initialisieren
  if (s_button_up_bmp == 0)
  {
    s_button_up_bmp   = CreateBitmap ("button", "up"  );
    s_button_down_bmp = CreateBitmap ("button", "down");
    
    ASSERT (s_button_up_bmp->getSize() == s_button_down_bmp->getSize());
  }
  
  setSize(s_button_up_bmp->getSize());
}

//******************************************************************************
void ButtonCtrl::draw()
//------------------------------------------------------------------------------
{
  Point p = getAbsPos();
  
  // background
  WinDrawBitmap (isPushed() ? s_button_down_bmp : s_button_up_bmp, p);

  // smiley
  if (isPushed())
  {
    p.x += (s_button_down_bmp->getSize().x
           - m_symb_down_bmp->getSize().x) / 2 + 1;

    p.y += (s_button_down_bmp->getSize().y
           - m_symb_down_bmp->getSize().y) / 2 + 1;
           
    WinDrawBitmap(m_symb_down_bmp, p);
  }
  else           
  {
    p.x += (s_button_up_bmp->getSize().x
           - m_symb_up_bmp->getSize().x) / 2;

    p.y += (s_button_up_bmp->getSize().y
           - m_symb_up_bmp->getSize().y) / 2;

    WinDrawBitmap(m_symb_up_bmp, p);
  }
}

//******************************************************************************
void ButtonCtrl::onMouseEvent (const MouseEvent& ev)
//------------------------------------------------------------------------------
{
  if (ev.m_type == MouseEvent::LEFT_DOWN)
  {
    setPushed(contains(ev.m_pos - getAbsPos()));
  }
  else if (ev.m_type == MouseEvent::MOVE)
  {
    if (isPushed() && !contains(ev.m_pos - getAbsPos()))
      setPushed(false);
  }
  else if (ev.m_type == MouseEvent::LEFT_UP)
  {
    if (isPushed())
    {
      setPushed(false);
      onClick(); // ruft ueberladene Funktion auf
    }
  }
}


