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

#ifndef OPTIONS_H
#define OPTIONS_H

#include <time.h>
#include <vector>
#include <string>
#include <fstream>
#include <list>

using namespace std;

namespace MinesPerfect {


//------------------------------------------------------------------------------
const int  MAX_COLS  = 78;
const int  MAX_ROWS  = 44;
const int  NAME_LEN  = 16;
const int  MAX_STAGE = 3;
const int  MAX_MSECS = 999000;
const int  MAX_SECS  = 999;

enum LevelNr { BEGINNER = 0, INTERMEDIATE, EXPERT, USER_DEFINED };
enum Modus    { ORIGINAL = 0, IMMUNE, LUCKY, HINTS, STARTUP };

struct Level
{
  LevelNr  nr;
  int       height;
  int       width;
  int       deep;
  int       num_mines;
  int       num_wholes;

  Level() : nr(BEGINNER), height(1), width(1), deep(0),
            num_mines(0), num_wholes(-1) {;}
};

typedef int  BoardNr;
typedef int  StageNr;

//******************************************************************************
  struct Record
//------------------------------------------------------------------------------
{
  string  name;
  int     time;
  bool    certified_board;
  bool    was_send;
  time_t  date;

  void reset() { name = "Anonymous"; time = MAX_MSECS; date = 0;
                 certified_board = false; was_send = false; }
  Record() { reset(); }
};

//******************************************************************************
  struct StringChecker //: public string
//------------------------------------------------------------------------------
{
  public:

    virtual bool isValidChar   (char          ch,  string* errtext = 0) = 0;
    bool         isValidString (const string& str, string* errtext = 0);

  protected:

    virtual bool isValidStringBase (const string& str, string* errtext = 0) = 0;
};

//******************************************************************************
  class User
//------------------------------------------------------------------------------
{
  public:

  struct NameChecker : public StringChecker
  {
      bool isValidChar       (char ch,           string* errtext = 0);
      bool isValidStringBase (const string& str, string* errtext = 0);
  };

  struct PasswordChecker : public StringChecker
  {
      bool isValidChar       (char ch,           string* errtext = 0);
      bool isValidStringBase (const string& str, string* errtext = 0);
  };

  string name;
  string password;
};

//******************************************************************************
  struct BoardType
//------------------------------------------------------------------------------
{
  string  name;
  bool    enabled;
  Record  records[3];

  BoardType() : enabled(true) {;}
  BoardType (string n) : name(n), enabled(true) {;}
  
  void deleteRecord(int nr) { if (0 <= nr && nr <= 2) records[nr].reset(); }
  void deleteRecords();
  int  getChecksum(int nr, int version);
};

//******************************************************************************
  class Options
//------------------------------------------------------------------------------
{
private:

  // persistente Daten
  BoardNr            board_nr;
  Level              level;
  Modus              modus;
  bool               murphys_law;
  int                xpos;          // Horizontal Position des Fensters
  int                ypos;          // Vertikale Position des Fensters
  vector<BoardType>  board_types;   // Namen und Rekorde der Bordtypen
  vector<User>       users;

  // nur im Hauptspeicher
  StageNr            auto_stage;
  StageNr            max_stage;
  bool               with_sound;
  bool               with_speedo;
  bool               show_mines;

public:
  Options();
  ~Options(){}

  bool   loadIni();
  void   saveIni();
  void   deleteRecords();
  void   addBoard (string n) { board_types.push_back (BoardType (n)); }
  void   actBoards();
  
  // set
  bool   setBoardNr    (BoardNr nr);
  bool   setLevel      (const Level& lvl);
  bool   setModus      (Modus mod);
  void   setMurphysLaw (bool valid)   { murphys_law = valid;}
  void   setXPos       (int  x)       { xpos        = x; }
  void   setYPos       (int  y)       { ypos        = y; }
  void   setWithSound  (bool  sound)  { with_sound  = sound; }
  void   setWithSpeedo (bool  speedo) { with_speedo = speedo; }
  void   setAutoStage  (StageNr  stg) { auto_stage  = stg; }
  void   setMaxStage   (StageNr  stg) { max_stage   = stg; }
  void   setShowMines  (bool  show)   { show_mines  = show; }
  void   addUser       (const User& new_user) { users.push_back(new_user); }
  void   renameUser    (const string& old_name, const string& new_name); 
  void   setRecord     (int nr, string name, int time, bool certified_board);


  // get
  LevelNr     getLevelNr()    const { return level.nr; }
  int         getHeight()     const { return level.height; }
  int         getWidth()      const { return level.width; }
  int         getDeep()       const { return level.deep; }
  int         getNumMines()   const { return level.num_mines; }
  int         getNumWholes()  const { return level.num_wholes; }
  Modus       getModus()      const { return modus; }
  bool        getMurphysLaw() const { return murphys_law; }
  BoardNr     getBoardNr()    const { return board_nr; }
  int         getNumBoards()  const { return board_types.size(); }
  Level       getLevel()      const { return level; }
  int         getXPos()       const { return xpos; }
  int         getYPos()       const { return ypos; }
  StageNr     getAutoStage()  const { return auto_stage; }
  StageNr     getMaxStage()   const { return max_stage; }
  bool        getShowMines()  const { return show_mines; }
  bool        getWithSpeedo() const { return with_speedo; }
  bool        getWithSound()  const { return with_sound; }
  int         getNumUsers()   const { return users.size(); }
  User        getUser(int nr) const { return users[nr]; }

  const string  getBoardName (BoardNr nr) const;
  
  BoardType*  getBoardType (BoardNr nr);

  const string  getBoardName (void) const {
    return getBoardName (board_nr);
  }

  BoardType*  getBoardType (void) {
    return getBoardType (board_nr);
  }

  const Record&  getRecord (int nr) const {
    return board_types[board_nr].records[nr];
  }

  void getUserlist(vector<string>& user_list);

  int findBoardNr (const string& name);
};


} // namespace MinesPerfect

#endif
