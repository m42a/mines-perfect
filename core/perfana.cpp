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

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <set>
#include <time.h>
#include <stdio.h>

#ifdef VISUAL_CPP
  #include <iostream> // visual studio (cerr)
  #include <minmax.h> // visual studio (max)
#endif

using namespace std;

#include "perfana.h"
#include "api.h"

using namespace MinesPerfect;


static vector<int>       call_stack;
static vector<PerfFunc>  functions;

//******************************************************************************
static ulong  TimeStrLen (ulong time)
//------------------------------------------------------------------------------
{
  int  len = 0;

  for (; time; time /= 10)
    len++;

  return max (len + 1, 5);
}

//******************************************************************************
static char*  TimeToA (ulong time, char* const buf, ulong len)
//------------------------------------------------------------------------------
{
  if (len < 5)
    return 0;

  buf [len] = 0;
  buf [--len] = (char) (time % 10 + '0');   time /= 10;
  buf [--len] = (char) (time % 10 + '0');   time /= 10;
  buf [--len] = (char) (time % 10 + '0');   time /= 10;
  buf [--len] = '.';
  buf [--len] = (char) (time % 10 + '0');   time /= 10;

  for (; time; time /= 10)
    buf [--len] = (char) (time % 10 + '0');

  while (len > 0)
    buf [--len] = ' ';

  return  buf;
}

//******************************************************************************
Perf::Perf (const string& fname)
//------------------------------------------------------------------------------
{
  func_nr   = functions.size();
  is_static = true;
  functions.push_back (PerfFunc (fname));
}

//******************************************************************************
Perf::Perf (const int fnr)
//------------------------------------------------------------------------------
{
  func_nr   = fnr;

  clock0    = clock();
  ASSERT (clock0 != -1);

  is_static = false;
  functions[fnr].num_calls++;
  functions[fnr].rec_level++;
  call_stack.push_back (fnr);
}

//******************************************************************************
Perf::~Perf()
//------------------------------------------------------------------------------
{
  if (is_static || !Glob::perfana_on)
    return;

  clock_t   clock1 = clock();

  ASSERT (clock0 != -1);
  
  ulong     total  = (ulong) ((1000000.0 / CLK_TCK) * (clock1 - clock0));
  int       fnr    = call_stack.back();
  call_stack.pop_back();
  if (functions[fnr].rec_level == 1)
    functions[fnr].time_total += total;
  functions[fnr].time_self += total;
  functions[fnr].rec_level--;

  if (!call_stack.empty())
  {
    functions[call_stack.back()].time_self -= total;
  }
  else
  {
    // Ausgabe
    ofstream  out ("perfana.txt");

    const string  title_name       = "name";
    const string  title_percent    = "%";
    const string  title_total      = "total/ms";
    const string  title_self       = "self/ms";
    const string  title_num_calls  = "calls";
    const string  title_self_aver  = "self/call";
    const string  title_total_aver = "total/call";
    ulong         max_name_len     = title_name.length();
    ulong         max_percent      = max (title_percent.length(), 4U);
    ulong         max_total        = title_total.length();
    ulong         max_self         = title_self.length();
    ulong         max_num_calls    = title_num_calls.length();
    ulong         max_self_aver    = title_self_aver.length();
    ulong         max_total_aver   = title_total_aver.length();
    ulong         total_time;
    set<string>    name_set;
    char           buf[40];

    sort (functions.begin(), functions.end(), PerfFuncCmp());

    // max
    for (unsigned i = 0; i < functions.size(); i++)
    {
      max_name_len   = max (max_name_len,   (unsigned long) functions[i].name.length());
      max_total      = max (max_total,      functions[i].time_total);
      max_self       = max (max_self,       functions[i].time_self);
      max_num_calls  = max (max_num_calls,  (ulong) functions[i].num_calls);

      if (functions[i].num_calls == 0)
      {
        max_self_aver  = 0;
        max_total_aver = 0;
      }
      else
      {
        max_self_aver  = max (max_self_aver,  functions[i].time_self
                         / functions[i].num_calls);
        max_total_aver = max (max_total_aver, functions[i].time_total
                         / functions[i].num_calls);
      }

      ulong  set_size = name_set.size();
      name_set.insert (functions[i].name);
      if (set_size == name_set.size())
        cerr << "'" << functions[i].name << "' ist doppelt" << endl;
    }
    total_time = max_total;

    // max -> len
    sprintf (buf, "%lu", max_num_calls);
    max_total      = max (TimeStrLen (max_total), (ulong) title_total.length());
    max_self       = max (TimeStrLen (max_self ), (ulong) title_self.length());
    max_num_calls  = max (strlen (buf), title_num_calls.length());
    max_self_aver  = max (TimeStrLen (max_self_aver ),
                          (ulong) title_self_aver.length());
    max_total_aver = max (TimeStrLen (max_total_aver),
                          (ulong) title_total_aver.length());

    // Title
    out << '\n'
        << title_name       << string (max_name_len   - title_name.length()       + 2, ' ')
        << title_percent    << string (max_percent    - title_percent.length()    + 2, ' ')
        << title_total      << string (max_total      - title_total.length()      + 2, ' ')
        << title_self       << string (max_self       - title_self.length()       + 2, ' ')
        << title_num_calls  << string (max_num_calls  - title_num_calls.length()  + 2, ' ')
        << title_self_aver  << string (max_self_aver  - title_self_aver.length()  + 2, ' ')
        << title_total_aver << string (max_total_aver - title_total_aver.length() + 2, ' ')
        << '\n';

    // Rest
    { // visual studio
    for (unsigned i = 0; i < functions.size(); i++)
    {
      out << functions[i].name
          << string (max_name_len - functions[i].name.length() + 2, ' ');
      out << setw (2) << 100 * functions[i].time_self / total_time << '.'
          << 1000 * functions[i].time_self / total_time % 10 << "  ";
      out << TimeToA (functions[i].time_total, buf, max_total) << "  ";
      out << TimeToA (functions[i].time_self, buf, max_self) << "  ";
      out << setw (max_num_calls) << functions[i].num_calls << "  ";
      out << TimeToA (functions[i].num_calls == 0 ?
                            0 : functions[i].time_self / functions[i].num_calls,
                      buf, max_self_aver) << "  ";
      out << TimeToA (functions[i].num_calls == 0 ?
                           0 : functions[i].time_total / functions[i].num_calls,
                      buf, max_total_aver) << '\n';
    }
    } // visual studio

    // aufraeumen
    { // visual studio
    for (unsigned i = 0; i < functions.size(); i ++)
    {
      functions[i].time_self  = 0;
      functions[i].time_total = 0;
      functions[i].num_calls  = 0;
    }
    } // visual studio
  }
}

