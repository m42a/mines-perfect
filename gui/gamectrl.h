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

//------------------------------------------------------------------------------

#ifndef GAMECTRL_H
#define GAMECTRL_H

#include "../core/utils.h"
#include "../core/board.h"
#include "../core/logbook.h"
#include "../core/options.h"
#include "../core/api.h"
#include "ctrl.h"
#include "lcdctrl.h"
#include "smileyctrl.h"
#include "boardctrl.h"
#include "bevelctrl.h"


namespace MinesPerfect {

//******************************************************************************
class GameCtrl : public Ctrl
//------------------------------------------------------------------------------
{
  public: //--------------------------------------------------------------------

    Options*  m_options;
    Board*    m_board;
    Logbook*  m_logbook;
    Timer*    m_timer;

    // sounds
    Sound*    m_open_sound;
    Sound*    m_flag_sound;
    Sound*    m_error_sound;
    Sound*    m_new_sound;
    Sound*    m_won_sound;
    Sound*    m_lost_sound;
    Sound*    m_deadend_sound;

    GameCtrl(Options* options);

    void changeLevel     (const Level& lvl);
    void changeLevel     (LevelNr lvl_nr);
    void changeModus     (Modus mod);
    void setMurphysLaw   (bool set_on);
    bool changeBoard     (BoardNr nr);
    void giveHint        (void);
    void solveOne        (void);
    void solveAll        (StageNr stage);
    void changeSolveAuto (StageNr stage);
    void changeMaxStage  (StageNr stage);
    void setShowMines    (bool set_on);
    void newGame         (int change_level = 0);

    Board*    getBoard()          { return m_board; }
    Options*  getOptions()        { return m_options; }

    void actWin();
    void load (const string& fname);
    void save (const string& fname);
    void undo (bool all);
    void redo();
    void onMouseEvent (const MouseEvent& ev);
    void showTime (int secs);

    void setOpenAfterStart() { m_open_after_start = true; }

  private: //-------------------------------------------------------------------

    uint32       m_rand_seq;        // -> m_options ?
    CellNr       m_cur_cell;            // -> state
    BOARD_STATE  m_state;               // -> state
    StageNr      m_next_move_stage;     // -> state
    bool         m_help_used;
    bool         m_open_after_start;  // for besttimes
//    bool         m_left_mousebutton_down;   // -> state

    SmileyCtrl*  m_smiley_button;
    LcdCtrl*     m_left_lcd;
    LcdCtrl*     m_right_lcd;
    BoardCtrl*   m_board_ctrl;

//    Modus        modus;

    // bevels
    BevelCtrl*   m_win_bevel;
    BevelCtrl*   m_win_head_bevel;
    BevelCtrl*   m_left_lcd_bevel;
    BevelCtrl*   m_left_sep_bevel;
    BevelCtrl*   m_smiley_bevel;
    BevelCtrl*   m_right_sep_bevel;
    BevelCtrl*   m_right_lcd_bevel;
    BevelCtrl*   m_win_body_bevel;
    bool         m_bevels_dirty;

    // elementfunctionen
    void initSound();
    void initCtrls();
    void alignCtrls();
    void playLog (const Log& log, bool with_sound);
    void playLogbook();
    void setHelpUsed();
    void alignCtrlInBevel (Ctrl* ctrl, const BevelCtrl* bvl, int h, int v, 
                           bool recursive = false);
};


} // namespace MinesPerfect

#endif

