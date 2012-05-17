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

#ifndef UTIL_H
#define UTIL_H

#include <string>
//#include <vector>
//#include <time.h>

#undef ASSERT
#define ASSERT(cond) \
  if (!(cond)) \
    throw MinesPerfect::AssertException(__FILE__,__LINE__)

namespace MinesPerfect {

//struct Bevel;

typedef unsigned long uint32;

//******************************************************************************
class RandInt
//------------------------------------------------------------------------------
{
public:

  RandInt();
  RandInt(uint32 s)         { setStart(s); }

  uint32 getStart()         { return startx; }
  void   setStart(uint32 s) { startx = randx = s; }

  uint32 operator()();
  uint32 operator()(uint32 m);

private:

  uint32 startx;
  uint32 randx;
};

//******************************************************************************
struct Point
//------------------------------------------------------------------------------
{
  int x;
  int y;

  Point() {};
  Point (int x0, int y0) : x(x0), y(y0) {;}

  Point& operator+= (Point sec)       { x += sec.x; y += sec.y; return *this;}
  Point& operator-= (Point sec)       { x -= sec.x; y -= sec.y; return *this;}
  bool   operator== (Point sec) const { return x == sec.x && y == sec.y; };
  bool   operator!= (Point sec) const { return x != sec.x || y != sec.y; };
};

inline Point operator+ (Point a, Point b) { return a += b; }
inline Point operator- (Point a, Point b) { return a -= b; }
inline Point operator* (int   a, Point b) { return Point (a * b.x, a * b.y); }
inline Point operator* (Point a, int   b) { return Point (a.x * b, a.y * b); }
inline int   operator* (Point a, Point b) { return a.x * b.x + a.y * b.y; }

//******************************************************************************
/*
struct Size
//------------------------------------------------------------------------------
{
  int width;
  int height;

  Size() {};
  Size (int w, int h) : width(w), height(h) {};

  bool operator== (const Size& sec) { return width == sec.width && height == sec.height; };
  bool operator!= (const Size& sec) { return width != sec.width || height != sec.height; };
};
*/
//******************************************************************************
class Rect
//------------------------------------------------------------------------------
{
public:

  Rect() : left(0), top(0), width(0), height(0) {}
  Rect (int l, int t, int w, int h) : left(l), top(t), width(w), height(h) {}
  Rect (Point p, Point s) : left(p.x), top(p.y), width(s.x), height(s.y) {}

  Point  getPos()  const { return Point (left, top);     }
  Point  getSize() const { return Point (width, height); }

  int getLeft()   const { return left; }
  int getTop()    const { return top; }
  int getWidth()  const { return width; }
  int getHeight() const { return height;}
  
  void setPos   (const Point& p) { left = p.x; top = p.y; }
  bool contains (const Point& p);
  
private:

  int left;
  int top;
  int width;
  int height;
};

//******************************************************************************
class Exception
//------------------------------------------------------------------------------
{
public:

  Exception ()                     { text = ""; }
  Exception (const std::string& t) { text = t; }

  std::string  getText() const { return text; }

protected:

  std::string  text;
};

//******************************************************************************
class AssertException : public Exception
//------------------------------------------------------------------------------
{
public:

  AssertException (const char* fname, int linenr);
};

//******************************************************************************
std::string Lower (const std::string& s);

} // namespace MinesPerfect

#endif
