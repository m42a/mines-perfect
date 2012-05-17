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

#ifndef CELL_H
#define CELL_H

#include <fstream> // fuer vector etc.
#include <list>
#include <set>
#include <vector>


namespace MinesPerfect {


//------------------------------------------------------------------------------
enum CellState  { CLEAN, OPEN, FLAGGED };

typedef int                      CellNr;
typedef vector<CellNr>           CellNrs;
typedef CellNrs::iterator        CellNrIter;
typedef CellNrs::const_iterator  CellNrCIter;

#define  for_all_adj(a,p)                               \
           for (CellNrCIter  a  = cells[p].adj.begin();     \
                             a != cells[p].adj.end();  a++)

#define  for_all(type, iter, cont)                      \
           for (type iter = cont.begin(); iter != cont.end(); iter++)

// g++: extern int trace_level;

//******************************************************************************
  struct FrameType
//------------------------------------------------------------------------------
{
  int  x;  // Ausgangsposition
  int  y;  //       "
  int  width;
  int  height;
};

//******************************************************************************
  struct Frame
//------------------------------------------------------------------------------
{
  int  type_nr; // Typ-nr
  int  x;       // Zielposition
  int  y;       //      "

  bool cmpType (const Frame& fr2) { return type_nr < fr2.type_nr; }
};

//******************************************************************************
  struct CellType
//------------------------------------------------------------------------------
{
  int            x;  // Ausgangsposition
  int            y;  //       "
  int            width;
  int            height;
  int            centre_x;
  int            centre_y;
  vector<Frame>  frames;
};

//******************************************************************************
  struct Cell
//------------------------------------------------------------------------------
{
  Cell() : adj() { state = CLEAN; is_mined = false; num = 0;
                   is_hint = false; }
  ~Cell(){}

  CellNrs     adj;
  CellState   state;
  int          num;
  bool         is_mined;
  bool         is_solved;
  bool         is_hint;
  int          num_open_adj;
  int          num_flagged_adj;

  // Bitmap
  int          type_nr;
  int          x;  // Zielposition
  int          y;  //      "

  //
  bool  isOpen        (void) const { return state == OPEN;    }
  bool  isClean       (void) const { return state == CLEAN;   }
  bool  isFlagged     (void) const { return state == FLAGGED; }
  bool  isMined       (void) const { return is_mined;         }
  bool  isSolved      (void) const { return is_solved;        }

  bool  isCond        (void) const { return  state == OPEN; }

  bool  isImpossCond  (void) const { return  state == OPEN 
                                             && num < num_flagged_adj;  }

  bool  isVisibleVar  (void) const { return  state == CLEAN
                                             && num_open_adj > 0; }

  bool  isRealVar     (void) const { return state != OPEN && !is_solved
                                            && num_open_adj > 0; }
};


} // namespace MinesPerfect

#endif
