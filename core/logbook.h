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

#ifndef LOGBOOK_H
#define LOGBOOK_H

#include <string>
#include <fstream> // fuer vector etc.
#include <vector>
#include <time.h>
#include "utils.h"
#include "options.h"

namespace MinesPerfect {


/* Probleme mit Konstanten in Headerfiles
// stat.
const string LOG_RAND_SEQ    = "rand_seq";
const string LOG_BOARD_NR    = "board_nr";
const string LOG_LEVEL       = "level";
const string LOG_HEIGHT      = "height";
const string LOG_WIDTH       = "width";
const string LOG_DEEP        = "deep";
const string LOG_NUM_MINES   = "num_mines";
const string LOG_NUM_WHOLES  = "num_wholes";

// stat. und dyn.
const string LOG_MODUS       = "modus";
const string LOG_MURPHYS_LAW = "murphys_law";
const string LOG_AUTO_STAGE  = "auto_stage";
const string LOG_MAX_STAGE   = "max_stage";
const string LOG_SHOW_MINES  = "show_mines";

// dyn.
const string LOG_FLAG        = "flag";
const string LOG_OPEN        = "open";
const string LOG_SOLVE_ONE   = "solve_one";
const string LOG_SOLVE_ALL   = "solve_all";
const string LOG_OUT_OF_TIME = "out_of_time";
*/

// stat.
#define LOG_VERSION      "version"
#define LOG_VARIANT      "variant"
#define LOG_RAND_SEQ     "rand_seq"
#define LOG_BOARD_NAME   "board_name"
#define LOG_BOARD_CHKSUM "board_chksum"
#define LOG_LEVEL        "level"
#define LOG_HEIGHT       "height"
#define LOG_WIDTH        "width"
#define LOG_DEEP         "deep"
#define LOG_NUM_MINES    "num_mines"
#define LOG_NUM_WHOLES   "num_wholes"

// stat. und dyn.
#define LOG_MODUS       "modus"
#define LOG_MURPHYS_LAW "murphys_law"
#define LOG_AUTO_STAGE  "auto_stage"
#define LOG_MAX_STAGE   "max_stage"
#define LOG_SHOW_MINES  "show_mines"

// dyn.
#define LOG_FLAG        "flag"
#define LOG_OPEN        "open"
#define LOG_SOLVE_ONE   "solve_one"
#define LOG_SOLVE_ALL   "solve_all"
#define LOG_HINT        "hint"
#define LOG_OUT_OF_TIME "out_of_time"
#define LOG_START_TIMER "start_timer"

#define LOGS            "logs"

//******************************************************************************
class LogException : public Exception
//------------------------------------------------------------------------------
{
public:

   LogException (const std::string& t) : Exception (t) {}
};

//******************************************************************************
struct Log
//------------------------------------------------------------------------------
{
  long         time1;
  string       name;
  int          val;  // muss mind. 32 Bit lang sein (rand_seq)
  bool         valid;

  Log (void) : time1(0), name(""), val(0), valid(true) {;}
  Log (string n, int v)
  : time1(0), name(n), val(v), valid(true) {;}

  bool isComputerLog() const { return name == LOG_OUT_OF_TIME 
                                      || name == LOG_START_TIMER; }
  void write (const string& fname) const;


  enum Type // 2-Bit
  { 
    TYPE_END = 0, TYPE_OPEN, TYPE_FLAG, TYPE_MISC 
  };

  enum Misc // 6-Bit
  { 
    MISC_MODUS       = 0, 
    MISC_MURPHYS_LAW = MISC_MODUS       + 5, // 0- 4
    MISC_HINT        = MISC_MURPHYS_LAW + 2, // 0 - 1
    MISC_SOLVE_ONE,   
    MISC_SOLVE_ALL, 
    MISC_AUTO_STAGE  = MISC_SOLVE_ALL   + 4, // 1 - 3
    MISC_MAX_STAGE   = MISC_AUTO_STAGE  + 4, // 0 - 3
    MISC_SHOW_MINES  = MISC_MAX_STAGE   + 4, // 0 - 3
    MISC_OUT_OF_TIME = MISC_SHOW_MINES  + 2, // 0 - 1
    MISC_START_TIMER 
  };

  int  toInt() const;
  Log (int num);
};

//******************************************************************************
class Logbook
//------------------------------------------------------------------------------
{
  private:

    clock_t      clock0;
    Options      options;
    vector<Log>  logs;
    uint32       rand_seq;
    bool         is_playing;
    unsigned     play_index;  // akt. Index zum Schreiben und Lesen
    unsigned     cur_index;   // Index ab dem neue Eintraege hinzugefuegt werden
                              // sollen.
                              // ! Zeigt immer auf einen gueltigen Log oder auf
                              // ! den ersten
    void    writeOptions (const string& fname) const;
    char    Int6ToChar (int num) const;
    int     CharToInt6 (char ch) const;

  public:

    Logbook (uint32 ran, const Options&  opt);

    const Log&      getPlayLog() const;
    uint32          getRandSeq() const { return rand_seq;    }
    const Options&  getOptions() const { return options;     }
    const unsigned  getNumLogs() const { return logs.size(); }
    bool            isPlaying()  const { return is_playing;  }
    bool            eop()        const { return play_index >= cur_index; }
                                 // eop = end-of-play

    void            write        (const string& fname);
    void            read         (const string& fname);
    void            startPlaying ();
    void            stopPlaying  () { is_playing = false; }
    void            operator>>   (Log& l);
    void            operator<<   (const Log& l);
    bool            undo         (bool all);
    bool            redo         (Log& log);
    bool            invalidateLast ();
//    string          getLogsAsStr () const;
//    void            setLogs      (const string& text);
    string          exportStr () const;
    void            importStr (const string& text);
};


} // namespace MinesPerfect

#endif
