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
#include <cairo.h>
#include <cairo-svg.h>
#include <cairo-pdf.h>
#include <cairo-ps.h>
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

  protected:
    std::string fname;
    int width, height;
    double scale_x, scale_y;

  public:

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
    virtual void fill_color (const std::string &name) = 0;
    virtual void fill_opacity (int op) = 0;
    virtual void font (const std::string &name) = 0;
    virtual void line (int x1, int y1, int x2, int y2) = 0;
    virtual void point_size (int size) = 0;
    virtual void polygon (const std::list<Coordinate> &points) = 0;
    virtual void pop (void) = 0;
    virtual void push (void) = 0;
    virtual void scaling (double hscale, double vscale) = 0;
    virtual void stroke_color (const std::string &name) = 0;
    virtual void set_stroke_width (int w) = 0;
    virtual void text (int x, int y, const std::string &text) = 0;
  };

#ifndef LITE

  class cairo_gc : public gc {
  protected:
    cairo_surface_t *surface;
    cairo_t *cr;

  private:
    double fill_color_r, fill_color_g, fill_color_b, fill_color_a;
    double stroke_color_r, stroke_color_g, stroke_color_b, stroke_color_a;
    int stroke_width;
    int font_size;

    void uniform_stroke();
    void uniform_text(const std::string &text);

  public:
    cairo_gc (const std::string& filename);
    virtual ~cairo_gc (void);

    virtual cairo_surface_t* create_surface() = 0;
    virtual void finish_surface() = 0;

    void set_surface_size(double w, double h);
    double get_label_width(const data &d);
    void bezier (const std::list<Coordinate> &points);
    void fill_color (const std::string &name);
    void fill_opacity (int op);
    void font (const std::string &name);
    void line (int x1, int y1, int x2, int y2);
    void point_size (int size);
    void polygon (const std::list<Coordinate> &points);
    void pop (void);
    void push (void);
    void scaling (double hscale, double vscale);
    void stroke_color (const std::string &name);
    void set_stroke_width (int w);
    void text (int x, int y, const std::string &text);
  };

#ifdef CAIRO_HAS_PNG_FUNCTIONS
  class cairo_png_gc : public cairo_gc {
  public:
    cairo_png_gc(const std::string& filename);
    cairo_surface_t* create_surface();
    void finish_surface();
  };
#endif

#ifdef CAIRO_HAS_PDF_SURFACE
  class cairo_pdf_gc : public cairo_gc {
  public:
    cairo_pdf_gc(const std::string& filename);
    cairo_surface_t* create_surface();
    void finish_surface();
  };
#endif

#ifdef CAIRO_HAS_SVG_SURFACE
  class cairo_svg_gc : public cairo_gc {
  public:
    cairo_svg_gc(const std::string& filename);
    cairo_surface_t* create_surface();
    void finish_surface();
  };
#endif

#ifdef CAIRO_HAS_PS_SURFACE
  class cairo_ps_gc : public cairo_gc {
  public:
    cairo_ps_gc(const std::string& filename);
    cairo_surface_t* create_surface();
    void finish_surface();
  };
  class cairo_eps_gc : public cairo_gc {
  public:
    cairo_eps_gc(const std::string& filename);
    cairo_surface_t* create_surface();
    void finish_surface();
  };
#endif

#endif /* ! LITE */
  class postscript_gc : public gc {
    std::ostringstream ps_text;

  public:
    postscript_gc (const std::string& filename);
    virtual ~postscript_gc (void);

    void finish_surface(void);
    void set_surface_size(double w, double h);
    double get_label_width(const data &d);
    void bezier (const std::list<Coordinate> &points);
    void fill_color (const std::string &name);
    void fill_opacity (int op);
    void font (const std::string &name);
    void line (int x1, int y1, int x2, int y2);
    void point_size (int size);
    void polygon (const std::list<Coordinate> &points);
    void pop (void);
    void push (void);
    void scaling (double hscale, double vscale);
    void stroke_color (const std::string &name);
    void set_stroke_width (int w);
    void text (int x, int y, const std::string &text);

    void print (std::ostream& out) const;
  };

  bool has_ext (const std::string &filename, const std::string& ext);
  void render (gc &gc, const data &d, double scale);
  void render (gc &gc, const data &d, int w, int h, bool fixAspect);

};

std::ostream &operator<< (std::ostream &f, const timing::data &d);
std::ostream &operator<< (std::ostream &f, const timing::sigdata &d);
std::ostream &operator<< (std::ostream &f, const timing::depdata &d);

#endif
