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

#include <map>
#include <string>
#include <cstdlib>

#ifdef LINUX
  #define stricmp strcasecmp
#endif

using namespace std;

#include "options.h"
#include "api.h"

namespace MinesPerfect
{
  namespace Glob
  {
    extern int VERSION;
    extern int MOD_VAL;
  }
}

using namespace MinesPerfect;

string TITLE = "MinesPerfect";
string USERS = "users";

const char INI_FNAME[] = "mineperf.ini";


//******************************************************************************
  void ModChecksum(int& chksum, int val)
//------------------------------------------------------------------------------
{
  chksum += val;
  chksum *= Glob::MOD_VAL;
  chksum %= 10000;    
}

//******************************************************************************
  bool StringChecker::isValidString (const string& str, string* errtext)
//------------------------------------------------------------------------------
{
  for (unsigned i = 0; i < str.size(); ++i)
    if (!isValidChar(str[i], errtext))
      return false;

  return isValidStringBase (str, errtext);
}

//******************************************************************************
  bool User::NameChecker::isValidChar (char ch, string* errtext)
//------------------------------------------------------------------------------
{
  if (isalnum(ch) || ch == '-' || ch == '_')
  {
    return true; // valid
  }
  else
  {
    if (errtext != 0)
      *errtext = "should contain only alphabetic, numeric, '-' and '_' characters";

    return false; // invalid
  }
}

//******************************************************************************
  bool User::NameChecker::isValidStringBase (const string& str, string* errtext)
//------------------------------------------------------------------------------
{
  string err = "";

  if (str.size() < 3)
    err = "should contain at least 3 characters.";
  else if (str.size() > 32)
    err = "should contain not more than 32 characters.";
  else if (!isalpha(str[0]))
    err = "should begin with a alphabetic character.";

  if (errtext != 0)
    *errtext = err;

  return err == "";
}

//******************************************************************************
  bool User::PasswordChecker::isValidChar (char ch, string* errtext)
//------------------------------------------------------------------------------
{
  if (isalnum(ch) || ch == '-' || ch == '@' || ch == '.')
  {
     return true; // valid
  }
  else
  {
    if (errtext != 0)
      *errtext = "should contain only alphabetic, numeric, '-', '.' and '@' characters";

    return false; // invalid
  }
}

//******************************************************************************
  bool User::PasswordChecker::isValidStringBase (const string& str, string* errtext)
//------------------------------------------------------------------------------
{
  string err = "";

  if (str.size() < 5)
    err = "should contain at least 5 characters.";
  else if (str.size() > 32)
    err = "should contain not more than 32 characters.";

  if (errtext != 0)
    *errtext = err;

  return err == "";
}

//******************************************************************************
  void BoardType::deleteRecords()
//------------------------------------------------------------------------------
{
  records[0].reset();
  records[1].reset();
  records[2].reset();
}

//******************************************************************************
  int BoardType::getChecksum(int nr, int version)
//------------------------------------------------------------------------------
{
  ASSERT (0 <= nr && nr <= 2);

  int chksum = 0;
  
  for (unsigned i = 0; i < records[nr].name.size(); ++i)
    if (records[nr].name[i] != ' ')
      ModChecksum (chksum, tolower(records[nr].name[i]));
  ModChecksum (chksum, ' ');

  if (version >= 140)
  {
    ModChecksum (chksum, records[nr].time / 100000 % 10 + '0');
    ModChecksum (chksum, records[nr].time /  10000 % 10 + '0');
    ModChecksum (chksum, records[nr].time /   1000 % 10 + '0');
  }
  ModChecksum (chksum, records[nr].time / 100 % 10 + '0');
  ModChecksum (chksum, records[nr].time /  10 % 10 + '0');
  ModChecksum (chksum, records[nr].time /   1 % 10 + '0');
  ModChecksum (chksum, ' ');
  
  ModChecksum (chksum, nr + '0');
  ModChecksum (chksum, ' ');
  
  ModChecksum (chksum, records[nr].certified_board ? '1' : '0');
  ModChecksum (chksum, ' ');
  
  { // visual studio
  for (unsigned i = 0; i < name.size(); ++i)
    if (name[i] != ' ')
      ModChecksum (chksum, tolower(name[i]));
  } // visual studio
  
  return chksum;
}

//******************************************************************************
  Options::Options()
//------------------------------------------------------------------------------
{
  if (!loadIni())
  {
    // default Werte
    Level lvl;

    lvl.nr         = BEGINNER;
    lvl.height     = 8;
    lvl.width      = 8;
    lvl.deep       = 0;
    lvl.num_mines  = 10;
    lvl.num_wholes = -1;

    board_types.clear();
    board_types.push_back (BoardType ("Square"));
    board_types.push_back (BoardType ("Hexagon"));
    board_types.push_back (BoardType ("Triangle"));
    board_types.push_back (BoardType ("3d-Grid"));

    setLevel (lvl);
    setBoardNr (0);  // muss nach board_types init. werden
    setModus (LUCKY);
    setXPos (100);
    setYPos (100);
    setMurphysLaw (true);
  }

  actBoards();

  setAutoStage  (0);
  setMaxStage   (MAX_STAGE); // -1; // != MAX_STAGE
  setShowMines  (false);
  setWithSound  (false);
  setWithSpeedo (false);
}

//******************************************************************************
  void Options::deleteRecords()
//------------------------------------------------------------------------------
{
  board_types[board_nr].deleteRecords();

  saveIni();
}

//******************************************************************************
  bool Options::setBoardNr (BoardNr nr)
//------------------------------------------------------------------------------
{
  if (nr < 0 || (BoardNr) board_types.size() <= nr)
    return false;

  board_nr = nr;
  return true;
}

//******************************************************************************
  bool Options::setLevel (const Level& lvl)
//------------------------------------------------------------------------------
{
  if (lvl.nr != BEGINNER && lvl.nr != INTERMEDIATE
  &&  lvl.nr != EXPERT   && lvl.nr != USER_DEFINED)
    return false;

  if (lvl.height     <  1 ||  999 < lvl.height
  ||  lvl.width      <  1 ||  999 < lvl.width
  ||  lvl.deep       <  0 ||  999 < lvl.deep
  ||  lvl.num_mines  <  0 || 9999 < lvl.num_mines
  ||  lvl.num_wholes < -1 || 9999 < lvl.num_wholes)
    return false;

  level = lvl;
  return true;
}

//******************************************************************************
  bool Options::setModus (Modus mod)
//------------------------------------------------------------------------------
{
  if (mod != ORIGINAL && mod != IMMUNE && mod != LUCKY
  &&  mod != HINTS    && mod != STARTUP)
    return false;

  modus = mod;
  return true;
}

//******************************************************************************
  void Options::setRecord (int nr, string name, int time, bool certified_board)
//------------------------------------------------------------------------------
{
  time_t now;

  if (0 <= nr && nr < 3)
  {
    board_types[board_nr].records[nr].name            = name;
    board_types[board_nr].records[nr].time            = time;
    board_types[board_nr].records[nr].date            = ::time(&now);
    board_types[board_nr].records[nr].certified_board = certified_board;
    board_types[board_nr].records[nr].was_send        = false;
  }
}


//******************************************************************************
  bool Options::loadIni()
//------------------------------------------------------------------------------
{
  ifstream           in (INI_FNAME);
  Level              lvl;
  Modus              mod = ORIGINAL;  // default
  BoardNr            bnr = 0;         // default
  int                val_int;
  string             line, val_str, key, m_law;
  string::size_type  p1, p2, p3;
  vector<int>        chksums[3];
  int                version = 0;

  enum { E_BASE, E_USER, E_BOARD } input_mode = E_BASE;

  if (in == 0)
    return false;

  // 1. Zeile testen
  getline (in, line);
  if (line[0] != '['
  ||  line.substr (1, TITLE.length()) != TITLE)
    return false;

  version = atoi (line.substr(1 + TITLE.length()).c_str());
  if (version < 100 || 999 < version)
    return false;

  // Rest einlesen
  while (!in.eof())
  {
    getline (in, line);

    p1 = line.find_first_not_of (" \t");
    if (p1 == string::npos)
      continue;

    if (line[p1] == '[')
    {
      string users_str = "[" + USERS + "]";
      if (line.substr (0, users_str.length()) != users_str)
        return false;

      input_mode = E_USER;
    }
    else if (line[p1] == '<')
    {
      // neuer Board-Typ
      p2 = line.find ('>');
      if (p2 == string::npos || p2 <= p1 + 1
      ||  line.find_first_not_of (" \t", p2 + 1) != string::npos)
        return false;

      input_mode = E_BOARD;

      board_types.push_back (BoardType());
      board_types.back().name = line.substr (p1 + 1, p2 - p1 - 1);
    }
    else
    {
      p2 = line.find ('=', p1 + 1);
      if (p2 == string::npos)
        return false;

      key = line.substr (p1, p2 - p1);

      p3 = line.find_first_of (" \t", p2 + 1);
      if (p3 != string::npos)
        line[p3] = 0;

      val_str = line.substr (p2 + 1);
      val_int = atoi (val_str.c_str());

      // if (board_types.size() == 0)
      if (input_mode == E_BASE)
      {
        if      (key == "Xpos"      )  setXPos (val_int);
        else if (key == "Ypos"      )  setYPos (val_int);
        else if (key == "Level"     )  lvl.nr         = (LevelNr) val_int;
        else if (key == "Height"    )  lvl.height     = val_int;
        else if (key == "Width"     )  lvl.width      = val_int;
        else if (key == "Deep"      )  lvl.deep       = val_int;
        else if (key == "Mines"     )  lvl.num_mines  = val_int;
        else if (key == "Wholes"    )  lvl.num_wholes = val_int;
        else if (key == "Modus"     )  mod            = (Modus) val_int;
        else if (key == "BoardNr"   )  bnr            = (BoardNr) val_int;
        else if (key == "MurphysLaw")  m_law          = val_str;
        else                           return false;
      }
      else if (input_mode == E_USER)
      {
        User user;

        user.name     = key;
        user.password = val_str;

        users.push_back(user);
      }
      else
      {
        if      (key == "Name0")  board_types.back().records[0].name = val_str;
        else if (key == "Name1")  board_types.back().records[1].name = val_str;
        else if (key == "Name2")  board_types.back().records[2].name = val_str;
        
        else if (key == "Time0")  board_types.back().records[0].time = val_int;
        else if (key == "Time1")  board_types.back().records[1].time = val_int;
        else if (key == "Time2")  board_types.back().records[2].time = val_int;
        
        else if (key == "Date0")  board_types.back().records[0].date = val_int;
        else if (key == "Date1")  board_types.back().records[1].date = val_int;
        else if (key == "Date2")  board_types.back().records[2].date = val_int;
        
        else if (key == "CertifiedBoard0")  
          board_types.back().records[0].certified_board = (val_int == 1);
        else if (key == "CertifiedBoard1")  
          board_types.back().records[1].certified_board = (val_int == 1);
        else if (key == "CertifiedBoard2")  
          board_types.back().records[2].certified_board = (val_int == 1);
        
        else if (key == "Checksum0")  chksums[0].push_back(val_int);
        else if (key == "Checksum1")  chksums[1].push_back(val_int);
        else if (key == "Checksum2")  chksums[2].push_back(val_int);

        else if (key == "WasSend0")  
          board_types.back().records[0].was_send = (val_int == 1);
        else if (key == "WasSend1")  
          board_types.back().records[1].was_send = (val_int == 1);
        else if (key == "WasSend2")  
          board_types.back().records[2].was_send = (val_int == 1);
        
        else                          return false;
      }
    }
  }

  // Werte testen
  for (int i = 0; i < (int) board_types.size(); i++)
  {
    int max_time = (version < 140) ? MAX_SECS : MAX_MSECS;

    if (board_types[i].records[0].time < 0
    ||  board_types[i].records[0].time > max_time
    ||  board_types[i].records[1].time < 0
    ||  board_types[i].records[1].time > max_time
    ||  board_types[i].records[2].time < 0
    ||  board_types[i].records[2].time > max_time)
      return false;
  }

  if      (m_law == "on")   setMurphysLaw (true);
  else if (m_law == "off")  setMurphysLaw (false);
  else                      return false;

  if (!setLevel (lvl) || !setModus (mod) || !setBoardNr (bnr))
    return false;

  { // visual studio
  for (int i = 0; i < (int) board_types.size(); i++)
  {
    for (int j = 0; j < 3; j++)
    {
      if (board_types[i].getChecksum(j,version) != chksums[j][i])
        board_types[i].records[j].reset();

      if (version < 140 )
        board_types[i].records[j].time *= 1000; 
    }
  }
  } // visual studio
  return true;
}

//******************************************************************************
  void Options::saveIni()
//------------------------------------------------------------------------------
{
  // speichern
  ofstream  out(INI_FNAME);
  unsigned  i;

  out << "[" << TITLE << " " << Glob::VERSION << "]"       << '\n'
      << "Level="      << getLevel().nr                    << '\n'
      << "Modus="      << getModus()                       << '\n'
      << "Height="     << getLevel().height                << '\n'
      << "Width="      << getLevel().width                 << '\n'
      << "Deep="       << getLevel().deep                  << '\n'
      << "Mines="      << getLevel().num_mines             << '\n'
      << "Wholes="     << getLevel().num_wholes            << '\n'
      << "Xpos="       << getXPos()                        << '\n'
      << "Ypos="       << getYPos()                        << '\n'
      << "BoardNr="    << getBoardNr()                     << '\n'
      << "MurphysLaw=" << (getMurphysLaw() ? "on" : "off") << '\n';


  out << '\n';
  out << "[" << USERS << "]\n";
  for (i = 0; i < users.size(); ++i)
  {
    out << users[i].name << "=" << users[i].password << '\n';
  }

  for (i = 0; i < board_types.size(); ++i)
  {
    out << '\n';
    out << '<'           << board_types[i].name            << ">\n";
    out << "Time0="      << board_types[i].records[0].time << '\n';
    out << "Date0="      << board_types[i].records[0].date << '\n';
    out << "Name0="      << board_types[i].records[0].name << '\n';
    out << "CertifiedBoard0=" 
                         << (board_types[i].records[0].certified_board ? 1 : 0) 
                         << '\n';
    out << "Checksum0="  << board_types[i].getChecksum(0,Glob::VERSION) << '\n';
    out << "WasSend0="   << board_types[i].records[0].was_send << '\n';
    
    out << "Time1="      << board_types[i].records[1].time << '\n';
    out << "Date1="      << board_types[i].records[1].date << '\n';
    out << "Name1="      << board_types[i].records[1].name << '\n';
    out << "CertifiedBoard1=" 
                         << (board_types[i].records[1].certified_board ? 1 : 0) 
                         << '\n';
    out << "Checksum1="  << board_types[i].getChecksum(1,Glob::VERSION) << '\n';
    out << "WasSend1="   << board_types[i].records[1].was_send << '\n';
    
    out << "Time2="      << board_types[i].records[2].time << '\n';
    out << "Date2="      << board_types[i].records[2].date << '\n';
    out << "Name2="      << board_types[i].records[2].name << '\n';
    out << "CertifiedBoard2=" 
                         << (board_types[i].records[2].certified_board ? 1 : 0) 
                         << '\n';
    out << "Checksum2="  << board_types[i].getChecksum(2,Glob::VERSION) << '\n';
    out << "WasSend2="   << board_types[i].records[2].was_send << '\n';
  }
}

//******************************************************************************
  const string Options::getBoardName (BoardNr nr) const
//------------------------------------------------------------------------------
{
  ASSERT (0 <= nr && nr < (int) board_types.size());

  return board_types[nr].name;
}

//******************************************************************************
  BoardType* Options::getBoardType (BoardNr nr)
//------------------------------------------------------------------------------
{
  ASSERT (0 <= nr && nr < (int) board_types.size());
    
  return &board_types[nr];
}

//******************************************************************************
  void Options::actBoards()
//------------------------------------------------------------------------------
{
  // fnames: alle Dateien die mit .txt und mit .bmp existieren
  vector<string>  fnames;
  vector<string>  bmp_files;

  FindFiles (bmp_files, "./boards/*.txt"); 

  for (vector<string>::const_iterator f =  bmp_files.begin();
                                      f != bmp_files.end(); ++f)
  {
    string  fname = f->substr (0, f->find ("."));

    if (!FileExist (string("./boards/") + fname + ".bmp"))
      continue;

    fnames.push_back (fname);
  }

  // new_board_types
  vector<BoardType>                          new_board_types;
  typedef vector<BoardType>::const_iterator  BT_CI;

  for (BT_CI  b =  board_types.begin(); b != board_types.end(); ++b)
  {
    // f
    vector<string>::iterator  f;

    for (f = fnames.begin(); f != fnames.end(); ++f)
      if (stricmp (f->c_str(), b->name.c_str()) == 0)  // Datei gefunden
        break;

    // new_board_types
    //   ex. Datei?
    if (f != fnames.end())
    {
      new_board_types.push_back (*b);
      new_board_types.back().enabled = true;  // uebernehmen
      new_board_types.back().name = *f;  // Gross-/Kleinschreibung von
                                         // Datei uebernehemen
      fnames.erase (f);
    }
    // Standardboard?
    else if (stricmp (b->name.c_str(), "square")   == 0
         ||  stricmp (b->name.c_str(), "hexagon")  == 0
         ||  stricmp (b->name.c_str(), "triangle") == 0
         ||  stricmp (b->name.c_str(), "3d-grid")  == 0)
    {
      new_board_types.push_back (*b);
      new_board_types.back().enabled = true;  // uebernehmen
    }
    // Existiert Rekord?
    else if (b->records[0].time < MAX_MSECS
         ||  b->records[1].time < MAX_MSECS
         ||  b->records[2].time < MAX_MSECS)
    {
      new_board_types.push_back (*b);
      new_board_types.back().enabled = false;  // eingeschraenkt uebernehmen
    }
    // kein Rekord.
    else
    {
      continue;  // nicht uebernehmen
    }
  }

  // board_types (veralteten geloescht)
  board_types = new_board_types;

  // neuen Boards hinzufuegen
  { // visual studio
  for (vector<string>::const_iterator f =  fnames.begin();
                                      f != fnames.end(); f++)
  {
    addBoard (*f);
  }
  } // visual studio
}

//******************************************************************************
  void Options::getUserlist(vector<string>& user_list)
//------------------------------------------------------------------------------
// create a list with usernames in the order of the recorddate
{
  // user_map
  map<string,Record*> user_map;

  for (int b = 0; b < getNumBoards(); ++b)
  {
    BoardType*  btype  = getBoardType(b);
    int         ntypes = sizeof(btype->records) / sizeof(*btype->records);

    for (int r = 0; r < ntypes; ++r)
    {
      Record* rec = &btype->records[r];
      string  key = Lower(rec->name);

      map<string,Record*>::iterator ui = user_map.find(key);

      if (ui == user_map.end()) 
        user_map[key] = rec;
      else if (ui->second->date > rec->date)
        ui->second = rec;
    }
  }

  // date_map
  multimap<int,Record*> date_map;

  map<string,Record*>::const_iterator  ui;
  for (ui = user_map.begin(); ui != user_map.end(); ++ui)
    date_map.insert( make_pair(ui->second->date,ui->second) );

  // user_list
  user_list.clear();

  multimap<int,Record*>::reverse_iterator  di;
  for (di = date_map.rbegin(); di != date_map.rend(); ++di)
  {
    string             name = di->second->name;
    User::NameChecker  checker;

    if (checker.isValidString(name))
      user_list.push_back(name);
  }
}

//******************************************************************************
  void Options::renameUser (const string& old_name, const string& new_name)
//------------------------------------------------------------------------------
{
  for (int b = 0; b < getNumBoards(); ++b)
  {
    BoardType*  btype  = getBoardType(b);
    int         ntypes = sizeof(btype->records) / sizeof(*btype->records);

    for (int r = 0; r < ntypes; ++r)
    {
      Record* rec = &btype->records[r];

      if (Lower(rec->name) == Lower(old_name))
        rec->name = new_name;
    }
  }
}

//******************************************************************************
  int Options::findBoardNr (const string& name)
//------------------------------------------------------------------------------
{
  for (int b = 0; b < getNumBoards(); ++b)
  {
    if (Lower (getBoardName(b)) == Lower (name))
      return b;
  }
          
  return -1;
}