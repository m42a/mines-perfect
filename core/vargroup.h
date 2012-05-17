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

#ifndef VARGROUP_H
#define VARGROUP_H

#include <functional>
#include <fstream> // fuer vector etc.
#include <vector>
#include <list>
#include <set>
#include <map>
#include <algorithm>
#include <time.h>      // clock

#include "cell.h"
#include "options.h"


namespace MinesPerfect {


#define  for_all_vars(xi,x) \
           for (vector<Var*>::const_iterator  xi =  x.begin(); \
                                              xi != x.end();  xi++)

#define  for_all_eqs(bj,b) \
           for (vector<Eq*>::const_iterator bj =  b.begin(); \
                                            bj != b.end();  bj++)

#define  for_all_groups(g,tab) \
           for (vector<VarGroup*>::const_iterator g =  tab.begin(); \
                                                g != tab.end();  g++)

#define  for_all_sols(s,sols) \
           for (Sols::const_iterator s =  sols.begin(); \
                                     s != sols.end();  s++)

#define  for_all_sums(s,sums) \
           for (SolSums::const_iterator s =  sums.begin(); \
                                         s != sums.end();  s++)


//------------------------------------------------------------------------------
template <class R, class T>
class mem_eq_t : public unary_function<T*,R> {
//------------------------------------------------------------------------------
private:
  R val;
  R T::*pm;
public:
  explicit mem_eq_t(R T::*p, const R& v) : val(v), pm(p) {}
  bool operator()(T* p) const { return p->*pm == val; }
};

template <class R, class T>
inline mem_eq_t<R,T> mem_eq(R T::*p, const R& v)
  { return mem_eq_t<R,T>(p,v); }


//------------------------------------------------------------------------------
template <class R, class T>
class mem_cmp_t : public unary_function<T*,R> {
//------------------------------------------------------------------------------
private:
  R T::*pm;
public:
  explicit mem_cmp_t(R T::*p) : pm(p) {}
  bool operator()(T* p1, T* p2) const { return p1->*pm < p2->*pm; }
};

template <class R, class T>
inline mem_cmp_t<R,T> mem_cmp(R T::*p)
  { return mem_cmp_t<R,T>(p); }


//------------------------------------------------------------------------------
enum Conclusion { CONCL_NOT_TRIED, CONCL_NOT_COMPLETE,
                  CONCL_CONTRA,    CONCL_SOLUTION };

enum VarType    { VAR_VISIBLE, VAR_REAL };

class OutOfTime{};

class  Eq;
class  Var;
class  VarGroup;
class  Eqs;
struct Sols;
struct SolSums;

typedef map<string,SolSums>                   CombSums;

typedef vector<Var*>::iterator                VarIter;
typedef vector<Var*>::const_iterator          VarCIter;
typedef vector<VarGroup*>::iterator           VarGroupIter;
typedef vector<VarGroup*>::const_iterator     VarGroupCIter;
typedef vector<vector<int> >::iterator        CombIter;
typedef vector<vector<int> >::const_iterator  CombCIter;
//typedef SolSums::iterator                   SumIter;
//typedef SolSums::const_iterator             SumCIter;


//******************************************************************************
  struct Sols : public vector<int>  // von vector abgeleitet, weil sols sehr klein sind (und viele)
//------------------------------------------------------------------------------
{
  //------------------------------------
  void insertInt (int a) // muss anders heissen als insert (wegen for_each)
  {
    if (find (begin(), end(), a) == end())
      push_back (a);
  }

  //------------------------------------
  bool insert (const Sols& sols2)
  {
    for_all_sols (s, sols2)
      insertInt (*s);
    return true; // nur fuer den Scheiss-Compiler
  }
};

//******************************************************************************
  struct SolSums : public vector<int>  // von vector abgeleitet, weil sums sehr klein sind (und viele)
//------------------------------------------------------------------------------
{
  //------------------------------------
  void insertInt (int a) // muss anders heissen als insert (wegen for_each)
  {
    if (find (begin(), end(), a) == end())
      push_back (a);
  }

  //------------------------------------
  bool insert (const SolSums& sums2)
  {
    for_all_sums (s, sums2)
      insertInt (*s);
    return true; // nur fuer den Scheiss-Compiler
  }

  //------------------------------------
  void add (int sum)
  {
     for (unsigned i = 0; i < size(); i++)
       (*this)[i] += sum;
  }

  //------------------------------------
  void add (const SolSums& sums2)
  {
    SolSums res;

//    for_all_sums (s1, *this)
    for (SolSums::const_iterator s1 = begin(); s1 != end(); s1++)
      for_all_sums (s2, sums2)
        res.insertInt (*s1 + *s2);

    *this = res;
  }

  //------------------------------------
  void sub (int sum)
  {
     for (unsigned i = 0; i < size(); i++)
       (*this)[i] -= sum;
  }

  //------------------------------------
  void sub (const SolSums& sums2)
  {
    SolSums res;

//    for_all_sums (s1, *this)
    for (SolSums::const_iterator s1 = begin(); s1 != end(); s1++)
      for_all_sums (s2, sums2)
        res.insertInt (*s1 - *s2);

    *this = res;
  }
};

//******************************************************************************
  class Var
//------------------------------------------------------------------------------
{
  friend class Eq;
  friend class VarGroup;
  friend class Eqs;

private:

  // stat
  int            id;        // nur fuer Trace
  VarGroup*      group;     // Gruppe

  vector<Eq*>    b;         // anliegende Konstanten
  vector<Var*>   x;         // anliegende Variablen
  CellNrs        cells;     // Zellennummern
  int            val_min;
  int            val_max;
  int            one_sol_val_min;
  int            one_sol_val_max;

  // dyn
  vector<Sols>   sols;  // sols[0]: endgueltig, sols[>0]: nur prophylaktisch
  int            val;
  int            one_sol_val; // Wert fuer eine bel. Loesung
  bool           manual;      // fuer Trace
  bool           flag;        // Arbeitsflag

  //------------------------------------
  bool init       (void); // nur dyn.

  void tie        (int v, bool man = false);
  void untie      (void);

  bool setFlag    (void) { flag = true;  return true; }
  bool clearFlag  (void) { flag = false; return true; }

  bool cmpGroupId (const Var* x2);
  bool cmpB       (const Var* x2) { return lexicographical_compare (b.begin(),
                                    b.end(), x2->b.begin(), x2->b.end()); }
};

//******************************************************************************
  class VarGroup                                          // Gruppe von Variablen
//------------------------------------------------------------------------------
{
  friend class Eq;
  friend class Var;
  friend class Eqs;

private:

  // stat.
  int            id;
  int            deep;
  VarGroup*      parent;
  VarGroup*      child1;
  VarGroup*      child2;
  Eqs*           eqs;     

  vector<Var*>   x;

  // dyn.
  int            num_free_vars;   // Anzahl der freien Variablen
  int            num_free_cells;  // Anzahl der freien Zellen
  CombSums       comb_sums;       // fuer jede Kombination der Eltern- und Grosselterngruppen
  SolSums*       cur_sums;
  bool           note_one_sol;    // Loesung fuer FoundOneSolution notieren

  //------------------------------------
  VarGroup()  { id = deep = 0;
                parent = child1 = child2 = 0; eqs = 0;}
//  ~VarGroup() { delete child1; delete child2; }

  // stat.
  void  createIsles       (vector<VarGroup*>& parts) const;
  void  split             (const VarGroup& part1, VarGroup& part2, VarGroup& part3);
  float calcVal           (void) const;
  void  unconnectedToTree (vector<VarGroup*>& grp_tab);
  void  buildTree         (void);

  // dyn.
  void        printDyn    (void); // kein const wegen this-Benutzung
  bool        init        (int pass = 1);     // nur dyn.
  string      calcCombkey (void);
  int         sumVal      (void);
  void        clearSols   (int the_deep);     // loescht sols[the_deep] rek.
  void        raiseSols   (int to_deep);      // hebt rek. eine Stufe an
  Conclusion  conclude    (vector<Var*>& ties);
  bool        noteSol     (SolSums* permit_sums);
  bool        solve       (SolSums* permit_sums = 0);
};

} // namespace MinesPerfect

#endif
