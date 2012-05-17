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

#ifndef APP_H
#define APP_H

#include <wx/app.h>

// The main Application
class MainApp: public wxApp
{
  public:
    bool OnInit();
    int  MainLoop();
    
  private:
    void showException       (const MinesPerfect::Exception&       exc) const;
    //void showAssertException (const MinesPerfect::AssertException& exc) const;
};


// The main Window
class MainWindow: public wxFrame
{
  public:
      MinesPerfect::GameCtrl*  game;  

      void ActMenu();
      void CreateBoardMenu();
      void OpenFile();
      void SaveFile();
      void Undo(bool all);
      void Redo();
      
      // Constructor
      MainWindow(const wxString &title, const wxPoint &pos, const wxSize &size);

      // Event handler (should *NOT* be virtual!)
      void OnGameNew          (wxCommandEvent &event);
      void OnGameOpen         (wxCommandEvent &event);
      void OnGameSave         (wxCommandEvent &event);
      void OnGameBeginner     (wxCommandEvent &event);
      void OnGameIntermediate (wxCommandEvent &event);
      void OnGameExpert       (wxCommandEvent &event);
      void OnGameSelfdefined  (wxCommandEvent &event);
      void OnGameOriginal     (wxCommandEvent &event);
      void OnGameImmune       (wxCommandEvent &event);
      void OnGameLucky        (wxCommandEvent &event);
      void OnGameHint         (wxCommandEvent &event);
      void OnGameStartup      (wxCommandEvent &event);
      void OnGameMurphysLaw   (wxCommandEvent &event);
      void OnGameBestTimes    (wxCommandEvent &event);
      void OnGameExit         (wxCommandEvent &event);

      void OnBoard          (wxCommandEvent &event);
      
      void OnHelpHint       (wxCommandEvent &event);
      void OnHelpSolveOne   (wxCommandEvent &event);
      void OnHelpSolveAll   (wxCommandEvent &event);
      void OnHelpSolveAll1  (wxCommandEvent &event);
      void OnHelpSolveAll2  (wxCommandEvent &event);
      void OnHelpSolveAll3  (wxCommandEvent &event);
      void OnHelpSolveAuto  (wxCommandEvent &event);
      void OnHelpSolveAuto0 (wxCommandEvent &event);
      void OnHelpSolveAuto1 (wxCommandEvent &event);
      void OnHelpSolveAuto2 (wxCommandEvent &event);
      void OnHelpSolveAuto3 (wxCommandEvent &event);
      void OnHelpMaxStage   (wxCommandEvent &event);
      void OnHelpMaxStage1  (wxCommandEvent &event);
      void OnHelpMaxStage2  (wxCommandEvent &event);
      void OnHelpMaxStage3  (wxCommandEvent &event);
      void OnHelpShowMines  (wxCommandEvent &event);
      void OnHelpUndo       (wxCommandEvent &event);
      void OnHelpRedo       (wxCommandEvent &event);
      void OnHelpUndoAll    (wxCommandEvent &event);
      void OnHelpContents   (wxCommandEvent &event);
      void OnHelpHomepage   (wxCommandEvent &event);
      void OnHelpAbout      (wxCommandEvent &event);
      
      void OnMouseEvent     (wxMouseEvent& event);
      void OnCloseWindow    (wxCloseEvent& event);
      void OnPaintEvent     (wxPaintEvent& event);

  private:
      wxMenu    *GameMenu;
      wxMenu    *BoardMenu;
      wxMenu    *HelpMenu;
      wxMenu    *SolveAllMenu;
      wxMenu    *SolveAutoMenu;
      wxMenu    *MaxStageMenu;
      wxMenuBar *MenuBar;
      wxString  dlg_dir;
      wxString  dlg_file;
      
      // Any class wishing to process wxWindows events must use this macro
      DECLARE_EVENT_TABLE()
};    
      
#endif
      
