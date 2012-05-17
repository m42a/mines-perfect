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

#include "smileyctrl.h"
#include "gamectrl.h"

using namespace std;
using namespace MinesPerfect;

Bitmap* SmileyCtrl::s_smiley_happy_bmp    = 0;
Bitmap* SmileyCtrl::s_smiley_1eye_bmp     = 0;
Bitmap* SmileyCtrl::s_smiley_2eyes_bmp    = 0;
Bitmap* SmileyCtrl::s_smiley_3eyes_bmp    = 0;
Bitmap* SmileyCtrl::s_smiley_4eyes_bmp    = 0;
Bitmap* SmileyCtrl::s_smiley_norm_bmp     = 0;
Bitmap* SmileyCtrl::s_smiley_ooh_bmp      = 0;
Bitmap* SmileyCtrl::s_smiley_worry_bmp    = 0;
Bitmap* SmileyCtrl::s_smiley_sunglass_bmp = 0;
  
//******************************************************************************
SmileyCtrl::SmileyCtrl (Ctrl* parent, GameCtrl* game)
//------------------------------------------------------------------------------
: ButtonCtrl(parent), m_game(game)
{
  // nocht nicht initialisiert?
  if (s_smiley_happy_bmp == 0)
  {
    // Bitmaps initialisieren
    s_smiley_happy_bmp    = CreateBitmap ("smiley", "happy"    );
    s_smiley_1eye_bmp     = CreateBitmap ("smiley", "1eye"     );
    s_smiley_2eyes_bmp    = CreateBitmap ("smiley", "2eyes"    );
    s_smiley_3eyes_bmp    = CreateBitmap ("smiley", "3eyes"    );
    s_smiley_4eyes_bmp    = CreateBitmap ("smiley", "4eyes"    );
    s_smiley_norm_bmp     = CreateBitmap ("smiley", "norm"     );
    s_smiley_ooh_bmp      = CreateBitmap ("smiley", "ooh"      );
    s_smiley_worry_bmp    = CreateBitmap ("smiley", "worry"    );
    s_smiley_sunglass_bmp = CreateBitmap ("smiley", "sunglass" );

    // check size
    ASSERT (s_smiley_happy_bmp->getSize() == s_smiley_1eye_bmp->getSize());
    ASSERT (s_smiley_happy_bmp->getSize() == s_smiley_2eyes_bmp->getSize());
    ASSERT (s_smiley_happy_bmp->getSize() == s_smiley_3eyes_bmp->getSize());
    ASSERT (s_smiley_happy_bmp->getSize() == s_smiley_4eyes_bmp->getSize());
    ASSERT (s_smiley_happy_bmp->getSize() == s_smiley_norm_bmp->getSize());
    ASSERT (s_smiley_happy_bmp->getSize() == s_smiley_ooh_bmp->getSize());
    ASSERT (s_smiley_happy_bmp->getSize() == s_smiley_worry_bmp->getSize());
    ASSERT (s_smiley_happy_bmp->getSize() == s_smiley_sunglass_bmp->getSize());
  }
}

//******************************************************************************
void SmileyCtrl::act()
//------------------------------------------------------------------------------
{
  // new_smiley
  Bitmap*  new_smiley = 0;

  if      (m_game->m_board->gameIsWon())           new_smiley = s_smiley_sunglass_bmp;
  else if (m_game->m_board->gameIsLost())          new_smiley = s_smiley_worry_bmp;
  else if (m_game->m_board->getState() == BEFORE)  new_smiley = s_smiley_happy_bmp;
  else if (m_game->m_board->getState() == PLAY)
  {
    if      (m_game->m_board->getCurStage() == 0)   new_smiley = s_smiley_happy_bmp;
    else if (m_game->m_board->getCurStage() == 1)   new_smiley = s_smiley_1eye_bmp;
    else if (m_game->m_board->getCurStage() == 2)   new_smiley = s_smiley_2eyes_bmp;
    else if (m_game->m_board->getCurStage() == 3)   new_smiley = s_smiley_3eyes_bmp;
    else if (m_game->m_board->getCurStage() == 4)   new_smiley = s_smiley_4eyes_bmp;
  }

  setUpBitmap   (new_smiley);
  setDownBitmap (s_smiley_ooh_bmp);
  
//  ButtonCtrl::draw();
}

//******************************************************************************
void SmileyCtrl::onClick()
//------------------------------------------------------------------------------
{
  m_game->newGame();
}


