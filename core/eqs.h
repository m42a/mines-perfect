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

#ifndef EQS_H
#define EQS_H

#include <functional>
#include <fstream> // fuer vector etc.
#include <vector>
#include <list>
#include <set>
#include <map>
#include <algorithm>
#include <time.h>      // clock

#include "cell.h"
#include "options.h"
#include "vargroup.h"


namespace MinesPerfect {

//------------------------------------------------------------------------------
enum Found      { FOUND_MOVE, FOUND_NO_MOVE, FOUND_CONTRA };

class  Eq;
class  Eqs;
struct Move;

typedef vector<Eq*>::iterator                 EqIter;
typedef vector<Eq*>::const_iterator           EqCIter;
typedef vector<Move>                          Moves;
typedef Moves::iterator                       MovesIter;
typedef Moves::const_iterator                 MovesCIter;


//******************************************************************************
  struct Move
//------------------------------------------------------------------------------
{
  CellNr     cell_nr; 
  CellState  new_state;

  Move ()                             : cell_nr (-1), new_state (OPEN) {;}
  Move (CellNr nr, CellState state) : cell_nr (nr), new_state (state){;}
};

bool operator== (Move m1, Move m2);
bool operator!= (Move m1, Move m2);

//******************************************************************************
  class Eq
//------------------------------------------------------------------------------
{
  friend class Var;
  friend class VarGroup;
  friend class Eqs;

private:

  // stat
  int            id;              // nur fuer Trace

  vector<Var*>   x;               // anliegende Variablen
  vector<Eq*>    b;               // anliegende Konstanten
  CellNr        cell;            // Pos. der Zelle
  int            val;             // Anzahl der Minen

  // dyn
  int            num_free_vars;   // Anzahl der freien Variablen
  int            num_free_cells;  // Anzahl der freien Zellen
  bool           flag;            // Arbeitsflag

  //------------------------------------
  bool init     (void); // nur dyn.
  bool XisEmpty (void)  { return x.size() == 0; }
};

//******************************************************************************
  class Eqs
//------------------------------------------------------------------------------
{
  friend class Eq;
  friend class Var;
  friend class VarGroup;

private:

  // stat.
  vector<Var*>        x;
  vector<Eq*>         b;
  vector<VarGroup*>   groups;

  int                 num_mines_remain;
  int                 num_vars_inside;
  bool                x_and_b_optimized;

  CellNrs             inner_cells;
  CellNrs             imposs_conds;
  CellNrs             inner_flags; // Ist beim Einlesen schon ein Widerspruch
                                   // aufgetaucht?
                                   //
                                   // x x x  -> Es entstehen bei diesem Beispiel
                                   // x 1 x     keine Variablen, d.h. es wuerde
                                   // x x x     unter Umstaenden kein Zug
                                   //           gefunden werden (-> Stage 3),
                                   // obwohl ein offensichtlicher Widerspruch
                                   // vorliegt.
  clock_t             clock0;
  clock_t             diff_clock;

  // dyn.
  Conclusion          conclusion;
  StageNr             no_move_stage;
  StageNr             solve_stage;

  //------------------------------------
  // stat.
  void  setXtoX     (void);
  void  setBtoB     (void);
  void  simplifyB   (void); // gleiche und ueberschn. Gleichungen zsm-fassen
  void  uniteX      (void); // Vars mit gleichen b's zsm-fassen
  void  buildGroups (void);
  void  init        (const int pass = 1); // nur dyn.

public:
  Eqs (const vector<Cell>& cells, bool (Cell::* isVar)(void) const);
  ~Eqs();

  // stat
  void  printStat        (void    ) const;
  void  logVal           (char res) const;

  // dyn.
  Found  findMoves1 (Moves& moves, CellNrs* const hints = 0); // Stage 1
  Found  findMoves2 (Moves& moves, CellNrs* const hints = 0); // Stage 2
  Found  findMoves3 (Moves& moves, int max_time,
                     const Move* const special_move = 0);      // Stage 3

  bool   findOneSolution (const vector<Cell>& cells, CellNrs& to_toggle,
                          Moves& moves, int max_time);

  StageNr  getSolveStage (void) const { return solve_stage; }
};


} // namespace MinesPerfect

#endif
