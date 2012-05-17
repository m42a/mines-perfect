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

//---------------------------------------------------------------------------

#include <algorithm>
#include "ctrl.h"

//******************************************************************************
Ctrl::Ctrl(Ctrl* new_parent) 
//------------------------------------------------------------------------------
: m_parent(new_parent), m_abs_pos(0,0), m_size(0,0), m_dirty(true)
{
  if (new_parent != 0) 
    new_parent->m_children.push_back (this);
}

//******************************************************************************
void Ctrl::delAllChildren()
//------------------------------------------------------------------------------
{
  typedef vector<Ctrl*>::iterator CI;
  for (CI i = m_children.begin(); i != m_children.end(); ++i)
  {
    (*i)->m_parent = 0; // Aus Effizienzgruenden wird m_children am Ende geloescht
    delete *i;
  }
    
  m_children.clear();
}

//******************************************************************************
Ctrl::~Ctrl()
//------------------------------------------------------------------------------
{ 
  if (m_parent != 0)
  {
    vector<Ctrl*>&           c = m_parent->m_children;
    vector<Ctrl*>::iterator  i = remove(c.begin(), c.end(), this);
    c.erase(i, c.end());
  }
  
  delAllChildren();
}

//******************************************************************************
const Ctrl* Ctrl::getTopCtrl() const
//------------------------------------------------------------------------------
// Liefert den obersten Knoten
{
  const Ctrl* c = this;
  
  while (c->getParent() != 0)
    c = c->getParent();
    
  return c;
}

//******************************************************************************
Point Ctrl::getRelPos() const 
//------------------------------------------------------------------------------
{
  Point rel_pos = m_abs_pos;
  
  if (getParent() != 0)
     rel_pos -= getParent()->getAbsPos(); 

  return  rel_pos;
}

//******************************************************************************
Rect Ctrl::getAbsRect() const
//------------------------------------------------------------------------------
{ 
  Point  pos  = getAbsPos();
  Point  size = getSize();
  
  return Rect (pos.x, pos.y, size.x, size.y);
}

//******************************************************************************
// visual studio (linker Fehler, falls inline)
bool Ctrl::setElem (int&           p, int           sec) { return setElem2(p, sec); }
bool Ctrl::setElem (bool&          p, bool          sec) { return setElem2(p, sec); }
bool Ctrl::setElem (Point&         p, Point         sec) { return setElem2(p, sec); }
bool Ctrl::setElem (const Bitmap*& p, const Bitmap* sec) { return setElem2(p, sec); }
//------------------------------------------------------------------------------


//******************************************************************************
template<class T>
bool Ctrl::setElem2 (T& p, T sec)
//------------------------------------------------------------------------------
{
  if (p != sec)
  {
    p = sec;
    setDirty();

    return true;
  }
  return false;
}

//******************************************************************************
void Ctrl::setDirty()
//------------------------------------------------------------------------------
{
  m_dirty = true;
  
  vector<Ctrl*>::iterator  i;
  for (i = m_children.begin(); i != m_children.end(); ++i)
    (*i)->setDirty();
}

//******************************************************************************
bool  Ctrl::setRelPos (Point new_rel_pos, bool recursive)
//------------------------------------------------------------------------------
// new_pos: In parent-coordinates
{ 
  // new_abs_pos
  Point new_abs_pos = new_rel_pos;
  
  if (getParent() != 0)
    new_abs_pos += getParent()->getAbsPos();

  // set children
  if (recursive)
  {
    Point diff = new_abs_pos - m_abs_pos;
    
    vector<Ctrl*>::iterator  i;
    for (i = m_children.begin(); i != m_children.end(); ++i)
      (*i)->setRelPos((*i)->getRelPos() + diff, recursive);
  }

  // set
  return setElem(m_abs_pos, new_abs_pos); 
}

//******************************************************************************
bool Ctrl::contains(const Point& p) const
//------------------------------------------------------------------------------
{
  return  0 <= p.x && p.x < m_size.x 
      &&  0 <= p.y && p.y < m_size.y;
}

//******************************************************************************
void Ctrl::show()
//------------------------------------------------------------------------------
{
  if (m_dirty)
  {  
    draw();
    m_dirty = false;
  }

  vector<Ctrl*>::iterator  i;
  for (i = m_children.begin(); i != m_children.end(); ++i)
    (*i)->show();
}

//******************************************************************************
void Ctrl::onMouseEvent (const MouseEvent& ev)
//------------------------------------------------------------------------------
// hierbei handelt es sich um die Default-Ereignisbehandlungsroutine:
// es wird nichts gemacht, ausser die Funktion rekursiv fuer alle 
// Kind-Ctrls aufzurufen.
//
// Mousekoordinaten sind absolut
{
  vector<Ctrl*>::iterator  i;
  for (i = m_children.begin(); i != m_children.end(); ++i)
  {
    // hier koennte noch opitmiert werden:
    // children ueberspringen die keine Ereignisbehandlungsroutine haben,
    // wie z.B. frames.
    
    Rect rect ((*i)->getAbsPos(), (*i)->getSize()); // Rechteck der Ctrl
    if (rect.contains(ev.m_pos) || rect.contains(ev.m_prev_pos))
    {
      (*i)->onMouseEvent(ev); // rek. Aufruf
    }
  }   
}

