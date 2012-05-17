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

// Konstanten zur Fehlersuche
namespace MinesPerfect
{
  namespace Glob
  {
    extern int trace_level;
  }
}

//#include <Forms.hpp>  // Exception

#include <iterator>
#include <iomanip>

#ifdef VISUAL_CPP
  #include <math.h> // visual studio
#endif

using namespace std;

#include "vargroup.h"
#include "eqs.h"
#include "api.h"
#include "perfana.h"

using namespace MinesPerfect;

template<class T> T* delete_ptr (T* p) { delete p; return 0; }

//******************************************************************************
bool Var::init (void) // nur dyn.  (ok)
//------------------------------------------------------------------------------
{
  val = -1;
  sols.clear();

  return true; // nur fuer den scheiss Compiler
}

//******************************************************************************
bool Var::cmpGroupId (const Var* x2) // (ok)
//------------------------------------------------------------------------------
// Funktion kann nicht in Headerdatei definiert werden, da group->id erst
// nach Var definiert wird.
{
  return  group->id < x2->group->id;
}

//******************************************************************************
void Var::tie (int v, bool man) // (ok)
//------------------------------------------------------------------------------
{
  for_all_eqs (bj, b)
  {
    (*bj)->val            -= v;
    (*bj)->num_free_vars  -= 1;
    (*bj)->num_free_cells -= cells.size();
  }

  val    = v;
  manual = man;

  group->num_free_vars  -= 1;
  group->num_free_cells -= cells.size();

  group->eqs->conclusion = CONCL_NOT_TRIED;
}

//******************************************************************************
void Var::untie (void) // (ok)
//------------------------------------------------------------------------------
{
  for_all_eqs (bj, b)
  {
    (*bj)->val            += val;
    (*bj)->num_free_vars  += 1;
    (*bj)->num_free_cells += cells.size();
  }

  val = -1;

  group->num_free_vars  +=  1;
  group->num_free_cells += cells.size();

  group->eqs->conclusion = CONCL_NOT_TRIED;
}

//******************************************************************************
void VarGroup::printDyn (void) // (ok)
//------------------------------------------------------------------------------
{
  ofstream     out ("group.txt");

  // group->id
  vector<VarGroup*>  call_stack;

  out << "cur.group-path: ";

  for (VarGroup* g = this; g != 0; g = g->parent)
    call_stack.push_back (g);

  for (unsigned i = call_stack.size(); i > 0; i--)
  {
    VarGroup*  g    = call_stack[i-1];
    string   key  = g->calcCombkey();
    string   gval = key.substr (key.size() - g->x.size());

    out << g->id << " (" << gval << ")";
    if (i > 1)
      out << " -> ";
  }
  out << "\n\n";

  // x->group->id
  out << "x->group->id  " << setfill ('-');
  for_all_vars (xi, eqs->x)
  {
    out << setw ((*xi)->val_max - (*xi)->val_min + 2)
        << (*xi)->group->id << "-";
  }
  out << "\n" << setfill (' ');

  // x->sols
  if (eqs->x.size() > 0)
  {
    for (unsigned d = 0; d < eqs->x[0]->sols.size(); d++)
    {
      out << "x->sols[" << d << "] = { ";
      for_all_vars (xi, eqs->x)
      {
          for (unsigned s = 0; s < (*xi)->sols[d].size(); s++)
            out << (*xi)->sols[d][s];
      
        out << setw ((*xi)->val_max - (*xi)->val_min + 1 - (*xi)->sols[d].size())
            << "";
        if (xi + 1 != eqs->x.end())
          out << ", ";
      }
      out << " }\n";
    }
  }

  // x->val
  out << "x->val     = { ";

  { // visual studio
	  for_all_vars (xi, eqs->x)
	  {
	  if ((*xi)->val == -1)
	    out << setw ((*xi)->val_max - (*xi)->val_min + 1) << "-";
	  else
	    out << setw ((*xi)->val_max - (*xi)->val_min + 1) << (*xi)->val;
	  if (xi + 1 != eqs->x.end())
	    out << ", ";
	  }
  } // visual studio

  out << " }\n";

  // x->id
  out << "x->id         " << setfill ('-');

  { // visual studio
    for_all_vars (xi, eqs->x)
      out << setw ((*xi)->val_max - (*xi)->val_min + 2)
          << (*xi)->id << "-";
  } // visual studio

  out << "\n\n" << setfill (' ');

  // group->sol_sums
  for_all_groups (gr, eqs->groups)
  {
    for (CombSums::const_iterator ci =  (*gr)->comb_sums.begin();
                                  ci != (*gr)->comb_sums.end(); ci++)
    {  
      out << "group[" << (*gr)->id << "].comb_sums[" << (*ci).first << "] = { ";
      for_all_sums (s, (*ci).second)
      {
        out << *s;
        if (s + 1 != (*ci).second.end())
          out << ", ";
      }
      out << " }\n";
    }
  }
  out << "\n\n";

  // b->num_id
  out << "b->id                 =   ";
  for_all_eqs (bj, eqs->b)
  {
    out << (*bj)->id;
    if (bj + 1 != eqs->b.end())
      out << ", ";
  }
  out << "\n";

  // b->num_free_vars
  out << "b->num_free_vars      = { ";

  { // visual studio
    for_all_eqs (bj, eqs->b)
    {
      out << (*bj)->num_free_vars;
      if (bj + 1 != eqs->b.end())
        out << ", ";
    }
  } // visual studio

  out << " }\n";

  // b->num_free_cells
  out << "b->num_free_cells     = { ";

  { // visual studio
    for_all_eqs (bj, eqs->b)
	  {
      out << (*bj)->num_free_cells;
      if (bj + 1 != eqs->b.end())
        out << ", ";
    }
  } // visual studio

  out << " }\n\n";

  // group->id               
  out << "group->id             =   ";

  { // visual studio
    for_all_groups (gr, eqs->groups)
    {
      out << (*gr)->id;
      if (gr + 1 != eqs->groups.end())
        out << ", ";
    }
  } // visual studio

  out << "\n";

  // group->num_free_vars
  out << "group->num_free_vars  = { ";

  { // visual studio
    for_all_groups (gr, eqs->groups)
    {
      out << (*gr)->num_free_vars;
      if (gr + 1 != eqs->groups.end())
        out << ", ";
    }
  } // visual studio

  out << " }\n";

  // group->num_free_cells
  out << "group->num_free_cells = { ";

  { // visual studio
    for_all_groups (gr, eqs->groups)
    {
      out << (*gr)->num_free_cells;
      if (gr + 1 != eqs->groups.end())
        out << ", ";
    }
  } // visual studio

  out << " }\n\n";
}

//******************************************************************************
void VarGroup::createIsles (vector<VarGroup*>&  parts) const // (ok)
//------------------------------------------------------------------------------
//*** Erzeugt eine Liste von nicht zusammenhaengenden Gruppen ***
{
  set<Var*>  x_set (x.begin(), x.end());

  while (x_set.size() > 0)
  {
    // eine Gruppenteil aufbauen
    Var*  xi = *x_set.begin();

    x_set.erase (x_set.begin());

    parts.push_back (new VarGroup);
    parts.back()->x.push_back (xi);

    vector<Var*>  x_remain;
    x_remain.push_back (xi);

    while (x_remain.size() > 0)
    {
      vector<Var*>  x_new;

      for_all_vars (xi2, x_remain)
        for_all_vars (xi3, (*xi2)->x)
        {
          set<Var*>::iterator xi_set = x_set.find (*xi3);

          if (xi_set != x_set.end())
          {
            x_set.erase (xi_set);

            x_new.push_back (*xi3);
            parts.back()->x.push_back (*xi3);
          }
        }

       x_remain = x_new;
    }
  }
}

//******************************************************************************
void VarGroup::split (const VarGroup&  part1, // (ok)
                    VarGroup&        part2,
                    VarGroup&        part3)
//------------------------------------------------------------------------------
// in:  part1   
// out: part2   alle Variablen, die direkt an part1 liegen
//      part3   restlichen Variablen
{
  // init
  part2.x.clear();
  part3.x.clear();
  for_each (x.begin(), x.end(), mem_fun (&Var::clearFlag));

  // part2 - alle Var. aus der Gruppe, die an part1 grenzen
  for_all_vars (xi, part1.x)
    for_each ((*xi)->x.begin(), (*xi)->x.end(), mem_fun (&Var::setFlag));
  for_each (part1.x.begin(), part1.x.end(), mem_fun (&Var::clearFlag));

  { // visual studio
    for_all_vars (xi, x)
      if ((*xi)->flag)
        part2.x.push_back (*xi);
  } // visual studio

  // part3 - Rest
  for_each (part1.x.begin(), part1.x.end(), mem_fun (&Var::setFlag));

  { // visual studio
    for_all_vars (xi, x)
      if (!(*xi)->flag)
        part3.x.push_back (*xi);
  } // visual studio
}

//******************************************************************************
float VarGroup::calcVal (void) const // (ok)
//------------------------------------------------------------------------------
// Bestimmt einen Wert zu der Gruppe. Dieser Wert sollte moeglichst genau
// den erwarteten Aufwand beim Loesen wiederspiegeln.
{
  float val = 1.0;
  for_all_vars (xi, x)
    val *= sqrt ((*xi)->cells.size() + 1) - 0.001; // log ((*xi)->cells.size()); // ???
//     val *= pow (2, (*xi)->cells.size() / 8); // log ((*xi)->cells.size()); // ???

  return val;
}

//******************************************************************************
void VarGroup::unconnectedToTree (vector<VarGroup*>& grp_tab) // (ok)
//------------------------------------------------------------------------------
// Die nichtzusammenhaengenden Gruppen, werden zu einem Baum umgeformt, mit
// *this als Wurzel.
{
  if (grp_tab.size() <= 1)
  {
    if (grp_tab.size() == 1)
       delete grp_tab[0];

    child1 = 0;
    child2 = 0;
  }
  else // grp_tab -> Tree
  {
    while (grp_tab.size() > 2)
    {
      vector<VarGroup*>  new_tab;

      for (unsigned i = 0; i < grp_tab.size(); i++)
      {
        if (i == grp_tab.size() - 1)  // letzter?
        {
          new_tab.push_back (grp_tab[i]);
        }
        else
        {
          // 2 Gruppen zsm.fassen
          new_tab.push_back (new VarGroup);
          new_tab.back()->child1 = grp_tab[i];
          new_tab.back()->child2 = grp_tab[i+1];
          i++; // 1 ueberspringen
        }
      }

      grp_tab = new_tab;
    }

    child1 = grp_tab[0];
    child2 = grp_tab[1];
    x.clear();
  }
}

//******************************************************************************
void VarGroup::buildTree (void) // (ok)
//------------------------------------------------------------------------------
// Zerlege die Gruppe und repraesentiere sie als Baum.
// Versuche dafuer eine moeglichst kleine Menge von Variablen herauszunehmen,
// so dass der Rest in moeglichst zwei (oder mehr) gleich grosse, unzusammen-
// haengende Teile zerfaellt (!! das sind zwei Optimierungsziele).
// Diese werden anschliessend rekursiv weiter zerlegt.
//
// Algorithmus: 1. Suche eine Variable mit moeglichst wenigen Nachbarn
//                 (das erhoeht die Chance, dass es sich um eine 
//                  'Randvariable' handelt).
//              2. Bestimme alle Variablen, die direkt an der ersten liegen.
//                 (Das ist die oben genannt Menge von Variablen, da die
//                  restlichen Variablen nicht mit der ersten zusammenhaengen
//                  koennen. (->VarGroup::split))
//              3. Erweitere nun iterativ die Menge mit der ersten Variablen um 
//                 eine moeglichst 'gute' zusaetzliche Variable. Bewerte nun 
//                 alle diese moeglichen Zerlegungen und bestimme davon die       
//                 beste.

// Fehler: best_xi darf in part1 nicht schon vorkommen (ok)
// Opt.:   am Rand anfangen (ok)
// Opt.:   kein split jedesmal (ok)
// Opt.:   best_xi sollte part1 beruehren (ok)
// Opt.:   keine Aufteilung, falls x klein ist (ok)
// Opt.:   unzsmhaengede Gruppen zu Anfang bilden (ok)
// Opt.:   zuerst Bruecken suchen?!
{
  // x aufteilen?
  if (x.size() >= 5 && child1 == 0 && child2 == 0)
  {
    VarGroup    part1, part2, part3;
    unsigned  best_sz     = 0;
    float     best_sz_val = calcVal();

    // 1. Var -> part1
    Var*  best_xi = x[0];

    for_all_vars (pxi, x)
    {
      Var*  xi = *pxi;

      if (xi->x.size() <  best_xi->x.size()
      ||  xi->x.size() == best_xi->x.size()
          && xi->cells.size() > best_xi->cells.size())
         best_xi = xi;
    }

    part1.x.push_back (best_xi);

    // part1 sukzessiv vergroessern
    for (unsigned sz = 2; sz <= x.size() - 3; sz++)
    {
      // poss
      VarGroup  poss, dummy;
      split (part1, poss, dummy);

      // suche opt. xi
      float  best_xi_val = 1E30f; // g++: numeric_limits<float>::max();

      for_all_vars (pxi, poss.x)
      {
        Var*  xi = *pxi;

        // xi testweise in part1 packen
        part1.x.push_back (xi);

        // val
        vector<VarGroup*>  grp_tab;
        float           val; // = 0.0

        split (part1, part2, part3);

        // zu langsam
//      part1.createIsles (grp_tab);
//      part3.createIsles (grp_tab);

//      for_all_groups (g, grp_tab)
//      {
//        val += (*g)->calcVal();
//        delete *g;
//      }

//      val *= part2.calcVal();

        val = part2.calcVal() * (part1.calcVal() + part3.calcVal());

        // best_xi
        if (val < best_xi_val)
        {
           best_xi_val = val;
           best_xi     = xi;
        }

        // xi wieder aus part1 entfernen
        part1.x.erase (part1.x.end() - 1);
      }

      // best_xi in jedem Fall in part1 packen
      part1.x.push_back (best_xi);

      // best_sz
      if (best_xi_val < best_sz_val)
      {
        best_sz_val = best_xi_val;
        best_sz     = sz;
      }
    }

    // part1, part2, part3
    if (best_sz != 0)
    {
      part1.x.erase (part1.x.begin() + best_sz, part1.x.end());
      split (part1, part2, part3);

      if (part1.x.size() > 0 && part3.x.size() > 0)
      {
        child1 = new VarGroup;
        child2 = new VarGroup;

        x         = part2.x;
        child1->x = part1.x;
        child2->x = part3.x;
      }
/*
      // grp_tab
      vector<VarGroup*>  grp_tab;

      part1.createIsles (grp_tab);
      part3.createIsles (grp_tab);

      // aufteilen
      if (grp_tab.size() == 1)
      {
        unconnectedToTree (grp_tab);
      }
      else if (grp_tab.size() > 1)
      {
        unconnectedToTree (grp_tab);
        x = part2.x;
      }
*/
    }
  } // x.size() != 0

   // rek. weiter
  if (child1 != 0)
    child1->buildTree();

  if (child2 != 0)
    child2->buildTree();
}

//******************************************************************************
bool VarGroup::init (int pass) // nur dyn.  (ok)
//------------------------------------------------------------------------------
{
  num_free_vars = x.size();

  num_free_cells = 0;
  for_all_vars (xi, x)
    num_free_cells += (*xi)->cells.size();

  if (pass == 1)
    comb_sums.clear();
  cur_sums = 0;

  note_one_sol = false;

  return true; // nur fuer den scheiss Compiler
}

//******************************************************************************
Conclusion VarGroup::conclude (vector<Var*>& ties) // (ok)
//------------------------------------------------------------------------------
// Algorithmus kann noch folgendermassen verbessert werden:
// 1. Liste mit allen Eq's bilden
// 2. Folgerungen
// 3. Liste mit allen betroffenen Vars bilden
// 4. neue Liste der Eq's = Nachbarn der oben genanntem Vars
{
  PERF_ANA ("VarGroup::conclude");

  if (eqs->conclusion != CONCL_NOT_TRIED)
  {
    if (eqs->conclusion == CONCL_CONTRA)  return CONCL_CONTRA;
    else if (num_free_vars == 0)          return CONCL_SOLUTION;
    else if (num_free_vars >  0)          return CONCL_NOT_COMPLETE;
  }

  bool ready = false;

  while (!ready)
  {
    ready = true;

    for_all_eqs (pbj, eqs->b)
    {
      Eq*  bj = *pbj;

      // Widerspruch? -> zurueck
      if (bj->val < 0 || bj->num_free_cells < bj->val)
      {
        eqs->conclusion = CONCL_CONTRA;
        return CONCL_CONTRA;
      }
      else if (bj->num_free_vars > 0) // oder free_cells > 0
      {
        // Eine Gleichung loesbar?
        if (bj->val == 0)
        {
          for_all_vars (xi, bj->x)
            if ((*xi)->val == -1)
            {
              (*xi)->tie (0);
              ties.push_back (*xi);
            }
        }
        else if (bj->val == bj->num_free_cells)
        {
          for_all_vars (xi, bj->x)
            if ((*xi)->val == -1)
            {
              (*xi)->tie ((*xi)->cells.size());
              ties.push_back (*xi);
            }
        }
        else if (bj->num_free_vars == 1)
        {
          for_all_vars (xi, bj->x)
            if ((*xi)->val == -1)
            {
              (*xi)->tie (bj->val);
              ties.push_back (*xi);
            }
        }
        else // nich loesbar
          continue;

        // noch einmal alle b's durchlaufen
        ready = false;
      }
    }
  }

  if (num_free_vars == 0)   // aequiv. zu num_free_cells == 0
    eqs->conclusion = CONCL_SOLUTION;
  else
    eqs->conclusion = CONCL_NOT_COMPLETE;

  return  eqs->conclusion;
}

//******************************************************************************
string VarGroup::calcCombkey (void) // (ok)
//------------------------------------------------------------------------------
{
  string  key = (parent == 0) ? string("")
                              : parent->calcCombkey();
  for_all_vars (xi, x)
    key += (char) ('0' + (*xi)->val);

  return  key;
}

//******************************************************************************
int VarGroup::sumVal (void) // (ok)
//------------------------------------------------------------------------------
{
  int  s = 0;

  for_all_vars (xi, x)
    s += (*xi)->val;

  return s;
}

//******************************************************************************
void VarGroup::clearSols (int the_deep) // (ok)
//------------------------------------------------------------------------------
//*** Loescht Loesungen auf der Stufe 'the_deep' (inkl. Untergruppen)
{
  for_all_vars (xi, x)
    (*xi)->sols[the_deep].clear();

  if (child1 != 0)
    child1->clearSols (the_deep);

  if (child1 != 0)
    child2->clearSols (the_deep);
}

//******************************************************************************
void VarGroup::raiseSols (int to_deep) // (ok)
//------------------------------------------------------------------------------
// Erhoehe alle Loesungen der Gruppe (und Untergruppen)
// von 'to_deep+1' nach 'to_deep'.
{
  for_all_vars (xi, x)
    (*xi)->sols[to_deep].insert ((*xi)->sols[to_deep+1]);

  if (child1 != 0)
    child1->raiseSols (to_deep);

  if (child1 != 0)
    child2->raiseSols (to_deep);
}

//******************************************************************************
bool VarGroup::noteSol (SolSums* permit_sums) // (ok)
//------------------------------------------------------------------------------
{
  // comb_sums, cur_sums
  int  sum = sumVal();

  // 1. Durchgang?
  if (permit_sums == 0)
  {
    if (child1 == 0 || child2 == 0)
    {
      cur_sums->insertInt (sum);
    }
    else
    {
      string  comb_key = calcCombkey();

      SolSums  temp_sums = *child1->cur_sums;
      temp_sums.add (sum);
      temp_sums.add (*child2->cur_sums);

      cur_sums->insert (temp_sums);
    }
  }
  else // 2. Durchgang - cur_sums nicht aendern
  {
    if ((child1 == 0 || child2 == 0)
    &&  find (permit_sums->begin(), permit_sums->end(), sum)
        == permit_sums->end())
      return false;
  }

  // x->sols
  for_all_vars (xi, x)
    (*xi)->sols[deep].insertInt ((*xi)->val);

  if (child1 != 0 && child2 != 0)
  {
    child1->raiseSols (deep);
    child2->raiseSols (deep);
  }

  // note_one_sol
  if (note_one_sol
  &&  (child1 == 0 || !child1->note_one_sol)
  &&  (child2 == 0 || !child2->note_one_sol))
  {
    bool ok = true;

    for_all_vars (xi, x)
      if ((*xi)->val < (*xi)->one_sol_val_min
      ||  (*xi)->one_sol_val_max < (*xi)->val)
        ok = false;

    if (ok)
    {
      for_all_vars (xi, x)
        (*xi)->one_sol_val = (*xi)->val;

      note_one_sol = false;
    }
  }

  return true;
}

//******************************************************************************
bool VarGroup::solve (SolSums* permit_sums) // (ok)
//------------------------------------------------------------------------------
{
  PERF_ANA ("VarGroup::solve");

  if (eqs->diff_clock >= 0 && clock() - eqs->clock0 > eqs->diff_clock)
    throw OutOfTime();

  vector<Var*>   ties;
  Conclusion     conclusion = conclude (ties);
  bool           found_sol = false;

  // Trace
  if (Glob::trace_level >= 2)
    printDyn();

  // noch freie Var. in der Gruppe? --------------------------------------------
  if (conclusion == CONCL_NOT_COMPLETE)
  {
    // Trace
    if (Glob::trace_level >= 2)
      eqs->logVal (' ');

    // freie Var. suche
    VarCIter  pxi = find_if (x.begin(), x.end(), mem_eq (&Var::val, -1));
    if (pxi != x.end())
    {
      Var*  xi = *pxi;

      // rekursiv - gleiche Gruppe
      for (int v = xi->val_min; v <= xi->val_max; v++)
      {
        // !!! ev. Reihenfolge der Werte aendern, um fuer foundOneSol eine
        //     mehr zufaellige Loesung zu finden.

        xi->tie (v, true);

        if (solve (permit_sums))
          found_sol = true;

        xi->untie();
      }
    }
  }
  // alle Var. in der Gruppe gebunden? -----------------------------------------
  else if (conclusion == CONCL_SOLUTION)
  {
    // rekursiv - Untergruppen?
    if (child1 == 0 || child2 == 0) // aequiv. (child1 != 0 || child2 != 0)
    {
      found_sol = noteSol (permit_sums);

      // Trace
      if (Glob::trace_level >= 2)
        eqs->logVal ((char) ('0' + id));
    }
    else // mit childs
    {
      // Trace
      if (Glob::trace_level >= 2)
        eqs->logVal (' ');

      string  comb_key = calcCombkey();

      child1->cur_sums = &child1->comb_sums[comb_key];
      child2->cur_sums = &child2->comb_sums[comb_key];

      child1->clearSols (deep + 1);
      child2->clearSols (deep + 1);

      // note_one_sol
      child1->note_one_sol = note_one_sol;
      child2->note_one_sol = note_one_sol;

      // 1. Durchgang?
      if (permit_sums == 0)
      {
        // rek
        if (child1->solve()
        &&  child2->solve())
          found_sol = noteSol (permit_sums);
      }
      // 2. Durchgang
      else if (child1->cur_sums->size() > 0 && child2->cur_sums->size() > 0)
      // Die cur_sums.sizes koennen Null wenn diese Kombination nichts 
      // zur Loesung beigetragen hat
      {
        int      min_child1, min_child2, min_permit2;
        int      max_child1, max_child2, max_permit2;
        SolSums  permit_sums2;

        // child1
        permit_sums2 = *permit_sums;
        permit_sums2.sub (sumVal());
        permit_sums2.sub (*child2->cur_sums);

        min_permit2 = *min_element (permit_sums2.begin(), permit_sums2.end());
        max_permit2 = *max_element (permit_sums2.begin(), permit_sums2.end());
        min_child1  = *min_element (child1->cur_sums->begin(),
                                    child1->cur_sums->end());
        max_child1  = *max_element (child1->cur_sums->begin(),
                                    child1->cur_sums->end());
        // ... rek
        if (min_permit2 <= max_child1 && min_child1 <= max_permit2
        &&  child1->solve (&permit_sums2))
        {
          // child2
          permit_sums2 = *permit_sums;
          permit_sums2.sub (sumVal());
          permit_sums2.sub (*child1->cur_sums);

          min_permit2 = *min_element (permit_sums2.begin(), permit_sums2.end());
          max_permit2 = *max_element (permit_sums2.begin(), permit_sums2.end());
          min_child2  = *min_element (child2->cur_sums->begin(),
                                      child2->cur_sums->end());
          max_child2  = *max_element (child2->cur_sums->begin(),
                                      child2->cur_sums->end());
          // ... rek
          if (min_permit2 <= max_child2 && min_child2 <= max_permit2
          &&  child2->solve (&permit_sums2))
            found_sol = noteSol (permit_sums);
        }
      }

      // Trace
      if (Glob::trace_level >= 2)
        eqs->logVal ((char)('0' + id));
    }
  }
  else // Widerspruch
  {
    // Trace
    if (Glob::trace_level >= 2)
      eqs->logVal ('-');
  }

  if (Glob::trace_level >= 2)
    printDyn();

  // aufraeumen
  for_all_vars (xi, ties)
    (*xi)->untie();

  return found_sol;
}

