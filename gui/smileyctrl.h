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

#ifndef SMILEYCTRL_H
#define SMILEYCTRL_H

#include "../core/api.h"
#include "buttonctrl.h"

namespace MinesPerfect {

class GameCtrl;

//******************************************************************************
class SmileyCtrl : public ButtonCtrl
//------------------------------------------------------------------------------
{
  public:

    SmileyCtrl (Ctrl* parent, GameCtrl* game);

    void  act();
    void  draw() { ButtonCtrl::draw(); }
    void  onClick();

  private:

    GameCtrl*  m_game;

    static Bitmap*  s_smiley_happy_bmp;
    static Bitmap*  s_smiley_1eye_bmp;
    static Bitmap*  s_smiley_2eyes_bmp;
    static Bitmap*  s_smiley_3eyes_bmp;
    static Bitmap*  s_smiley_4eyes_bmp;
    static Bitmap*  s_smiley_norm_bmp;
    static Bitmap*  s_smiley_ooh_bmp;
    static Bitmap*  s_smiley_worry_bmp;
    static Bitmap*  s_smiley_sunglass_bmp;
};


} // namespace MinesPerfect

#endif
