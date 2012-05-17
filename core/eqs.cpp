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

//#include <Forms.hpp>  // Exception

#include <iterator>
#include <iomanip>

#ifdef VISUAL_CPP
  #include <minmax.h> // visual studio
#endif

using namespace std;

#include "eqs.h"
#include "api.h"
#include "perfana.h"

// Konstanten zur Fehlersuche
namespace MinesPerfect
{
  namespace Glob
  {
    extern int     trace_level;
    extern RandInt rand_int; // ohne Wert -> randomize
  }
}

using namespace MinesPerfect;

template<class T> T* delete_ptr (T* p) { delete p; return 0; }

//******************************************************************************
bool operator== (Move m1, Move m2)
//------------------------------------------------------------------------------
{
  return m1.cell_nr == m2.cell_nr && m1.new_state == m2.new_state;
}

//******************************************************************************
bool operator!= (Move m1, Move m2)
//------------------------------------------------------------------------------
{
  return m1.cell_nr != m2.cell_nr || m1.new_state != m2.new_state;
}

//******************************************************************************
bool Eq::init (void) // nur dyn.  (ok)
//------------------------------------------------------------------------------
{
  num_free_vars = x.size();

  num_free_cells = 0;
  for_all_vars (xi, x)
    num_free_cells += (*xi)->cells.size();

  return true; // nur fuer den scheiss Compiler
}

//******************************************************************************
Eqs::Eqs (const vector<Cell>& cells, bool (Cell::* isVar)(void) const)   // (in Erweiterung)
//------------------------------------------------------------------------------
{
  PERF_ANA ("Eqs::Eqs");

  vector<Eq*>  k_to_b (cells.size(), 0); // Zelle -> Var

  // imposs_conds, inner_flags
  if (isVar == &Cell::isVisibleVar)
  {
    for (CellNr k = 0; k < (CellNr) cells.size(); k++)
      if (cells[k].isImpossCond())
        imposs_conds.push_back (k);
      else if (cells[k].isFlagged() && !cells[k].isSolved()
           &&  cells[k].num_open_adj == 0)
        inner_flags.push_back (k);
  }
  else if (isVar != &Cell::isRealVar)
  {
    ; // !!! exception
  }

  // b, x
  for (CellNr k = 0; k < (CellNr) cells.size(); k++)
  {
    if ((cells[k].*isVar)())
    {
      Var*  xi = new Var;

      x.push_back (xi);

      xi->id = x.size() - 1;
      xi->cells.push_back (k);

      for_all_adj (k2, k)
      {
        if (cells[*k2].isCond())
        {
          // Eq neu?
          if (k_to_b[*k2] == 0)
          {
            // b
            Eq*  bj = new Eq;
     
            b.push_back (bj);
     
            bj->id   = b.size() - 1;
            bj->cell = *k2;
            bj->val  = cells[*k2].num;
     
            if (isVar == &Cell::isVisibleVar)
            {
              bj->val -= cells[*k2].num_flagged_adj;
            }
            else // real
            {
              for_all_adj (k3, *k2)
                if (cells[*k3].isSolved() && cells[*k3].isMined())
                  bj->val--;
            }

            // k_to_b
            k_to_b[*k2] = bj;
          }

          // Var <-> Cond
          k_to_b[*k2]->x.push_back (xi);
          xi->b.push_back (k_to_b[*k2]);
        }
      }
    }
  }

  // inner_cells, num_vars_inside, num_mines_remain
  num_vars_inside  = 0;
  num_mines_remain = 0;

  { // visual studio
  for (CellNr k = 0; k < (CellNr) cells.size(); k++)
  {
    // visible
    if (isVar == &Cell::isVisibleVar)
    {
      if (cells[k].isClean() && cells[k].num_open_adj == 0)
      {
        num_vars_inside++;
        inner_cells.push_back(k); // wird eigentlich nur bei real benutzt
      }
      
      if (cells[k].isMined())
        num_mines_remain++;

      if (cells[k].isFlagged())
        num_mines_remain--;
    }
    else // real
    {
      if (!cells[k].isOpen() && cells[k].num_open_adj == 0
      &&  !cells[k].isSolved()) // fuer Startup-Modus
      {
        num_vars_inside++;
        inner_cells.push_back(k);
      }
      
      if (cells[k].isMined() && !cells[k].is_solved)
        num_mines_remain++;
    }
  }
  } // visual studio

  // no_move_stage, solve_stage
  no_move_stage = 0; // nocht nichts geloest
  solve_stage = 0;

  //
  x_and_b_optimized = false;

  // trace
  if (Glob::trace_level >= 1)
    printStat();
}

//******************************************************************************
Eqs::~Eqs() // (ok)
//------------------------------------------------------------------------------
{
   for_all_vars (xi, x)
      delete *xi;

   for_all_eqs (bj, b)
      delete *bj;

   for_all_groups (gr, groups)
      delete *gr;
}

//******************************************************************************
void Eqs::printStat (void) const // (ok)
//------------------------------------------------------------------------------
{
  ofstream  out ("eqs.txt");

  // x
  for_all_vars (pxi, x)
  {
    Var*  xi = *pxi;

    out << "x" << xi->id << ": k=";
    for_all (CellNrCIter, k, xi->cells)
      out << *k << ",";
    out << " ";

    out << "[" << xi->val_min << "-" << xi->val_max << "], ";

    out << "b=";
    for_all_eqs (bj, xi->b)
      out << (*bj)->id << ",";
    out << " ";

    out << "x=";
    for_all_vars (xi2, xi->x)
      out << (*xi2)->id << ",";

    out << "\n";
  }
  out << "\n";

  // b
  for_all_eqs (pbj, b)
  {
    Eq*  bj = *pbj;

    out << "b" << bj->id << ": k=" << bj->cell << ", ";
    out << "val=" << bj->val << ", ";

    out << "x=";
    for_all_vars (xi, bj->x)
      out << (*xi)->id << ",";
    out << " ";

    out << "b=";
    for_all_eqs (bj2, bj->b)
      out << (*bj2)->id << ",";
    out << "\n";
  }
  out << "\n";

  // groups
  for_all_groups (pgr, groups)
  {
    VarGroup*  gr = *pgr;

    out << "g" << gr->id;

    out << ": childs=";
    if (gr->child1 == 0)  out << "-";
    else                  out << gr->child1->id;

    out << ",";
    if (gr->child2 == 0)  out << "-";
    else                  out << gr->child2->id;

    out << ", x=";
    for_all_vars (xi, gr->x)
      out << (*xi)->id << ",";
    out << "\n";
  }
  out << "\n\n";

  // board
  int   board_height = 8;
  int   board_width  = 8;
  vector<CellNr>  cell_b (board_height * board_width, -1);
  vector<CellNr>  cell_x (board_height * board_width, -1);
  vector<CellNr>  cell_g (board_height * board_width, -1);

  for_all_eqs (bj, b)
    if ((*bj)->cell < (int) cell_b.size())
      cell_b[(*bj)->cell] = (*bj)->id;

  for_all_vars (xi, x)
    for (CellNr k = 0; k < (CellNr) (*xi)->cells.size(); k++)
      if ((*xi)->cells[k] < (CellNr) cell_x.size())
        cell_x[(*xi)->cells[k]] = (*xi)->id;

  { // visual studio
  for_all_vars (xi, x)
    for (CellNr k = 0; k < (CellNr) (*xi)->cells.size(); k++)
      if ((*xi)->cells[k] < (CellNr) cell_g.size())
        cell_g[(*xi)->cells[k]] = (groups.size() == 0) ? 0 : (*xi)->group->id;
  } // visual studio

  out << "b:";
  for (int pos_y = 0; pos_y < board_height; pos_y++)
    out << "   ";

  { // visual studio
  out << "x:";
  for (int pos_y = 0; pos_y < board_height; pos_y++)
    out << "   ";
  } // visual studio

  { // visual studio
  out << "g:";
  for (int pos_y = 0; pos_y < board_height; pos_y++)
    out << "   ";
  out << "\n";
  } // visual studio

  { // visual studio
  for (int pos_y = 0; pos_y < board_height; pos_y++)
  {
    for (int pos_x = 0; pos_x < board_width; pos_x++)
    {
      CellNr k = pos_y * board_height + pos_x;

      if (cell_b[k] == -1)  out << setw (3) << "-";
      else                  out << setw (3) << cell_b [k];
    }
    out << "  ";

    { // visual studio
      for (int pos_x = 0; pos_x < board_width; pos_x++)
      {
        CellNr k = pos_y * board_height + pos_x;

        if (cell_x[k] == -1)  out << setw (3) << "-";
        else                  out << setw (3) << cell_x [k];
      }
      out << "  ";
    } // visual studio

    { // visual studio
      for (int pos_x = 0; pos_x < board_width; pos_x++)
      {
        CellNr k = pos_y * board_height + pos_x;

        if (cell_g[k] == -1)  out << setw (3) << "-";
        else                  out << setw (3) << cell_g [k];
      }
      out << "\n";
    } // visual studio
  }
  out << "\n";
  } // visual studio
}

//******************************************************************************
void Eqs::logVal (char res) const // (ok)
//------------------------------------------------------------------------------
{
  if (res == 0)
  {
    ofstream  out ("val.txt"); 

    // x->group->id
    for_all_vars (xi, x)
      out << setw (2) << (*xi)->group->id << " ";
    out << "\n";

    // x->id
    { // visual studio
      for_all_vars (xi, x)
        out << setw (2) << (*xi)->id << " ";
      out << "\n";
    } // visual studio
  }
  else
  {
//    ofstream  out ("val.txt", ios_base::out | ios_base::app);
    ofstream  out ("val.txt", ios::out | ios::app);

    // x->val
    for_all_vars (xi, x)
    {
      if ((*xi)->val == -1)
        out << " . ";
      else if ((*xi)->manual)
        out << setw (2) << (*xi)->val << "*";
      else
        out << setw (2) << (*xi)->val << " ";
    }
    out << "  " << res << "\n";
  }
}

//******************************************************************************
void Eqs::setXtoX (void) // (ok)
//------------------------------------------------------------------------------
// Erstellt die Verzeigerung x->x
{
  for_all_vars (xi, x)
    (*xi)->x.clear();

  // x->x
  for_all_vars (pxi, x)
  {
    Var*  xi = *pxi;

    // x->flag
    for_all_vars (xi2, x)
      (*xi2)->flag = false;
    xi->flag = true;  // kein Verweis auf sich selber

    // xi->x
    for_all_eqs (bj, xi->b)
    {
      for_all_vars (pxi2, (*bj)->x)
      {
        Var*  xi2 = *pxi2;

        if (!xi2->flag)
        {
          xi2->flag = true;
          xi->x.push_back (xi2);
        }
      }
    }
  }
}

//******************************************************************************
void Eqs::setBtoB (void) // (ok)
//------------------------------------------------------------------------------
// Erstellt die Verzeigerung b->b
{
  for_all_eqs (bj, b)
    (*bj)->b.clear();

  // b->b
  for_all_eqs (pbj, b)
  {
    Eq*  bj = *pbj;

    // b->flag
    for_all_eqs (bj2, b)
      (*bj2)->flag = false;
    bj->flag = true;  // kein Verweis auf sich selber

    // bj->b
    for_all_vars (xi, bj->x)
    {
      for_all_eqs (pbj2, (*xi)->b)
      {
        Eq*  bj2 = *pbj2;

        if (!bj2->flag)
        {
          bj2->flag = true;
          bj->b.push_back (bj2);
        }
      }
    }
  }
}

//******************************************************************************
void Eqs::simplifyB (void)  // (ok)
//------------------------------------------------------------------------------
// Falls eine Gleichung alle Var. einer anderen enthaelt, so wird die kleinere
// von der groesseren abgezogen. Z.B.
//   b0 b1 x3 |        b1 = { x = {0,1,2,3}, val = 2 }
//   x0 x1 x2 |        b0 = { x = {0,1},     val = 1 }
//            |    ->  b1 = { x = {    2,3}, val = 1 }
{
  if (x_and_b_optimized)
    return;

  // sortieren
  for_all_eqs (bj, b)
    sort ((*bj)->x.begin(), (*bj)->x.end()); // Sortierkriterium egal,
                                             // aber fuer alle x's gleich
  // b->x vereinfachen
  bool ready = false;

  while (!ready)
  {
    ready = true;

    //
    for_all_eqs (pbj, b)
    {
      Eq*  bj = *pbj;

      //
      for_all_eqs (pbj2, bj->b)
      {
        Eq*  bj2 = *pbj2;

        // bj2->x Teilmenge von bj->x?
        if (bj != bj2
        &&  0 < bj2->x.size() && bj2->x.size() <= bj->x.size()
        &&  includes (bj->x.begin(), bj->x.end(), bj2->x.begin(), bj2->x.end()))
        {
          vector<Var*>  new_x;

          set_difference (bj->x.begin(),  bj->x.end(),
                          bj2->x.begin(), bj2->x.end(),
                          inserter (new_x, new_x.begin()));

          bj->x    = new_x;
          bj->val -= bj2->val;

          for_all_vars (xi, bj2->x)
            (*xi)->b.erase (remove ((*xi)->b.begin(), (*xi)->b.end(), bj),
                            (*xi)->b.end());

          ready = false;
        }
      }
    }
  }

  // ueberfluessige b's loeschen
  for (unsigned j = 0; j < b.size(); j++)  // for_all nur fuer const bj's
    if (b[j]->XisEmpty())
    {
      delete b[j];
      b[j] = 0;
    }

  b.erase (remove (b.begin(), b.end(), (void*) 0), b.end());
}

//******************************************************************************
void Eqs::uniteX (void) // Vars mit gleichen Conds zsm-fassen   (ok)
//------------------------------------------------------------------------------
// Variablen, die immer nur zusammen in Gleichungen auftauchen werden zu einer
// zusammengefasst. Z.B:
//   b0 b1 x3 |                x0   x1   x2   x3
//   x0 x1 x2 |        cells:   8    9   10    2
//            |   ->   cells:   8    9  2,10        (x3 kommt weg)
{
  if (x_and_b_optimized)
    return;

  // sortieren
  for_all_vars (xi, x)
    sort ((*xi)->b.begin(), (*xi)->b.end()); // Sortierkriterium egal,
                                             // aber fuer alle x's gleich

  #ifdef VISUAL_CPP
    sort (x.begin(), x.end(), mem_fun1 (&Var::cmpB)); // visual studio
  #else
    sort (x.begin(), x.end(), mem_fun (&Var::cmpB));
  #endif

  // xis mit gleichen bjs zsm.fassen
  for (unsigned i = 0; i < x.size() - 1; i++)
  {
    // x[i]->b != x[i+1]->b?
    if (x[i]->cmpB(x[i+1]) || x[i+1]->cmpB(x[i]))
      continue;

    // ks uebernehmen
    x[i]->cells.push_back (x[i+1]->cells.front());

    // Verweise von x[i+1] loeschen
    for_all_eqs (bj, x[i+1]->b)
      (*bj)->x.erase (remove ((*bj)->x.begin(), (*bj)->x.end(), x[i+1]),
                      (*bj)->x.end());

    // x[i+1] loeschen
    delete x[i+1];
    x.erase (x.begin() + i + 1);

    i--;
  }
}

//******************************************************************************
void Eqs::buildGroups (void) // (ok)
//------------------------------------------------------------------------------
{
  //*** Baue Gruppen auf ***
  PERF_ANA ("Eqs::buildGroups");

  if (groups.size() > 0)
    return;

  // erste Gruppe
  groups.push_back (new VarGroup);
  groups[0]->id   = 0;
  groups[0]->x    = x;

  // Unzusammenhaengede Gruppen aufbauen
  vector<VarGroup*>  grp_tab;
  groups[0]->createIsles (grp_tab);
  groups[0]->unconnectedToTree (grp_tab);

  // Rest des Baumes aufbauen
  groups[0]->buildTree();

  // Baum -> groups - groups waechst dyn.
  for (unsigned r = 0; r < groups.size(); r++)
  {
    if (groups[r]->child1 != 0)
    {
      groups.push_back (groups[r]->child1);
      groups.back()->id = groups.size() - 1;
    }
    if (groups[r]->child2 != 0)
    {
      groups.push_back (groups[r]->child2);
      groups.back()->id = groups.size() - 1;
    }
  }

  // group -> parent
  groups[0]->parent = 0;

  for_all_groups (pgr, groups)
  {
    VarGroup*  gr = *pgr;

    if (gr->child1 != 0)
      gr->child1->parent = gr;

    if (gr->child2 != 0)
      gr->child2->parent = gr;
  }

  // group -> deep (nutzt aus: Kind-Id > Eltern-Id)
  for_all_groups (gr, groups)
    if ((*gr)->parent == 0)
      (*gr)->deep = 0;
    else
      (*gr)->deep = (*gr)->parent->deep + 1;

  // group -> eqs
  { // visual studio
  for_all_groups (gr, groups)
    (*gr)->eqs = this;
  } // visual studio

  // var -> group
  { // visual studio
  for_all_groups (gr, groups)
    for_all_vars (xi, (*gr)->x)
      (*xi)->group = *gr;
  } // visual studio

  // var->val_min, var->val_max
  for_all_vars (pxi, x)
  {
    Var*  xi = *pxi;
    xi->val_min = 0;
    xi->val_max = min ((int) xi->cells.size(),
                       (*min_element (xi->b.begin(), xi->b.end(),
                                      mem_cmp (&Eq::val)))->val);
  }

  // var fuer Trace sortieren und neue Id's
  #ifdef VISUAL_CPP
    sort (x.begin(), x.end(), mem_fun1 (&Var::cmpGroupId)); // visual studio
  #else
    sort (x.begin(), x.end(), mem_fun (&Var::cmpGroupId));
  #endif

  for (unsigned i = 0; i < x.size(); i++)
    x[i]->id = i;

  // trace
  if (Glob::trace_level >= 1)
    printStat();
}

//******************************************************************************
void Eqs::init (const int pass) // nur dyn.  (ok)
//------------------------------------------------------------------------------
{
  // x, b
  for_each (x.begin(), x.end(), mem_fun (&Var::init));
  for_each (b.begin(), b.end(), mem_fun (&Eq::init));

  // groups
//  for_each (groups.begin(), groups.end(),
//            bind2nd (mem_fun1 (&VarGroup::init), pass));
  for_all_groups (gr, groups)
    (*gr)->init (pass);

  groups[0]->cur_sums = &groups[0]->comb_sums[""];

  // x->sols.resize()
  int  max_deep = (*max_element (groups.begin(), groups.end(),
                                 mem_cmp (&VarGroup::deep)))->deep;
  for_all_vars (xi, x)
    (*xi)->sols.resize (max_deep + 1);

  // rest
  conclusion = CONCL_NOT_TRIED;
}

//******************************************************************************
Found Eqs::findMoves1 (Moves& moves, CellNrs* const hints) // Stage 1  (Erweiterung testen)
//------------------------------------------------------------------------------
// sucht Zuege mit Stage 1
// -----------------------
// Falls hints != 0 -> suche nur einen Zug, sonst alle
// Falls Widerspruch gefunden?
//   hints != 0 -> Nur hints setzen, (moves bleibt leer)? - geflaggten Felder?
//   hints == 0 -> moves enthaelt hints? - geflaggte Felder?
{
  PERF_ANA ("Eqs::findMoves1");

  // init
  moves.clear();

  if (hints != 0)
    hints->clear();

  if (no_move_stage >= 1)
    return FOUND_NO_MOVE;

  // Widerspruch (beim Aufbau entdeckt)?
  if (imposs_conds.size() > 0)
  {
    // Nur 1 Zug?
    if (hints != 0)
      hints->push_back (imposs_conds[0]);
    else // alle Zuege
      for_all (CellNrCIter, k, imposs_conds)
        moves.push_back (Move (*k, CLEAN));

    return FOUND_CONTRA;
  }
  // Flaggen im Inneren
  else if (inner_flags.size() > 0)
  {
    // Nur 1 Zug?
    if (hints != 0)
    {
      hints->push_back (inner_flags[0]);
      moves.push_back  (Move (inner_flags[0], CLEAN));
    }
    else // alle Zuege
      for_all (CellNrCIter, k, inner_flags)
        moves.push_back (Move (*k, CLEAN));

    return FOUND_MOVE;
  }
  // Nur 1 Zug?
  else if (hints != 0)
  {
    // options
    if (x.size() == 0)  // x.size() > 0 -> b.size() > 0
      return FOUND_NO_MOVE;

    for_all_eqs (pbj, b)
    {
      Eq*  bj = *pbj;

      int num_cells = 0;
      for_all_vars (xi, bj->x)
        num_cells += (*xi)->cells.size();

      // Zug - open
      if (bj->val == 0)
      {
        moves.push_back (Move (bj->x[0]->cells[0], OPEN));
        hints->push_back (bj->cell);
        return FOUND_MOVE;
      }
      // Zug - flag
      else if (num_cells == bj->val)
      {
        moves.push_back (Move (bj->x[0]->cells[0], FLAGGED));
        hints->push_back (bj->cell);
        return FOUND_MOVE;
      }
    }

    no_move_stage = 1;

    if (solve_stage < 1)
      solve_stage = 1;

    return FOUND_NO_MOVE;
  }
  else // alle Zuege
  {
    vector<Var*>  ties;

    // init
    for_all_groups (gr, groups)
      delete *gr;
    groups.clear();

    groups.push_back (new VarGroup);
    groups[0]->eqs  = this;
    groups[0]->x    = x;

    for_all_vars (xi, x)
      (*xi)->group = groups[0];

    init();

    // loesen
    if (groups[0]->conclude (ties) == CONCL_CONTRA)
    {
      // Widersprueche! (nicht beim Aufbau entdeckte)
      for_all_eqs (bj, b)
        moves.push_back (Move ((*bj)->cell, CLEAN));

      return FOUND_CONTRA;
    }

    // Loesungen notieren
    { // visual studio
    for_all_vars (xi, ties)
      if ((*xi)->val == 0)
        for (unsigned l = 0; l < (*xi)->cells.size(); l++)
          moves.push_back (Move ((*xi)->cells[l], OPEN));
      else if ((*xi)->val == (int) (*xi)->cells.size())
        for (unsigned l = 0; l < (*xi)->cells.size(); l++)
          moves.push_back (Move ((*xi)->cells[l], FLAGGED));
    } // visual studio

    // aufraeumen
    { // visual studio
    for_all_vars (xi, ties)
      (*xi)->untie();
    } // visual studio

    delete groups[0];
    groups.clear();

    // no_move_stage, solve_stage
    if (moves.size() == 0)
      no_move_stage = 1;

    if (solve_stage < 1)
      solve_stage = 1;

    return (moves.size() > 0) ? FOUND_MOVE : FOUND_NO_MOVE;
  }
}

//******************************************************************************
Found Eqs::findMoves2 (Moves& moves, CellNrs* const hints) // Stage 2 (Erweiterung testen)
//------------------------------------------------------------------------------
// sucht Zuege mit Stage 2
// -----------------------
// Falls hints != 0 -> suche nur einen Zug, sonst alle
// Falls Widerspruch gefunden?
//   hints != 0 -> Nur hints setzen, moves bleibt leer
//   hints == 0 -> moves enthaelt hints

// Widerspruch Beispiele
// 1.)  ? 3      2.)  ? 3      3.)  ? ? ? x
//      ? ? ?         ? ? ?           4 3
//      x 2           x 2 ?           ? ? ?
{
  PERF_ANA ("Eqs::findMoves2");

  bool  contra2_exist = false; // Widerspruch gefunden (bei Stage 2)

  // init
  moves.clear();

  if (hints != 0)
    hints->clear();

  // options
  if (x.size() == 0 || no_move_stage >= 2)
    return FOUND_NO_MOVE;

  ASSERT (!x_and_b_optimized);

  // x's muessen sortiert sein
  for_all_eqs (bj, b)
    sort ((*bj)->x.begin(), (*bj)->x.end()); // Sortiertkriterium egal

  // xi
  for_all_vars (pxi, x)
  {
    Var*  xi = *pxi;

    // bj
    for_all_eqs (pbj, xi->b)
    {
      Eq*  bj = *pbj;

      // bj2
      for_all_eqs (pbj2, xi->b)
      {
        Eq*  bj2 = *pbj2;

        if (bj >= bj2) // Moeglichst wenig Widerholungen
          continue;

        vector<Var*> xj;  // Var. die nur an bj  und nicht an bj2 liegen.
        vector<Var*> xj2; // Var. die nur an bj2 und nicht an bj  liegen.

        // xj
        set_difference (bj->x.begin(),  bj->x.end(),
                        bj2->x.begin(), bj2->x.end(),
                        inserter (xj, xj.begin()));

        // xc_size
        unsigned  xc_size = bj->x.size() - xj.size();  // xc: gem. Variablen

        if (xc_size < 2)
          continue;

        // xj2
        set_difference (bj2->x.begin(), bj2->x.end(),
                        bj->x.begin(),  bj->x.end(),
                        inserter (xj2, xj2.begin()));

        if (xc_size == bj->x.size() && xc_size == bj2->x.size())
          continue;

        // xc_min, xc_max
        int  xc_min = max (0, max (bj->val  - (int) xj.size(),
                                   bj2->val - (int) xj2.size()));

        int  xc_max = min ((int) xc_size, min (bj->val, bj2->val));

        // Widerspruch?
        if (xc_min > xc_max)
        {
          // genuegt 1 Zug?
          if (hints != 0)
          {
            moves.clear();  // Es koennte bereits ein Zug gefunden worden sein
            hints->clear();
            hints->push_back (bj->cell);
            hints->push_back (bj2->cell);
            return FOUND_CONTRA;  // Ein Widerspruch genuegt
          }
          else // alle Zuege
          {
            if (!contra2_exist)
            {
              moves.clear();        // gueltige Zuege loeschen
              contra2_exist = true;
            }

            moves.push_back (Move (bj->cell,  CLEAN));
            moves.push_back (Move (bj2->cell, CLEAN));
          }
        }

        if (contra2_exist)  // Falls ein Widerspruch gefunden wurde,
          continue;         // dann ausschliesslich Widersprueche sammeln

        // Bereits einen Zug gefunden
        if (hints != 0 && hints->size() > 0)
          continue;

        // Zug gefunden? (1)
        if (xj.size() > 0
        && (bj->val == xc_min || bj->val - xc_max == (int) xj.size()))
        {
          CellState  new_state = (bj->val == xc_min) ? OPEN : FLAGGED;

          // genuegt 1 Zug?
          if (hints != 0)
          {
            moves.push_back (Move (xj[0]->cells[0], new_state));

            hints->push_back (bj->cell);
            hints->push_back (bj2->cell);
            // kein return, da noch ein Widerspruch gefunden werden koennte
          }
          else // alle Zuege
          {
            for_all_vars (xji, xj)
            {
              for (unsigned l = 0; l < (*xji)->cells.size(); l++)
              {
                MovesCIter  m;

                for (m = moves.begin(); m != moves.end(); m++)
                  if ((*m).cell_nr == (*xji)->cells[l])
                    break;

                if (m == moves.end())
                  moves.push_back (Move ((*xji)->cells[l], new_state));

                // ??? Was bei:  - - ? - - - -   (!: Falsch gesetzte Flaggen)
                //               - ! 2 ? 2 - -
                //               - - 2 ? 2 - -
                //               - - 2 ? 2 - -
                //               - - 2 ? 2 ! -
                //               - - - - ? - -
                // (Endlosschleife?)
              }
            }
          }
        }

        // Zug gefunden? (2)
        if (xj2.size() > 0
        && (bj2->val == xc_min || bj2->val - xc_max == (int) xj2.size()))
        {
          CellState  new_state = (bj2->val == xc_min) ? OPEN : FLAGGED;

          // genuegt 1 Zug?
          if (hints != 0)
          {
            moves.push_back (Move (xj2[0]->cells[0], new_state));

            hints->push_back (bj->cell);
            hints->push_back (bj2->cell);
            // kein return, da noch ein Widerspruch gefunden werden koennte
          }
          else // alle Zuege
          {
            for_all_vars (xj2i, xj2)
            {
              for (unsigned l = 0; l < (*xj2i)->cells.size(); l++)
              {
                MovesCIter  m;

                for (m = moves.begin(); m != moves.end(); m++)
                  if ((*m).cell_nr == (*xj2i)->cells[l])
                    break;

                // Was tun bei Widerspruch???

                if (m == moves.end())
                  moves.push_back (Move ((*xj2i)->cells[l], new_state));
              }
            }
          }
        }
      }
    }
  }

  // no_move_stage, solve_stage
  if (!contra2_exist && moves.size() == 0)
    no_move_stage = 2;

  if (solve_stage < 2 && hints == 0)
    solve_stage = 2;

  // return
  if      (contra2_exist)      return FOUND_CONTRA;
  else if (moves.size() == 0)  return FOUND_NO_MOVE;
  else                         return FOUND_MOVE;
}

//******************************************************************************
Found Eqs::findMoves3 (Moves& moves, int max_time,
                       const Move* const special_move) // Stage 3
//------------------------------------------------------------------------------
// sucht alle Zuege mit Stage 3
// ----------------------------
// Falls Widerspruch gefunden? -> moves enthaelt hints
{
  if (max_time == 0)
    throw OutOfTime();

  PERF_ANA ("Eqs::findMoves3");

  // clock0, diff_clock
  clock0 = clock();
  if (max_time < 0) diff_clock = -1;
  else              diff_clock = (clock_t) max_time * CLK_TCK / 1000;

  // options
  moves.clear();

  if ((x.size() == 0 || no_move_stage >= 3) && special_move == 0)
    return FOUND_NO_MOVE;

  // init
  if (!x_and_b_optimized)
  {
    setBtoB();

    simplifyB();
    setBtoB();

    uniteX();
    setXtoX();

    x_and_b_optimized = true;

    if (Glob::trace_level >= 1)
      printStat();
  }

  ASSERT (findMoves1 (moves) != FOUND_CONTRA);
  
  if (moves.size() > 0)
  {
    if (special_move == 0)
      return FOUND_MOVE; // bel. Zug genuegt

    for_all (MovesCIter, m, moves)
      if ((*m).cell_nr == special_move->cell_nr)
        return FOUND_MOVE; // spez. Zug gefunden
  }

  // groups
  buildGroups();

  // Trace
  if (Glob::trace_level >= 2)
    logVal (0);

  // special_move?
  if (special_move != 0)
  {
    for_all_vars (pxi, x)
    {
      Var*        xi = *pxi;
      CellNrCIter  k  = find (xi->cells.begin(), xi->cells.end(),
                             special_move->cell_nr);

      xi->one_sol_val_min = xi->val_min;
      xi->one_sol_val_max = xi->val_max;

      if (k != xi->cells.end())
      {
        if (special_move->new_state == FLAGGED)
          xi->one_sol_val_min = 1;
        else if (special_move->new_state == OPEN)
          xi->one_sol_val_max = min ((int) xi->cells.size() - 1,
                                     (*min_element (xi->b.begin(), xi->b.end(),
                                                    mem_cmp (&Eq::val)))->val);
      }
    }
  }

  // 1. Durchgang
  if (special_move == 0 || solve_stage <= 3)
  {
    init (1);

    if (special_move != 0)
      groups[0]->note_one_sol = true;

    // ...Widerspruch?
    ASSERT (groups[0]->solve());
    ASSERT (groups[0]->comb_sums[""].size() != 0);

    solve_stage = 3;
  }

  int  num_var_cells = 0;

  for_all_vars (xi, x)
    num_var_cells += (*xi)->cells.size();

  int  sol_sum_smallest = *min_element (groups[0]->comb_sums[""].begin(),
                                        groups[0]->comb_sums[""].end());
  int  sol_sum_largest  = *max_element (groups[0]->comb_sums[""].begin(),
                                        groups[0]->comb_sums[""].end());
  int  sol_sum_min      = max (0, num_mines_remain - num_vars_inside);
  int  sol_sum_max      = min (num_var_cells, num_mines_remain);

  // gibt es Loesungen ausserhalb des erlaubten Intervalls?
  if (sol_sum_smallest < sol_sum_min || sol_sum_max < sol_sum_largest
  ||  solve_stage == 4)
  {
    // 2. Durchgang - nur Loesungen innerhalb des erlaubten Intervalls notieren
    init (2);

    if (special_move != 0)
      groups[0]->note_one_sol = true;

    SolSums  permit_sums;
    for (int i = sol_sum_min; i <= sol_sum_max; i++)
      permit_sums.push_back (i);

    // Widerspruch?
    if (!groups[0]->solve (&permit_sums))
    {
      ASSERT (special_move != 0);

      return FOUND_MOVE;
    }
    ASSERT (groups[0]->comb_sums[""].size() != 0);

    solve_stage = 4;
  }

  // Zuege notieren
  for_all_vars (pxi, x)
  {
    Var*  xi = *pxi;

    if (xi->sols[0].size() == 1
    && (xi->sols[0][0] == 0 || xi->sols[0][0] == (int) xi->cells.size()))
    {
      for (unsigned l = 0; l < xi->cells.size(); l++)
      {
        if (xi->sols[0][0] == 0)
          moves.push_back (Move (xi->cells[l], OPEN));
        else
          moves.push_back (Move (xi->cells[l], FLAGGED));
      }
    }
  }

  // ... inner_cells
  if (special_move == 0 
  &&  sol_sum_smallest == sol_sum_largest 
  &&  inner_cells.size() > 0)
  {
    // alle Minen in den Randvariablen
    if (sol_sum_smallest == num_mines_remain)
      for (unsigned l = 0; l < inner_cells.size(); ++l)
        moves.push_back (Move (inner_cells[l], OPEN));
        
    // Alle Innere Var. haben eine Mine
    else if (sol_sum_largest == num_mines_remain - (int)inner_cells.size())
      for (unsigned l = 0; l < inner_cells.size(); ++l)
        moves.push_back (Move (inner_cells[l], FLAGGED));
  }
  
  // no_move_stage, solve_stage
  if (moves.size() == 0)
    no_move_stage = 3;

  // return
  if (special_move == 0)
    return (moves.size() == 0) ? FOUND_NO_MOVE : FOUND_MOVE;
  else
    return (groups[0]->note_one_sol) ? FOUND_MOVE : FOUND_NO_MOVE;
}

//******************************************************************************
bool Eqs::findOneSolution (const vector<Cell>& cells, CellNrs& to_toggle,
                           Moves& moves, int max_time)
//------------------------------------------------------------------------------
// Voraussetzung: - Real-Modus
// in:  cells
// in:  to_toggle  Zelle, in der die Mine getogglt werden soll
// out: to_toggle  alle zu aendernden Zellen
// out: moves      gefundene Zuege
{
  PERF_ANA ("Eqs::findOneSolution");
  
  if (max_time == 0)
    throw OutOfTime();

  ASSERT (to_toggle.size() == 1);            // || no_move_stage >= 3)

  if (cells[to_toggle[0]].isSolved())  // || no_move_stage >= 3)
    return false;                      // toggeln nicht moeglich

  bool  add_mine = !cells[to_toggle[0]].isMined();
  int   num_vars_inside_old  = num_vars_inside;
  int   num_mines_remain_old = num_mines_remain;

  // Zelle to_toggle im 'inneren'
  if (cells[to_toggle[0]].num_open_adj == 0)
  {
    // toggle2?
    int     r = Glob::rand_int();
    int     v = 0;
    CellNr  toggle2 = -1;

    for (CellNr k = 0; k < (CellNr) cells.size(); k++)
      if (!cells[k].isOpen() && cells[k].num_open_adj == 0
      &&  !cells[k].isSolved()
      &&  cells[k].isMined() != cells[to_toggle[0]].isMined()
      &&  r % ++v == 0)
        toggle2 = k;

    if (toggle2 != -1)
    {
      to_toggle.push_back (toggle2);
      return true;
    }
  }

  if (x.size() == 0)
    return false;

  // loesen
  if (cells[to_toggle[0]].num_open_adj == 0)
  {
    num_vars_inside--;
    
    if (add_mine)
      num_mines_remain--;
  }

  Move   special_move = Move (to_toggle[0], add_mine ? FLAGGED : OPEN);
  Found  found = findMoves3 (moves, max_time, &special_move);

  ASSERT (found != FOUND_CONTRA);

  if (found == FOUND_MOVE)
  {
    num_vars_inside  = num_vars_inside_old;
    num_mines_remain = num_mines_remain_old;

    return false; // keine Loesung gefunden
  }

  // neu verteilen - Var-Zellen
  for_all_vars (pxi, x)
  {
    Var*  xi = *pxi;

    vector<bool>  is_mined (xi->cells.size(), false);

    fill (is_mined.begin(), is_mined.begin() + xi->one_sol_val, true);

    unsigned  l = find (xi->cells.begin(), xi->cells.end(), to_toggle[0])
                  - xi->cells.begin();

    if (l < xi->cells.size() && 1 < xi->cells.size())
    {
      ASSERT (!add_mine || xi->one_sol_val != 0);
      ASSERT (add_mine || xi->one_sol_val != (int) xi->cells.size());

      if (add_mine)
      {
        random_shuffle (is_mined.begin() + 1, is_mined.end(), Glob::rand_int);
        is_mined[0]  = is_mined[l];
        is_mined[l] = true;      // l verminen
      }
      else
      {
        random_shuffle (is_mined.begin(), is_mined.end() - 1, Glob::rand_int);
        is_mined.back() = is_mined[l];
        is_mined[l]    = false;  // l entminen
      }
    }
    else
    {
      random_shuffle (is_mined.begin(), is_mined.end(), Glob::rand_int);
    }

    for (l = 0; l < xi->cells.size(); l++)
      if (cells[xi->cells[l]].isMined() != is_mined[l]
      &&  xi->cells[l] != to_toggle[0])
        to_toggle.push_back (xi->cells[l]);
  }

  // neu verteilen - inneren Zellen
  for_all_vars (xi, x)
    num_mines_remain -= (*xi)->one_sol_val;

  vector<bool>  is_mined (num_vars_inside, false);

  fill_n         (is_mined.begin(), num_mines_remain, true);
  random_shuffle (is_mined.begin(), is_mined.end(), Glob::rand_int);

  int l = 0;
  for (CellNr k = 0; k < (CellNr) cells.size(); k++)
    if (!cells[k].isOpen() && cells[k].num_open_adj == 0
    &&  !cells[k].isSolved() && k != to_toggle[0])
    {
      if (cells[k].isMined() != is_mined[l])
        to_toggle.push_back (k);
      l++;
    }

  // test 'to_toggle'
  int  mines_diff = 0;
  { // visual studio
  for_all (CellNrCIter, k, to_toggle)
    mines_diff += (cells[*k].isMined()) ? 1 : -1;
  } // visual studio

  ASSERT (mines_diff == 0);

  // aufraeumen
  num_vars_inside  = num_vars_inside_old;
  num_mines_remain = num_mines_remain_old;

  return true;
}

