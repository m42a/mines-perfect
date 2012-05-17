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

#include <stdio.h>

#include "boardctrl.h"
#include "gamectrl.h"

using namespace MinesPerfect;


const CellNr  INVALID_CELL = -1;

Bitmap*   BoardCtrl::s_board_square_bmp   = 0;
Bitmap*   BoardCtrl::s_board_triangle_bmp = 0;
Bitmap*   BoardCtrl::s_board_hexagon_bmp  = 0;
Bitmap*   BoardCtrl::s_board_grid3d_bmp   = 0;

Bitmap*   BoardCtrl::s_symb_flag_bmp  = 0;
Bitmap*   BoardCtrl::s_symb_quest_bmp = 0;
Bitmap*   BoardCtrl::s_symb_mine_bmp  = 0;
Bitmap*   BoardCtrl::s_symb_cross_bmp = 0;
Bitmap*   BoardCtrl::s_symb_zero_bmp  = 0;
Bitmap*   BoardCtrl::s_symb_digit_bmps[28] = { 0 };

//******************************************************************************
void BoardCtrl::initStatic()
//------------------------------------------------------------------------------
{
  // Board-Bitmaps
  s_board_square_bmp   = CreateBitmap ("board", "Square"  );
  s_board_triangle_bmp = CreateBitmap ("board", "Triangle");
  s_board_hexagon_bmp  = CreateBitmap ("board", "Hexagon" );
  s_board_grid3d_bmp   = CreateBitmap ("board", "3d-Grid" );
  
  // Symb-Bitmaps
  s_symb_flag_bmp  = CreateBitmap ("symbol", "flag" );
  s_symb_quest_bmp = CreateBitmap ("symbol", "quest");
  s_symb_mine_bmp  = CreateBitmap ("symbol", "mine" );
  s_symb_cross_bmp = CreateBitmap ("symbol", "cross");

  char      buf[20];
  unsigned  i; // visual studio

  for (i = 0; i < sizeof (s_symb_digit_bmps) / sizeof (*s_symb_digit_bmps); i++)
  {
     sprintf (buf, "%i", i);
     s_symb_digit_bmps[i] = CreateBitmap ("symbol", buf);
  }
  
  // Symb-Bitmaps: check size
  ASSERT (s_symb_flag_bmp->getSize() == s_symb_quest_bmp->getSize());
  ASSERT (s_symb_flag_bmp->getSize() == s_symb_mine_bmp->getSize());
  ASSERT (s_symb_flag_bmp->getSize() == s_symb_cross_bmp->getSize());

  for (i = 0; i < sizeof (s_symb_digit_bmps) / sizeof (*s_symb_digit_bmps); i++)
    ASSERT (s_symb_flag_bmp->getSize() == s_symb_digit_bmps[i]->getSize());
}

//******************************************************************************
BoardCtrl::BoardCtrl (Ctrl* parent, GameCtrl* game) : Ctrl(parent), m_game(game), m_source_bmp(0)
//------------------------------------------------------------------------------
{
  // ev. initialisieren
  if (s_board_square_bmp == 0)
    initStatic();
}

//******************************************************************************
BoardCtrl::~BoardCtrl()
//------------------------------------------------------------------------------
{
  delete m_source_bmp;
}

//******************************************************************************
void BoardCtrl::setLevel (const Level& lvl) // oder so aehnlich
//------------------------------------------------------------------------------
{
  delAllChildren();

  Board*  board = m_game->getBoard();

  //  frames
  m_frame_bmps.clear();  
  for (const FrameType* ft = board->getFirstFrameType(); ft != 0;
                        ft = board->getNextFrameType())
  {
    Rect  rect (ft->x, ft->y, ft->width, ft->height);

    m_frame_bmps.push_back (CreateBitmap (m_source_bmp, rect));
  }

  m_frame_ctrls.clear();
  for (const Frame* fr = board->getFirstFrame(); fr != 0;
                    fr = board->getNextFrame())
  {
    BitmapCtrl*  bc = new BitmapCtrl(this);
    m_frame_ctrls.push_back(bc);
    
    bc->setRelPos (Point (fr->x, fr->y));
    bc->setBitmap (m_frame_bmps[fr->type_nr]);
  }
  
  // cells
  m_cell_close_bmps.clear();
  m_cell_open_bmps.clear();
  m_cell_hintclose_bmps.clear();
  m_cell_hintopen_bmps.clear();
  m_cell_error_bmps.clear();
  for (const CellType* ct = board->getFirstCellType(); ct != 0;
                       ct = board->getNextCellType())
  {
    Point pos   (ct->x, ct->y);
    Point delta (ct->width + 1, 0);
    Rect  rect  (ct->x, ct->y, ct->width, ct->height);

    rect.setPos (pos + 0 * delta);
    m_cell_close_bmps.push_back (CreateBitmap (m_source_bmp, rect));

    rect.setPos (pos + 1 * delta);
    m_cell_open_bmps.push_back (CreateBitmap (m_source_bmp, rect));

    rect.setPos (pos + 2 * delta);
    m_cell_hintclose_bmps.push_back (CreateBitmap (m_source_bmp, rect));

    rect.setPos (pos + 3 * delta);
    m_cell_hintopen_bmps.push_back (CreateBitmap (m_source_bmp, rect));

    rect.setPos (pos + 4 * delta);
    m_cell_error_bmps.push_back (CreateBitmap (m_source_bmp, rect));
  }
  
  m_cell_ctrls.clear();
  m_symbol_ctrls.clear();
  for (CellNr k = 0; k < (CellNr) m_game->m_board->getNumCells(); k++)
  {
    // cell_ctrl:
    BitmapCtrl*  cc = new BitmapCtrl(this);
    m_cell_ctrls.push_back(cc);

    Point cp (board->getCellX(k), board->getCellY(k));
    cc->setRelPos(cp);
    
    // symbol_ctrl    
    BitmapCtrl*  sc = new BitmapCtrl(cc);
    m_symbol_ctrls.push_back(sc);
    
    const CellType*  ct = board->getTypeOfCell(k);
    Point            sp (ct->centre_x - s_symb_mine_bmp->getSize().x / 2,
                         ct->centre_y - s_symb_mine_bmp->getSize().y / 2);
    sc->setRelPos(sp);
  }
  
  m_changed_cells.clear();
}

//******************************************************************************
void BoardCtrl::setBoard (BoardNr nr)// setSourceBmp
//------------------------------------------------------------------------------
{
  // m_source_bmp
  delete m_source_bmp;
  m_source_bmp = CreateBitmap ("board", m_game->m_options->getBoardName (nr));

  setLevel(m_game->m_options->getLevel());
}

//******************************************************************************
void BoardCtrl::actCell (CellNr k)
//------------------------------------------------------------------------------
// Diese Funktion aktualisiert die Zelle und das dazugehoerige Symbol
{
  Bitmap*  cell_bmp = 0;
  Bitmap*  symb_bmp = 0;
  Board*   board = m_game->getBoard();
  int      ct_nr = board->getCellTypeNr(k);

  // act. cell
  if (board->cellIsOpen(k))
  {
    if (board->cellIsMined(k) && board->gameOver())
      cell_bmp = m_cell_error_bmps[ct_nr];
    else if (board->cellIsHint(k))
      cell_bmp = m_cell_hintopen_bmps[ct_nr];
    else
      cell_bmp = m_cell_open_bmps[ct_nr];
  }
  else // cell is closed
  {
    if (board->cellIsHint(k))
      cell_bmp = m_cell_hintclose_bmps[ct_nr];
    else
      cell_bmp = m_cell_close_bmps[ct_nr];
  }
  
  // act. symb
  if (m_game->m_options->getShowMines())
  {
    if (board->cellIsFlagged(k) && !board->cellIsMined(k))
      symb_bmp = s_symb_cross_bmp;
    else if (board->cellIsFlagged(k))
      symb_bmp = s_symb_flag_bmp;
    else if (board->cellIsMined(k))
      symb_bmp = s_symb_mine_bmp;
    else if (board->cellIsClean (k))
      symb_bmp = 0;
    else // CellIsOpen
      symb_bmp = s_symb_digit_bmps[board->getCellNum(k)];
  }
  else
  {
    if (board->cellIsFlagged(k)
    ||  board->gameIsWon() && board->cellIsMined(k))
      symb_bmp = s_symb_flag_bmp;
    else if (board->cellIsClean (k))
      symb_bmp = 0;
    else if (board->cellIsMined(k))
      symb_bmp = s_symb_mine_bmp;
    else // CellIsOpen
    {
      symb_bmp = s_symb_digit_bmps[board->getCellNum(k)];
    }
  }

  // Spiel verloren?
  if (board->gameIsLost())
  {
    // mined, covered + unflagged => mine
    if (board->cellIsMined(k) && board->cellIsClean(k))
    {
      cell_bmp = m_cell_open_bmps[ct_nr];
      symb_bmp = s_symb_mine_bmp;
    }
    // mined + open => error
    else if (board->cellIsMined(k) && board->cellIsOpen(k))
    {
      cell_bmp = m_cell_error_bmps[ct_nr];
      symb_bmp = s_symb_mine_bmp;
    }
    // flagged + not mined => X
    else if (!board->cellIsMined(k) && board->cellIsFlagged(k))
    {
      cell_bmp = m_cell_open_bmps[ct_nr];
      symb_bmp = s_symb_cross_bmp;
    }
  }

  // Falls neue Symbol leer ist oder altes Symbol uebermalt werden soll, 
  // muss Zelle neu gemalt werden, da Sybole transparent sind.
  // (sollte nicht verallgemeinert werden, also nicht board_ctrl neu malen,
  //  falls sich die Zelle aendert)
  if (m_symbol_ctrls[k]->getBitmap() != 0
  &&  (symb_bmp == 0 || symb_bmp != m_symbol_ctrls[k]->getBitmap()))
    m_cell_ctrls[k]->setDirty();

  m_cell_ctrls[k]->setBitmap(cell_bmp);
  m_symbol_ctrls[k]->setBitmap(symb_bmp);
}

//******************************************************************************
void BoardCtrl::actAllCells()
//------------------------------------------------------------------------------
{
  for (CellNr k = 0; k < (CellNr) m_cell_ctrls.size(); k++)
//    m_cell_ctrls[k]->setDirty();
    actCell(k);

  m_changed_cells.clear();
}

//******************************************************************************
void BoardCtrl::actChangedCells()
//------------------------------------------------------------------------------
{
  if (m_game->m_options->getShowMines())
  {
    actAllCells();
  }
  else
  {
    for (CellNrIter p = m_changed_cells.begin(); p != m_changed_cells.end(); p++)
//      m_cell_ctrls[*p]->setDirty();
      actCell(*p);

    m_changed_cells.clear();
  }
}

//******************************************************************************
CellNr BoardCtrl::getCellAtPoint (const Point& p) const
//------------------------------------------------------------------------------
// p: in BoardCtrl Koordinaten

// sollte mit CellCtrl arbeiten !!!
{
  CellNrs  candidates;
  Board*   board = m_game->getBoard();

  board->getCellsWithPoint (candidates, p.x, p.y);

  for (CellNr i = 0; i < (CellNr) candidates.size(); i++)
  {
    CellNr  k     = candidates[i];
    int     ct_nr = board->getCellTypeNr(k);
    Point   cell_pos;

    cell_pos.x = p.x - board->getCellX(k);
    cell_pos.y = p.y - board->getCellY(k);

    if (!m_cell_close_bmps[ct_nr]->pointIsTransparent(cell_pos))
      return k;
  }

  return INVALID_CELL;
}

//******************************************************************************
void BoardCtrl::actCurCells (bool left_is_down, bool right_is_down)
//------------------------------------------------------------------------------
// aktualisiert 'm_cur_cells'
//
// In:  m_cur_cell, m_cur_cells
// Out: m_cur_cells
{
  Board*  board = m_game->getBoard();

  // m_cur_cells freigeben  
  set<CellNr>::const_iterator i; // visual studio
  for (i = m_cur_cells.begin(); i != m_cur_cells.end(); ++i)
  {
    actCell(*i);
  }

  // m_cur_cell -> m_cur_cells
  m_cur_cells.clear();

  if (!board->gameOver() && left_is_down
  &&  0 <= m_cur_cell && m_cur_cell < board->getNumCells())
  {
    if (board->cellIsOpen (m_cur_cell) || right_is_down)
    {
      const CellNrs&  adj = board->getAdjacentCells (m_cur_cell);

      for (unsigned j = 0; j < adj.size(); j++)
        if (board->cellIsClean(adj[j]))
          m_cur_cells.insert (adj[j]);
    }
    else
    {
      if (board->cellIsClean(m_cur_cell))
        m_cur_cells.insert(m_cur_cell);
    }
  }
  
  // neue m_cur_cells setzen  
  for (i = m_cur_cells.begin(); i != m_cur_cells.end(); ++i)
  {
    Bitmap* bmp = m_cell_open_bmps[board->getCellTypeNr(*i)];
    m_cell_ctrls[*i]->setBitmap(bmp);
  }
}

//******************************************************************************
void BoardCtrl::setCellsDirty()
//------------------------------------------------------------------------------
{
  for (unsigned i = 0; i < m_cell_ctrls.size(); ++i)
    m_cell_ctrls[i]->setDirty();
}

//******************************************************************************
void BoardCtrl::onMouseEvent (const MouseEvent& ev)
//------------------------------------------------------------------------------
// p: in BoardCtrl Koordinaten
{
  Board*  board = m_game->getBoard();

  if (board->gameOver())
    return;

  if (ev.m_type == MouseEvent::MOVE
  ||  ev.m_left_is_down && ev.m_right_is_down)
  {
    if (!ev.m_left_is_down)
      return;

    m_cur_cell = getCellAtPoint(ev.m_pos - getAbsPos());
    actCurCells(ev.m_left_is_down, ev.m_right_is_down);
    
    return;
  }

  m_cur_cell = getCellAtPoint (ev.m_pos - getAbsPos());

  if (ev.m_type == MouseEvent::LEFT_DOWN || ev.m_type == MouseEvent::RIGHT_DOWN)
  {
    board->delHints (m_changed_cells);

    if (m_cur_cell != INVALID_CELL)
    {
      if (ev.m_type == MouseEvent::LEFT_DOWN)
      {
        actCurCells(ev.m_left_is_down, ev.m_right_is_down);
      }
      else
      {
        *m_game->m_logbook << Log (LOG_FLAG, m_cur_cell);

        if (!board->flagCells (m_cur_cell, m_changed_cells, *m_game->m_logbook))
        {
          m_game->m_error_sound->play();
          m_game->m_logbook->invalidateLast();
        }
        else if ((board->getCurStage() == 0 || board->getCurStage() == 4)
        &&       m_game->m_deadend_sound->isOk())
        {
          m_game->m_deadend_sound->play();
        }
        else
        {
          m_game->m_flag_sound->play();
        }
      }
    }
  }
  else // mousebutton up
  {
    // actCurCells(ev.m_left_is_down, ev.m_right_is_down);

    if (ev.m_type == MouseEvent::LEFT_UP && m_cur_cell != INVALID_CELL
    &&  !ev.m_right_is_down)
    {
      bool was_in_play_mode = (board->getState() == PLAY);

      *m_game->m_logbook << Log (LOG_OPEN, m_cur_cell);

      if (board->openCells (m_cur_cell, m_changed_cells, *m_game->m_logbook))
      {
        if ((board->getCurStage() == 0 || board->getCurStage() == 4)
        &&  m_game->m_deadend_sound->isOk())
        {
          m_game->m_deadend_sound->play();
        }
        else
        {
          m_game->m_open_sound->play();
        }

        if (was_in_play_mode)
          m_game->setOpenAfterStart();
      }
      else // not open
      {
        m_game->m_error_sound->play();
        m_game->m_logbook->invalidateLast();
      }
    }

    // Damit die Schnittmenge von m_cur_cells und m_changed_cells nicht doppelt
    // gezeichnet werden und damit ein haessliches Flackern verursachen
    // werden beide Mengen vereinigt und in m_changed_cells gepackt.
    sort (m_changed_cells.begin(), m_changed_cells.end());
    
    CellNrs diff;
    
    sort (m_changed_cells.begin(), m_changed_cells.end());
    set_difference (m_cur_cells.begin(),     m_cur_cells.end(), 
                    m_changed_cells.begin(), m_changed_cells.end(), 
                    back_inserter(diff));
    copy (diff.begin(), diff.end(), back_inserter(m_changed_cells));                
      
    m_cur_cells.clear();
  }
}


