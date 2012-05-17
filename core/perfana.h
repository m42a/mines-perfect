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

#ifndef PERFANA_H
#define PERFANA_H

#include <string>
#include <time.h>

using namespace std;

namespace MinesPerfect {

#ifndef LINUX
  typedef unsigned long  ulong;
#endif

namespace Glob
{
  extern bool  perfana_on;
}

#define PERF_ANA(fname)                            \
   static Perf static_perf (fname);                \
   Perf        dyn_perf (static_perf.GetFuncNr());

//******************************************************************************
struct PerfFunc
//------------------------------------------------------------------------------
{
  string  name;
  ulong   time_total;
  ulong   time_self;
  int     num_calls;
  int     rec_level; 

  PerfFunc (const string& n) : name(n), time_total(0), time_self(0),
                                num_calls(0), rec_level(0) {}
  PerfFunc () : name(), time_total(0), time_self(0), num_calls (0),
                 rec_level(0) {}
};

//******************************************************************************
struct PerfFuncCmp
//------------------------------------------------------------------------------
{
  bool operator() (const PerfFunc& a, const PerfFunc& b) const { 
    return  a.time_self > b.time_self; 
  }
};

//******************************************************************************
class Perf
//------------------------------------------------------------------------------
{
private:
  int       func_nr;
  clock_t   clock0;
  bool      is_static;

public:
  Perf (const string&  fname);
  Perf (const int      fnr);
  ~Perf();

  int  GetFuncNr() { return func_nr; }
};


} // namespace MinesPerfect

#endif
