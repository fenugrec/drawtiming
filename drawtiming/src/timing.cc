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

#include "timing.h"
#include <map>
using namespace std;
using namespace timing;
using namespace Magick;

// ------------------------------------------------------------

sigdata::sigdata (const signame &n) : name (n) {
}

// ------------------------------------------------------------

sigdata &data::find_signal (const signame &name) {
  // search for a signal by name
  list<sigdata>::iterator i;
  for (i = signals.begin (); i != signals.end (); ++ i) {
    if (i->name == name)
      break;
  }

  // if not found, create a new signal
  if (i == signals.end ())
    i = signals.insert (i, sigdata (name));

  return *i;
}

// ------------------------------------------------------------

void data::add_dependencies (const signame &name, const siglist &deps) {
  // find the signal
  sigdata &sig = find_signal (name);

  for (siglist::const_iterator j = deps.begin (); j != deps.end (); ++ j) {
    sigdata &trigger = find_signal (*j);
    depdata d;
    d.trigger = trigger.name;
    d.effect = sig.name;
    if ((d.n_trigger = trigger.data.size ()) > 0)
      -- d.n_trigger;
    if ((d.n_effect = sig.data.size ()) > 0)
      -- d.n_effect;
    dependencies.push_back (d);
  }
}

// ------------------------------------------------------------

void data::set_value (const signame &name, unsigned n, const sigvalue &value) {
  // find the signal
  sigdata &sig = find_signal (name);

  // pad the sequence so there are n data values
  sigvalue lastval = (sig.data.size () == 0 ? "X" : sig.data.back ());
  if (lastval == "pulse")
    lastval = "0";

  while (sig.data.size () < n) 
    sig.data.push_back (lastval);

  // append the value to the sequence data
  sig.data.push_back (value);

  if (n + 1 > maxlen)
    maxlen = n + 1;
}

// ------------------------------------------------------------

void data::pad (unsigned n) {
  // pad all sequences to length n
  if (n > maxlen)
    maxlen = n;
  for (list<sigdata>::iterator i = signals.begin (); i != signals.end (); ++ i) {
    sigvalue lastval = (i->data.size () == 0 ? "X" : i->data.back ());
    if (lastval == "pulse")
      lastval = "0";
    while (i->data.size () < maxlen)
      i->data.push_back (lastval);
  }
}

// ------------------------------------------------------------

ostream &operator<< (ostream &f, const data &data) {
  f << "signals: " << endl;
  for (list<sigdata>::const_iterator i = data.signals.begin ();
       i != data.signals.end (); ++ i) 
    f << "  " << *i << endl;

  f << endl << "dependencies: " << endl;
  for (list<depdata>::const_iterator i = data.dependencies.begin ();
       i != data.dependencies.end (); ++ i) 
    f << "  " << *i << endl;

  return f;
}

// ------------------------------------------------------------

ostream &operator<< (ostream &f, const sigdata &sig) {
  f << sig.name << ": ";
  int n = 0;
  for (sequence::const_iterator i = sig.data.begin ();
       i != sig.data.end (); ++ i, ++ n) {
    if (n)
      f << ", ";
    f << *i;
  }
  return f;
}

// ------------------------------------------------------------

ostream &operator<< (ostream &f, const depdata &dep) {
  f << dep.trigger << "(" << dep.n_trigger << ") => "
    << dep.effect << "(" << dep.n_effect << ")";
  return f;
}

// ------------------------------------------------------------

diagram::diagram (void) {
  scale = 1.0;
  width = height = 0;
}

// ------------------------------------------------------------

void diagram::render (const data &d) {
  int labelWidth = 0;
  Image img;
  TypeMetric m;
  for (list<sigdata>::const_iterator i = d.signals.begin ();
       i != d.signals.end (); ++ i) {
    img.fontTypeMetrics (i->name, &m);
    if (m.textWidth () > labelWidth)
      labelWidth = (int) m.textWidth ();
  }

  push_back (DrawablePushGraphicContext ());
  push_back (DrawableScaling (scale, scale));

  map<signame,int> ypos;
  int y = 0;
  for (list<sigdata>::const_iterator i = d.signals.begin ();
       i != d.signals.end (); ++ i) {
    push_back (DrawableText (8, y + 24, i->name));
    ypos[i->name] = y;
    int x = labelWidth + 16;
    sigvalue last;
    for (sequence::const_iterator j = i->data.begin (); j != i->data.end (); ++ j) {
      add_transition (x, y, last, *j);
      last = *j;
      x += 64;
    }
    y += 32;
  }

  for (list<depdata>::const_iterator i = d.dependencies.begin (); 
       i != d.dependencies.end (); ++ i)
    add_arrow (labelWidth + 16 + 64 * i->n_trigger, 16 + ypos[i->trigger], 
	       labelWidth + 16 + 64 * i->n_effect, 16 + ypos[i->effect]);


  push_back (DrawablePopGraphicContext ());

  width = (int)(scale * (16 + labelWidth + 64 * d.maxlen));
  height = (int)(scale * (32 * d.signals.size ()));
}

// ------------------------------------------------------------

void diagram::add_transition (int x, int y, const sigvalue &last,
			      const sigvalue &value) {
  if (value == "0" || value == "false") {
    if (last == "" || last == "0" || last == "false" || last == "X" || last == "tick" || last == "pulse") {
      push_back (DrawableLine (x, y + 28, x + 64, y + 28));
    }
    else if (last == "1" || last == "true") {
      push_back (DrawableLine (x, y + 4, x + 16, y + 28));
      push_back (DrawableLine (x + 16, y + 28, x + 64, y + 28));
    }
    else if (last == "Z") {
      push_back (DrawableLine (x, y + 16, x + 16, y + 28));
      push_back (DrawableLine (x + 16, y + 28, x + 64, y + 28));
    }
    else {
      push_back (DrawableLine (x, y + 4, x + 16, y + 28));
      push_back (DrawableLine (x, y + 28, x + 64, y + 28));
    }
  }

  else if (value == "1" || value == "true") {
    if (last == "" || last == "1" || last == "true" || last == "X") {
      push_back (DrawableLine (x, y + 4, x + 64, y + 4));
    }
    else if (last == "0" || last == "false" || last == "tick" || last == "pulse") {
      push_back (DrawableLine (x, y + 28, x + 16, y + 4));
      push_back (DrawableLine (x + 16, y + 4, x + 64, y + 4));
    }
    else if (last == "Z") {
      push_back (DrawableLine (x, y + 16, x + 16, y + 4));
      push_back (DrawableLine (x + 16, y + 4, x + 64, y + 4));
    }
    else {
      push_back (DrawableLine (x, y + 28, x + 16, y + 4));
      push_back (DrawableLine (x, y + 4, x + 64, y + 4));
    }
  }

  else if (value == "tick" || value == "pulse") {
    if (last == "1" || last == "true" || last == "X") {
      push_back (DrawableLine (x, y + 4, x + 32, y + 4));
      push_back (DrawableLine (x + 32, y + 4, x + 48, y + 28));
      push_back (DrawableLine (x + 48, y + 28, x + 64, y + 28));
    }
    else if (last == "" || last == "0" || last == "false" || last == "tick" || last == "pulse") {
      push_back (DrawableLine (x, y + 28, x + 16, y + 4));
      push_back (DrawableLine (x + 16, y + 4, x + 32, y + 4));
      push_back (DrawableLine (x + 32, y + 4, x + 48, y + 28));
      push_back (DrawableLine (x + 48, y + 28, x + 64, y + 28));
    }
    else if (last == "Z") {
      push_back (DrawableLine (x, y + 16, x + 16, y + 4));
      push_back (DrawableLine (x + 16, y + 4, x + 32, y + 4));
      push_back (DrawableLine (x + 32, y + 4, x + 48, y + 28));
      push_back (DrawableLine (x + 48, y + 28, x + 64, y + 28));
    }
    else {
      push_back (DrawableLine (x, y + 28, x + 16, y + 4));
      push_back (DrawableLine (x, y + 4, x + 32, y + 4));
      push_back (DrawableLine (x + 32, y + 4, x + 48, y + 28));
      push_back (DrawableLine (x + 48, y + 28, x + 64, y + 28));
    }
  }
  
  else if (value == "X") {
    for (int i = 0; i < 4; ++ i) {
      push_back (DrawableLine (x+i*16, y + 28, x+(i+1)*16, y + 4));
      push_back (DrawableLine (x+i*16, y + 4, x+(i+1)*16, y + 28));
    }
  }
  
  else if (value == "Z") {
    if (last == "" || last == "X" || last == "Z") {
      push_back (DrawableLine (x, y + 16, x + 64, y + 16));
    }
    else if (last == "0" || last == "false" || last == "tick" || last == "pulse") {
      push_back (DrawableLine (x, y + 28, x + 16, y + 16));
      push_back (DrawableLine (x + 16, y + 16, x + 64, y + 16));
    }
    else if (last == "1" || last == "true") {
      push_back (DrawableLine (x, y + 4, x + 16, y + 16));
      push_back (DrawableLine (x + 16, y + 16, x + 64, y + 16));
    }
    else {
      push_back (DrawableLine (x, y + 4, x + 8, y + 16));
      push_back (DrawableLine (x, y + 28, x + 8, y + 16));
      push_back (DrawableLine (x + 8, y + 16, x + 64, y + 16));
    }
  }
  
  else {
    if (last == "0" || last == "false" || last == "tick" || last == "pulse") {
      push_back (DrawableLine (x, y + 28, x + 16, y + 4));
      push_back (DrawableLine (x + 16, y + 4, x + 64, y + 4));
      push_back (DrawableLine (x, y + 28, x + 64, y + 28));
      push_back (DrawableText (x + 16, y + 24, value));
    }
    else if (last == "1" || last == "true") {
      push_back (DrawableLine (x, y + 4, x + 16, y + 28));
      push_back (DrawableLine (x + 16, y + 28, x + 64, y + 28));
      push_back (DrawableLine (x, y + 4, x + 64, y + 4));
      push_back (DrawableText (x + 16, y + 24, value));
    }
    else if (last == "Z") {
      push_back (DrawableLine (x, y + 16, x + 8, y + 28));
      push_back (DrawableLine (x, y + 16, x + 8, y + 4));
      push_back (DrawableLine (x + 8, y + 28, x + 64, y + 28));
      push_back (DrawableLine (x + 8, y + 4, x + 64, y + 4));
      push_back (DrawableText (x + 8, y + 24, value));
    }
    else if (value != last) {
      push_back (DrawableLine (x, y + 4, x + 16, y + 28));
      push_back (DrawableLine (x, y + 28, x + 16, y + 4));
      push_back (DrawableLine (x + 16, y + 4, x + 64, y + 4));
      push_back (DrawableLine (x + 16, y + 28, x + 64, y + 28));
      push_back (DrawableText (x + 16, y + 24, value));
    }
    else {
      push_back (DrawableLine (x, y + 4, x + 64, y + 4));
      push_back (DrawableLine (x, y + 28, x + 64, y + 28));
    }
  }
}

// ------------------------------------------------------------

void diagram::add_arrow (int x0, int y0, int x1, int y1) {
  list<Coordinate> shaft, head;

  push_back (DrawablePushGraphicContext ());
  push_back (DrawableStrokeColor ("blue"));

  if (x0 == x1) {
    x0 += 8;
    x1 += 8;
    if (y0 < y1) {
      y1 -= 8;
      push_back (DrawableLine (x0, y0, x1, y1));
      push_back (DrawableFillColor ("blue"));
      head.push_back (Coordinate (x1, y1));
      head.push_back (Coordinate (x1 - 3, y1 - 5));
      head.push_back (Coordinate (x1, y1 - 3));
      head.push_back (Coordinate (x1 + 3, y1 - 5));
      push_back (DrawablePolygon (head));
    }
    else {
      y1 += 8;
      push_back (DrawableLine (x0, y0, x1, y1));
      push_back (DrawableFillColor ("blue"));
      head.push_back (Coordinate (x1, y1));
      head.push_back (Coordinate (x1 - 3, y1 + 5));
      head.push_back (Coordinate (x1, y1 + 3));
      head.push_back (Coordinate (x1 + 3, y1 + 5));
      push_back (DrawablePolygon (head));
    }
  }
  else {
    x0 += 8;
    x1 += 4;
    push_back (DrawableFillColor ("none"));
    shaft.push_back (Coordinate (x0, y0));
    shaft.push_back (Coordinate ((x0 + x1) / 2, y1));
    shaft.push_back (Coordinate ((x0 + x1) / 2, y1));
    shaft.push_back (Coordinate (x1, y1));
    push_back (DrawableBezier (shaft));
    push_back (DrawableFillColor ("blue"));
    head.push_back (Coordinate (x1, y1));
    head.push_back (Coordinate (x1 - 5, y1 - 3));
    head.push_back (Coordinate (x1 - 3, y1));
    head.push_back (Coordinate (x1 - 5, y1 + 3));
    push_back (DrawablePolygon (head));
  }

  push_back (DrawablePopGraphicContext ());
}

