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

#ifndef __TIMING_H
#define __TIMING_H
#include <string>
#include <list>
#include <map>
#include <iostream>
#include <sstream>
#include <exception>
#ifndef LITE
#include <Magick++.h>
#else /* LITE */

namespace Magick
{
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
};

#endif /* LITE */

namespace timing {

  enum valuetype {UNDEF, ZERO, ONE, X, Z, PULSE, TICK, STATE};

  struct sigvalue {
    valuetype type;
    std::string text;
    sigvalue (void);
    sigvalue (const sigvalue &);
    sigvalue (const std::string &s, valuetype n = UNDEF);
    sigvalue &operator= (const sigvalue &);
  };

  typedef std::string signame;
  typedef std::list<signame> signal_sequence;
  typedef std::list<sigvalue> value_sequence;

  extern int vFontPointsize, vLineWidth, vCellHt, vCellW;
  extern std::string vFont;

  class exception : public std::exception {
  };

  class not_found : public exception {
    signame text;
  public:
    not_found (const signame &n) throw ();
    ~not_found () throw ();
    const char *what (void) const throw ();
  };

  struct depdata {
    signame trigger;		// name of trigger signal
    signame effect;		// name of effect signal
    unsigned n_trigger;		// sequence number of trigger signal
    unsigned n_effect;		// sequence number for effect signal
  };

  struct delaydata {
    std::string text;
    signame trigger;		// name of trigger signal
    signame effect;		// name of effect signal
    unsigned n_trigger;		// sequence number of trigger signal
    unsigned n_effect;		// sequence number for effect signal
    int offset;			// prevent arrows from overlapping
  };

  struct sigdata {
    value_sequence data;
    int numdelays, maxdelays;
    sigdata (void);
    sigdata (const sigdata &);
    sigdata &operator= (const sigdata &);
  };

  typedef std::map<signame, sigdata> signal_database;

  struct data {
    unsigned maxlen;
    signal_database signals;
    signal_sequence sequence;
    std::list<depdata> dependencies;
    std::list<delaydata> delays;
    data (void);
    data (const data &);
    data &operator= (const data &);
    sigdata &find_signal (const signame &name);
    const sigdata &find_signal (const signame &name) const;
    void add_dependency (const signame &name, const signame &dep);
    void add_dependencies (const signame &name, const signal_sequence &deps);
    void add_delay (const signame &name, const signame &dep, const std::string &text);
    void set_value (const signame &name, unsigned n, const sigvalue &value);
    void pad (unsigned n);
  };

  class gc {
  public:
    int width, height;

    gc (void) : width(0), height(0) { }
    virtual ~gc() { }

    virtual void bezier (const std::list<Magick::Coordinate> &points) = 0;
    virtual void fill_color (const std::string &name) = 0;
    virtual void fill_opacity (int op) = 0;
    virtual void font (const std::string &name) = 0;
    virtual void line (int x1, int y1, int x2, int y2) = 0;
    virtual void point_size (int size) = 0;
    virtual void polygon (const std::list<Magick::Coordinate> &points) = 0;
    virtual void pop (void) = 0;
    virtual void push (void) = 0;
    virtual void scaling (double hscale, double vscale) = 0;
    virtual void stroke_color (const std::string &name) = 0;
    virtual void stroke_width (int w) = 0;
    virtual void text (int x, int y, const std::string &text) = 0;
  };

#ifndef LITE
  class magick_gc : public gc {
    std::list<Magick::Drawable> drawables;

  public:
    ~magick_gc (void);

    void bezier (const std::list<Magick::Coordinate> &points);
    void fill_color (const std::string &name);
    void fill_opacity (int op);
    void font (const std::string &name);
    void line (int x1, int y1, int x2, int y2);
    void point_size (int size);
    void polygon (const std::list<Magick::Coordinate> &points);
    void pop (void);
    void push (void);
    void scaling (double hscale, double vscale);
    void stroke_color (const std::string &name);
    void stroke_width (int w);
    void text (int x, int y, const std::string &text);

    void draw (Magick::Image& img) const;
  };

#endif /* ! LITE */
  class postscript_gc : public gc {
    std::ostringstream ps_text;

  public:
    postscript_gc (void);
    ~postscript_gc (void);

    void bezier (const std::list<Magick::Coordinate> &points);
    void fill_color (const std::string &name);
    void fill_opacity (int op);
    void font (const std::string &name);
    void line (int x1, int y1, int x2, int y2);
    void point_size (int size);
    void polygon (const std::list<Magick::Coordinate> &points);
    void pop (void);
    void push (void);
    void scaling (double hscale, double vscale);
    void stroke_color (const std::string &name);
    void stroke_width (int w);
    void text (int x, int y, const std::string &text);

    void print (std::ostream& out) const;
    void print (const std::string& filename) const;

    static bool has_ps_ext (const std::string& filename);
  };

  void render (gc &gc, const data &d, double scale);
  void render (gc &gc, const data &d, int w, int h, bool fixAspect);
};

std::ostream &operator<< (std::ostream &f, const timing::data &d);
std::ostream &operator<< (std::ostream &f, const timing::sigdata &d);
std::ostream &operator<< (std::ostream &f, const timing::depdata &d);

#endif
