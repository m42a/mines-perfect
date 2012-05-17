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

#include <iostream.h>
#include <stdlib.h>
#include <vector> // visual studio (nur fuer debug-konfiguration)
#include <fstream> // visual studio (nur fuer debug-konfiguration)

#ifdef __GNUG__
  #pragma implementation
  #pragma interface
#endif

#ifdef __GNUWIN32__
  #include <windows.h>
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

//#include <wx/image.h>
#include <wx/imagjpeg.h>

#if !wxCHECK_VERSION (2, 4, 0)
  #include <wx/xpmhand.h>  // xpm ab wx240 nativ
#endif
  
//#include <wx/wxhtml.h>
//#include <wx/mimetype.h>
#include <wx/file.h>

#include "../gui/gamectrl.h"
#include "../core/api.h"

#include "app.h"
#include "dialogs.h"
#include "mine16.xpm"

MainWindow*  main_win = 0; // fuer api.cpp

// IDs for the menu command
enum
{
   MENU_GAME_NEW = 1,
   MENU_GAME_OPEN,
   MENU_GAME_SAVE,
   MENU_GAME_BEGINNER,
   MENU_GAME_INTERMEDIATE,
   MENU_GAME_EXPERT,
   MENU_GAME_SELFDEFINED,
   MENU_GAME_ORIGINAL,
   MENU_GAME_IMMUNE,
   MENU_GAME_LUCKY,
   MENU_GAME_HINT,
   MENU_GAME_STARTUP,
   MENU_GAME_MURPHY_S_LAW,
   MENU_GAME_BEST_TIMES,
   MENU_GAME_EXIT,
     
   MENU_HELP_HINT,
   MENU_HELP_SOLVE_ONE,
   MENU_HELP_SOLVE_ALL,
   MENU_HELP_SOLVE_ALL1,
   MENU_HELP_SOLVE_ALL2,
   MENU_HELP_SOLVE_ALL3,
   MENU_HELP_SOLVE_AUTO,
   MENU_HELP_SOLVE_AUTO0,
   MENU_HELP_SOLVE_AUTO1,
   MENU_HELP_SOLVE_AUTO2,
   MENU_HELP_SOLVE_AUTO3,
   MENU_HELP_MAX_STAGE,
   MENU_HELP_MAX_STAGE1,
   MENU_HELP_MAX_STAGE2,
   MENU_HELP_MAX_STAGE3,
   MENU_HELP_SHOW_MINES,
   MENU_HELP_UNDO,
   MENU_HELP_REDO,
   MENU_HELP_UNDO_ALL,
   MENU_HELP_CONTENTS,
   MENU_HELP_HOMEPAGE,
   MENU_HELP_ABOUT,

   MENU_BOARD_FIRST
};


BEGIN_EVENT_TABLE( MainWindow, wxFrame )
   EVT_MENU( MENU_GAME_NEW,           MainWindow::OnGameNew          )
   EVT_MENU( MENU_GAME_OPEN,          MainWindow::OnGameOpen         )
   EVT_MENU( MENU_GAME_SAVE,          MainWindow::OnGameSave         )
   EVT_MENU( MENU_GAME_BEGINNER,      MainWindow::OnGameBeginner     )
   EVT_MENU( MENU_GAME_INTERMEDIATE,  MainWindow::OnGameIntermediate )
   EVT_MENU( MENU_GAME_EXPERT,        MainWindow::OnGameExpert       )
   EVT_MENU( MENU_GAME_SELFDEFINED,   MainWindow::OnGameSelfdefined  )
   EVT_MENU( MENU_GAME_ORIGINAL,      MainWindow::OnGameOriginal     )
   EVT_MENU( MENU_GAME_IMMUNE,        MainWindow::OnGameImmune       )
   EVT_MENU( MENU_GAME_LUCKY,         MainWindow::OnGameLucky        )
   EVT_MENU( MENU_GAME_HINT,          MainWindow::OnGameHint         )
   EVT_MENU( MENU_GAME_STARTUP,       MainWindow::OnGameStartup      )
   EVT_MENU( MENU_GAME_MURPHY_S_LAW,  MainWindow::OnGameMurphysLaw   )
   EVT_MENU( MENU_GAME_BEST_TIMES,    MainWindow::OnGameBestTimes    )
   EVT_MENU( MENU_GAME_EXIT,          MainWindow::OnGameExit         )

   EVT_MENU_RANGE( MENU_BOARD_FIRST, MENU_BOARD_FIRST + 99, MainWindow::OnBoard  )
   
   EVT_MENU( MENU_HELP_HINT,          MainWindow::OnHelpHint         )
   EVT_MENU( MENU_HELP_SOLVE_ONE,     MainWindow::OnHelpSolveOne     )
   EVT_MENU( MENU_HELP_SOLVE_ALL,     MainWindow::OnHelpSolveAll     )
   EVT_MENU( MENU_HELP_SOLVE_ALL1,    MainWindow::OnHelpSolveAll1    )
   EVT_MENU( MENU_HELP_SOLVE_ALL2,    MainWindow::OnHelpSolveAll2    )
   EVT_MENU( MENU_HELP_SOLVE_ALL3,    MainWindow::OnHelpSolveAll3    )
   EVT_MENU( MENU_HELP_SOLVE_AUTO,    MainWindow::OnHelpSolveAuto    )
   EVT_MENU( MENU_HELP_SOLVE_AUTO0,   MainWindow::OnHelpSolveAuto0   )
   EVT_MENU( MENU_HELP_SOLVE_AUTO1,   MainWindow::OnHelpSolveAuto1   )
   EVT_MENU( MENU_HELP_SOLVE_AUTO2,   MainWindow::OnHelpSolveAuto2   )
   EVT_MENU( MENU_HELP_SOLVE_AUTO3,   MainWindow::OnHelpSolveAuto3   )
   EVT_MENU( MENU_HELP_MAX_STAGE,     MainWindow::OnHelpMaxStage     )
   EVT_MENU( MENU_HELP_MAX_STAGE1,    MainWindow::OnHelpMaxStage1    )
   EVT_MENU( MENU_HELP_MAX_STAGE2,    MainWindow::OnHelpMaxStage2    )
   EVT_MENU( MENU_HELP_MAX_STAGE3,    MainWindow::OnHelpMaxStage3    )
   EVT_MENU( MENU_HELP_SHOW_MINES,    MainWindow::OnHelpShowMines    )
   EVT_MENU( MENU_HELP_UNDO,          MainWindow::OnHelpUndo         )
   EVT_MENU( MENU_HELP_REDO,          MainWindow::OnHelpRedo         )
   EVT_MENU( MENU_HELP_UNDO_ALL,      MainWindow::OnHelpUndoAll      )
   EVT_MENU( MENU_HELP_CONTENTS,      MainWindow::OnHelpContents     )
   EVT_MENU( MENU_HELP_HOMEPAGE,      MainWindow::OnHelpHomepage     )
   EVT_MENU( MENU_HELP_ABOUT,         MainWindow::OnHelpAbout        )
   
   EVT_MOUSE_EVENTS (MainWindow::OnMouseEvent)
   EVT_CLOSE        (MainWindow::OnCloseWindow)
   EVT_PAINT        (MainWindow::OnPaintEvent)
END_EVENT_TABLE()                                                

IMPLEMENT_APP(MainApp)


// Implementation

// Main Application
/*
void MainApp::showAssertException (const MinesPerfect::AssertException& exc) const
{
  wxString msg_text = "Please send the 'auto.log'-file to 'mail@czeppi.de'.\n"
                      "(!! The 'auto.log-file will be rewritten, when a new game starts !!)\n"
                      "\n"
                      + wxString(exc.getText().c_str())
+ "\n" + wxString(main_win->game->m_logbook->exportStr().c_str());


  wxMessageDialog  dlg (main_win, msg_text, 
                        "Exception", wxOK | wxICON_ERROR);
  dlg.ShowModal();

  main_win->Destroy();
}
*/
void MainApp::showException (const MinesPerfect::Exception& exc) const
{
  ExceptionDialog  dlg (main_win, exc.getText().c_str());
  dlg.ShowModal();
  dlg.Destroy(); // notwendig?
  
  main_win->Destroy();
}

bool MainApp::OnInit()
{
  wxImage::AddHandler(new wxJPEGHandler);
  
#if !wxCHECK_VERSION (2, 4, 0)
  wxBitmap::AddHandler(new wxXPMFileHandler); // xpm ab wx240 nativ
  wxBitmap::AddHandler(new wxXPMDataHandler);
#endif
  
  try
  {
    Options* opt = new MinesPerfect::Options();
    main_win = new MainWindow("Mines-Perfect", 
                              wxPoint(opt->getXPos(), opt->getYPos()),
                              wxSize(500, 500));
    SetTopWindow(main_win);

    MinesPerfect::InitApi();

    main_win->game    = new MinesPerfect::GameCtrl(opt);
    main_win->Move(main_win->game->m_options->getXPos(), 
                   main_win->game->m_options->getYPos());
    main_win->CreateBoardMenu();
    main_win->ActMenu();
  }
  catch (const MinesPerfect::Exception& exc)
  {
    showException(exc);
  }
  catch (...)
  {
    MinesPerfect::Exception exc("unknown exception.");
    showException(exc);
  }

  // Do not return FALSE or the app will terminate immediately 
  return TRUE;
}

int MainApp::MainLoop()
{
  try
  {
    return wxApp::MainLoop();
  }
  catch (const MinesPerfect::Exception& exc)
  {
    showException(exc);
  }
  catch (...)
  {
    MinesPerfect::Exception exc("unknown exception.");
    showException(exc);
  }
  
  return 0;
}

// Main Window

void MainWindow::ActMenu()
{
  GameMenu->Check(MENU_GAME_BEGINNER,     game->m_options->getLevelNr() == MinesPerfect::BEGINNER);
  GameMenu->Check(MENU_GAME_INTERMEDIATE, game->m_options->getLevelNr() == MinesPerfect::INTERMEDIATE);
  GameMenu->Check(MENU_GAME_EXPERT,       game->m_options->getLevelNr() == MinesPerfect::EXPERT);
  GameMenu->Check(MENU_GAME_SELFDEFINED,  game->m_options->getLevelNr() == MinesPerfect::USER_DEFINED);

  GameMenu->Check(MENU_GAME_ORIGINAL,     game->m_options->getModus() == MinesPerfect::ORIGINAL);
  GameMenu->Check(MENU_GAME_IMMUNE,       game->m_options->getModus() == MinesPerfect::IMMUNE);
  GameMenu->Check(MENU_GAME_LUCKY,        game->m_options->getModus() == MinesPerfect::LUCKY);
  GameMenu->Check(MENU_GAME_HINT,         game->m_options->getModus() == MinesPerfect::HINTS);
  GameMenu->Check(MENU_GAME_STARTUP,      game->m_options->getModus() == MinesPerfect::STARTUP);
  GameMenu->Check(MENU_GAME_MURPHY_S_LAW, game->m_options->getMurphysLaw());

  for (int i = 0; i < (int) BoardMenu->GetMenuItemCount(); i++)
    BoardMenu->Check(MENU_BOARD_FIRST + i, i == game->m_options->getBoardNr());
  
//  HelpMenu->Check(MENU_HELP_SOLVE_AUTO, game->m_options.getAutoStage() != 0);
  SolveAutoMenu->Check(MENU_HELP_SOLVE_AUTO0, game->m_options->getAutoStage() == 0);
  SolveAutoMenu->Check(MENU_HELP_SOLVE_AUTO1, game->m_options->getAutoStage() == 1);
  SolveAutoMenu->Check(MENU_HELP_SOLVE_AUTO2, game->m_options->getAutoStage() == 2);
  SolveAutoMenu->Check(MENU_HELP_SOLVE_AUTO3, game->m_options->getAutoStage() == 3);

//  HelpMenu->Check(MENU_HELP_MAX_STAGE, game->m_options.getMaxStage() < MinesPerfect::MAX_STAGE);
  MaxStageMenu->Check(MENU_HELP_MAX_STAGE1, game->m_options->getMaxStage() == 1);
  MaxStageMenu->Check(MENU_HELP_MAX_STAGE2, game->m_options->getMaxStage() == 2);
  MaxStageMenu->Check(MENU_HELP_MAX_STAGE3, game->m_options->getMaxStage() == 3);

  HelpMenu->Check(MENU_HELP_SHOW_MINES, game->m_options->getShowMines());

  GameMenu->Enable(MENU_GAME_LUCKY,  game->m_options->getMaxStage() == 3);

  HelpMenu->Enable(MENU_HELP_MAX_STAGE, game->m_options->getModus() != MinesPerfect::LUCKY);
}

MainWindow::MainWindow(const wxString &title, const wxPoint &pos, const wxSize &size)
    : wxFrame((wxFrame *) NULL, -1, title, pos, size, wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION)
{
//    wxIcon  app_icon ("mine16.ico", wxBITMAP_TYPE_ICO);
  wxIcon  app_icon (mine16_xpm);
  SetIcon(app_icon);

  // Backgroundcolour
//  SetBackgroundColour(*wxLIGHT_GREY); // old

#if wxCHECK_VERSION (2, 4, 0)
  SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT));
#else
  SetBackgroundColour(wxSystemSettings::GetSystemColour(wxSYS_COLOUR_3DLIGHT));
#endif

  // wxSYS_COLOUR_* : SCROLLBAR, MENU, ACTIVEBORDER, INACTIVEBORDER, BTNFACE
  //                  INACTIVECAPTIONTEXT, 3DFACE, 3DLIGHT

  // Menu
  GameMenu      = new wxMenu;
  BoardMenu     = new wxMenu;
  HelpMenu      = new wxMenu;
  SolveAllMenu  = new wxMenu;
  SolveAutoMenu = new wxMenu;
  MaxStageMenu  = new wxMenu;
  MenuBar       = new wxMenuBar;

  // create a Game-Menu
  GameMenu->Append( MENU_GAME_NEW,          "&New\tCtrl-N",  "New game" );
  GameMenu->Append( MENU_GAME_OPEN,         "&Open\tCtrl-O", "Open a saved game" );
  GameMenu->Append( MENU_GAME_SAVE,         "&Save\tCtrl-S", "Save the game" );
  GameMenu->AppendSeparator();
  GameMenu->Append( MENU_GAME_BEGINNER,     "&Beginner",     "", true );
  GameMenu->Append( MENU_GAME_INTERMEDIATE, "&Intermediate", "", true );
  GameMenu->Append( MENU_GAME_EXPERT,       "&Expert",       "", true );
  GameMenu->Append( MENU_GAME_SELFDEFINED,  "Self&defined",  "", true );
  GameMenu->AppendSeparator();
  GameMenu->Append( MENU_GAME_ORIGINAL,     "O&riginal",     "", true );
  GameMenu->Append( MENU_GAME_IMMUNE,       "Imm&une",       "", true );
  GameMenu->Append( MENU_GAME_LUCKY,        "&Lucky",        "", true );
  GameMenu->Append( MENU_GAME_HINT,         "&Hint",         "", true );
  GameMenu->Append( MENU_GAME_STARTUP,      "&Startup",      "", true );
  GameMenu->Append( MENU_GAME_MURPHY_S_LAW, "&MurphysLaw",   "", true );
  GameMenu->AppendSeparator();
  GameMenu->Append( MENU_GAME_BEST_TIMES,   "Best&Times"    );
  GameMenu->AppendSeparator();
  GameMenu->Append( MENU_GAME_EXIT,         "E&xit"         );

  GameMenu->Enable( MENU_GAME_LUCKY, false );
  GameMenu->Check ( MENU_GAME_BEGINNER, true);

  // Create Help-Menu
  HelpMenu->Append( MENU_HELP_HINT,      "&Hint\tCtrl-H"    );
  HelpMenu->Append( MENU_HELP_SOLVE_ONE, "Solve&One\tCtrl-L");
  
  SolveAllMenu->Append (MENU_HELP_SOLVE_ALL1, "Stage 1\tCtrl-1");
  SolveAllMenu->Append (MENU_HELP_SOLVE_ALL2, "Stage 2\tCtrl-2");
  SolveAllMenu->Append (MENU_HELP_SOLVE_ALL3, "Stage 3\tCtrl-3");
  HelpMenu->Append     (MENU_HELP_SOLVE_ALL,  "Solve&All",  SolveAllMenu,  "");
  
  SolveAutoMenu->Append (MENU_HELP_SOLVE_AUTO0, "Stage 0", "", true);
  SolveAutoMenu->Append (MENU_HELP_SOLVE_AUTO1, "Stage 1", "", true);
  SolveAutoMenu->Append (MENU_HELP_SOLVE_AUTO2, "Stage 2", "", true);
  SolveAutoMenu->Append (MENU_HELP_SOLVE_AUTO3, "Stage 3", "", true);
  HelpMenu->Append      (MENU_HELP_SOLVE_AUTO,  "&SolveAuto", SolveAutoMenu, "");
 
  MaxStageMenu->Append (MENU_HELP_MAX_STAGE1, "Stage 1", "", true);
  MaxStageMenu->Append (MENU_HELP_MAX_STAGE2, "Stage 2", "", true);
  MaxStageMenu->Append (MENU_HELP_MAX_STAGE3, "Stage 3", "", true);
  HelpMenu->Append     (MENU_HELP_MAX_STAGE,  "&MaxStage",  MaxStageMenu,  "");
  
  HelpMenu->Append( MENU_HELP_SHOW_MINES, "Show Mines", "", true);

  HelpMenu->AppendSeparator();
  HelpMenu->Append( MENU_HELP_UNDO,     "&Undo\tCtrl-Z");
  HelpMenu->Append( MENU_HELP_REDO,     "&Redo\tCtrl-Y");
  HelpMenu->Append( MENU_HELP_UNDO_ALL, "&UndoAll\tShift-Ctrl-Z");
  
  HelpMenu->AppendSeparator();
  HelpMenu->Append( MENU_HELP_CONTENTS, "&Contents");
  HelpMenu->Append( MENU_HELP_HOMEPAGE, "Home&page");
  HelpMenu->Append( MENU_HELP_ABOUT,    "A&bout");

  // Add it to the menu bar
  MenuBar->Append( GameMenu,  "&Game");
  MenuBar->Append( BoardMenu, "&Board");
  MenuBar->Append( HelpMenu,  "&Help");
  SetMenuBar(MenuBar);
    
  dlg_dir = "";
  dlg_file = "user.log";
}

void MainWindow::CreateBoardMenu()
{
  for (int i = 0; i < game->m_options->getNumBoards(); i++)
  {
    BoardMenu->Append (MENU_BOARD_FIRST + i, 
                       game->m_options->getBoardName(i).c_str(), "", true);
  }
}

void MainWindow::OpenFile()
{
  wxString      start_dir = wxGetCwd();
  wxFileDialog  dlg(this, "Choose a file", dlg_dir, dlg_file, "*.log", wxOPEN);

  int rc = dlg.ShowModal();
  
  dlg_file = dlg.GetFilename();
  dlg_dir  = dlg.GetDirectory();
  wxString  dlg_path = dlg.GetPath();

  wxSetWorkingDirectory(start_dir);
  dlg.Destroy();
  
  if (rc == wxID_OK)
    game->load(dlg_path.c_str());
  
  ActMenu();
}

void MainWindow::SaveFile()
{
  wxString      start_dir = wxGetCwd();
  wxFileDialog  dlg(this, "Choose a file", dlg_dir, dlg_file, "*.log", wxSAVE);

  int rc = dlg.ShowModal();

  dlg_file = dlg.GetFilename();
  dlg_dir  = dlg.GetDirectory();
  wxString  dlg_path = dlg.GetPath();

  dlg.Destroy();
  wxSetWorkingDirectory(start_dir);
  
  if (rc == wxID_OK)
    game->save(dlg_path.c_str());

  ActMenu();
}

void MainWindow::Undo(bool all)
{
  game->undo(all);
  ActMenu();
}

void MainWindow::Redo()
{
  game->redo();
  ActMenu();
}

void MainWindow::OnGameNew (wxCommandEvent& WXUNUSED(event))
{
  game->newGame();
  game->show();
}

void MainWindow::OnGameOpen (wxCommandEvent& WXUNUSED(event))
{
  OpenFile();
  game->show();
}

void MainWindow::OnGameSave (wxCommandEvent& WXUNUSED(event))
{
  SaveFile();
  game->show();
}

void MainWindow::OnGameBeginner (wxCommandEvent& WXUNUSED(event))
{
  game->changeLevel (MinesPerfect::BEGINNER);
  ActMenu();
  game->show();
}

void MainWindow::OnGameIntermediate (wxCommandEvent& WXUNUSED(event))
{
  game->changeLevel (MinesPerfect::INTERMEDIATE);
  ActMenu();
  game->show();
}

void MainWindow::OnGameExpert (wxCommandEvent& WXUNUSED(event))
{
  game->changeLevel (MinesPerfect::EXPERT);
  ActMenu();
  game->show();
}

void MainWindow::OnGameSelfdefined (wxCommandEvent& WXUNUSED(event))
{
  // level
  MinesPerfect::Level level = game->m_options->getLevel();
  level.nr = MinesPerfect::USER_DEFINED;
  
  // Dialog
  SelfdefinedDialog  dlg(this, &level);
  if (dlg.ShowModal() == wxID_OK)
    game->changeLevel(level);
  dlg.Destroy();
  
  ActMenu();
  game->show();
}

void MainWindow::OnGameOriginal (wxCommandEvent& WXUNUSED(event))
{
  game->changeModus (MinesPerfect::ORIGINAL);
  ActMenu();
  game->show();
}

void MainWindow::OnGameImmune (wxCommandEvent& WXUNUSED(event))
{
  game->changeModus (MinesPerfect::IMMUNE);
  ActMenu();
  game->show();
}

void MainWindow::OnGameLucky (wxCommandEvent& WXUNUSED(event))
{
  game->changeModus (MinesPerfect::LUCKY);
  ActMenu();
  game->show();
}

void MainWindow::OnGameHint (wxCommandEvent& WXUNUSED(event))
{
  game->changeModus (MinesPerfect::HINTS);
  ActMenu();
  game->show();
}

void MainWindow::OnGameStartup (wxCommandEvent& WXUNUSED(event))
{
  game->changeModus (MinesPerfect::STARTUP);
  ActMenu();
  game->show();
}

void MainWindow::OnGameMurphysLaw (wxCommandEvent& WXUNUSED(event))
{
  game->setMurphysLaw (!game->m_options->getMurphysLaw());
  ActMenu();
  game->show();
}

void MainWindow::OnGameBestTimes (wxCommandEvent& WXUNUSED(event))
{
  ShowBestTimesDialog dlg(this, game->m_options);
  dlg.ShowModal();
  dlg.Destroy();
}

void MainWindow::OnGameExit (wxCommandEvent& WXUNUSED(event))
{
  // Force the window to close
  Close(TRUE);
}

void MainWindow::OnBoard (wxCommandEvent& event)
{
  game->changeBoard(event.GetId() - MENU_BOARD_FIRST);
  ActMenu();
  game->show();
}

void MainWindow::OnHelpHint (wxCommandEvent& WXUNUSED(event))
{
  game->giveHint();
  game->show();
}

void MainWindow::OnHelpSolveOne (wxCommandEvent& WXUNUSED(event))
{
  game->solveOne();
  game->show();
}

void MainWindow::OnHelpSolveAll (wxCommandEvent& WXUNUSED(event))
{
}

void MainWindow::OnHelpSolveAll1 (wxCommandEvent& WXUNUSED(event))
{
  game->solveAll(1);
  game->show();
}

void MainWindow::OnHelpSolveAll2 (wxCommandEvent& WXUNUSED(event))
{
  game->solveAll(2);
  game->show();
}

void MainWindow::OnHelpSolveAll3 (wxCommandEvent& WXUNUSED(event))
{
  game->solveAll(3);
  game->show();
}

void MainWindow::OnHelpSolveAuto (wxCommandEvent& WXUNUSED(event))
{
}

void MainWindow::OnHelpSolveAuto0 (wxCommandEvent& WXUNUSED(event))
{
  game->changeSolveAuto(0);
  ActMenu();
  game->show();
}

void MainWindow::OnHelpSolveAuto1 (wxCommandEvent& WXUNUSED(event))
{
  game->changeSolveAuto(1);
  ActMenu();
  game->show();
}

void MainWindow::OnHelpSolveAuto2 (wxCommandEvent& WXUNUSED(event))
{
  game->changeSolveAuto(2);
  ActMenu();
  game->show();
}

void MainWindow::OnHelpSolveAuto3 (wxCommandEvent& WXUNUSED(event))
{
  game->changeSolveAuto(3);
  ActMenu();
  game->show();
}

void MainWindow::OnHelpMaxStage (wxCommandEvent& WXUNUSED(event))
{
}

void MainWindow::OnHelpMaxStage1 (wxCommandEvent& WXUNUSED(event))
{
  game->changeMaxStage(1);
  ActMenu();
  game->show();
}

void MainWindow::OnHelpMaxStage2 (wxCommandEvent& WXUNUSED(event))
{
  game->changeMaxStage(2);
  ActMenu();
  game->show();
}

void MainWindow::OnHelpMaxStage3 (wxCommandEvent& WXUNUSED(event))
{
  game->changeMaxStage(3);
  ActMenu();
  game->show();
}

void MainWindow::OnHelpShowMines (wxCommandEvent& WXUNUSED(event))
{
  game->setShowMines (!game->m_options->getShowMines());
  ActMenu();
  game->show();
}

void MainWindow::OnHelpUndo (wxCommandEvent& WXUNUSED(event))
{
  Undo(false);
  game->show();
}

void MainWindow::OnHelpUndoAll (wxCommandEvent& WXUNUSED(event))
{
  Undo(true);
  game->show();
}

void MainWindow::OnHelpRedo (wxCommandEvent& WXUNUSED(event))
{
  Redo();
  game->show();
}

void MainWindow::OnHelpContents (wxCommandEvent& WXUNUSED(event))
{
  // help_fname
  wxString  help_fname = wxGetCwd() + "/help/help.html";
  if (!wxFile::Exists (help_fname))
  {
    wxMessageDialog  dlg (this, wxString("File '") + help_fname + "' not found!",
                          "Error", wxOK | wxICON_ERROR);
    dlg.ShowModal();
    return;
  }

  StartBrowser (string("file"), string(help_fname.c_str()));
}

void MainWindow::OnHelpHomepage (wxCommandEvent& WXUNUSED(event))
{
  StartBrowser (string("http"), string("www.czeppi.de"));
}

void MainWindow::OnHelpAbout (wxCommandEvent& WXUNUSED(event))
{
  AboutDialog dlg(this);
  dlg.ShowModal();
}

void MainWindow::OnMouseEvent (wxMouseEvent& event)
{
  static MinesPerfect::Point  prev_pos(-1, -1);
  MinesPerfect::MouseEvent    mp_event;

  if (event.Moving())
    mp_event.m_type = MinesPerfect::MouseEvent::MOVE;
  else if (event.LeftDown())
    mp_event.m_type = MinesPerfect::MouseEvent::LEFT_DOWN;
  else if (event.RightDown())
    mp_event.m_type = MinesPerfect::MouseEvent::RIGHT_DOWN;
  else if (event.LeftUp())  
    mp_event.m_type = MinesPerfect::MouseEvent::LEFT_UP;
  else if (event.RightUp())  
    mp_event.m_type = MinesPerfect::MouseEvent::RIGHT_UP;
    
  mp_event.m_pos           = MinesPerfect::Point(event.GetX(), event.GetY());
  mp_event.m_prev_pos      = prev_pos;
  mp_event.m_left_is_down  = event.LeftIsDown();
  mp_event.m_right_is_down = event.RightIsDown();  
  prev_pos                 = mp_event.m_pos;
  
  game->onMouseEvent(mp_event);
  game->show();
}

void MainWindow::OnCloseWindow (wxCloseEvent& event)
{
  int x, y;

  GetPosition(&x, &y);
  game->m_options->setXPos(x);
  game->m_options->setYPos(y);
  game->m_options->saveIni();
  
  Destroy();
}

void MainWindow::OnPaintEvent (wxPaintEvent& event)
{
  wxPaintDC dc(this); // Zeile muss sein!!!
  
  game->setDirty();
  game->show();
}



