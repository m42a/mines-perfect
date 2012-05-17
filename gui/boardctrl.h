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

#ifndef BOARDCTRL_H
#define BOARDCTRL_H

#include "../core/board.h"
#include "../core/api.h"
#include "bitmapctrl.h"
//#include "cellctrl.h"

namespace MinesPerfect {

class GameCtrl;

//******************************************************************************
class BoardCtrl : public Ctrl
//------------------------------------------------------------------------------
{
  public:

    GameCtrl*  m_game;
    Rect       m_rect;
    CellNrs    m_changed_cells;       // -> state

    BoardCtrl (Ctrl* parent, GameCtrl* game);
    ~BoardCtrl();

    void setBoard (BoardNr nr);
    void setLevel (const Level& lvl); // oder so aehnlich
    void drawFrames();
    void actAllCells();
    void actCurCells (bool left_is_down, bool right_is_down);
    void actChangedCells();
    void setCellsDirty();
    void onMouseEvent (const MouseEvent& ev);

  private: //-------------------------------------------------------------------

    void initStatic();
    
    CellNr       m_cur_cell;            // -> state
    set<CellNr>  m_cur_cells;

    // bitmaps
    Bitmap*   m_source_bmp; // (->board_cur_bmp)?, aus <board>.bmp

    static Bitmap*   s_board_square_bmp;
    static Bitmap*   s_board_triangle_bmp;
    static Bitmap*   s_board_hexagon_bmp;
    static Bitmap*   s_board_grid3d_bmp;

    static Bitmap*   s_symb_flag_bmp;
    static Bitmap*   s_symb_quest_bmp;
    static Bitmap*   s_symb_mine_bmp;
    static Bitmap*   s_symb_cross_bmp;
    static Bitmap*   s_symb_zero_bmp;
    static Bitmap*   s_symb_digit_bmps[28];

    vector<Bitmap*>  m_cell_close_bmps;
    vector<Bitmap*>  m_cell_open_bmps;
    vector<Bitmap*>  m_cell_hintclose_bmps;
    vector<Bitmap*>  m_cell_hintopen_bmps;
    vector<Bitmap*>  m_cell_error_bmps;
    vector<Bitmap*>  m_frame_bmps;
    
    vector<BitmapCtrl*>  m_frame_ctrls;
    vector<BitmapCtrl*>  m_cell_ctrls;
    vector<BitmapCtrl*>  m_symbol_ctrls;

    // elementfunctionen
    void    actCell (CellNr k);
    CellNr  getCellAtPoint (const Point& p) const;
    void    actCursorCells();

//    void changeLevel     (const Level& lvl);
};

} // namespace MinesPerfect

#endif

