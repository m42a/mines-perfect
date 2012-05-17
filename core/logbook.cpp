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

//#include <Forms.hpp>  // g++: Exception

#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>

#ifdef VISUAL_CPP
  #include <minmax.h> // visual studio (max)
#endif

using namespace std;

#include "logbook.h"
#include "api.h"  // Exception

namespace MinesPerfect // wegen Compiler
{
  namespace Glob
  {
    extern  int   VERSION;
    extern  bool  log_on;  // Erstellt 'auto.log'-Datei (zum Fehlerreproduzieren)
  }
}

using namespace MinesPerfect;

const char*  AUTO_LOG_FNAME = "auto.log";

//******************************************************************************
void Log::write (const string& fname) const
//------------------------------------------------------------------------------
{
  ofstream  out (fname.c_str(), ios::out | ios::app);

  if (!out)
    throw LogException (("Log::write(): " + fname
                        + " couldn't open.").c_str());

  out << time1 << " " << name << " " << val << (valid ? "" : "!") << "\n";
}

//******************************************************************************
int Log::toInt() const
//------------------------------------------------------------------------------
{
  if (!valid)
    return 0;

  int misc;

  if      (name == LOG_FLAG)         return TYPE_FLAG + (val << 2);
  else if (name == LOG_OPEN)         return TYPE_OPEN + (val << 2);
  else if (name == LOG_MODUS)        misc = MISC_MODUS + val;
  else if (name == LOG_MURPHYS_LAW)  misc = MISC_MURPHYS_LAW + val;
  else if (name == LOG_HINT)         misc = MISC_HINT;
  else if (name == LOG_SOLVE_ONE)    misc = MISC_SOLVE_ONE;
  else if (name == LOG_SOLVE_ALL)    misc = MISC_SOLVE_ALL + val;
  else if (name == LOG_AUTO_STAGE)   misc = MISC_AUTO_STAGE + val;
  else if (name == LOG_MAX_STAGE)    misc = MISC_MAX_STAGE + val;
  else if (name == LOG_SHOW_MINES)   misc = MISC_SHOW_MINES;
  else if (name == LOG_OUT_OF_TIME)  misc = MISC_OUT_OF_TIME;
  else if (name == LOG_START_TIMER)  misc = MISC_START_TIMER;
  else                               return 0;

  return TYPE_MISC + (misc << 2);
}

//******************************************************************************
Log::Log (int num)
//------------------------------------------------------------------------------
{
  valid = true;
  time1 = 0;

  // val, name
  Type type = static_cast<Type> (num & 3);
  num >>= 2;

  if      (type == TYPE_FLAG)  { name = LOG_FLAG; val = num; }
  else if (type == TYPE_OPEN)  { name = LOG_OPEN; val = num; }
  else if (type == TYPE_MISC)
  {
    Misc misc = static_cast<Misc> (num);

    if      (misc == MISC_MODUS + 0)        { name = LOG_MODUS;       val = 0; }
    else if (misc == MISC_MODUS + 1)        { name = LOG_MODUS;       val = 1; }
    else if (misc == MISC_MODUS + 2)        { name = LOG_MODUS;       val = 2; }
    else if (misc == MISC_MODUS + 3)        { name = LOG_MODUS;       val = 3; }
    else if (misc == MISC_MODUS + 4)        { name = LOG_MODUS;       val = 4; }

    else if (misc == MISC_MURPHYS_LAW + 0)  { name = LOG_MURPHYS_LAW; val = 0; }
    else if (misc == MISC_MURPHYS_LAW + 1)  { name = LOG_MURPHYS_LAW; val = 1; }

    else if (misc == MISC_HINT)             { name = LOG_HINT;        val = 0; }
    else if (misc == MISC_SOLVE_ONE)        { name = LOG_SOLVE_ONE;   val = 0; }

    else if (misc == MISC_SOLVE_ALL + 1)    { name = LOG_SOLVE_ALL;   val = 1; }
    else if (misc == MISC_SOLVE_ALL + 2)    { name = LOG_SOLVE_ALL;   val = 2; }
    else if (misc == MISC_SOLVE_ALL + 3)    { name = LOG_SOLVE_ALL;   val = 3; }

    else if (misc == MISC_AUTO_STAGE + 0)   { name = LOG_AUTO_STAGE;  val = 0; }
    else if (misc == MISC_AUTO_STAGE + 1)   { name = LOG_AUTO_STAGE;  val = 1; }
    else if (misc == MISC_AUTO_STAGE + 2)   { name = LOG_AUTO_STAGE;  val = 2; }
    else if (misc == MISC_AUTO_STAGE + 3)   { name = LOG_AUTO_STAGE;  val = 3; }

    else if (misc == MISC_MAX_STAGE + 1)    { name = LOG_MAX_STAGE;   val = 1; }
    else if (misc == MISC_MAX_STAGE + 2)    { name = LOG_MAX_STAGE;   val = 2; }
    else if (misc == MISC_MAX_STAGE + 3)    { name = LOG_MAX_STAGE;   val = 3; }

    else if (misc == MISC_SHOW_MINES + 0)   { name = LOG_SHOW_MINES;  val = 0; }
    else if (misc == MISC_SHOW_MINES + 1)   { name = LOG_SHOW_MINES;  val = 1; }

    else if (misc == MISC_OUT_OF_TIME)      { name = LOG_OUT_OF_TIME; val = 0; }
    else if (misc == MISC_START_TIMER)      { name = LOG_START_TIMER; val = 0; }

    else                                    valid = false;
  }
  else
  {
    valid = false;
  }
}

//******************************************************************************
Logbook::Logbook (uint32 ran, const Options& opt)
//------------------------------------------------------------------------------
{
  rand_seq   = ran;
  options    = opt;
  is_playing = false;
  play_index = 0;
  cur_index  = 0;
  clock0     = clock();

  logs.clear();

  if (Glob::log_on)
    writeOptions (AUTO_LOG_FNAME);
}

//******************************************************************************
const Log& Logbook::getPlayLog() const
//------------------------------------------------------------------------------
{
  ASSERT (play_index < logs.size());

  return logs[play_index];
}

//******************************************************************************
void Logbook::writeOptions (const string& fname) const
//------------------------------------------------------------------------------
{
  ofstream  out (fname.c_str());

  if (!out)
    throw LogException (("Logbook::writeOptions(): " + fname
                       + " konnte nicht geoeffnet werden!").c_str());

  out << 0 << " " << LOG_VERSION      << " " << Glob::VERSION          << "\n";
  out << 0 << " " << LOG_VARIANT      << " " << _VARIANT_              << "\n";
  out << 0 << " " << LOG_RAND_SEQ     << " " << rand_seq               << "\n";
  out << 0 << " " << LOG_BOARD_NAME   << " " << options.getBoardName() << "\n";
  out << 0 << " " << LOG_BOARD_CHKSUM << " " << 0                      << "\n";
  out << 0 << " " << LOG_LEVEL        << " " << options.getLevelNr()   << "\n";
  out << 0 << " " << LOG_HEIGHT       << " " << options.getHeight()    << "\n";
  out << 0 << " " << LOG_WIDTH        << " " << options.getWidth()     << "\n";
  out << 0 << " " << LOG_DEEP         << " " << options.getDeep()      << "\n";
  out << 0 << " " << LOG_NUM_MINES    << " " << options.getNumMines()  << "\n";
  out << 0 << " " << LOG_NUM_WHOLES   << " " << options.getNumWholes() << "\n";
  out << 0 << " " << LOG_MODUS        << " " << options.getModus()     << "\n";
  out << 0 << " " << LOG_MURPHYS_LAW  << " " << options.getMurphysLaw()<< "\n";
  out << 0 << " " << LOG_AUTO_STAGE   << " " << options.getAutoStage() << "\n";
  out << 0 << " " << LOG_MAX_STAGE    << " " << options.getMaxStage()  << "\n";
  out << 0 << " " << LOG_SHOW_MINES   << " " << options.getShowMines() << "\n";
  out << "\n";
}

//******************************************************************************
void Logbook::write (const string& fname)
//------------------------------------------------------------------------------
// Schreiben eines Logfiles
{
  writeOptions (fname);

  for (unsigned i = 0; i < play_index; i++)
    logs[i].write(fname);
}

//******************************************************************************
void Logbook::read (const string& fname)
//------------------------------------------------------------------------------
// Lesen eines Logfiles
{
  if (fname == AUTO_LOG_FNAME)
    throw LogException ((string ("Logbook::read(): ") + AUTO_LOG_FNAME
                       + " isn't allowed to read.").c_str());

  ifstream  in (fname.c_str());

  if (!in)
    throw LogException (("Logbook::read(): " + fname
                       + " cannot open.").c_str());

  Log    log;
  Level  lvl;

  logs.clear();

  while (!in.eof())
  {
    // str1, str2, str3
    string str1, str2, str3;
    
    in >> str1;
    if (str1.size() == 0)
      break;
    else if (in.eof())
      throw LogException (string("Logbook::read(1): Number of entries must be a "
                                 "multiple of three. (") + str1 + ")");
    in >> str2;
    if (in.eof())
      throw LogException ("Logbook::read(2): Number of entries must be a "
                          "multiple of three.");
      
    in >> str3;
 
    // log
    if (str1.find_first_not_of ("0123456789!") != string::npos)
      throw LogException (string("Logbook::read: '") + str1 + "' must not contains non-digits.");
   
    log.time1 = atoi (str1.c_str());
    log.name = str2;

    if (log.name == LOG_BOARD_NAME)
    {    
      log.val = options.findBoardNr(str3);
          
      if (log.val == -1)
        throw LogException (string("Logbook::read: board '") + str3 + "' not found!");
    }
    else
    {
      if (str3.find_first_not_of ("-0123456789!") != string::npos)
        throw LogException (string("Logbook::read: '") + str3 + "' must not contains non-digits.");
        
      log.val = atoi (str3.c_str());   
    }

    // log auswerten
    if (log.time1 == 0)
    {
      if      (log.name == LOG_RAND_SEQ)    rand_seq                = log.val;
      else if (log.name == LOG_BOARD_NAME)  options.setBoardNr       (log.val);
      else if (log.name == LOG_MODUS)       options.setModus ((Modus) log.val);
      else if (log.name == LOG_LEVEL)       lvl.nr       = (LevelNr) log.val;
      else if (log.name == LOG_HEIGHT)      lvl.height              = log.val;
      else if (log.name == LOG_WIDTH)       lvl.width               = log.val;
      else if (log.name == LOG_DEEP)        lvl.deep                = log.val;
      else if (log.name == LOG_NUM_MINES)   lvl.num_mines           = log.val;
      else if (log.name == LOG_NUM_WHOLES)  lvl.num_wholes          = log.val;
      else if (log.name == LOG_MURPHYS_LAW) options.setMurphysLaw (log.val != 0);
      else if (log.name == LOG_AUTO_STAGE)  options.setAutoStage     (log.val);
      else if (log.name == LOG_MAX_STAGE)   options.setMaxStage      (log.val);
      else if (log.name == LOG_SHOW_MINES)  options.setShowMines  (log.val != 0);
      else if (log.name != LOG_VERSION && log.name != LOG_VARIANT
           &&  log.name != LOG_BOARD_CHKSUM)
        throw LogException (string("Logbook::read: '") + log.name + "' not recognize.");
    }
    else
    {
      logs.push_back (log);
    }
  }

  options.setLevel (lvl);
  play_index = cur_index = logs.size();

  if (Glob::log_on)
    write (AUTO_LOG_FNAME);
}

//******************************************************************************
void Logbook::startPlaying (void)
//------------------------------------------------------------------------------
{
  if (Glob::log_on)
    writeOptions (AUTO_LOG_FNAME);

  play_index = 0;
  is_playing = true;
  clock0 = clock(); // Zeit synchronisieren
}

//******************************************************************************
void Logbook::operator>> (Log& log)
//------------------------------------------------------------------------------
// Lesen eines Log-Eintrages
{
  ASSERT (play_index < logs.size());
  ASSERT (play_index < cur_index);

//   logs[play_index].time1 = getTime();  //???

  log = logs[play_index];
  play_index++;

  Log  log2 = log;
  
  if (Glob::log_on)
    log.write (AUTO_LOG_FNAME); // auto_log protokolliert auch das Lesen.

  // Zeit synchronisieren
  clock0 = clock() - log.time1 * CLK_TCK / 1000;
}

//******************************************************************************
void Logbook::operator<< (const Log& log)
//------------------------------------------------------------------------------
// schreiben eines Log-Eintrages
{
  // check play_index
  if (isPlaying())
  {
    ASSERT (play_index < cur_index || log.isComputerLog());
  }
  else
  {
    // Leseaktionen muessen beendet sein!
    ASSERT (play_index == cur_index);
  }

  Log  log2 = log;

  // logs, play_index
  if (isPlaying())
  {
    if (log2.name == LOG_START_TIMER)
      log2.val = 0; // Wert steckt schon in time1

    // Waehrend eines Abspielens duerfen nur Out-Of-Time und Start-Timer
    // Logs geschrieben werden!
    if (!logs[play_index].isComputerLog())
      throw LogException (string("logs[].name == '") + logs[play_index].name 
                         + "' invalid!");
    // Kommen Computerlogs in der selben Reihenfolge?  
    else if (logs[play_index].name != log2.name)
      throw LogException ("Logbook::operator<<: log.name invalid");
    else if (logs[play_index].val != log2.val)
      throw LogException ("Logbook::operator<<: log.val invalid");

    // Log schreiben
    if (Glob::log_on)
    {
      log2.time1 = logs[play_index].time1; 
      log2.write (AUTO_LOG_FNAME);
    }

    play_index++;
  }
  else // not playing
  {
    // AUTO_LOG_FNAME
    if (log.name == LOG_START_TIMER)
    {
      // Hier wird die clock0-Zeit vom Timer uebergeben
      log2.time1 = (log.val - clock0) * 1000 / CLK_TCK;
      log2.val    = 0; // Wert steckt schon in time1
    }
    else
    {
      log2.time1 = (clock() - clock0) * 1000 / CLK_TCK;
    }

    if (log2.time1 == 0)
      log2.time1 = 1;   // 0 wird für die Startoptionen benutzt

    if (Glob::log_on)
      log2.write (AUTO_LOG_FNAME);

    if (cur_index < logs.size())
      logs.erase (logs.begin() + cur_index, logs.end());  // moegliche redo-
                                                        // Actionen loeschen
    logs.push_back (log2);
    play_index = cur_index = logs.size();
  }
}

//******************************************************************************
bool Logbook::undo(bool all)
//------------------------------------------------------------------------------
// bereitet Logbook fuer ein Undo vor
// anschliessend muss es noch einmal durchgespielt werden
{
  if (isPlaying())
    throw LogException ("Logbook::undo(): is read only!");
  else if (cur_index > logs.size())
    throw LogException ("Logbook::undo(): invalid cur_index");
  else if (play_index != cur_index)
    throw LogException ("Logbook::undo(): invalid play_index");
    
  // first_valid
  bool     found_valid = false;
  unsigned first_valid; 

  for (first_valid = 0; first_valid < cur_index; ++first_valid)
  {
    if (!logs[first_valid].isComputerLog() && logs[first_valid].valid)
    {
      found_valid = true;
      break;
    }
  }

  // Keinen gueltigen -> Ende
  if (!found_valid)
    return false; 

/*
  // Keine gueltigen Logs -> nur auto.log akt.
  if (!found_valid)
  {
    if (Glob::log_on)
    {
      writeOptions(AUTO_LOG_FNAME);

      for (unsigned i = 0; i < cur_index && logs[i].isComputerLog(); ++i)
        logs[i].write(AUTO_LOG_FNAME);
    }
    
    // setze cur_index
    cur_index  = i;
    play_index = cur_index; // play_index mitanpassen
    
    return false; // keine wesentliche Aenderung
  }
*/
  if (cur_index < logs.size() 
  && (!logs[cur_index].valid || logs[cur_index].isComputerLog()))
    throw LogException ("Logbook::undo(): invalid log!");

  if (all)
  {
    cur_index  = first_valid;
    play_index = cur_index; // mitanpassen
  }
  else
  {
    while (true)
    { 
      if (cur_index == first_valid)
      {
        if (Glob::log_on)
          writeOptions (AUTO_LOG_FNAME);
        return false;
      }
         
      --cur_index;
      play_index = cur_index; // mitanpassen
      
      if (logs[cur_index].valid && !logs[cur_index].isComputerLog())
        break;
    }
  }
    
  return true;
}

//******************************************************************************
bool Logbook::redo (Log& log)
//------------------------------------------------------------------------------
// bereitet Logbook fuer ein Redo vor
// anschliessend muss log noch durchgefuehrt werden
{
  if (isPlaying())
    throw LogException ("Logbook::redo(): is read only!");

  if (cur_index >= logs.size())
    return false; // kein redo durchgefuehrt

/*
  // Falls cur_index == 0 und ungueltig ist, wird er auf den naechsten guelten
  // gesetzt
  if (cur_index == 0)
  {
    while (cur_index < logs.size() 
           && (!logs[cur_index].valid || logs[cur_index].isComputerLog()))
    {
      if (!logs[cur_index].isComputerLog() && Glob::log_on)
        logs[cur_index].write(AUTO_LOG_FNAME);
        
      ++cur_index;
      play_index = cur_index; // play_index mitaktualisieren
    }
    
    if (cur_index >= logs.size())
      return false; // kein redo durchgefuehrt
  }
*/  
  log = logs[cur_index];

  // cur_index muss immer auf einem gueltigen stehen
  if (!log.valid || log.isComputerLog())
    throw LogException (string("Logbook::redo(): invalid log! (") + log.name + "')");

  // cur_index wird auf den naechsten freien oder den naechsten gueltigen Log
  // gesetzt.  
  do
  {
    if (!logs[cur_index].isComputerLog() && Glob::log_on)
      logs[cur_index].write(AUTO_LOG_FNAME);
      
    ++cur_index;
    play_index = cur_index; // play_index mitaktualisieren
  }
  while (cur_index < logs.size() 
         && (!logs[cur_index].valid || log.isComputerLog()));
  
  // Zeit synchronisieren
  clock0 = clock() - log.time1 * CLK_TCK / 1000;
  
  return true;
}

//******************************************************************************
bool Logbook::invalidateLast()
//------------------------------------------------------------------------------
{
  if (play_index < 1 || logs.size() < play_index)
    return false;

  logs[play_index - 1].valid = false;    
  return true;
}

//******************************************************************************
char Logbook::Int6ToChar (int val) const
//------------------------------------------------------------------------------
{
  if      (val <  10)  return '0' + val;
  else if (val <  36)  return 'A' + val - 10;
  else if (val <  62)  return 'a' + val - 36;
  else if (val == 62)  return '-';
  else if (val == 63)  return '_';

  throw LogException ("Logbook::Int6ToChar: num_bits too large.");
}

//******************************************************************************
int Logbook::CharToInt6 (char ch) const
//------------------------------------------------------------------------------
{
  if      ('0' <= ch && ch <= '9')  return ch - '0';
  else if ('A' <= ch && ch <= 'Z')  return ch - 'A' + 10;
  else if ('a' <= ch && ch <= 'z')  return ch - 'a' + 36;
  else if (ch == '-')               return 62;
  else if (ch == '_')               return 63;

  throw LogException ("Logbook::CharToInt6: num_bits too large.");
}

//******************************************************************************
string Logbook::exportStr () const
//------------------------------------------------------------------------------
{
  string    func_name = "Logbook::exportStr: ";
  string    text;
  char      buf[20]; // fuer atoi
  unsigned  i;
  char      sep = ',';

  //--- stat. Informationen ---

  //
  text =  itoa (Glob::VERSION, buf, 10); 
  text += sep;
  text += itoa (_VARIANT_, buf, 10);
  text += sep;
  text += options.getBoardName();
  text += sep;
  text += itoa (options.getLevelNr(), buf, 10);
  text += sep;

  if (options.getLevelNr() == USER_DEFINED)
  {
    text += itoa (options.getHeight(), buf, 10);
    text += sep;
    text += itoa (options.getWidth(), buf, 10);
    text += sep;
    text += itoa (options.getDeep(), buf, 10);
    text += sep;
    text += itoa (options.getNumMines(), buf, 10);
    text += sep;
    text += itoa (options.getNumWholes(), buf, 10);
    text += sep;
  }

  text += itoa (options.getModus(), buf, 10);
  text += sep;
  text += itoa (options.getMurphysLaw(), buf, 10);
  text += sep;

  text += itoa (options.getAutoStage(), buf, 10);
  text += sep;
  text += itoa (options.getMaxStage(), buf, 10);
  text += sep;
  text += itoa (options.getShowMines(), buf, 10);
  text += sep;
  text += itoa (rand_seq, buf, 10);
  text += sep;

  //--- dyn. Informationen ---

  // max_val
  int max_val = 0;
  for (i = 0; i < logs.size(); ++i)
  {
    if (logs[i].valid)
      max_val = max (max_val, logs[i].val);
  }

  // num_bits
  int num_bits = 2; // Logs::Type
  while (max_val > 0)
  {
    ++num_bits;
    max_val >>= 1;
  }
  num_bits = max (num_bits, 8); // 8 == Logs::Type + Logs::Misc

  if (num_bits + 6 - 1 > 31) // gibt sonst Probleme mit word
    throw LogException (func_name + "num_bits too large.");

  text += Int6ToChar (num_bits);

  // 
  int word      = 0; // 32-Bit (max. 31 werden benutzt)
  int word_bits = 0; // belegte Bits von word

  for (i = 0; i < logs.size(); ++i)
  {
    int val = logs[i].toInt();

    if (val <= 0)
      continue;

    if (val > (1 << num_bits))
      throw LogException (func_name + "val too large.");

    word += val << word_bits; // val an word anhaengen
    word_bits += num_bits;

    if (word >= (1 << word_bits))
      throw LogException (func_name + "word too large.");

    // vollstaendige Buchstaben ausgeben
    while (word_bits >= 6)
    {
      text += Int6ToChar (word & ((1 << 6) - 1));
      word >>= 6;
      word_bits -= 6;

      if (word > (1 << word_bits))
        throw LogException (func_name + "word too large.");
    }
  }

  if (word_bits >= 6)
    throw LogException (func_name + "word_bits too large.");

  if (word >= (1 << word_bits))
    throw LogException (func_name + "word too large.");

  // letzten unvollstaendigen Buchstaben ausgeben
  if (word_bits > 0)
    text += Int6ToChar (word);

  return text;
}

//******************************************************************************
void Logbook::importStr (const string& text)
//------------------------------------------------------------------------------
{
  string func_name = "Logbook::importStr: ";

  //--- stat. Informationen ---

  // parts
  string::size_type  p1, p2;
  vector<string>     parts;

  for (p1 = p2 = 0; p2 != string::npos; p1 = p2 + 1)
  {
    p2 = text.find (',', p1);

    parts.push_back (text.substr (p1, p2 - p1));
  }

  if (parts.size() < 4) // min. bis Level
    throw LogException (func_name + "too few parts.");

  unsigned i = 0;

  // Version + Variante
  int version = atoi (parts[i++].c_str()); 
//  int variant = atoi (parts[i++].c_str()); // wird nicht gebraucht
  i++; // variant ueberspringen

  if (version > Glob::VERSION)
    throw LogException (func_name + "invalid version.");

  // board_name
  int board_nr = options.findBoardNr (parts[i++]);

  if (board_nr == -1)
    throw LogException (func_name + "invalid boardname.");

  options.setBoardNr(board_nr);

  // level
  Level lvl;

  lvl.nr = (LevelNr) atoi (parts[i++].c_str());

  if (lvl.nr == USER_DEFINED)
  {
    if (parts.size() != 11 + 5)
      throw LogException (func_name + "number of parts != 16.");

    lvl.height     = atoi (parts[i++].c_str());
    lvl.width      = atoi (parts[i++].c_str());
    lvl.deep       = atoi (parts[i++].c_str());
    lvl.num_mines  = atoi (parts[i++].c_str());
    lvl.num_wholes = atoi (parts[i++].c_str());
  }
  else
  {
    if (parts.size() != 11)
      throw LogException (func_name + "number of parts != 11.");
  }
  
  if (!options.setLevel(lvl))
    throw LogException (func_name + "error at setLevel().");

  // modus + murphys_law
  options.setModus      ((Modus) atoi (parts[i++].c_str()));
  options.setMurphysLaw (atoi (parts[i++].c_str()) != 0);

  // auto_stage, max_stage, show_mines
  options.setAutoStage (atoi (parts[i++].c_str()));
  options.setMaxStage  (atoi (parts[i++].c_str()));
  options.setShowMines (atoi (parts[i++].c_str()) != 0);

  // rand_seq
  rand_seq = atoi (parts[i++].c_str());

  //--- dyn. Informationen ---

  string log_data = parts[i++];

  if (i != parts.size())
    throw LogException (func_name + "too much parts.");

  logs.clear();

  if (log_data.size() <= 1) // 1. Zeichen ist nur Laenge
    return;

  //
  unsigned k         = 0;
  int      num_bits  = CharToInt6 (log_data[0]);
  int      word      = 0;      // 31-Bits genuegen
  int      word_bits = 0;      // belegte Bits von word

  if (num_bits + 6 - 1 > 31) // gibt sonst Probleme mit word
    throw LogException (func_name + "num_bits too large.");

  if (num_bits < 8) // 8 == Logs::Type + Logs::Misc
    throw LogException (func_name + "num_bits too small.");

  while (true)
  {
    while (word_bits < num_bits)
    {
      ++k;
      if (k >= log_data.size())
        break; // fertig

      word += CharToInt6 (log_data[k]) << word_bits;
      word_bits += 6;

      if (word >= (1 << word_bits))
        throw LogException (func_name + "word too large.");
    }

    if (k >= log_data.size())
      break; // fertig

    Log new_log (word & ((1 << num_bits) - 1));
    word >>= num_bits;
    word_bits -= num_bits;

    new_log.time1 = 1; // != 0
    logs.push_back (new_log);
  }

  //
  play_index = cur_index = logs.size();

  if (Glob::log_on)
    write (AUTO_LOG_FNAME);
}
