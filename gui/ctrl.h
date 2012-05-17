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

#ifndef CTRL_H
#define CTRL_H

#include <vector>
#include "../core/utils.h"
#include "../core/api.h"

class Ctrl;

using namespace MinesPerfect;

//******************************************************************************
class Ctrl
//------------------------------------------------------------------------------
{
  public:
    Ctrl(Ctrl* new_parent);
    virtual ~Ctrl();
    
    void  delAllChildren();
    void  show(); // sollte nicht ueberladen werden
    
    const Ctrl* getParent()  const { return m_parent; }
    const Ctrl* getTopCtrl() const;
    Point       getRelPos()  const;
    Point       getSize()    const { return m_size; }
    Point       getAbsPos()  const { return m_abs_pos; }
    Rect        getAbsRect() const;
    
    bool        contains(const Point& p) const; // p is rel.
    
    void  setDirty();
    bool  setRelPos (Point new_rel_pos, bool recursive = false);
    bool  setSize   (Point new_size) { return setElem(m_size, new_size); }
    
    virtual void  onMouseEvent (const MouseEvent& ev);
    
  protected: 
    bool setElem (int&           p, int           sec);
    bool setElem (bool&          p, bool          sec);
    bool setElem (const Bitmap*& p, const Bitmap* sec);
    bool setElem (Point&         p, Point         sec);
    
    virtual void draw() {}
    
  private:
    Ctrl*          m_parent;
    Point          m_abs_pos; // es effizienter die abs. Pos. zu speichern,
                              // da die rel. leichter aus der abs. zu bestimmen 
                              // ist als umgekehrt.
                              // Und zum zeichnen wird i.A. die abs. benoetigt.
    Point          m_size;  
    bool           m_dirty;
    vector<Ctrl*>  m_children;
    
    template<class T> bool setElem2 (T& p, T sec);
};

#endif


