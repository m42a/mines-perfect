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

#ifndef BOARD_H
#define BOARD_H

#include <string>
#include <fstream>
#include <vector>

using namespace std;

#include "eqs.h"
#include "options.h"
#include "logbook.h"
#include "api.h"

namespace MinesPerfect {

//------------------------------------------------------------------------------
enum BOARD_STATE { BEFORE = 0, PLAY, WON, LOST };

//******************************************************************************
  class Board
//------------------------------------------------------------------------------
{
private:
    // externe Variablen
    vector<CellType>   cell_types;
    vector<Cell>       cells;
    vector<FrameType>  frame_types;
    vector<Frame>      frames;

    Modus              modus;
    int                num_mines;
    bool               murphys_law;
    StageNr            auto_stage;
    StageNr            max_stage;
    bool               certified;
    bool               auto_hints_visible; // z.Zeit immer false
    unsigned           trace;              // z.Zeit noch nicht benutzt
    int                print_width;        // nur fuer print

    // interne Variablen
    vector<Level>      levels;
    BOARD_STATE        state;
    int                num_open_cells;
    int                num_flagged_cells;
    StageNr            startup_stage;
    Move               poss_move;
    StageNr            poss_move_stage;
    CellNrs            poss_move_hints;
    int                cur_cell_type_nr;
    int                cur_frame_type_nr;
    int                cur_frame_nr;
    CellNrs            visible_hints;
    Eqs*               eqs_visible; // ??? eqs_extern
    Eqs*               eqs_real;    // ??? eqs_intern
    int                poss_oot_nr; // Zaehler aus dem die oot_nr bestimmt wird
    clock_t            clock0;      //
    bool               multi_solve;

    // Funktionen
    void   setSquare   (Level& lvl);
    void   setHexagon  (Level& lvl);
    void   setTriangle (Level& lvl);
    void   set3dGrid   (Level& lvl);

    void   openCellBase    (CellNr k, CellNrs& changed_cells);
    void   flagCellBase    (CellNr k, CellNrs& changed_cells);
    bool   move            (Move    m, CellNrs& changed_cells);
    void   act             (CellNrs& changed_cells, Logbook& logbook);

    CellNr  getRandCell            (void);
    Found   findOneMove            (Logbook& logbook);
    Found   findMovesInSolvedCells (Moves& moves);

    void   solveAllBase        (int max_s, CellNrs& changed_cells, Logbook& logbook);
    bool   toggleMineIfPoss    (CellNr k, Logbook& logbook);
    void   chkMines            (void);
    int    calcMaxTime         (Logbook& logbook);
    void   read                (string fname, Level& level);
    void   readChkVal (int    val,      int val_min, int    val_max,
                       string obj_name, int obj_nr,  string obj_komp);

public:
    Board (Options& opt, Logbook& logbook);

    ~Board() {delete eqs_visible; delete eqs_real;}

    void  reset     (Logbook& logbook);
    bool  openCells (CellNr k,  CellNrs& changed_cells, Logbook& logbook);
    bool  flagCells (CellNr k,  CellNrs& changed_cells, Logbook& logbook);
    void  delHints  (            CellNrs& changed_cells);
    void  solveOne  (            CellNrs& changed_cells, Logbook& logbook);
    void  solveAll  (int max_s,  CellNrs& changed_cells, Logbook& logbook);
    void  giveHint  (CellNrs& changed_cells);

    void  setNumMines       (int n_mines, Logbook& logbook);
    bool  setModus          (Modus mod, CellNrs& changed_cells, Logbook& logbook);
    bool  setMaxStage       (StageNr stage, CellNrs& changed_cells, Logbook& logbook);
    bool  setAutoStage      (StageNr stage, CellNrs& changed_cells, Logbook& logbook);
    void  setTrace          (unsigned trc) { trace = trc;         }
    void  setMurphysLaw     (bool valid)   { murphys_law = valid; }

    // Konstant-Methoden
    bool  murphysLawIsValid (void)      const { return murphys_law;               }
    bool  cellIsMined       (CellNr k) const { return cells[k].is_mined;         }
    bool  cellIsOpen        (CellNr k) const { return cells[k].state == OPEN;    }
    bool  cellIsClean       (CellNr k) const { return cells[k].state == CLEAN;   }
    bool  cellIsFlagged     (CellNr k) const { return cells[k].state == FLAGGED; }
    int   getCellNum        (CellNr k) const { return cells[k].num;              }
    bool  cellIsHint        (CellNr k) const { return cells[k].is_hint;          }
    bool  isCertified       (void)     const { return certified; }

    bool  gameIsLost()    const { return state == LOST;                 }
    bool  gameIsWon()     const { return state == WON;                  }
    bool  gameOver()      const { return state == LOST || state == WON; }

    int       getNumMines()   const { return num_mines;         }
    int       getNumFlagged() const { return num_flagged_cells; }
    int       getNumOpen()    const { return num_open_cells; }
    int       getNumCells()   const { return cells.size();      }
    StageNr  getCurStage()   const { return poss_move_stage;   }
    bool      getDeadend()    const { return poss_move_stage == 0
                                           || max_stage < poss_move_stage; }

    BOARD_STATE      getState()                   const { return state;        }
    const CellNrs&  getAdjacentCells (CellNr k) const { return cells[k].adj; }

    void  getFieldsize      (int& height, int& width) const;
    void  getCellsWithPoint (CellNrs& found_cells, int x, int y) const;

    int getCellX (CellNr k) const { 
      ASSERT (0 <= k && (unsigned) k < cells.size());
      return cells[k].x; 
    }
    int getCellY (CellNr k) const { 
      ASSERT (0 <= k && (unsigned) k < cells.size());
      return cells[k].y;       
    } 
    int getCellTypeNr (CellNr k) const { 
      ASSERT (0 <= k && (unsigned) k < cells.size());
      return cells[k].type_nr; 
    }

    // Celltype + Frame
    const CellType*  getTypeOfCell (CellNr k) const {
      ASSERT (0 <= k && (unsigned) k < cells.size());
      return &cell_types[cells[k].type_nr];
    }
    const CellType*  getFirstCellType (void) {
      cur_cell_type_nr = 0;
      return cell_types.size() > 0 ? &cell_types[0] : 0;
    }
    const CellType*  getNextCellType (void) {
      return ++cur_cell_type_nr < (int) cell_types.size() ?
             &cell_types[cur_cell_type_nr] : 0;
    }
    int  getFrameTypeNr (int k) const {
      ASSERT (0 <= k && (unsigned) k < frames.size());
      return frames[k].type_nr;
    }
    const FrameType*  getFirstFrameType (void) {
      cur_frame_type_nr = 0;
      return frame_types.size() > 0 ? &frame_types[0] : 0;
    }
    const FrameType*  getNextFrameType (void) {
      return ++cur_frame_type_nr < (int) frame_types.size() ?
             &frame_types[cur_frame_type_nr] : 0;
    }
    const Frame*  getFirstFrame (void) {
      cur_frame_nr = 0;
      return frames.size() > 0 ? &frames[0] : 0;
    }
    const Frame*  getNextFrame (void) {
      return ++cur_frame_nr < (int) frames.size() ?
             &frames[cur_frame_nr] : 0;
    }
    void  print (void) const;
};

}

#endif
