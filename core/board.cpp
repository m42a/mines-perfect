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

#include <string>
#include <cstdlib>
#include <sstream>
#include <algorithm>

#ifdef VISUAL_CPP
  #include <minmax.h> // visual studio
#endif

#ifndef LINUX
  #ifndef VISUAL_CPP
//    #include <io.h>
  #endif
#endif

using namespace std;

#include "board.h"
#include "perfana.h"
#include "api.h"


// Konstanten zur Fehlersuche
namespace MinesPerfect
{
  namespace Glob
  {
    extern int     trace_level;
    extern RandInt rand_int; // ohne Wert -> randomize
  }
}

using namespace MinesPerfect;


const int  inv_num         = 9999;
const int  max_time_simple =  300; // msec, Wartezeit fuer einfaches Loesen
const int  max_time_multi  = 1000; // msec, Gesamtwartezeit fuer mehrfaches Loesen


struct CellPatternAdj
{
  int  nr;       // Nr. der Musterzelle
  int  tile_dh;  // Welcher Block (relativ horizontal)
  int  tile_dv;  // Welcher Block (relativ vertikal)

  CellPatternAdj() : nr(inv_num), tile_dh(inv_num), tile_dv(inv_num) {;}
};

struct CellPattern
{
  int                     type_nr;
  int                     x;    // rel. Ausgangsposition
  int                     y;    //       "
  int                     col;  // rel. Spalte (im Tile)
  int                     row;  // rel. Zeile  (im Tile)
  int                     cond; // bedingungen
  vector<CellPatternAdj>  adj;  // Nachbarn
/*
    vector<int>               adj_cell_nr;
    vector<int>               adj_tile_hor;
    vector<int>               adj_tile_ver;
*/
};

typedef vector<CellPattern>::const_iterator     CellPatternCIter;
typedef vector<CellPatternAdj>::const_iterator  CellPatternAdjCIter;
typedef vector<Frame>::const_iterator           FrameCIter;
typedef vector<FrameType>::const_iterator       FrameTypeCIter;
typedef vector<Cell>::const_iterator            CellCIter;
typedef vector<CellType>::const_iterator        CellTypeCIter;

typedef vector<CellPattern>::iterator     CellPatternIter;
typedef vector<CellPatternAdj>::iterator  CellPatternAdjIter;
typedef vector<Frame>::iterator           FrameIter;
typedef vector<FrameType>::iterator       FrameTypeIter;
typedef vector<Cell>::iterator            CellIter;
typedef vector<CellType>::iterator        CellTypeIter;


struct BoardReadError
{
  int     row;
  int     col;
  string  text;

  BoardReadError (int r, int c, const string& t) : row(r), col(c), text(t)
  {;}
};


//******************************************************************************
//  void MsgReadError (int col, int row, string text)
//------------------------------------------------------------------------------
/*
{
  ostringstream  ost;

  ost << "(" << col << ", " << row << "): " << text;

  Application->MessageBox ((char*) ost.str().c_str(), "Fehler beim Lesen!",
                           MB_OK + MB_DEFBUTTON1);
}
*/

//******************************************************************************
  void Board::readChkVal (int    val,      int val_min, int    val_max,
                          string obj_name, int obj_nr,  string obj_komp)
//------------------------------------------------------------------------------
{
  ostringstream  ost;

  ost << obj_name << "[" << obj_nr << "]" << "." << obj_komp
      << " = " << val << " : ";

  if (val < val_min)
  {
    ost << "too small (" << "< " << val_min << ")";
    throw BoardReadError (-1, -1, (char*) ost.str().c_str());
  }
  else if (val > val_max)
  {
    ost << "too large (" << "> " << val_max << ")";
    throw BoardReadError (-1, -1, (char*) ost.str().c_str());
  }
}

//******************************************************************************
  void Board::read (string fname, Level& lvl)
//------------------------------------------------------------------------------
{
  enum
  {
    BLOCK_INVALID,
    BLOCK_LEVELS,
    BLOCK_FRAME_TYPES,
    BLOCK_CELL_TYPES,
    BLOCK_CELL_PATTERNS,
    BLOCK_TILE
  }                     block_type;
  struct
  {
    int  hx;
    int  hy;
    int  vx;
    int  vy;
  }                    tile;
  vector<CellPattern>  cell_patterns;
  ifstream             in (fname.c_str());
//  ofstream             out ("read.txt");
  string               line, word;
  int                  file_row, file_col;
  int                  block_row = -1, block_col = -1;
  int                  val;
  string::size_type    p1, p2;

  if (in == 0)
    return;

  levels.clear();
  frame_types.clear();
  cell_types.clear();
  frames.clear();
  cells.clear();

  block_type = BLOCK_INVALID;
  file_row   = 0;

  //----------------------------------------------------------------------------
  // Einlesen
  //----------------------------------------------------------------------------
  while (!in.eof())
  {
    getline (in, line);
    ++file_row;

    p2 = 0;
    while (true)
    {
      // word
      p1 = line.find_first_not_of (" \t", p2);
      if (p1 == string::npos
      ||  p1 >= line.size())  // Fehler von Borland!!
        break;

      p2 = line.find_first_of (" \t", p1 + 1);

      word     = line.substr (p1, p2 - p1);
      file_col = p1 + 1;

      if (word == "//")
        break;

      // block_type
      if (p1 == 0 && line[0] == '<')
      {
        if      (word == "<levels>"       )  block_type = BLOCK_LEVELS;
        else if (word == "<frame-types>"  )  block_type = BLOCK_FRAME_TYPES;
        else if (word == "<cell-types>"   )  block_type = BLOCK_CELL_TYPES;
        else if (word == "<cell-patterns>")  block_type = BLOCK_CELL_PATTERNS;
        else if (word == "<tile>"         )  block_type = BLOCK_TILE;
        else
        {
          block_type = BLOCK_INVALID;
          throw BoardReadError (file_row, file_col, "invalid titel!");
        }

        block_row = -1;
        block_col = 0;
        continue;
      }

      // val
      if ((word[0] < '0' || '9' < word[0]) && word[0] != '-')
        throw BoardReadError (file_row, file_col, "invalid number!");

      if (word.size() >= 2
      && (word[1] < '0' || '9' < word[1]))
        throw BoardReadError (file_row, file_col + 1, "invalid number!");

      val = atoi (word.c_str());

      // block_row, block_col
      if (p1 == 0)
      {
        block_row++;
        block_col = 0;
      }
      else
        block_col++;

      if (block_row < 0)
        throw BoardReadError (file_row, file_col,
                                "1st number must be in the 1st column!");

      // Level
      if (block_type == BLOCK_LEVELS)
      {
        if (block_row > 2)
          throw BoardReadError (file_row, file_col,
                                  "too many levels!");
        else if (block_col > 3)
          throw BoardReadError (file_row, file_col,
                                  "too many numbers in line!");

        else if (block_col == 0)
        {
          levels.push_back (Level());
          levels.back().height = val;
        }
        else if (block_col == 1)
          levels.back().width = val;
        else if (block_col == 2)
          levels.back().num_mines = val;
        else
          levels.back().num_wholes = val;
      }
      // Frame-Type
      else if (block_type == BLOCK_FRAME_TYPES)
      {
        if (block_col == 0)
          frame_types.push_back (FrameType());

        if      (block_col == 0)  frame_types.back().x      = val;
        else if (block_col == 1)  frame_types.back().y      = val;
        else if (block_col == 2)  frame_types.back().width  = val;
        else if (block_col == 3)  frame_types.back().height = val;
        else if (block_col == 4)  throw BoardReadError (file_row, file_col,
                                               "too many numbers in line!");
      }
      // Cell-Type
      else if (block_type == BLOCK_CELL_TYPES)
      {
        if (block_col == 0)
          cell_types.push_back (CellType());

        if      (block_col == 0)  cell_types.back().x        = val;
        else if (block_col == 1)  cell_types.back().y        = val;
        else if (block_col == 2)  cell_types.back().width    = val;
        else if (block_col == 3)  cell_types.back().height   = val;
        else if (block_col == 4)  cell_types.back().centre_x = val;
        else if (block_col == 5)  cell_types.back().centre_y = val;
        else
        {
          if (block_col % 3 == 0)
          {
            cell_types.back().frames.push_back (Frame());
            cell_types.back().frames.back().type_nr = val;
          }
          else if (block_col % 3 == 1)
            cell_types.back().frames.back().x = val;
          else
            cell_types.back().frames.back().y = val;
        }
      }
      // Cell-Pattern
      else if (block_type == BLOCK_CELL_PATTERNS)
      {
        if (block_col == 0)
          cell_patterns.push_back (CellPattern());

        if      (block_col == 0)  cell_patterns.back().row     = val;
        else if (block_col == 1)  cell_patterns.back().col     = val;
        else if (block_col == 2)  cell_patterns.back().type_nr = val;
        else if (block_col == 3)  cell_patterns.back().x       = val;
        else if (block_col == 4)  cell_patterns.back().y       = val;
        else if (block_col == 5)  cell_patterns.back().cond    = val;
        else
        {
          if (block_col % 3 == 0)
          {
            cell_patterns.back().adj.push_back (CellPatternAdj());
            cell_patterns.back().adj.back().nr = val;
          }
          else if (block_col % 3 == 1)
            cell_patterns.back().adj.back().tile_dv = val;
          else //  block_col % 3 == 2
            cell_patterns.back().adj.back().tile_dh = val;
        }
      }
      // Tile
      else if (block_type == BLOCK_TILE)
      {
        if (block_row > 0)
          throw BoardReadError (file_row, file_col,
                                  "too many tiles!");
        else if (block_col > 4)
          throw BoardReadError (file_row, file_col,
                                  "too many numbers in line!");

        else if (block_col == 0)       tile.hx = val;
        else if (block_col == 1)       tile.hy = val;
        else if (block_col == 2)       tile.vx = val;
        else                           tile.vy = val;
      }

      //
      if (p2 == string::npos)
        break;
    }
  }

  //----------------------------------------------------------------------------
  // Werte ueberpruefen
  //----------------------------------------------------------------------------

  // Check Level
  if (levels.size() < 3)
    throw BoardReadError (-1, -1, "too few levels!");

  for (unsigned i = 0; i < levels.size(); i++)
  {
    readChkVal (levels[i].height,      1,   999, "level", i, "height");
    readChkVal (levels[i].width,       1,   999, "level", i, "width");
    readChkVal (levels[i].num_mines,   0, 99999, "level", i, "mines");
    readChkVal (levels[i].num_wholes, -1, 99999, "level", i, "wholes");
  }

//  // Check Frame-Type
//  if (frame_types.size() <= 0)
//    throw BoardReadError (-1, -1, "no frame-types!");

  { // visual studio
    for (unsigned i = 0; i < frame_types.size(); i++)
    {
      readChkVal (frame_types[i].x,      0, 1599, "frame-type", i, "x");
      readChkVal (frame_types[i].y,      0, 1199, "frame-type", i, "y");
      readChkVal (frame_types[i].width,  0, 1599, "frame-type", i, "width");
      readChkVal (frame_types[i].height, 0, 1199, "frame-type", i, "height");
    }
  } // visual studio

  // Check Cell-Type
  if (cell_types.size() <= 0)
    throw BoardReadError (-1, -1, "no cell-types!");

  { // visual studio
    for (unsigned i = 0; i < cell_types.size(); i++)
    {
      readChkVal (cell_types[i].x,        0, 1599, "cell-type", i, "x");
      readChkVal (cell_types[i].y,        0, 1199, "cell-type", i, "y");
      readChkVal (cell_types[i].width,    0, 1599, "cell-type", i, "width");
      readChkVal (cell_types[i].height,   0, 1199, "cell-type", i, "height");
      readChkVal (cell_types[i].centre_x, 0, 1599, "cell-type", i, "centre_x");
      readChkVal (cell_types[i].centre_y, 0, 1199, "cell-type", i, "centre_y");

      for (unsigned j = 0; j < cell_types[i].frames.size(); j++)
      {
        readChkVal (cell_types [i].frames[j].type_nr, 0, frame_types.size() - 1,
                   "cell-types",i,"frames[?].type_nr");
        readChkVal (cell_types [i].frames[j].x, -1599, 1599,
                   "cell-types",i,"frames[?].x");
        readChkVal (cell_types [i].frames[j].y, -1199, 1199,
                   "cell-types",i,"frames[?].y");
      }
    }
  } // visual studio


  // Check Cell-Pattern
  if (cell_patterns.size() <= 0)
    throw BoardReadError (-1, -1, "no cell-patterns!");

  { // visual studio
    for (unsigned i = 0; i < cell_patterns.size(); i++)
    {
      readChkVal (cell_patterns [i].row, 0, 999,
                 "cell-patterns",i,"row");
      readChkVal (cell_patterns [i].col, 0, 999,
                 "cell-patterns",i,"col");
      readChkVal (cell_patterns [i].type_nr, 0, cell_types.size() - 1,
                 "cell-patterns",i,"type_nr");
      readChkVal (cell_patterns [i].x, 0, 1599,
                 "cell-patterns",i,"x");
      readChkVal (cell_patterns [i].y, 0, 1199,
                 "cell-patterns",i,"y");
      readChkVal (cell_patterns [i].cond, 1, 777,
                 "cell-patterns",i,"cond");

      for (unsigned j = 0; j < cell_patterns[i].adj.size(); j++)
      {
        readChkVal (cell_patterns [i].adj[j].nr, 0, cell_patterns.size() - 1,
                   "cell-patterns",i,"adj[?].nr");
        readChkVal (cell_patterns [i].adj[j].tile_dv, -999, 999,
                   "cell-patterns",i,"adj[?].tile_dv");
        readChkVal (cell_patterns [i].adj[j].tile_dh, -999, 999,
                   "cell-patterns",i,"adj[?].tile_dh");
      }
    }
  } // visual studio

  // Check Tile
  readChkVal (tile.hx, 0, 1599, "tile", 0, "hx");
  readChkVal (tile.hy, 0, 1199, "tile", 0, "hy");
  readChkVal (tile.vx, 0, 1599, "tile", 0, "vx");
  readChkVal (tile.vy, 0, 1199, "tile", 0, "vy");

  //----------------------------------------------------------------------------
  // auswerten
  //----------------------------------------------------------------------------
  int  min_col = 999, max_col = -999;
  int  min_row = 999, max_row = -999;
  int  min_x = 9999, max_x = -9999;
  int  min_y = 9999, max_y = -9999;

  { // visual studio
    for (unsigned i = 0; i < cell_patterns.size(); i++)
    {
      if (min_col > cell_patterns[i].col)  min_col = cell_patterns[i].col;
      if (min_row > cell_patterns[i].row)  min_row = cell_patterns[i].row;
      if (max_col < cell_patterns[i].col)  max_col = cell_patterns[i].col;
      if (max_row < cell_patterns[i].row)  max_row = cell_patterns[i].row;

      if (min_x > cell_patterns[i].x)  min_x = cell_patterns[i].x;
      if (min_y > cell_patterns[i].y)  min_y = cell_patterns[i].y;
      if (max_x < cell_patterns[i].x)  max_x = cell_patterns[i].x;
      if (max_y < cell_patterns[i].y)  max_y = cell_patterns[i].y;
    }
  } // visual studio

  if (lvl.nr < (int) levels.size())
  {
    lvl.height     = levels[lvl.nr].height;
    lvl.width      = levels[lvl.nr].width;
    lvl.deep       = levels[lvl.nr].deep; // immer default
    lvl.num_mines  = levels[lvl.nr].num_mines;
    lvl.num_wholes = levels[lvl.nr].num_wholes;
  }
  else if (lvl.num_wholes == -1)
  {
    for (unsigned l = 0; l < levels.size(); l++)
      if (levels[l].num_wholes >= 0)
        lvl.num_wholes = 0;
  }

  int  num_rows   = max_row + 1;
  int  num_cols   = max_col + 1;
  int  num_tile_v = (lvl.height - 1) / num_rows + 1;
  int  num_tile_h = (lvl.width  - 1) / num_cols + 1;

  cells.resize (num_tile_v * num_tile_h * cell_patterns.size());

  vector<bool>  cell_valid (cells.size());

  for (int tile_v = 0; tile_v < num_tile_v; tile_v++)
  {
    for (int tile_h = 0; tile_h < num_tile_h; tile_h++)
    {
      int  x0 = tile_v * tile.vx + tile_h * tile.hx;
      int  y0 = tile_v * tile.vy + tile_h * tile.hy;

      for (unsigned i = 0; i < cell_patterns.size(); i++)
      {
        int row = tile_v * num_rows + cell_patterns[i].row;
        int col = tile_h * num_cols + cell_patterns[i].col;
        int k   = (num_tile_h * tile_v + tile_h) * cell_patterns.size() + i;

        int  cond = cell_patterns[i].cond;

        if      (row == 0        )      cond =  cond / 100;
        else if (row < lvl.height - 1)  cond = (cond / 10) % 10;
        else                            cond =  cond       % 10;

        if      (col == 0)             cond &= 4;
        else if (col < lvl.width - 1)  cond &= 2;
        else                           cond &= 1;

        cell_valid[k] = (cond != 0 && row < lvl.height && col < lvl.width);

        cells[k].type_nr    = cell_patterns[i].type_nr;
        cells[k].x          = x0 + cell_patterns[i].x;
        cells[k].y          = y0 + cell_patterns[i].y;

        for (unsigned j = 0; j < cell_patterns[i].adj.size(); j++)
        {
          int  adj_tile_v  = tile_v + cell_patterns[i].adj[j].tile_dv;
          int  adj_tile_h  = tile_h + cell_patterns[i].adj[j].tile_dh;
          int  adj_pattern = cell_patterns[i].adj[j].nr;

          int  adj_row = adj_tile_v * num_rows + cell_patterns[adj_pattern].row;
          int  adj_col = adj_tile_h * num_cols + cell_patterns[adj_pattern].col;

          int  k2 = (num_tile_h * adj_tile_v + adj_tile_h)
                     * cell_patterns.size() + cell_patterns[i].adj[j].nr;

          if (0 <= adj_row && adj_row < lvl.height
          &&  0 <= adj_col && adj_col < lvl.width)
            cells[k].adj.push_back (k2);
        }
      }
    }
  }

//  readChkVal (levels[i].num_mines,   0, 99999, "level", i, "mines");
//  readChkVal (levels[i].num_wholes, -1, 99999, "level", i, "wholes");

  // wholes
  vector<int>  wholes;

  for (int k = 0; k < (int) cells.size(); k++)
    if (cell_valid[k])
      wholes.push_back(k); // erstmal alle gueltigen

  int num_valid_cells = wholes.size();

  if (num_valid_cells == 0)
    throw BoardReadError (-1, -1, "Boardsize too small - no valid cells!");

  if (lvl.num_wholes >= num_valid_cells)
  {
    if (lvl.nr < (int) levels.size())
    {
      ostringstream  ost;

      ost << "level[" << lvl.nr << "].wholes = " << lvl.num_wholes
          << " : too big (>= " << num_valid_cells << ")";

      throw BoardReadError (-1, -1, (char*) ost.str().c_str());
    }

    lvl.num_wholes = num_valid_cells - 1;
  }

  int num_cells_remain = num_valid_cells - lvl.num_wholes;

  if (lvl.num_mines >= num_cells_remain)
  {
    if (lvl.nr < (int) levels.size())
    {
      ostringstream  ost;

      ost << "level[" << lvl.nr << "].mines = " << lvl.num_mines
          << " : too big (>= " << num_cells_remain << ")";

      throw BoardReadError (-1, -1, (char*) ost.str().c_str());
    }

    lvl.num_mines = num_cells_remain - 1;
  }

// sonst geht undo nicht 
//  random_shuffle (wholes.begin(), wholes.end(), Glob::rand_int);

  for (int w = 0; w + 1 < (int) wholes.size() && w < lvl.num_wholes; w++)
    cell_valid[wholes[w]] = false;


  // old_to_new
  vector<int>  old_to_new (cells.size());
  unsigned     k_new;

  k_new = 0;
  for (unsigned k_old = 0; k_old < cells.size(); k_old++)
    if (cell_valid[k_old])
    {
      old_to_new[k_old] = k_new;
      k_new++;
    }

  // ungueltige Zellen loeschen
  k_new = 0;
  { // visual studio
    for (unsigned k_old = 0; k_old < cells.size(); k_old++)
    {
      if (cell_valid[k_old])
      {
        if (k_new != k_old)
          cells[k_new] = cells[k_old];

        k_new++;
      }
    }
  } // visual studio

  cells.resize (k_new);

  // adj anpassen
  { // visual studio
    for (unsigned k = 0; k < cells.size(); k++)
    {
      unsigned j_new = 0;
      for (unsigned j_old = 0; j_old < cells[k].adj.size(); j_old++)
      {
        unsigned k2 = cells[k].adj[j_old];

        if (cell_valid[k2])
        {
          cells[k].adj[j_new] = old_to_new [k2];
          j_new++;
        }
      }
      cells[k].adj.resize (j_new);
    }
  } // visual studio

  //
  readChkVal (cells.size(), 1, 9999, "cells.size", 0, "");

  // Frames
  for (int ft = 0; ft < (int) frame_types.size(); ft++)
  {
    // Frames nach Frame-Type-Nr sortiert eintragen
    // koennte schneller gehen, wenn man erst alle Frames erzeugt und sie
    // dann nach den Typ-Nummern sortiert

    for (unsigned k = 0; k < cells.size(); k++)
    {
      CellType* ct = &cell_types[cells[k].type_nr];

      for (int f = 0; f < (int) ct->frames.size(); f++)
      {
        if (ct->frames[f].type_nr != ft)
          continue;

        frames.push_back (Frame());

        frames.back().type_nr = ct->frames[f].type_nr;
        frames.back().x       = cells[k].x + ct->frames[f].x;
        frames.back().y       = cells[k].y + ct->frames[f].y;
      }
    }
  }

// Compiler ist zu doof zum sortieren!!!
//  sort (frames.begin(), frames.end(), mem_cmp (&Frame::type_nr));
//  sort (frames.begin(), frames.end(), mem_fun1_ref (&Frame::cmpType));
//  sort (frames.begin(), frames.end(), FRAME_CMP_TYPE());

  // Koordinaten normieren
  min_x = 9999;
  min_y = 9999;

  // ... min_x, min_y
  for_all (FrameCIter, f, frames)
  {
    if (min_x > f->x)  min_x = f->x;
    if (min_y > f->y)  min_y = f->y;
  }

  { // visual studio
    for_all (CellCIter, k, cells)
    {
      if (min_x > k->x)  min_x = k->x;
      if (min_y > k->y)  min_y = k->y;
    }
  } // visual studio

  // ... normieren
  { // visual studio
    for_all (FrameIter, f, frames)
    {
      f->x -= min_x;
      f->y -= min_y;
    }
  } // visual studio

  { // visual studio
    for_all (CellIter, k, cells)
    {
      k->x -= min_x;
      k->y -= min_y;
    }
  } // visual studio

//        ReadChkVal (cells.back().x, 0, 1599, "cells", cells.size() - 1, "x");
//        ReadChkVal (cells.back().y, 0, 1199, "cells", cells.size() - 1, "y");

  certified = false;
}

//******************************************************************************
  void Board::setSquare (Level& lvl)
//------------------------------------------------------------------------------
{
  // level, height, width, n_mines
  if (lvl.nr == BEGINNER)
  {
    lvl.height     =  8;
    lvl.width      =  8;
    lvl.deep       =  0;
    lvl.num_mines  = 10;
    lvl.num_wholes = -1;
  }
  else if (lvl.nr == INTERMEDIATE)
  {
    lvl.height     = 16;
    lvl.width      = 16;
    lvl.deep       =  0;
    lvl.num_mines  = 40;
    lvl.num_wholes = -1;
  }
  else if (lvl.nr == EXPERT)
  {
    lvl.height     = 16;
    lvl.width      = 30;
    lvl.deep       =  0;
    lvl.num_mines  = 99;
    lvl.num_wholes = -1;
  }
  else
  {
    lvl.nr = USER_DEFINED;

    if (lvl.height  <  1)  lvl.height  =  1;
    if (lvl.height  > 44)  lvl.height  = 44;
    if (lvl.width   <  1)  lvl.width   =  1;
    if (lvl.width   > 78)  lvl.width   = 78;

    lvl.deep       =  0;
    lvl.num_wholes = -1;
  }

  // cell_types
  cell_types.resize(1);
  cell_types[0].x        =  0;
  cell_types[0].y        =  0;
  cell_types[0].width    = 16;
  cell_types[0].height   = 16;
  cell_types[0].centre_x =  7;
  cell_types[0].centre_y =  7;

  // cells...
  cells.resize (lvl.height * lvl.width);

  if (lvl.nr == USER_DEFINED)
  {
    if (lvl.num_mines < 0)
      lvl.num_mines = 0;
    if (lvl.num_mines > (int) cells.size() -1 )
      lvl.num_mines = cells.size() - 1;
  }

  // cells[].adj
  for (CellNr i = 0; i < (CellNr) cells.size(); i++)
  {
    int y0   = i / lvl.width;
    int x0   = i % lvl.width;
    int ymin = max (0, y0 - 1);
    int xmin = max (0, x0 - 1);
    int ymax = min (y0 + 1, lvl.height - 1);
    int xmax = min (x0 + 1, lvl.width - 1);

    cells[i].adj.clear();

    for (int y = ymin; y <= ymax; y++)
      for (int x = xmin; x <= xmax; x++)
        if (x != x0 || y != y0)
          cells[i].adj.push_back (x + y * lvl.width);
  }

  // cells[]->Bitmap
  { // visual studio
    for (CellNr i = 0; i < (CellNr) cells.size(); i++)
    {
      cells[i].type_nr = 0;
      cells[i].x       = 16 * (i % lvl.width) + 3;
      cells[i].y       = 16 * (i / lvl.width) + 3;
    }
  } // visual studio

  // frame_types
  frame_types.resize(8);

  // ...links-oben
  frame_types[0].x      =  0;
  frame_types[0].y      = 17;
  frame_types[0].width  =  3;
  frame_types[0].height =  3;

  // ...oben
  frame_types[1].x      =  4;
  frame_types[1].y      = 17;
  frame_types[1].width  = 16;
  frame_types[1].height =  3;

  // ...rechts-oben
  frame_types[2].x      = 21;
  frame_types[2].y      = 17;
  frame_types[2].width  =  3;
  frame_types[2].height =  3;

  // ...links
  frame_types[3].x      =  0;
  frame_types[3].y      = 21;
  frame_types[3].width  =  3;
  frame_types[3].height = 16;

  // ...rechts
  frame_types[4].x      = 21;
  frame_types[4].y      = 21;
  frame_types[4].width  =  3;
  frame_types[4].height = 16;

  // ...links-unten
  frame_types[5].x      =  0;
  frame_types[5].y      = 38;
  frame_types[5].width  =  3;
  frame_types[5].height =  3;

  // ...unten
  frame_types[6].x      =  4;
  frame_types[6].y      = 38;
  frame_types[6].width  = 16;
  frame_types[6].height =  3;

  // ...rechts-unten
  frame_types[7].x      = 21;
  frame_types[7].y      = 38;
  frame_types[7].width  =  3;
  frame_types[7].height =  3;

  // frames
  frames.resize (2 * lvl.height + 2 * lvl.width + 4);

  // ...links-oben
  int k = 0;
  frames[k].type_nr = 0;
  frames[k].x       = 0;
  frames[k].y       = 0;

  // ...oben
  { // visual studio
    for (int i = 0; i < lvl.width; i++)
    {
       k++;
       frames[k].type_nr = 1;
       frames[k].x       = 3 + 16 * i;
       frames[k].y       = 0;
    }
  }

  // ...rechts-oben
  k++;
  frames[k].type_nr = 2;
  frames[k].x       = 3 + lvl.width * 16;
  frames[k].y       = 0;

  // ...links
  { // visual studio
    for (int i = 0; i < lvl.height; i++)
    {
       k++;
       frames[k].type_nr = 3;
       frames[k].x       = 0;
       frames[k].y       = 3 + 16 * i;
    }
  } // visual studio

  // ...rechts
  { // visual studio
    for (int i = 0; i < lvl.height; i++)
    {
       k++;
       frames[k].type_nr = 4;
       frames[k].x       = 3 + 16 * lvl.width;
       frames[k].y       = 3 + 16 * i;
    }
  } // visual studio

  // ...links-unten
  k++;
  frames[k].type_nr = 5;
  frames[k].x       = 0;
  frames[k].y       = 3 + 16 * lvl.height;

  // ...unten
  { // visual studio
    for (int i = 0; i < lvl.width; i++)
    {
       k++;
       frames[k].type_nr = 6;
       frames[k].x       = 3 + 16 * i;
       frames[k].y       = 3 + 16 * lvl.height;
    }
  } // visual studio

  // ...rechts-unten
  k++;
  frames[k].type_nr = 7;
  frames[k].x       = 3 + 16 * lvl.width;
  frames[k].y       = 3 + 16 * lvl.height;

  // sonstiges  
  certified = true;
}

//******************************************************************************
  void Board::setHexagon (Level& lvl)
//------------------------------------------------------------------------------
{
  int     num_cells;
  CellNr  k;
  int     i;

  if (lvl.nr == BEGINNER)
  {
    lvl.height     =  5;
    lvl.width      =  5;
    lvl.deep       =  0;
    lvl.num_mines  = 10;
    lvl.num_wholes = -1;
  }
  else if (lvl.nr == INTERMEDIATE)
  {
    lvl.height     = 10;
    lvl.width      = 10;
    lvl.deep       =  0;
    lvl.num_mines  = 45;
    lvl.num_wholes = -1;
  }
  else if (lvl.nr == EXPERT)
  {
    lvl.height     = 13;
    lvl.width      = 13;
    lvl.deep       =  0;
    lvl.num_mines  = 95;
    lvl.num_wholes = -1;
  }
  else
  {
    lvl.nr = USER_DEFINED;
    if (lvl.height  <  1)  lvl.height  =  1;
    if (lvl.height  > 15)  lvl.height  = 15;
    if (lvl.width   <  1)  lvl.width   =  1;
    if (lvl.width   > 20)  lvl.width   = 20;

    lvl.deep       =  0;
    lvl.num_wholes = -1;
  }

  num_cells =  (2 * lvl.height - 1) * lvl.width
             + (lvl.height - 1) * (lvl.height - 1);

  if (lvl.nr == USER_DEFINED)
  {
    if (lvl.num_mines < 0            )  lvl.num_mines = 0;
    if (lvl.num_mines > num_cells - 1)  lvl.num_mines = num_cells - 1;
  }

  // cell_types...
  cell_types.resize(1);

  cell_types[0].x        =  0;
  cell_types[0].y        =  0;
  cell_types[0].width    = 20;
  cell_types[0].height   = 22;
  cell_types[0].centre_x =  9;
  cell_types[0].centre_y = 10;

  // cells...
  cells.resize (num_cells);

  // cells[].adj - obere Haelft + Mitte
  for (k = 0; k < (CellNr) cells.size(); k++)
    cells[k].adj.clear();

  k = 0;

  for (int w = lvl.width; w < lvl.width + lvl.height; w++)
  {
    for (int x0 = 0; x0 < w; x0++)
    {
      if (w > lvl.width && x0 > 0    )  cells[k].adj.push_back (k - w);
      if (w > lvl.width && x0 < w - 1)  cells[k].adj.push_back (k - w + 1);
      if (                 x0 > 0    )  cells[k].adj.push_back (k - 1);

      k++;
    }
  }

  // ...untere Haelfte
  { // visual studio
    for (int w = lvl.width + lvl.height - 2; w >= lvl.width; w--)
    {
      for (int x0 = 0; x0 < w; x0++)
      {
        cells[k].adj.push_back (k - w - 1);
        cells[k].adj.push_back (k - w);

        if (x0 > 0)
          cells[k].adj.push_back (k - 1);

        k++;
      }
    }
  } // visual studio

  // ...Rueckverzeigerung
  for (k = 0; k < (CellNr) cells.size(); k++)
    for (CellNr l = 0; l < (CellNr) cells[k].adj.size(); l++)
      cells[cells[k].adj[l]].adj.push_back(k);

  // cells[]->Bitmap
  for (k = 0; k < (CellNr) cells.size(); k++)
  {
    cells[k].type_nr = 0;
  }

  // ... x, y
  k = 0;
  for (int j = 0; j < lvl.height; j++)
  {
    for (int i = 0; i < lvl.width + j; i++, k++)
    {
      cells[k].x = 10 * (lvl.height - 1 - j) + 20 * i + 3;
      cells[k].y = 17 * j + 3;
    }
  }

  { // visual studio
    for (int j = 1; j < lvl.height; j++)
    {
      for (int i = 0; i < lvl.width + lvl.height - 1 - j; i++, k++)
      {
        cells[k].x = 10 * j + 20 * i + 3;
        cells[k].y = 17 * (lvl.height - 1 + j) + 3;
      }
    }
  }

  // frame_types...
  frame_types.resize(8);

  frame_types[0].x      = 15;
  frame_types[0].y      = 23;
  frame_types[0].width  = 20;
  frame_types[0].height =  8;

  frame_types[1].x      =  4;
  frame_types[1].y      = 28;
  frame_types[1].width  = 10;
  frame_types[1].height = 19;

  frame_types[2].x      = 36;
  frame_types[2].y      = 28;
  frame_types[2].width  = 10;
  frame_types[2].height = 20;

  frame_types[3].x      =  0;
  frame_types[3].y      = 45;
  frame_types[3].width  =  3;
  frame_types[3].height = 18;

  frame_types[4].x      = 47;
  frame_types[4].y      = 45;
  frame_types[4].width  =  3;
  frame_types[4].height = 17;

  frame_types[5].x      =  4;
  frame_types[5].y      = 60;
  frame_types[5].width  = 10;
  frame_types[5].height = 20;

  frame_types[6].x      = 36;
  frame_types[6].y      = 60;
  frame_types[6].width  = 10;
  frame_types[6].height = 19;

  frame_types[7].x      = 15;
  frame_types[7].y      = 77;
  frame_types[7].width  = 20;
  frame_types[7].height =  8;

  // frames...
  frames.resize (2 * lvl.width + 4 * lvl.height - 2);

  // ...oben
  k = -1;
  for (i = 0; i < lvl.width; i++)
  {
    k++;
    frames[k].type_nr = 0;
    frames[k].x       = 10 * lvl.height + 20 * i - 7;
    frames[k].y       = 0;
  }

  // ...links-oben
  for (i = 0; i < lvl.height - 1; i++)
  {
    k++;
    frames[k].type_nr = 1;
    frames[k].x       = 10 * (lvl.height - i) - 17;
    frames[k].y       = 17 * i + 5;
  }

  // ...rechts-oben
  for (i = 0; i < lvl.height - 1; i++)
  {
    k++;
    frames[k].type_nr = 2;
    frames[k].x       = 20 * lvl.width + 10 * (lvl.height + i) - 7;
    frames[k].y       = 17 * i + 5;
  }

  // ...links
  k++;
  frames[k].type_nr = 3;
  frames[k].x       = 0;
  frames[k].y       = 17 * lvl.height - 12;

  // ...rechts
  k++;
  frames[k].type_nr = 4;
  frames[k].x       = 20 * (lvl.height + lvl.width) - 17;
  frames[k].y       = 17 * lvl.height - 12;

  // ...links-unten
  for (i = 0; i < lvl.height - 1; i++)
  {
    k++;
    frames[k].type_nr = 5;
    frames[k].x       = 10 * i + 3;
    frames[k].y       = 17 * (lvl.height + i) + 3;
  }

  // ...rechts-unten
  for (i = 0; i < lvl.height - 1; i++)
  {
    k++;
    frames[k].type_nr = 6;
    frames[k].x       = 20 * (lvl.height + lvl.width) - 10 * i - 27;
    frames[k].y       = 17 * (lvl.height + i) + 3;
  }

  // ...unten
  for (i = 0; i < lvl.width; i++)
  {
    k++;
    frames[k].type_nr = 7;
    frames[k].x       = 10 * lvl.height + 20 * i - 7;
    frames[k].y       = 34 * lvl.height - 14;
  }

  // sonstiges  
  certified = true;
}

//******************************************************************************
  void Board::setTriangle (Level& lvl)
//------------------------------------------------------------------------------
{
  int num_cells;
  int i;

  if (lvl.nr == BEGINNER)
  {
    lvl.height     =  8;
    lvl.width      =  8;
    lvl.deep       =  0;
    lvl.num_mines  = 10;
    lvl.num_wholes = -1;
  }
  else if (lvl.nr == INTERMEDIATE)
  {
    lvl.height     = 14;
    lvl.width      = 14;
    lvl.deep       =  0;
    lvl.num_mines  = 30;
    lvl.num_wholes = -1;
  }
  else if (lvl.nr == EXPERT)
  {
    lvl.height     = 18;
    lvl.width      = 18;
    lvl.deep       =  0;
    lvl.num_mines  = 60;
    lvl.num_wholes = -1;
  }
  else
  {
    lvl.nr = USER_DEFINED;
    if (lvl.height  <  1)  lvl.height  =  1;
    if (lvl.height  > 20)  lvl.height  = 20;

    lvl.width      = lvl.height;
    lvl.deep       =  0;
    lvl.num_wholes = -1;
  }

  num_cells = lvl.height * lvl.height;

  if (lvl.nr == USER_DEFINED)
  {
    if (lvl.num_mines < 0            )  lvl.num_mines = 0;
    if (lvl.num_mines > num_cells - 1)  lvl.num_mines = num_cells - 1;
  }

  // cell_types...
  cell_types.resize(2);

  cell_types[0].x        =  0;
  cell_types[0].y        =  0;
  cell_types[0].width    = 26;
  cell_types[0].height   = 26;
  cell_types[0].centre_x = 12;
  cell_types[0].centre_y =  7;

  cell_types[1].x        =  0;
  cell_types[1].y        = 26;
  cell_types[1].width    = 26;
  cell_types[1].height   = 26;
  cell_types[1].centre_x = 12;
  cell_types[1].centre_y = 17;

  // cells...
  cells.resize (num_cells);

  // cells[].adj - obere Haelft + Mitte
  { // visual studio
  for (CellNr k = 0; k < (CellNr) cells.size(); k++)
    cells[k].adj.clear();
  } // visual studio

  // Die Zellnummerierung verlaeuft von unten nach oben und von links nach
  // rechts. Dadurch ist die Nummer der 1. Zelle in jeder Zeile eine
  // Quadratzahl.
  for (i = 0; i < lvl.height; i++)
  {
    for (int j = 0; j <= 2 * i; j++)
    {
      CellNr k = i * i + j;

      // Zeile drueber
      if (i < lvl.height - 1)
        for (int l  = max (0,         j - 1 + j % 2);
                 l <= min (2 * i + 2, j + 3 - j % 2); l++)
          cells[k].adj.push_back ((i + 1) * (i + 1) + l);

      // gleiche Zeile
      for (int l  = max (0,     j - 2);
               l <= min (2 * i, j + 2); l++)
        if (l != j)
          cells[k].adj.push_back (i * i + l);

      // Zeile drunter
      if (i > 0)
        for (int l  = max (0,         j - 2 - j % 2);
                 l <= min (2 * i - 2, j + 0 + j % 2); l++)
          cells[k].adj.push_back ((i - 1) * (i - 1) + l);
    }
  }

  // cells[]->Bitmap
  for (i = 0; i < lvl.height; i++)
  {
    for (int j = 0; j <= 2 * i; j++)
    {
      CellNr k = i * i + j;

      cells[k].type_nr = j % 2;
      cells[k].x       = 13 * (lvl.height - 1 - i + j) + 5;
      cells[k].y       = 26 * (lvl.height - 1 - i    ) + 3;
    }
  }

  // frame_types...
  frame_types.resize(6);

  frame_types[0].x      =  0;
  frame_types[0].y      = 53;
  frame_types[0].width  =  4;
  frame_types[0].height =  7;

  frame_types[1].x      =  6;
  frame_types[1].y      = 53;
  frame_types[1].width  = 26;
  frame_types[1].height =  3;

  frame_types[2].x      = 35;
  frame_types[2].y      = 53;
  frame_types[2].width  =  5;
  frame_types[2].height =  9;

  frame_types[3].x      =  5;
  frame_types[3].y      = 57;
  frame_types[3].width  = 13;
  frame_types[3].height = 31;

  frame_types[4].x      = 21;
  frame_types[4].y      = 58;
  frame_types[4].width  = 13;
  frame_types[4].height = 30;

  frame_types[5].x      = 19;
  frame_types[5].y      = 83;
  frame_types[5].width  =  1;
  frame_types[5].height =  3;

  // frames...
  frames.resize (3 * lvl.height + 3);
  int k = -1;

  // ...links-oben
  k++;
  frames[k].type_nr = 0;
  frames[k].x       = 0;
  frames[k].y       = 0;

  // ...oben
  for (i = 0; i < lvl.height; i++)
  {
    k++;
    frames[k].type_nr = 1;
    frames[k].x       = 26 * i + 4;
    frames[k].y       = 0;
  }

  // ...rechts-oben
  k++;
  frames[k].type_nr = 2;
  frames[k].x       = 26 * lvl.height + 4;
  frames[k].y       = 0;

  // ...links
  for (i = 0; i < lvl.height; i++)
  {
    k++;
    frames[k].type_nr = 3;
    frames[k].x       = 13 * i + 4;
    frames[k].y       = 26 * i + 2;
  }

  // ...rechts
  for (i = 0; i < lvl.height; i++)
  {
    k++;
    frames[k].type_nr = 4;
    frames[k].x       = 26 * lvl.height - 13 * i - 8;
    frames[k].y       = 26 * i + 3;
  }

  // unten
  k++;
  frames[k].type_nr = 5;
  frames[k].x       = 13 * lvl.height + 4;
  frames[k].y       = 26 * lvl.height + 2;

  // sonstiges  
  certified = true;
}

//******************************************************************************
  void Board::set3dGrid (Level& lvl)
//------------------------------------------------------------------------------
{
  // level, height, width, n_mines
  if (lvl.nr == BEGINNER)
  {
    lvl.height     =  3;
    lvl.width      =  3;
    lvl.deep       =  3;
    lvl.num_mines  =  3;
    lvl.num_wholes = -1;
  }
  else if (lvl.nr == INTERMEDIATE)
  {
    lvl.height     =  4;
    lvl.width      =  4;
    lvl.deep       =  4;
    lvl.num_mines  =  8;
    lvl.num_wholes = -1;
  }
  else if (lvl.nr == EXPERT)
  {
    lvl.height     =  5;
    lvl.width      =  5;
    lvl.deep       =  5;
    lvl.num_mines  = 20;
    lvl.num_wholes = -1;
  }
  else
  {
    lvl.nr = USER_DEFINED;

    if (lvl.height  <  1)  lvl.height  =  1;
    if (lvl.height  > 99)  lvl.height  = 99;
    if (lvl.width   <  1)  lvl.width   =  1;
    if (lvl.width   > 99)  lvl.width   = 99;
    if (lvl.deep    <  1)  lvl.deep    =  1;
    if (lvl.deep    > 99)  lvl.deep    = 99;
  }

  // cell_types
  cell_types.resize(27);
  for (unsigned t = 0; t < cell_types.size(); t++)
  {
    cell_types[t].x        =  0 + 120 * (t / 9);
    cell_types[t].y        =  0 +  22 * (t % 9);
    cell_types[t].width    = 21;
    cell_types[t].height   = 21;
    cell_types[t].centre_x = 10;
    cell_types[t].centre_y = 10;
  }

  // cells...
  cells.resize (lvl.height * lvl.width * lvl.deep);

  if (lvl.nr == USER_DEFINED)
  {
    if (lvl.num_mines < 0)
      lvl.num_mines = 0;
    if (lvl.num_mines > (int) cells.size() - 1)
      lvl.num_mines = cells.size() - 1;
  }

  // cells[].adj
  for (CellNr i = 0; i < (CellNr) cells.size(); i++)
  {
    int z0   =  i / (lvl.width * lvl.height);
    int y0   = (i % (lvl.width * lvl.height)) / lvl.width;
    int x0   = i % lvl.width;
    int zmin = max (0, z0 - 1);
    int ymin = max (0, y0 - 1);
    int xmin = max (0, x0 - 1);
    int zmax = min (z0 + 1, lvl.deep - 1);
    int ymax = min (y0 + 1, lvl.height - 1);
    int xmax = min (x0 + 1, lvl.width - 1);

    cells[i].adj.clear();

    for (int z = zmin; z <= zmax; z++)
      for (int y = ymin; y <= ymax; y++)
        for (int x = xmin; x <= xmax; x++)
          if (x != x0 || y != y0 || z != z0)
            cells[i].adj.push_back (  x
                                    + y * lvl.width
                                    + z * lvl.width * lvl.height);
  }

  // dx, dy
//  int dx = 21 + 16 * (lvl.deep - 1);
//  int dy = 21 + 16 * (lvl.deep - 1);
  int dx = 21 + 16 * lvl.deep;
  int dy = 21 + 16 * lvl.deep;

  // cells[]->Bitmap
  { // visual studio
    for (CellNr i = 0; i < (CellNr) cells.size(); i++)
    {
      int z =  i / (lvl.width * lvl.height);
      int y = (i % (lvl.width * lvl.height)) / lvl.width;
      int x = i % lvl.width;

      int zn = (z == 0) ? 0 : (z == lvl.deep   - 1) ? 2 : 1;
      int yn = (y == 0) ? 0 : (y == lvl.height - 1) ? 2 : 1;
      int xn = (x == 0) ? 0 : (x == lvl.width  - 1) ? 2 : 1;

      cells[i].type_nr = 9 * zn + 3 * yn + xn;
      cells[i].x       = dx * x + 16 * (lvl.deep - 1 - z);
      cells[i].y       = dy * y + 16 * z;
    }
  } // visual studio

  // frame_types
  frame_types.resize(2);

  // ...horizontal
  frame_types[0].x      =   0;
  frame_types[0].y      = 200;
  frame_types[0].width  =   5;
  frame_types[0].height =  16;

  // ...vertikal
  frame_types[1].x      =  10;
  frame_types[1].y      = 200;
  frame_types[1].width  =  16;
  frame_types[1].height =   5;

  // frames
  frames.resize (  (  lvl.width      * (lvl.height - 1)
                   + (lvl.width - 1) *  lvl.height     )
                 *  lvl.deep
//                 * (lvl.deep - 1));
                 *  lvl.deep);

  for (int z = 0, f = 0; z < lvl.deep; z++)
  {
    int y;
    // horizontal
    for (y = 0; y < lvl.height; y++)
    {
      for (int x = 0; x < lvl.width - 1; x++)
      {
//        for (int i = 0; i < lvl.deep - 1; i++, f++)
        for (int i = 0; i < lvl.deep; i++, f++)
        {
          frames[f].type_nr = 1;
          frames[f].x       = 21 + dx * x + 16 * (lvl.deep - 1 - z) + 16 * i;
          frames[f].y       =  8 + dy * y + 16 * z;
        }
      }
    }

    // vertikal
    for (y = 0; y < lvl.height - 1; y++)
    {
      for (int x = 0; x < lvl.width; x++)
      {
//        for (int i = 0; i < lvl.deep - 1; i++, f++)
        for (int i = 0; i < lvl.deep; i++, f++)
        {
          frames[f].type_nr = 0;
          frames[f].x       =  8 + dx * x + 16 * (lvl.deep - 1 - z);
          frames[f].y       = 21 + dy * y + 16 * z + 16 * i;
        }
      }
    }
  }

  // sonstiges  
  certified = true;
}

//******************************************************************************
  void Board::getFieldsize (int& height, int& width) const
//------------------------------------------------------------------------------
{
  CellNr k;

  height = width = 0;

  for (k = 0; k < (CellNr) cells.size(); k++)
  {
    height = max (height, cells[k].y + cell_types[cells[k].type_nr].height);
    width  = max (width,  cells[k].x + cell_types[cells[k].type_nr].width);
  }

  for (k = 0; k < (CellNr) frames.size(); k++)
  {
    height = max (height, frames[k].y + frame_types[frames[k].type_nr].height);
    width  = max (width,  frames[k].x + frame_types[frames[k].type_nr].width);
  }
}

//******************************************************************************
  void Board::getCellsWithPoint (CellNrs& found_cells, int x, int y) const
//------------------------------------------------------------------------------
{
  found_cells.clear();

  for (CellNr k = 0; k < (CellNr) cells.size(); k++)
    if (cells[k].x <= x && x < cells[k].x + cell_types[cells[k].type_nr].width
    &&  cells[k].y <= y && y < cells[k].y + cell_types[cells[k].type_nr].height)
      found_cells.push_back(k);
}

//******************************************************************************
  void Board::delHints (CellNrs& changed_cells)
//------------------------------------------------------------------------------
{
  for_all (CellNrCIter, p, visible_hints)
  {
    if (cells[*p].is_hint)
    {
      cells[*p].is_hint = false;
      changed_cells.push_back (*p);
    }
  }
  visible_hints.clear();
}

//******************************************************************************
  void Board::act (CellNrs& changed_cells, Logbook& logbook)
//------------------------------------------------------------------------------
{
  PERF_ANA ("Board::act");
  
  if (gameIsWon() && num_flagged_cells < num_mines)
  {
    for (CellNr k = 0; k < (CellNr) cells.size(); k++)
      if (cellIsMined(k) && !cellIsFlagged(k))
        flagCellBase (k, changed_cells);
  }

  if (state == BEFORE || gameOver())
    return;

  delHints (changed_cells);

  // ------------------
  if (modus == STARTUP)
  {
    if (startup_stage == 0 || startup_stage > max_stage)
    {
      Board       test_board  = *this; // !!! Kein Copy-Konstruktor definiert
      CellNrs    cc;                  // changed cells

      test_board.eqs_visible = 0;  // werden sonst am Blockende geloescht,
      test_board.eqs_real    = 0;  // da kein Copy-Konstruktor fuer Board existiert
      test_board.murphys_law = false;  // Geschwindigkeit

      while (!test_board.gameOver())
      {
//        unsigned cc_old_size = cc.size(); // changed cells alte Groesse
//        unsigned logbook_old_size = logbook.getNumLogs();

        test_board.solveAllBase (test_board.max_stage, cc, logbook);

        // max_stage runtersetzen, bis ein Zug gefunden wurde (Zeit sparen)
        // Dies soll verhindern, daß in einer Situation, in der mehrere Zellen
        // geoeffnet oder markiert werden muessen, jedesmal eine Komplett-
        // rechnung durchgefuehrt wird.
        // Allerdings bewirkt dies, dass u.U. mehr Hinweise als noetig gegeben
        // werden.
        //
        // cc_old_size == cc.size() => kein neuer Zug gefunden
        // test_board.poss_oot_nr == oot_tab.back() => es ist noch die gleiche
        //                                             Situation
        //
/*
        // ??? Was ist beim Replay ???
        // !!! logbook wird nicht kopiert !!!
        if (cc_old_size == cc.size() && max_stage >= 3
        &&  logbook.getNumLogs() == logbook_old_size)
          test_board.max_stage = 2;
        else
          test_board.max_stage = max_stage;
*/        

        if (test_board.gameOver())
          break;

        // Keine Schlussfolgerung mehr moeglich => Zeit fuer ein Hinweis
        CellNr  hint = test_board.getRandCell();

        if (cells[hint].isMined())
          test_board.move (Move (hint, FLAGGED), cc);
        else
          test_board.move (Move (hint, OPEN), cc);

        visible_hints.push_back (hint);
      }

      startup_stage = max_stage;

      for_all (CellNrCIter, k, visible_hints)
      {
        if (cells[*k].isMined())
          move (Move (*k, FLAGGED), changed_cells);
        else
          move (Move (*k, OPEN), changed_cells);

        cells[*k].is_solved = true;
      }

      if (auto_hints_visible)
        for_all (CellNrCIter, k, visible_hints)
          cells[*k].is_hint = true;
      else
        visible_hints.clear();
    }
  }
  // ---------------------
  else if (modus == HINTS)
  {
    do
    {
      if (auto_stage > 0)
        solveAllBase (auto_stage, changed_cells, logbook);

      // Zug finden
      while (!gameOver() && findOneMove(logbook) == FOUND_NO_MOVE)
      {
        CellNr  hint = poss_move_hints.back();

        if (auto_hints_visible)
        {
          cells[hint].is_hint = true;
          visible_hints.push_back (hint);
        }

        if (cells[hint].isMined())
          move (Move (hint, FLAGGED), changed_cells);
        else
          move (Move (hint, OPEN), changed_cells);

        cells[hint].is_solved = true;

        delete eqs_real;
        eqs_real = 0;
      }
    }
    while (!gameOver() && poss_move_stage <= auto_stage);
  }

  if (modus != HINTS) // aus Optimierungsgründen
  {
    if (auto_stage > 0)
    {
      solveAllBase (auto_stage, changed_cells, logbook);
    }

    findOneMove(logbook);
//    findOneMove (auto_stage + 1); // muesste eigentlich gehen
  }

  if (Glob::trace_level >= 1)
    print();
}

//******************************************************************************
  void Board::reset (Logbook& logbook)
//------------------------------------------------------------------------------
{
  poss_oot_nr = -1;
  clock0      = clock();

  // Variablen setzen
  num_flagged_cells =  0;
  num_open_cells    =  0;
  poss_move_stage   =  4;
  poss_move         = Move (-1, OPEN);
  startup_stage     =  0;

  delete eqs_visible;
  eqs_visible       =  0;

  delete eqs_real;
  eqs_real          =  0;

  // cells
  for (CellNr i = 0; i < (CellNr) cells.size(); i++)
  {
    cells[i].state           = CLEAN;
    cells[i].is_solved       = false;
    cells[i].is_hint         = false;
    cells[i].num_open_adj    = 0;
    cells[i].num_flagged_adj = 0;
  }

  // Minen verteilen
  // ...mines_dist
  vector<CellNr>  mines_distrib (cells.size(), 0);

  fill_n         (mines_distrib.begin(), num_mines, 1);
  random_shuffle (mines_distrib.begin(), mines_distrib.end(), Glob::rand_int);

  // ...cells
  for (CellNr k = 0; k < (CellNr) cells.size(); k++)
  {
    cells[k].is_mined  = (mines_distrib[k] == 1);
    cells[k].num       = 0;
  }

  { // visual studio
  for (CellNr k = 0; k < (CellNr) cells.size(); k++)
    if (cells[k].is_mined)
      for_all_adj (k2, k)
        cells[*k2].num++;
  } // visual studio

  // state
  state = BEFORE;
  if (modus == HINTS || modus == STARTUP)
    state = PLAY;

  // akt.
  CellNrs  changed_cells;
  act (changed_cells, logbook);
}

//******************************************************************************
  Board::Board (Options& opt, Logbook& logbook)
//------------------------------------------------------------------------------
{
  Level  lvl = opt.getLevel();

  if (FileExist (string("./boards/") + opt.getBoardName() + ".bmp")
  &&  FileExist (string("./boards/") + opt.getBoardName() + ".txt"))
  {
    string  fname = string("./boards/") + opt.getBoardName() + ".txt";

    try
    {
      read (fname, lvl);
    }
    catch (BoardReadError err)
    {
      ostringstream  ost;

      ost << "Error in: " << fname;

      if (err.col >= 0 && err.row >= 0)
        ost << "(" << err.row << "," << err.col << ")";

//      Application->MessageBox ((char*) err.text.c_str(),
//                               (char*) ost.str().c_str(),
//                                MB_OK + MB_DEFBUTTON1);
//      SetSquare (lvl);

      throw Exception ((err.text + "\n" + ost.str()).c_str());
    }
  }
  else if (opt.getBoardName() == "Square")
    setSquare (lvl);
  else if (opt.getBoardName() == "Hexagon")
    setHexagon (lvl);
  else if (opt.getBoardName() == "Triangle")
    setTriangle (lvl);
  else if (opt.getBoardName() == "3d-Grid")
    set3dGrid (lvl);

  // Variablen setzen
  num_mines   = lvl.num_mines;
  modus       = opt.getModus();
  auto_stage  = opt.getAutoStage();
  max_stage   = opt.getMaxStage();
  murphys_law = opt.getMurphysLaw();

  trace       = 0;
  print_width = lvl.width;
  eqs_visible = 0;
  eqs_real    = 0;

  auto_hints_visible = false;

  opt.setLevel (lvl);
  reset        (logbook);
}

//******************************************************************************
  void Board::setNumMines (int n_mines, Logbook& logbook)
//------------------------------------------------------------------------------
{
  num_mines = n_mines;
  reset (logbook);
}

//******************************************************************************
  void Board::giveHint (CellNrs& changed_cells)
//------------------------------------------------------------------------------
{
  poss_oot_nr = -1;
  clock0      = clock();

  if (gameOver())
    return;

  delHints (changed_cells);

  for_all (CellNrCIter, p, poss_move_hints)
  {
    cells[*p].is_hint = true;
    visible_hints.push_back (*p);
    changed_cells.push_back (*p);
  }
}

//******************************************************************************
  bool Board::setModus (Modus mod, CellNrs& changed_cells, Logbook& logbook)
//------------------------------------------------------------------------------
{
  poss_oot_nr = -1;
  clock0      = clock();

  if (mod != ORIGINAL && mod != IMMUNE && mod != HINTS && mod != LUCKY
  &&  mod != STARTUP)
    return false;

  if (state == BEFORE
  &&  (mod == HINTS || mod == STARTUP))
    state = PLAY;

  modus = mod;
  act (changed_cells, logbook);
  return true;
}

//******************************************************************************
  bool Board::setAutoStage (StageNr stage, CellNrs& changed_cells,
                            Logbook& logbook)
//------------------------------------------------------------------------------
{
  poss_oot_nr = -1;
  clock0      = clock();

  if (stage < 0 || 3 < stage || stage == auto_stage)
    return false;

  if (stage <= auto_stage)
  {
    auto_stage = stage; // kein act() noetig
  }
  else
  {
    auto_stage = stage;
    act (changed_cells, logbook);
  }

  return true;
}

//******************************************************************************
  bool Board::setMaxStage (StageNr stage, CellNrs& changed_cells,
                           Logbook& logbook)
//------------------------------------------------------------------------------
{
  poss_oot_nr = -1;
  clock0      = clock();

  if (stage < 1 || 3 < stage || stage == max_stage)
    return false;

  if (stage < max_stage || getDeadend())
  {
    max_stage = stage;
    act (changed_cells, logbook); // bei Deadend wird ggf. ein Auto-Solve zuviel
  }                               // berechnet (but who cares)
  else
  {
    max_stage = stage;
  }

  return true;
}

//******************************************************************************
  void Board::openCellBase (CellNr k, CellNrs& changed_cells)
//------------------------------------------------------------------------------
{
  if (cellIsOpen(k) || cellIsFlagged(k))
  {
    return;
  }

  if (cellIsMined(k))
  {
    cells[k].state = OPEN;
    state = LOST;
    return;
  }

  // num_vars_left
  for_all_adj (q, k)
    cells[*q].num_open_adj++;

  // Stati
  cells[k].state = OPEN;
  num_open_cells++;
  changed_cells.push_back (k);

  delete eqs_visible;
  delete eqs_real;
  eqs_visible = 0;
  eqs_real    = 0;

  // Ende?
  if ((int) cells.size() == num_mines + num_open_cells)
  {
    state = WON;
    return;
  }

  // rekursiv weiter
  if (cells[k].num == 0)
    for_all_adj (i, k)
      openCellBase (*i, changed_cells);
}

//******************************************************************************
  bool Board::openCells (CellNr k, CellNrs& changed_cells, Logbook& logbook)
//------------------------------------------------------------------------------
{
  poss_oot_nr = -1;
  clock0      = clock();

  if (gameOver() || cellIsFlagged(k))
    return false;

  delHints (changed_cells);

  if (cellIsOpen(k))
  {
    // alle Nachbarzellen oeffnen
    int  flg = cells[k].num_flagged_adj;
    int  cln = cells[k].adj.size() - cells[k].num_open_adj - flg;
    bool has_toggled = false;

    if (cln == 0
    ||  flg != cells[k].num)
      return false;

    for_all_adj (k2, k)
      if (cells[*k2].isClean())
      {
        if (murphysLawIsValid() && !cellIsMined(*k2)
        &&  poss_move_stage > 0 && state == PLAY)
        {
          toggleMineIfPoss (*k2, logbook);
          has_toggled = true;
        }

        openCellBase (*k2, changed_cells);
      }

    if (has_toggled)
      chkMines();
  }
  else // Zelle war geschlossen
  {
    if (state == BEFORE)
    {
      if (cellIsMined (k))
      {
        toggleMineIfPoss (k, logbook); // remove mine
        chkMines();
      }
      state = PLAY; // muss nach toggleMine erfolgen
    }
    else if (modus == LUCKY && cellIsMined(k) && getDeadend())
    {
      toggleMineIfPoss (k, logbook);  // remove mine
      chkMines();
    }
    else if (murphysLawIsValid() && !cellIsMined(k)
         &&  poss_move_stage > 0 && state == PLAY)
    {
      toggleMineIfPoss (k, logbook);
      chkMines();
    }

    if (modus == IMMUNE && cellIsMined(k) && getDeadend())
    {
      flagCellBase (k, changed_cells);

      cells[k].is_solved = true;

      delete eqs_real;
      eqs_real = 0;
    }
    else // Normalfall
      openCellBase (k, changed_cells);
  }

  act (changed_cells, logbook);

  return true;
}

//******************************************************************************
  void Board::flagCellBase (CellNr k, CellNrs& changed_cells)
//------------------------------------------------------------------------------
{
  if (cellIsOpen(k))
    return;

  if (cellIsFlagged(k))
  {
    // unflag
    num_flagged_cells--;
    cells[k].state = CLEAN;

    for_all_adj (q, k)
      cells[*q].num_flagged_adj--;
  }
  else
  {
    // flag
    num_flagged_cells++;
    cells[k].state = FLAGGED;

    for_all_adj (q, k)
      cells[*q].num_flagged_adj++;
  }

  // Stati
  delete eqs_visible;
  eqs_visible = 0;

  // changed_cells
  changed_cells.push_back(k);
}

//******************************************************************************
  bool Board::flagCells (CellNr k, CellNrs& changed_cells, Logbook& logbook)
//------------------------------------------------------------------------------
{
  poss_oot_nr = -1;
  clock0      = clock();

  if (gameOver())
    return false;

  delHints (changed_cells);

  if (cellIsOpen (k))
  {
    int flg = cells[k].num_flagged_adj;
    int cln = cells[k].adj.size() - cells[k].num_open_adj - flg;

    if (cln == 0 || flg + cln != cells[k].num)
      return false;

    for_all_adj (k2, k)
      if (cells[*k2].isClean())
        flagCellBase (*k2, changed_cells);
  }
  else
  {
    flagCellBase (k, changed_cells);
  }

  act (changed_cells, logbook);

  return true;
}

//******************************************************************************
  bool Board::move (Move the_move, CellNrs& changed_cells)
//------------------------------------------------------------------------------
{
  CellNr     k         = the_move.cell_nr;
  CellState  new_state = the_move.new_state;

  if (cellIsOpen (k)
  ||  new_state == FLAGGED &&  cellIsFlagged (k)
  ||  new_state == CLEAN   && !cellIsFlagged (k))
    return false;

  if (new_state == FLAGGED || new_state == CLEAN)
    flagCellBase (k, changed_cells);
  else if (new_state == OPEN)
    openCellBase (k, changed_cells);
  else
    return false;

  return true;
}

//******************************************************************************
  int Board::calcMaxTime (Logbook& logbook)
//------------------------------------------------------------------------------
{
  // not playing
  if (!logbook.isPlaying())
  {
    int  used_time = (clock() - clock0) * 1000 / CLK_TCK;

    if (used_time >= max_time_multi)
      return 0; // Zeit abgelaufen

//    return max ((int) 0, (max_time_multi - used_time) / 10);

    return max_time_multi / 10;
  }
  // playing
  else if (!logbook.eop()
  &&       logbook.getPlayLog().name == LOG_OUT_OF_TIME
  &&       logbook.getPlayLog().val == poss_oot_nr)
  {
    return 0; // Exception erzwingen
  }
  else
  {
    return -1; // Exception vermeiden
  }
}

//******************************************************************************
  Found Board::findOneMove (Logbook& logbook)
//------------------------------------------------------------------------------
{
  if (gameOver())
    return FOUND_NO_MOVE;

  poss_move_hints.clear();

  // eqs_visible aufbauen?
  if (eqs_visible == 0)
    eqs_visible = new Eqs (cells, &Cell::isVisibleVar);

  Moves  moves;

  // Stage 1
  Found  found = eqs_visible->findMoves1 (moves, &poss_move_hints);

  if (found == FOUND_MOVE || found == FOUND_CONTRA)
  {
    poss_move_stage = 1;
  }
  else if (2 <= max_stage)
  {
    // Stage 2
    found = eqs_visible->findMoves2 (moves, &poss_move_hints);

    if (found == FOUND_MOVE || found == FOUND_CONTRA)
    {
      poss_move_stage = 2;
    }
    else if (3 <= max_stage)
    {
      // Stage 3
      StageNr  stage = 1;  // irgendein Wert <= 3
      found           = findMovesInSolvedCells (moves);

      while (found == FOUND_NO_MOVE && stage <= 3)
      {
        // eqs_real aufbauen?
        if (eqs_real == 0)
          eqs_real = new Eqs (cells, &Cell::isRealVar);

        // findMoves
        for (stage = eqs_real->getSolveStage() + 1; stage <= 3; stage++)
        {
          Moves  moves_tmp;

          if      (stage == 1)  eqs_real->findMoves1 (moves_tmp);
          else if (stage == 2)  eqs_real->findMoves2 (moves_tmp);
          else
          {
            try
            {
              ++poss_oot_nr;
              eqs_real->findMoves3 (moves_tmp, calcMaxTime(logbook));
            }
            catch (OutOfTime)
            {
              logbook << Log (LOG_OUT_OF_TIME, poss_oot_nr);

              moves_tmp.clear();

              delete eqs_real;
              eqs_real = 0;
            }
          }

          for_all (MovesCIter, m, moves_tmp)
          {
            cells[(*m).cell_nr].is_solved = true;

            if (!cells[(*m).cell_nr].isFlagged() || (*m).new_state != FLAGGED)
              moves.push_back (*m);
          }

          if (moves_tmp.size() > 0)
          {
            delete eqs_real;
            eqs_real = 0;
          }

          if (moves.size() > 0)
          {
            found = FOUND_MOVE; // while-Schleife verlassen
            break;
          }
          else if (moves_tmp.size() > 0)
          {
            break;            // while-Schleife nicht verlassen
          }
        }
      } // while

      if (found == FOUND_MOVE)
        poss_move_stage = 3;
    }
  }

  // Widerspruch gefunden?
  if (found == FOUND_CONTRA)
  {
//  if (stage == 3)
//  {
//    set<CellNr>  new_hints;

//    for_all (CellNrCIter, k, poss_move_hints)
//      for_all_adj (k2, *k)
//        if (cells[*k2].isFlagged())
//          new_hints.insert (*k2);

//     poss_move_hints.clear();
//     copy (new_hints.begin(), new_hints.end(), poss_move_hints.begin());
//  }

    poss_move.cell_nr = -1;

    return FOUND_CONTRA; // obwohl nur ein Widerspruch gefunden wurde
  }
  // Zug gefunden?
  else if (found == FOUND_MOVE)
  {
    poss_move = moves.front();
    state     = PLAY;

    if (poss_move_stage == 3)
      for_all (MovesCIter, m, moves)
        poss_move_hints.push_back ((*m).cell_nr);

    return FOUND_MOVE;
  }

  // kein Zug gefunden!
  poss_move.cell_nr = -1;

  if (modus == ORIGINAL)
    poss_move_stage = 4;
  else
    poss_move_stage = 0;

  state = PLAY;

  CellNr hint = getRandCell();
  poss_move_hints.push_back (hint);

  return FOUND_NO_MOVE;
}

//******************************************************************************
  CellNr Board::getRandCell (void)
//------------------------------------------------------------------------------
{
  int     r = Glob::rand_int();
  CellNr  p = -1;
  int     v;

  if (num_open_cells == 0)
  {
    v = 0;
    for (CellNr k = 0; k < (CellNr) cells.size(); k++)
      if (cells[k].isClean()
      && !cells[k].isMined()
      &&  cells[k].num == 0
      &&  r % ++v == 0)
        p = k;

    if (p != -1)
      return p;
  }

  // Var. Zelle?
  v = 0;
  for (CellNr k = 0; k < (CellNr) cells.size(); k++)
    if (cells[k].isClean() && cells[k].num_open_adj > 0
    &&  r % ++v == 0)
      p = k;

  if (p != -1)
    return p;

  // nicht vermint?  (da 'innere Zelle)
  { // visual studio
  v = 0;
  for (CellNr k = 0; k < (CellNr) cells.size(); k++)
    if (!cells[k].isOpen() && !cells[k].isMined()
    &&  r % ++v == 0)
      p = k;
  } // visual studio

  return p;
}

//******************************************************************************
  void Board::solveOne (CellNrs& changed_cells, Logbook& logbook)
//------------------------------------------------------------------------------
{
  poss_oot_nr = -1;
  clock0      = clock();
  
  if (gameOver())
    return;

  // Nichts zu loesen?
  if (poss_move_stage <= 0)
//  if (poss_move_stage < 1 || max_stage < poss_move_stage) // fuer Original-Modus
  {
    delHints (changed_cells);
  }
  // Widerspruch?
  else if (poss_move.cell_nr == -1)
  {
    giveHint (changed_cells);
  }
  else // Zug moeglich
  {
    CellNr  solved_cell_nr = poss_move.cell_nr;
    move (poss_move, changed_cells);

    act (changed_cells, logbook);

    cells[solved_cell_nr].is_hint = true;
    visible_hints.push_back (solved_cell_nr);
  }
}

//******************************************************************************
  Found Board::findMovesInSolvedCells (Moves& moves)
//------------------------------------------------------------------------------
{
   for (CellNr k = 0; k < (CellNr) cells.size(); k++)
     if (cells[k].isSolved())
       if (cells[k].isMined() && !cells[k].isFlagged())
         moves.push_back (Move (k, FLAGGED));
       else if (!cells[k].isMined() && !cells[k].isOpen())
         moves.push_back (Move (k, OPEN));

   return  (moves.size() > 0) ? FOUND_MOVE : FOUND_NO_MOVE;
}

//******************************************************************************
  void Board::solveAllBase (int max_s, CellNrs& changed_cells, Logbook& logbook)
//------------------------------------------------------------------------------
// Loest das Board bis zum Stage max_s.
// Alle dabei geaenderten Zellen werden in changed_cells festgehalten
{
  Moves  moves;

  if (gameOver() || max_s <= 0)
    return;

  state = PLAY;

  // loesen
  while (true)
  {
    // eqs_visible aufbauen?
    if (eqs_visible == 0)
      eqs_visible = new Eqs (cells, &Cell::isVisibleVar);

    // Stage 1
    Found  found = eqs_visible->findMoves1 (moves);

    // Stage 2
    if (found == FOUND_NO_MOVE && 2 <= max_s)
      found = eqs_visible->findMoves2 (moves);

    // Stage 3
    if (found == FOUND_NO_MOVE && 3 <= max_s)
    {
      found           = findMovesInSolvedCells (moves);
      StageNr  stage = 1; // irgendein Wert <= 3

      while (found == FOUND_NO_MOVE && stage <= 3)
      {
        // eqs_real aufbauen?
        if (eqs_real == 0)
          eqs_real = new Eqs (cells, &Cell::isRealVar);

        // findMoves
        for (stage = eqs_real->getSolveStage() + 1; stage <= 3; stage++)
        {
          // moves_tmp
          Moves  moves_tmp;

          if      (stage == 1)  eqs_real->findMoves1 (moves_tmp);
          else if (stage == 2)  eqs_real->findMoves2 (moves_tmp);
          else
          {
            try
            {
              ++poss_oot_nr;
              eqs_real->findMoves3 (moves_tmp, calcMaxTime(logbook));
            }
            catch (OutOfTime)
            {
              logbook << Log (LOG_OUT_OF_TIME, poss_oot_nr);

              moves_tmp.clear();

              delete eqs_real;
              eqs_real = 0;
            }
          }

          // moves, cell->is_solved
          for_all (MovesCIter, m, moves_tmp)
          {
            cells[(*m).cell_nr].is_solved = true;

            if (!cells[(*m).cell_nr].isFlagged() || (*m).new_state != FLAGGED)
              moves.push_back (*m);
          }

          if (moves_tmp.size() > 0)
          {
            delete eqs_real;
            eqs_real = 0;
          }

          // auswerten
          if (moves.size() > 0)
          {
            found = FOUND_MOVE; // while-Schleife verlassen
            break;
          }
          else if (moves_tmp.size() > 0)
          {
            break;             // while-Schleife nicht verlassen
          }
        }
      } // while
    }

    // Kein Zug? -> fertig!
    if (found == FOUND_NO_MOVE)
      break;

    // Widerspruch gefunden? -> moves umarbeiten
    if (found == FOUND_CONTRA)
    {
      // moves beinhalten nur die Zellnummer der Konstanten,
      // da geflaggte Zellen keine Variablen sind
      set<CellNr>  hints;

      for_all (MovesCIter, m, moves)
        for_all_adj (k, (*m).cell_nr)
          if (cells[*k].isFlagged())
            hints.insert (*k);

      moves.clear();
      for_all (set<CellNr>::const_iterator, k, hints)
        moves.push_back (Move (*k, CLEAN));
    }

    // moves  auswerten
    for_all (MovesCIter, m, moves)
    {
      // Ausfuehren
      move (*m, changed_cells);

      if (gameOver()) // !!! neu
        return;
    }
  }
}

//******************************************************************************
  void Board::solveAll (int max_s, CellNrs& changed_cells, Logbook& logbook)
//------------------------------------------------------------------------------
{
  poss_oot_nr = -1;
  clock0      = clock();

  if (gameOver())
  {
    delHints (changed_cells);
    return;
  }

  solveAllBase (max_s, changed_cells, logbook);

  act (changed_cells, logbook);
}

//******************************************************************************
  bool Board::toggleMineIfPoss (CellNr a, Logbook& logbook)
//------------------------------------------------------------------------------
{
  if (gameOver() || cells[a].isSolved())
    return false;

  if (state == BEFORE)
  {
    if (!cellIsMined (a))
      return false;

    CellNr  b = -1; // Zelle, in die die Mine wandert oder von wo sie kommt
    int     r = Glob::rand_int();
    int     v = 0;

    for (CellNr k = 0; k < (CellNr) cells.size(); k++)
      if (!cellIsMined(k) && r % ++v == 0)
        b = k;

    if (b == -1)
      return false;

    // verschiebe Mine
    { // visual studio
      cells[a].is_mined = false;
      for_all_adj (k, a)
        cells[*k].num--;
    } // visual studio

    { // visual studio
      cells[b].is_mined = true;
      for_all_adj (k, b)
        cells[*k].num++;
    } // visual studio

    return true;
  }

  //
  vector<CellNr>  to_toggle;
  bool             found_one_sol = false;
  StageNr         stage = 1; // irgendein Wert <= 3

  to_toggle.push_back (a);

  // Zuege suchen
  while (stage <= 3)
  {
    // eqs_real aufbauen?
    if (eqs_real == 0)
      eqs_real = new Eqs (cells, &Cell::isRealVar);

    for (stage = min (3, eqs_real->getSolveStage() + 1); stage <= 3; stage++)
    {
      Moves  moves;

      if (stage == 1)
      {
        ASSERT (eqs_real->findMoves1 (moves) != FOUND_CONTRA);
      }
      else if (stage == 2)
      {
        ASSERT (eqs_real->findMoves2 (moves) != FOUND_CONTRA);
      }
      else if (stage == 3)
      {
        try
        {
          ++poss_oot_nr;
          found_one_sol = eqs_real->findOneSolution (cells, to_toggle, moves,
                                                     calcMaxTime(logbook));
        }
        catch (OutOfTime)
        {
          logbook << Log (LOG_OUT_OF_TIME, poss_oot_nr);

          moves.clear();
          found_one_sol = false;

          delete eqs_real;
          eqs_real = 0;
        }
      }

      for_all (MovesCIter, m, moves)
        cells[(*m).cell_nr].is_solved = true;

      // Kein Togglen moeglich?
      if (cells[a].isSolved())
      {
        delete eqs_real;
        eqs_real = 0;
        
        return false;
      }

      // ev. wieder bei stage 1 anfangen
      if (moves.size() > 0 && stage <= 2)
      {
        delete eqs_real;
        eqs_real = 0;
        
        break;
      }
    }
  }

  // Umverteilung suchen
  if (found_one_sol)
  {
    // Minen umverteilen
    for_all (CellNrCIter, k, to_toggle)
    {
      if (cells[*k].isMined())
      {
        cells[*k].is_mined = false;

        for_all_adj (k2, *k)
          cells[*k2].num--;
      }
      else
      {
        cells[*k].is_mined = true;

        for_all_adj (k2, *k)
          cells[*k2].num++;
      }
    }

    return true;
  }

  return false;
}

//******************************************************************************
  void Board::chkMines (void)
//------------------------------------------------------------------------------
{
  // Umverteilung testen
  for (CellNr k = 0; k < (CellNr) cells.size(); k++)
  {
    if (cells[k].isMined())
        continue;

    // num_mines_adj
    int num_mines_adj = 0;
    for_all_adj (k2, k)
      if (cells[*k2].isMined())
        num_mines_adj++;

    ASSERT (num_mines_adj == cells[k].num);
  }
}

//******************************************************************************
  void Board::print (void) const
//------------------------------------------------------------------------------
{
  ofstream      out ("board.txt");

  for (CellNr k = 0; k < (CellNr) cells.size(); k++)
  {
    char  ch;

    if (cells[k].isOpen())
      if (cells[k].num == 0)
        ch = '-';
      else
        ch = (char) ('0' + cells[k].num);
    else if (cells[k].isFlagged())
      ch = '!';
    else if (cells[k].isMined())
      ch = '*';
    else
      ch = '#';

    if (cells[k].isSolved())
      out << ch << '\'';
    else
      out << ch << ' ';

    if (k % print_width == print_width - 1)
      out << '\n';
  }

  if (poss_move.cell_nr != -1)
    out << "\nposs_moves = { " << poss_move.cell_nr << ", "
                               << poss_move.new_state << "}" << "\n";
}

