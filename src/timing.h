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
#include <exception>
#include <gc.h>
#include <cairo_gc.h>

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

  extern int vFontPointsize, vLineWidth, vThinLineWidth, vCellHt, vCellW;
  extern std::string vFont;
  extern bool draw_grid;

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

  bool has_ext (const std::string &filename, const std::string& ext);
  void render (gc &gc, const data &d, double scale);
  void render (gc &gc, const data &d, int w, int h, bool fixAspect);
  void decode_color (const std::string& name, double *r, double *g, double *b);

};

std::ostream &operator<< (std::ostream &f, const timing::data &d);
std::ostream &operator<< (std::ostream &f, const timing::sigdata &d);
std::ostream &operator<< (std::ostream &f, const timing::depdata &d);

#endif
