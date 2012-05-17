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

#include <list>
#include <set>
#include <algorithm>
#include <functional>
#include <iterator>

using namespace std;

#include "../core/utils.h"
#include "gamectrl.h"
#include "stdlib.h"
#include "boardctrl.h"


// _VARIANT_
// 0: Borland 3, Windows (V. 100-121)
// 1: Borland 5, Windows
// 2: Dev-Cpp, GCC 2.95, WX 2.29, Windows (V. 130)
// 3: VisualC++ 6.0, WX 2.40, Windows (V. 140)

// Konstanten zur Fehlersuche
namespace MinesPerfect
{
  namespace Glob
  {
    // nicht als const-deklariert, da es ansonsten Compilerwarnungen gibt
    int  VERSION = 140;
    int  MOD_VAL = 0x2321;
//    int     start_seq   = -1; //31345;                     // < 0 -> randomize
    int     trace_level = 0; //(0 <= start_seq) ? 1 : 0;  // eqs.txt, board.txt etc.
    bool    log_on      = false;                      // Erstellt 'auto.log'-Datei (zum Fehlerreproduzieren)
    bool    perfana_on  = false;
    int     time_mod    = 0; // 0: time, 1: EW

    RandInt rand_int; // ohne Wert -> randomize
  }
}


using namespace MinesPerfect;

//******************************************************************************
void GameCtrl::initSound()
//------------------------------------------------------------------------------
{
  m_open_sound    = CreateSound ("open");
  m_flag_sound    = CreateSound ("flag");
  m_error_sound   = CreateSound ("error");
  m_new_sound     = CreateSound ("new");
  m_won_sound     = CreateSound ("won");
  m_lost_sound    = CreateSound ("lost");
  m_deadend_sound = CreateSound ("deadend");
}

//******************************************************************************
void GameCtrl::initCtrls()
//------------------------------------------------------------------------------
{
  // Init Bevels                 (parent, thickness, raised);
  m_win_bevel       = new BevelCtrl (this,             1, true); 
  m_win_head_bevel  = new BevelCtrl (m_win_bevel,      2, true); 
  m_win_body_bevel  = new BevelCtrl (m_win_bevel,      2, true);
  m_left_lcd_bevel  = new BevelCtrl (m_win_head_bevel, 1, false);
  m_left_sep_bevel  = new BevelCtrl (m_win_head_bevel, 1, false);
  m_smiley_bevel    = new BevelCtrl (m_win_head_bevel, 1, false);
  m_right_sep_bevel = new BevelCtrl (m_win_head_bevel, 1, false);
  m_right_lcd_bevel = new BevelCtrl (m_win_head_bevel, 1, false);
  
  // restlichen Controls
  m_left_lcd      = new LcdCtrl    (m_left_lcd_bevel);
  m_smiley_button = new SmileyCtrl (m_smiley_bevel, this);
  m_right_lcd     = new LcdCtrl    (m_right_lcd_bevel);
  m_board_ctrl    = new BoardCtrl  (m_win_body_bevel, this);
}

//******************************************************************************
void GameCtrl::alignCtrlInBevel(Ctrl* ctrl, const BevelCtrl* bvl, int h, int v,
                                bool recursive)
//------------------------------------------------------------------------------
// setzt top und left, so dass *this komplett in bvl liegt
// h = horizontale Position
// v = vertikale Position
//
// h <   0  =>  (-h)    ist abs. Abstand von links
// h > 100  =>  (h-100) ist abs. Abstand von rechts
// sonst    =>  h ist rel. Pos. in Prozent
// v genauso
// 
// Diese Funktion liegt nicht in Bevel::, weil eine allg. Ctrl geaendert wird.
// Diese Funktion liegt nicht in Ctrl::, weil sie zu speziell ist und ein 
// Bevel nichts zu suchen hat in einer Methode von Ctrl::.
// Eine allg. Anordnungsbeschreibung mit Spacern etc. waere overkill fuer
// dieses Programm. Es waere auch nicht so einfach zu realisieren, da man
// wegen der Bevels ein ClientRect in Ctrl einfuehren muesste.
{
  int    left_min = bvl->getThickness();
  int    top_min  = bvl->getThickness();
  int    left_max = bvl->getSize().x - bvl->getThickness() - ctrl->getSize().x;
  int    top_max  = bvl->getSize().y - bvl->getThickness() - ctrl->getSize().y;
  Point  pos;

  if (h < 0)
    pos.x = left_min + (-h);
  else if (h > 100)
    pos.x = left_max - (h-100);
  else //
    pos.x = left_min + (left_max - left_min) * h / 100;

  if (v < 0)
    pos.y = top_min + (-v);
  else if (v > 100)
    pos.y = top_max - (v-100);
  else
    pos.y = top_min + (top_max - top_min) * v / 100;
    
  ctrl->setRelPos(pos, recursive);
}

//******************************************************************************
void GameCtrl::alignCtrls()
//------------------------------------------------------------------------------
{
  const int     thick = 8;
  const int     dist  = 6;

  // sizes:
  
  // head-ctrl-sizes
  m_left_lcd_bevel->setContSize  (m_left_lcd->getSize());
  m_smiley_bevel->setContSize    (m_smiley_button->getSize());
  m_right_lcd_bevel->setContSize (m_right_lcd->getSize());

  // board-size // besser m_board_ctrl->size = board->getFieldsize();
  Point fieldsize;
  m_board->getFieldsize(fieldsize.y, fieldsize.x);
  m_board_ctrl->setSize(fieldsize);

  // head_width
  int head_width =   m_left_lcd_bevel->getSize().x + thick  // vorlaeufig
                   + m_smiley_bevel->getSize().x   + thick
                   + m_right_lcd_bevel->getSize().x;

  if (head_width < m_board_ctrl->getSize().x)
     head_width = m_board_ctrl->getSize().x;   // endgueltig

  // win_head-size
  m_win_head_bevel->setContSize (Point (head_width + 2 * thick,
                                        m_left_lcd_bevel->getSize().y + 2 * dist));
                                     
  // win_body-size
  m_win_body_bevel->setContSize (Point (head_width + 2 * thick,
                                        m_board_ctrl->getSize().y + 2 * thick));

  // win size
  m_win_bevel->setContSize (Point (m_win_head_bevel->getSize().x,
                                 m_win_head_bevel->getSize().y + m_win_body_bevel->getSize().y));

  // posistions + sizes of sep_bevels : 
  
  // game pos
  setRelPos(Point(0,0));
  
  // win pos
  m_win_bevel->setRelPos(Point(0,0));

  alignCtrlInBevel (m_win_head_bevel, m_win_bevel, 50,   0); // center/top
  alignCtrlInBevel (m_win_body_bevel, m_win_bevel, 50, 100); // center/buttom

  // smiley pos 
  alignCtrlInBevel (m_smiley_bevel, m_win_head_bevel, 50, 50);
  m_smiley_button->setRelPos (m_smiley_bevel->getContPos());

  // m_left_lcd pos
  alignCtrlInBevel (m_left_lcd_bevel, m_win_head_bevel, -thick, 50);
  m_left_lcd->setRelPos (m_left_lcd_bevel->getContPos());

  // m_left_sep_bevel pos + size
  m_left_sep_bevel->setRelPos (m_left_lcd_bevel->getRelPos() 
                             + Point(m_left_lcd_bevel->getSize().x + thick, 0));
  m_left_sep_bevel->setSize (Point (m_smiley_bevel->getRelPos().x 
                                      - m_left_sep_bevel->getRelPos().x - thick,
                                    m_left_lcd_bevel->getSize().y));

  // right lcd pos
  alignCtrlInBevel (m_right_lcd_bevel, m_win_head_bevel, 100 + thick, 50);
  m_right_lcd->setRelPos (m_right_lcd_bevel->getContPos());

  // m_right_sep_bevel pos + size
  m_right_sep_bevel->setRelPos (Point (m_smiley_bevel->getRelPos().x 
                                          + m_smiley_bevel->getSize().x + thick,
                                        m_right_lcd_bevel->getRelPos().y));
  m_right_sep_bevel->setSize (Point (m_right_lcd_bevel->getRelPos().x 
                                     - m_right_sep_bevel->getRelPos().x - thick,
                                     m_right_lcd_bevel->getSize().y));

  // board pos
  alignCtrlInBevel (m_board_ctrl, m_win_body_bevel, 50, 50, true);
}

//******************************************************************************
void GameCtrl::showTime (int secs)
//------------------------------------------------------------------------------
{
  if (secs >= MAX_SECS)
    secs = MAX_SECS;
  
  int  val;
  int  ew = MAX_SECS;
  int  lvl = m_options->getLevelNr();
  int  rec = m_options->getRecord(lvl).time;

  if (m_board != 0)
  {
    int  op  = (m_board != 0) ? m_board->getNumOpen()    : 0;
    int  fl  = (m_board != 0) ? m_board->getNumFlagged() : 0;
    int  all = (m_board != 0) ? m_board->getNumCells()   : 0;
    int  mi  = (m_board != 0) ? m_board->getNumMines()   : 0;

    if (op > 0 && fl > 0)
      ew  =   (0 * secs * (all - mi)) / (4 * op)
            + (4 * secs *      mi   ) / (4 * fl);
  }

  if (Glob::time_mod == 1) // EW
    val = ew;
  else if (Glob::time_mod == 2) // diff
    val = ew - rec;
  else if (Glob::time_mod == 2) // velocity
    val = (100 * ew) / rec - 100;
  else // Zeit
    val = secs;

  m_right_lcd->setVal (val);
  m_right_lcd->show();
}

//******************************************************************************
void GameCtrl::setHelpUsed()
//------------------------------------------------------------------------------
{
  m_help_used = true;
  m_timer->stop();
  showTime(MAX_SECS);
}

//******************************************************************************
void GameCtrl::actWin(void) // eh. actBoard
//------------------------------------------------------------------------------
{
  m_left_lcd->setVal (m_board->getNumMines() - m_board->getNumFlagged());
//  m_left_lcd->show();
//  m_smiley_button->show();
  m_smiley_button->act();

  if (m_board->getState() == PLAY && !m_timer->isRunning() && !m_help_used)
  {
    clock_t clock0 = m_timer->start();
    *m_logbook << Log(LOG_START_TIMER, clock0);
  }

  // geaenderte Zellen
  m_board_ctrl->actChangedCells();

  // gewonnen oder verloren?
  if ((m_state != m_board->getState()
      || m_next_move_stage != m_board->getCurStage()))
//  && !m_left_mousebutton_down)
  {
    m_state = m_board->getState();
    m_next_move_stage = m_board->getCurStage();

    int num_msecs = m_timer->getMSecs();

    if (m_board->gameOver())
      m_timer->stop();

    if (m_board->gameIsWon())
    {
      m_won_sound->play();

      // Rekord?
      if (!m_help_used
      &&  num_msecs < m_options->getRecord(m_options->getLevelNr()).time
      &&  m_options->getLevelNr() != USER_DEFINED
      &&  m_open_after_start)
      {
        DlgNewRecord (m_options, num_msecs, m_board->isCertified());
      }
    }
    else if (m_board->gameIsLost())
    {
      m_board_ctrl->actAllCells();
      m_lost_sound->play();
    }
  }
}

//******************************************************************************
void GameCtrl::newGame (int change_level)
//------------------------------------------------------------------------------
// change_level: 0  cells/frames
//               1  new winsize
//               2  new board (new bitmaps)
{
  Glob::rand_int.setStart (++m_rand_seq); // 1 wurde beim Initialisieren abgezogen

  // m_logbook
  if (m_logbook == 0 || !m_logbook->isPlaying())
  {
    delete m_logbook;
    m_logbook = new Logbook (m_rand_seq, *m_options);
  }

  // board
  if (change_level == 0)
  {
    m_board->reset(*m_logbook);
  }
  else
  {
    delete m_board;
    m_board = new Board (*m_options, *m_logbook);
//    delete m_board_ctrl;
//    m_board_ctrl = new BoardCtrl (m_win_body_bevel, this);

    if (change_level >= 2)
      m_board_ctrl->setBoard(m_options->getBoardNr()); // act. Bitmaps
    else
      m_board_ctrl->setLevel(m_options->getLevel());
    
    alignCtrls();
    WinSetSize(m_win_bevel->getSize());
    setDirty(); // wegen WinSetSize
  }

  // num-mines + smiley
  m_left_lcd->setVal (m_board->getNumMines() - m_board->getNumFlagged());
  m_left_lcd->setDirty();
  m_smiley_button->act();
  m_smiley_button->setDirty();

  // m_timer
  m_timer->stop();
  m_timer->reset();

  if (m_options->getAutoStage() == 0 && m_options->getMaxStage() == MAX_STAGE
     &&  m_options->getLevelNr() != USER_DEFINED && !m_options->getShowMines())
  {
    m_help_used = false;
    m_right_lcd->setVal(0);
    m_right_lcd->setDirty();
  }
  else
  {
    setHelpUsed();
  }

  if (m_board->getState() == PLAY && !m_help_used)
  {
    clock_t clock0 = m_timer->start();
    *m_logbook << Log(LOG_START_TIMER, clock0);
  }
  
  // cells
  m_board_ctrl->setCellsDirty();
  m_board_ctrl->actAllCells();

  // Durch AutoSolve Spiel schon beendet?
  if (m_state == WON || m_state == LOST)
  {
    m_state = PLAY; // tricky
//    actWin();
  }

  // rest
  m_state            = m_board->getState();
  m_next_move_stage  = m_board->getCurStage();
  m_open_after_start = false;

  m_new_sound->play();
}

//******************************************************************************
bool GameCtrl::changeBoard (BoardNr nr)
//------------------------------------------------------------------------------
{
  if (nr < 0 || m_options->getNumBoards() <= nr)
    return false;

  m_options->setBoardNr(nr);

  newGame(2);

  return true;
}

//******************************************************************************
GameCtrl::GameCtrl(Options* options) : Ctrl(0)
//------------------------------------------------------------------------------
{
  initSound();
  initCtrls();

  m_timer   = CreateTimer();
  m_logbook = 0;
  m_board   = 0;
  m_options = options;

//  m_left_mousebutton_down  = false;

  m_rand_seq = Glob::rand_int.getStart() - 1; // -1 wird spaeter wieder draufaddiert

  if (!changeBoard(m_options->getBoardNr()))
    changeBoard(0);
}

//******************************************************************************
void GameCtrl::changeLevel (LevelNr lvl_nr)
//------------------------------------------------------------------------------
{
  Level  lvl;

  lvl.nr = lvl_nr;
  changeLevel(lvl);
}

//******************************************************************************
void GameCtrl::changeLevel (const Level& lvl)
//------------------------------------------------------------------------------
{
  m_options->setLevel(lvl);

  newGame(1);
}

//******************************************************************************
void GameCtrl::changeModus (Modus mod)
//------------------------------------------------------------------------------
{
  if (m_options->getModus() == mod)
    return;

  *m_logbook << Log (LOG_MODUS, mod);

  m_options->setModus (mod);
  m_board->setModus (mod, m_board_ctrl->m_changed_cells, *m_logbook);
  actWin();
}

//******************************************************************************
void GameCtrl::setMurphysLaw (bool set_on)
//------------------------------------------------------------------------------
{
  if (set_on != m_options->getMurphysLaw())
  {
    m_options->setMurphysLaw (set_on);

    *m_logbook << Log (LOG_MURPHYS_LAW, set_on);

    m_board->setMurphysLaw (set_on);
  }
}

//******************************************************************************
void GameCtrl::onMouseEvent (const MouseEvent& ev)
//------------------------------------------------------------------------------
{
//  if      (ev.m_type == MouseEvent::LEFT_DOWN)    m_left_mousebutton_down  = true;
//  else if (ev.m_type == MouseEvent::LEFT_UP)      m_left_mousebutton_down  = false;

//  m_smiley_button->onMouseEvent (ev, p);
//  m_board_ctrl->onMouseEvent (ev, p - m_board_ctrl->m_rect.getPos());

  Ctrl::onMouseEvent(ev);

  actWin(); // neu
}

//******************************************************************************
void GameCtrl::giveHint(void)
//------------------------------------------------------------------------------
{
  *m_logbook << Log (LOG_HINT, 0);

  m_board->giveHint (m_board_ctrl->m_changed_cells);
//  if (m_timer != 0)
//    m_timer->setNum (999);
  setHelpUsed();
  actWin();
}

//******************************************************************************
void GameCtrl::solveOne(void)
//------------------------------------------------------------------------------
{
  if (!m_board->gameOver() && m_board->getState() != BEFORE)
  {
    *m_logbook << Log (LOG_SOLVE_ONE, 0);

    m_board->solveOne (m_board_ctrl->m_changed_cells, *m_logbook);

    setHelpUsed();
    actWin();
  }
}

//******************************************************************************
void GameCtrl::solveAll (StageNr stage)
//------------------------------------------------------------------------------
{
  if (!m_board->gameOver() && m_board->getState() != BEFORE)
  {
    *m_logbook << Log (LOG_SOLVE_ALL, stage);

    m_board->solveAll (stage, m_board_ctrl->m_changed_cells, *m_logbook);

    setHelpUsed();
    actWin();
  }
}

//******************************************************************************
void GameCtrl::changeSolveAuto (StageNr stage)
//------------------------------------------------------------------------------
{
  if (stage != m_options->getAutoStage())
  {
    m_options->setAutoStage (stage);

    if (stage > 0)
    {
      setHelpUsed();
    }
/* ???    
    else if (m_board == 0 || m_board->getState() == BEFORE
         &&  m_options->getMaxStage() == MAX_STAGE)
    {
      showTime (0);
    }
*/

    *m_logbook << Log (LOG_AUTO_STAGE, stage);

    if (m_board != 0)
    {
      m_board->setAutoStage (m_options->getAutoStage(), m_board_ctrl->m_changed_cells,
                             *m_logbook);
      actWin();
    }
  }
}

//******************************************************************************
void GameCtrl::changeMaxStage (StageNr stage)
//------------------------------------------------------------------------------
{
  if (stage != m_options->getMaxStage())
  {
    m_options->setMaxStage (stage);

    if (m_options->getMaxStage() < MAX_STAGE)
    {
      setHelpUsed();
    }
/* ???      
    else if (m_board == 0 || m_board->getState() == BEFORE
         &&  m_options->getAutoStage() == 0)
      showTime (0);
*/

    *m_logbook << Log (LOG_MAX_STAGE, stage);

    if (m_board != 0)
    {
      m_board->setMaxStage (stage, m_board_ctrl->m_changed_cells, *m_logbook);
      actWin();
    }
  }
}

//******************************************************************************
void GameCtrl::setShowMines (bool set_on)
//------------------------------------------------------------------------------
{
  if (set_on != m_options->getShowMines())
  {
    *m_logbook << Log (LOG_SHOW_MINES, set_on);

    m_options->setShowMines (set_on);
    m_board_ctrl->actAllCells();

    if (set_on)
      setHelpUsed();
  }
}

//******************************************************************************
void GameCtrl::playLog (const Log& log, bool with_sound)
//------------------------------------------------------------------------------
{
  if (!log.valid)
    return;
    
  if (log.name == LOG_FLAG)
  {
    if (!m_board->flagCells (log.val, m_board_ctrl->m_changed_cells, *m_logbook))
      m_logbook->invalidateLast();
    
    if (with_sound)  
      m_flag_sound->play();
  }
  else if (log.name == LOG_OPEN)
  {
    if (!m_board->openCells (log.val, m_board_ctrl->m_changed_cells, *m_logbook))
      m_logbook->invalidateLast();
  
    if (with_sound)    
      m_open_sound->play();
  }
  else if (log.name == LOG_MODUS)
  {
    if (m_options->getModus() != log.val)
    {
      if (log.val == ORIGINAL
      ||  log.val == IMMUNE
      ||  log.val == LUCKY
      ||  log.val == HINTS
      ||  log.val == STARTUP)
      {
        m_options->setModus ((Modus) log.val);
        m_board->setModus ((Modus) log.val, m_board_ctrl->m_changed_cells, *m_logbook);
      }
    }
  }
  else if (log.name == LOG_MURPHYS_LAW)
  {
    bool  set_murphys = (log.val != 0);

    if (m_options->getMurphysLaw() != set_murphys)
    {
      m_options->setMurphysLaw (set_murphys);
      m_board->setMurphysLaw (set_murphys);
    }
  }
  else if (log.name == LOG_HINT)
  {
    m_board->giveHint (m_board_ctrl->m_changed_cells);
  }
  else if (log.name == LOG_SOLVE_ONE)
  {
    m_board->solveOne (m_board_ctrl->m_changed_cells, *m_logbook);
  }
  else if (log.name == LOG_SOLVE_ALL)
  {
    m_board->solveAll (log.val, m_board_ctrl->m_changed_cells, *m_logbook);
  }
  else if (log.name == LOG_AUTO_STAGE)
  {
    if (m_options->getAutoStage() != log.val)
    {
      m_options->setAutoStage (log.val);

      if (m_board != 0)
        m_board->setAutoStage (log.val, m_board_ctrl->m_changed_cells, *m_logbook);
    }
  }
  else if (log.name == LOG_MAX_STAGE)
  {
    if (m_options->getMaxStage() != log.val)
    {
      m_options->setMaxStage (log.val);

      if (m_board != 0)
        m_board->setMaxStage (m_options->getMaxStage(), m_board_ctrl->m_changed_cells,
                              *m_logbook);
    }
  }
  else if (log.name == LOG_SHOW_MINES)
  {
    m_options->setShowMines (log.val != 0);

    // Nach dem Ausschalten von ShowMines muessen alle Felder akt. werden
    // (dies geschieht nicht autom. in actWin())
    if (log.val == 0)
    {
      m_board_ctrl->m_changed_cells.clear();
      for (CellNr k = 0; k < m_board->getNumCells(); k++)
        m_board_ctrl->m_changed_cells.push_back (k);
    }
  }
  else 
  {
    ASSERT (log.name == LOG_START_TIMER);
  }
}

//******************************************************************************
void GameCtrl::playLogbook()
//------------------------------------------------------------------------------
{
  // Während der Wiederholung den Sound abstellen
  bool  with_sound_old = m_options->getWithSound();
  m_options->setWithSound (false);

  // init m_logbook
  m_logbook->startPlaying();

  // neues Board
  m_rand_seq = m_logbook->getRandSeq() - 1; // 1 wird bei newGame wieder draufaddiert
  *m_options = m_logbook->getOptions();

  changeBoard (m_options->getBoardNr()); // erzeugt Computerlogs
                                      // und setzt play_index entsprechend weiter
  // playing
  while (!m_logbook->eop())
  {
    Log log;
    *m_logbook >> log;        // hierbei wird auto.log autom. geschrieben
    playLog(log, false);    // auch hier werden Computerlogs neu erzeugt
                            // und der play_index entsprechend hochgesetzt
  }

  // fertig
  m_logbook->stopPlaying();

  setHelpUsed();

  m_board_ctrl->m_changed_cells.clear();
  for (CellNr k = 0; k < m_board->getNumCells(); k++)
    m_board_ctrl->m_changed_cells.push_back (k);

  actWin();

  m_cur_cell = -1;

  m_options->setWithSound (with_sound_old);
}

//******************************************************************************
void GameCtrl::load (const string& fname)
//------------------------------------------------------------------------------
{
  // load
  try
  {
    if (fname.find(',') == string::npos) 
    {
      m_logbook->read (fname);
    }
    else
    {
      string fname2 = fname;

      // Der Dateioeffnendialog fuegt am Anfang noch den
      // Pfad und am Ende eine Extension an, diese muessen
      // jetzt wieder entfernt werden
      string::size_type i = fname2.rfind('\\');
      if (i != string::npos)
        fname2 = fname2.substr(i + 1);

      i = fname2.rfind('.');
      if (i != string::npos)
        fname2 = fname2.substr(0, i);

      m_logbook->importStr (fname2);
    }
  }
  catch (LogException &exception)
  {
    m_error_sound->play();
    ShowMessageDlg (exception.getText(), "Load-Error");
    return;
  }

  // durchspielen
  playLogbook();
}

//******************************************************************************
void GameCtrl::save (const string& fname)
//------------------------------------------------------------------------------
{
  try
  {
    m_logbook->write (fname);
  }
  catch (LogException &exception)
  {
    m_error_sound->play();
    ShowMessageDlg (exception.getText(), "Save-Error");
  }
}

//******************************************************************************
void GameCtrl::undo(bool all)
//------------------------------------------------------------------------------
{
  if (m_logbook->undo(all))
    playLogbook();
  else
    m_error_sound->play();
}

//******************************************************************************
void GameCtrl::redo()
//------------------------------------------------------------------------------
{
  Log  log;

  if (!m_logbook->redo(log)) // auto.log wird von m_logbook->redo akt.
  {
    m_error_sound->play();
    return;
  }

  playLog (log, true);

  actWin();

  m_cur_cell = -1;
}

