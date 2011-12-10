// -*- mode: c++; -*-
// Copyright (c)2004 by Edward Counce, All rights reserved.
// Copyright (c)2008 by Daniel Beer, All rights reserved.
// This file is part of drawtiming.
//
// Drawtiming is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Drawtiming is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with drawtiming; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef __TIMING_GC_H
#define __TIMING_GC_H

#ifdef LITE
#include <sstream>
#endif

  struct Coordinate
  {
    double _x, _y;

    Coordinate (double x, double y) {
      _x = x;
      _y = y;
    }

    double x (void) const { return _x; }
    double y (void) const { return _y; }
  };


namespace timing {

  struct data;

  class gc {

  protected:
    std::string fname;
    double scale_x, scale_y;

  public:

    int width, height;

    gc (const std::string& filename) :
         fname(filename),
         width(0), height(0),
         scale_x(1), scale_y(1)
    { }
    virtual ~gc() { }

    virtual void finish_surface() = 0;
    virtual void set_surface_size(double w, double h) = 0;
    virtual double get_label_width(const data &d) = 0;
    virtual void bezier (const std::list<Coordinate> &points) = 0;
    virtual void font (const std::string &name) = 0;
    virtual void line (int x1, int y1, int x2, int y2) = 0;
    virtual void point_size (int size) = 0;
    virtual void polygon (const std::list<Coordinate> &points) = 0;
    virtual void pop (void) = 0;
    virtual void push (void) = 0;
    virtual void scaling (double hscale, double vscale) = 0;
    virtual void color (const std::string &name) = 0;
    virtual void set_stroke_width (int w) = 0;
    virtual void text (int x, int y, const std::string &text) = 0;
  };

#ifdef LITE

  class postscript_gc : public gc {
    std::ostringstream ps_text;

  public:
    postscript_gc (const std::string& filename);
    virtual ~postscript_gc (void);

    void finish_surface(void);
    void set_surface_size(double w, double h);
    double get_label_width(const data &d);
    void bezier (const std::list<Coordinate> &points);
    void font (const std::string &name);
    void line (int x1, int y1, int x2, int y2);
    void point_size (int size);
    void polygon (const std::list<Coordinate> &points);
    void pop (void);
    void push (void);
    void scaling (double hscale, double vscale);
    void color (const std::string &name);
    void set_stroke_width (int w);
    void text (int x, int y, const std::string &text);

    void print (std::ostream& out) const;
  };
#endif  //LITE

};
#endif
