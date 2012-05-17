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

#ifndef BITMAPCTRL_H
#define BITMAPCTRL_H

#include "../core/utils.h"
#include "../core/api.h"
#include "ctrl.h"

//******************************************************************************
class BitmapCtrl : public Ctrl 
//------------------------------------------------------------------------------
{
  public:
    BitmapCtrl(Ctrl* parent, const Bitmap* bmp = 0) : Ctrl(parent), m_bmp(bmp) {;}
    
    const Bitmap* getBitmap() { return m_bmp; }
    void          setBitmap (const Bitmap* bmp);
    virtual void  draw();

  private:
    const Bitmap*  m_bmp;
};

#endif


