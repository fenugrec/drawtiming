// -*- mode: c++; -*-
// Copyright (c)2004 by Edward Counce, All rights reserved.
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
#include <iostream>
#include <Magick++.h>

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
  typedef std::list<signame> siglist;
  typedef std::list<sigvalue> sequence;

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
    signame name;		// name of signal
    sequence data;
    int numdelays, maxdelays;
    sigdata (const signame &name);
    sigdata (const sigdata &);
    sigdata &operator= (const sigdata &);
  };

  struct data {
    unsigned maxlen;
    std::list<sigdata> signals;
    std::list<depdata> dependencies;
    std::list<delaydata> delays;
    sigdata &find_signal (const signame &name);
    data (void);
    data (const data &);
    data &operator= (const data &);
    void add_dependency (const signame &name, const signame &dep);
    void add_dependencies (const signame &name, const siglist &deps);
    void add_delay (const signame &name, const signame &dep, const std::string &text);
    void set_value (const signame &name, unsigned n, const sigvalue &value);
    void pad (unsigned n);
  };

  struct diagram : public std::list<Magick::Drawable> {
    double scale;
    int width, height;
    diagram (void);
    diagram (const diagram &);
    diagram &operator= (const diagram &);
    void draw_transition (int x, int y, const sigvalue &last, const sigvalue &value);
    void draw_dependency (int x0, int y0, int x1, int y1);
    void draw_delay (int x0, int y0, int x1, int y1, int y2, const std::string &text);
    void render (const data &d);
  };

};

std::ostream &operator<< (std::ostream &f, const timing::data &d);
std::ostream &operator<< (std::ostream &f, const timing::sigdata &d);
std::ostream &operator<< (std::ostream &f, const timing::depdata &d);

#endif
