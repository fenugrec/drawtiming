// Copyright (c)2004 by Edward Counce, All rights reserved. 
// Copyright (c)2006-7 by Salvador E. Tropea, All rights reserved.
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

int timing::vFontPointsize = 12;
int timing::vLineWidth = 1;
int timing::vCellHt = 32;
int timing::vCellW = 64;
string timing::vFont = "Helvetica";

static int vCellHsep, vCellH, vCellHtxt, vCellHdel, vCellHtdel, vCellWtsep,
            vCellWrm;

// ------------------------------------------------------------

not_found::not_found (const signame &name) throw () {
  text = "signal \"";
  text += name;
  text += "\" not found";
}

not_found::~not_found () throw () {
}

const char *not_found::what (void) const throw () {
  return text.c_str ();
}

// ------------------------------------------------------------

sigvalue::sigvalue (void) {
  type = UNDEF;
}

sigvalue::sigvalue (const sigvalue &t) {
  *this = t;
}

sigvalue::sigvalue (const std::string &s, valuetype n) {
  text = s;
  type = n;
  if (type == UNDEF) {
    if (text == "0" || text == "false")
      type = ZERO;
    else if (text == "1" || text == "true")
      type = ONE;
    else if (text == "pulse")
      type = PULSE;
    else if (text == "tick")
      type = TICK;
    else if (text == "X")
      type = X;
    else if (text == "Z")
      type = Z;
    else
      type = STATE;
  }
}

// ------------------------------------------------------------

sigvalue &sigvalue::operator= (const sigvalue &t) {
  type = t.type;
  text = t.text;
  return *this;
}

// ------------------------------------------------------------

sigdata::sigdata (void) {
  numdelays = 0;
  maxdelays = 0;
}

sigdata::sigdata (const sigdata &d) {
  *this = d;
}

// ------------------------------------------------------------

sigdata &sigdata::operator= (const sigdata &d) {
  numdelays = d.numdelays;
  maxdelays = d.maxdelays;
  data = d.data;
  return *this;
}

// ------------------------------------------------------------

data::data (void) : maxlen (0) {
}

data::data (const data &d) {
  *this = d;
}

// ------------------------------------------------------------

data &data::operator= (const data &d) {
  maxlen = d.maxlen;
  signals = d.signals;
  sequence = d.sequence;
  dependencies = d.dependencies;
  return *this;
}

// ------------------------------------------------------------

sigdata &data::find_signal (const signame &name) {
  signal_database::iterator i = signals.find (name);
  if (i == signals.end ()) {
    i = signals.insert (signal_database::value_type (name, sigdata ())).first;
    sequence.push_back (name);
  }
  return i->second;
}

// ------------------------------------------------------------

const sigdata &data::find_signal (const signame &name) const {
  signal_database::const_iterator i = signals.find (name);
  if (i == signals.end ()) 
    throw not_found (name);
  return i->second;
}

// ------------------------------------------------------------

void data::add_dependency (const signame &name, const signame &dep) {
  // find the signal
  sigdata &sig = find_signal (name);
  sigdata &trigger = find_signal (dep);
  depdata d;
  d.trigger = dep;
  d.effect = name;
  if ((d.n_trigger = trigger.data.size ()) > 0)
    -- d.n_trigger;
  if ((d.n_effect = sig.data.size ()) > 0)
    -- d.n_effect;
  dependencies.push_back (d);
}

// ------------------------------------------------------------

void data::add_dependencies (const signame &name, const signal_sequence &deps) {
  for (signal_sequence::const_iterator j = deps.begin (); j != deps.end (); ++ j) 
    add_dependency (name, *j);
}

// ------------------------------------------------------------

void data::add_delay (const signame &name, const signame &dep, const string &text) {
  // a delay always indicates a dependency
  // (but would require a way to select which is rendered)
  // add_dependency (name, dep);

  // find the signal
  sigdata &sig = find_signal (name);
  sigdata &trigger = find_signal (dep);
  delaydata d;
  d.text = text;
  d.trigger = dep;
  d.effect = name;
  d.offset = trigger.numdelays;

  if ((d.n_trigger = trigger.data.size ()) > 0)
    -- d.n_trigger;
  if ((d.n_effect = sig.data.size ()) > 0)
    -- d.n_effect;

  // allow self-referential signals
  if (name == dep && d.n_trigger > 0)
      -- d.n_trigger;

  if (d.n_trigger != d.n_effect
      && ++ trigger.numdelays > trigger.maxdelays)
    trigger.maxdelays = trigger.numdelays;
  delays.push_back (d);
}

// ------------------------------------------------------------

void data::set_value (const signame &name, unsigned n, const sigvalue &value) {
  // find the signal
  sigdata &sig = find_signal (name);

  // pad the sequence so there are n data values
  sigvalue lastval = (sig.data.size () == 0 ? sigvalue ("X", X) : sig.data.back ());
  if (lastval.type == PULSE)
    lastval = sigvalue ("0", ZERO);

  while (sig.data.size () < n) 
    sig.data.push_back (lastval);

  // append the value to the sequence data
  sig.data.push_back (value);
  sig.numdelays = 0;

  if (n + 1 > maxlen)
    maxlen = n + 1;
}

// ------------------------------------------------------------

void data::pad (unsigned n) {
  // pad all sequences to length n
  if (n > maxlen)
    maxlen = n;
  for (signal_database::iterator i = signals.begin (); i != signals.end (); ++ i) {
    sigvalue lastval = (i->second.data.size () == 0 ? sigvalue ("X", X) : 
			i->second.data.back ());
    if (lastval.type == PULSE)
      lastval = sigvalue ("0", ZERO);
    while (i->second.data.size () < maxlen)
      i->second.data.push_back (lastval);
  }
}

// ------------------------------------------------------------

ostream &operator<< (ostream &f, const sigvalue &data) {
  return f << data.text;
}

// ------------------------------------------------------------

ostream &operator<< (ostream &f, const data &data) {
  f << "signals: " << endl;
  for (signal_sequence::const_iterator i = data.sequence.begin ();
       i != data.sequence.end (); ++ i) 
    f << "  " << *i << ": " << data.find_signal (*i) << endl;

  f << endl << "dependencies: " << endl;
  for (list<depdata>::const_iterator i = data.dependencies.begin ();
       i != data.dependencies.end (); ++ i) 
    f << "  " << *i << endl;

  return f;
}

// ------------------------------------------------------------

ostream &operator<< (ostream &f, const sigdata &sig) {
  int n = 0;
  for (value_sequence::const_iterator i = sig.data.begin ();
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
  width = height = 0;
}

diagram::diagram (const diagram &d) {
  *this = d;
}

// ------------------------------------------------------------

diagram &diagram::operator= (const diagram &d) {
  width = d.width;
  height = d.height;
  return *this;
}

// ------------------------------------------------------------

void diagram::render (const data &d, double scale) {
  int base_width, base_height;
  base_size (d, base_width, base_height);

  width = (int)(scale * base_width);
  height = (int)(scale * base_height);

  render_common (d, scale, scale);
}

// ------------------------------------------------------------

void diagram::render (const data &d, int w, int h, bool fixAspect) {
  int base_width, base_height;
  base_size (d, base_width, base_height);

  width = w;
  height = h;

  double hscale = w / (double)base_width;
  double vscale = h / (double)base_height;

  if (fixAspect) {
      // to maintain aspect ratio, and fit the image:
      hscale = vscale = min (hscale, vscale);
  }

  render_common (d, hscale, vscale);
}

// ------------------------------------------------------------

void diagram::render_common (const data &d, double hscale, double vscale) {

  push_back (DrawablePushGraphicContext ());
  push_back (DrawableScaling (hscale, vscale));
  push_back (DrawableFont (vFont, AnyStyle, 100, AnyStretch));
  push_back (DrawablePointSize (vFontPointsize));
  push_back (DrawableStrokeWidth(vLineWidth));
  push_back (DrawableStrokeColor ("black"));

  int labelWidth = label_width (d);

  // draw a "scope-like" diagram for each signal
  map<signame,int> ypos;
  int y = 0;
  for (signal_sequence::const_iterator i = d.sequence.begin ();
       i != d.sequence.end (); ++ i) {
    const sigdata &sig = d.find_signal (*i);
    push_text (vCellWrm, y + vCellHtxt, *i);
    ypos[*i] = y;
    int x = labelWidth + vCellWtsep;
    sigvalue last;
    for (value_sequence::const_iterator j = sig.data.begin ();
	 j != sig.data.end (); ++ j) {
      draw_transition (x, y, last, *j);
      last = *j;
      x += vCellW;
    }
    y += vCellHt + vCellHdel * sig.maxdelays;
  }

  // draw the smooth arrows indicating the triggers for signal changes
  for (list<depdata>::const_iterator i = d.dependencies.begin (); 
       i != d.dependencies.end (); ++ i)
    draw_dependency (labelWidth + vCellWtsep + vCellWrm + vCellW * i->n_trigger,
                     vCellHt/2 + ypos[i->trigger],
		     labelWidth + vCellWtsep + vCellWrm + vCellW * i->n_effect,
                     vCellHt/2 + ypos[i->effect]);

  // draw the timing delay annotations
  for (list<delaydata>::const_iterator i = d.delays.begin (); 
       i != d.delays.end (); ++ i)
    draw_delay (labelWidth + vCellWtsep + vCellWrm + vCellW * i->n_trigger,
                vCellHt/2 + ypos[i->trigger],
		labelWidth + vCellWtsep + vCellWrm + vCellW * i->n_effect,
                vCellHt/2 + ypos[i->effect],
		ypos[i->trigger] + vCellHt + vCellHdel * i->offset + vCellHtdel,
                i->text);

  push_back (DrawablePopGraphicContext ());
}

// ------------------------------------------------------------
// add text to the diagram

void diagram::push_text (double xpos, double ypos, const string &text) {
  push_back (DrawableStrokeWidth(1));
  push_back (DrawableText (xpos, ypos, text));
  push_back (DrawableStrokeWidth(vLineWidth));
}

// ------------------------------------------------------------
// calculate the required label width

int diagram::label_width (const data &d) const {
  int labelWidth = 0;
  Image img;
  TypeMetric m;

  img.font (vFont);
  img.fontPointsize(vFontPointsize);
  for (signal_sequence::const_iterator i = d.sequence.begin ();
       i != d.sequence.end (); ++ i) {
    img.fontTypeMetrics (*i, &m);
    if (m.textWidth () > labelWidth)
      labelWidth = (int) m.textWidth ();
  }
  return labelWidth;
}

// ------------------------------------------------------------
// calculate the basic height and width required before scaling

void diagram::base_size (const data &d, int &w, int &h) const {

  vCellHsep = vCellHt / 8;
  vCellH=vCellHt-vCellHsep;
  vCellHtxt=vCellHt*3/4;
  vCellHdel = vCellHt * 3/8;
  vCellHtdel=vCellHt/4;
  vCellWtsep=vCellW/4;
  vCellWrm=vCellW/8;

  w = vCellWrm*2 + label_width (d) + vCellW * d.maxlen;

  h = 0;
  for (signal_sequence::const_iterator i = d.sequence.begin ();
       i != d.sequence.end (); ++ i) {
    const sigdata &sig = d.find_signal (*i);
    h += vCellHt + vCellHdel * sig.maxdelays;
  }
}

// ------------------------------------------------------------

void diagram::draw_transition (int x, int y, const sigvalue &last,
			       const sigvalue &value) {

  switch (value.type) {
  case ZERO:
    switch (last.type) {
    default:
      push_back (DrawableLine (x, y + vCellH, x + vCellW, y + vCellH));
      break;

    case ONE:
      push_back (DrawableLine (x, y + vCellHsep, x + vCellW/4, y + vCellH));
      push_back (DrawableLine (x + vCellW/4, y + vCellH, x + vCellW, y + vCellH));
      break;
    
    case Z:
      push_back (DrawableLine (x, y + vCellHt/2, x + vCellW/4, y + vCellH));
      push_back (DrawableLine (x + vCellW/4, y + vCellH, x + vCellW, y + vCellH));
      break;

    case STATE:
      push_back (DrawableLine (x, y + vCellHsep, x + vCellW/4, y + vCellH));
      push_back (DrawableLine (x, y + vCellH, x + vCellW, y + vCellH));
      break;
    }
    break;

  case ONE:
    switch (last.type) {
    default:
      push_back (DrawableLine (x, y + vCellHsep, x + vCellW, y + vCellHsep));
      break;

    case ZERO:
    case TICK:
    case PULSE:
      push_back (DrawableLine (x, y + vCellH, x + vCellW/4, y + vCellHsep));
      push_back (DrawableLine (x + vCellW/4, y + vCellHsep, x + vCellW, y + vCellHsep));
      break;

    case Z:
      push_back (DrawableLine (x, y + vCellHt/2, x + vCellW/4, y + vCellHsep));
      push_back (DrawableLine (x + vCellW/4, y + vCellHsep, x + vCellW, y + vCellHsep));
      break;

    case STATE:
      push_back (DrawableLine (x, y + vCellH, x + vCellW/4, y + vCellHsep));
      push_back (DrawableLine (x, y + vCellHsep, x + vCellW, y + vCellHsep));
      break;
    }
    break;

  case TICK:
  case PULSE:
    switch (last.type) {
    default:
      push_back (DrawableLine (x, y + vCellH, x + vCellW/4, y + vCellHsep));
      push_back (DrawableLine (x + vCellW/4, y + vCellHsep, x + vCellW/2, y + vCellHsep));
      push_back (DrawableLine (x + vCellW/2, y + vCellHsep, x + vCellW*3/4, y + vCellH));
      push_back (DrawableLine (x + vCellW*3/4, y + vCellH, x + vCellW, y + vCellH));
      break;

    case ONE:
    case X:
      push_back (DrawableLine (x, y + vCellHsep, x + vCellW/2, y + vCellHsep));
      push_back (DrawableLine (x + vCellW/2, y + vCellHsep, x + vCellW*3/4, y + vCellH));
      push_back (DrawableLine (x + vCellW*3/4, y + vCellH, x + vCellW, y + vCellH));
      break;

    case Z:
      push_back (DrawableLine (x, y + vCellHt/2, x + vCellW/4, y + vCellHsep));
      push_back (DrawableLine (x + vCellW/4, y + vCellHsep, x + vCellW/2, y + vCellHsep));
      push_back (DrawableLine (x + vCellW/2, y + vCellHsep, x + vCellW*3/4, y + vCellH));
      push_back (DrawableLine (x + vCellW*3/4, y + vCellH, x + vCellW, y + vCellH));
      break;

    case STATE:
      push_back (DrawableLine (x, y + vCellH, x + vCellW/4, y + vCellHsep));
      push_back (DrawableLine (x, y + vCellHsep, x + vCellW/2, y + vCellHsep));
      push_back (DrawableLine (x + vCellW/2, y + vCellHsep, x + vCellW*3/4, y + vCellH));
      push_back (DrawableLine (x + vCellW*3/4, y + vCellH, x + vCellW, y + vCellH));
      break;
    }
    break;
  
  case UNDEF:
  case X:
    for (int i = 0; i < 4; ++ i) {
      push_back (DrawableLine (x+i*(vCellW/4), y + vCellH,
                               x+(i+1)*(vCellW/4), y + vCellHsep));
      push_back (DrawableLine (x+i*(vCellW/4), y + vCellHsep,
                               x+(i+1)*(vCellW/4), y + vCellH));
    }
    break;
  
  case Z:
    switch (last.type) {
    default:
      push_back (DrawableLine (x, y + vCellHt/2, x + vCellW, y + vCellHt/2));
      break;

    case ZERO:
    case TICK:
    case PULSE:
      push_back (DrawableLine (x, y + vCellH, x + vCellW/4, y + vCellHt/2));
      push_back (DrawableLine (x + vCellW/4, y + vCellHt/2, x + vCellW, y + vCellHt/2));
      break;

    case ONE:
      push_back (DrawableLine (x, y + vCellHsep, x + vCellW/4, y + vCellHt/2));
      push_back (DrawableLine (x + vCellW/4, y + vCellHt/2, x + vCellW, y + vCellHt/2));
      break;

    case STATE:
      push_back (DrawableLine (x, y + vCellHsep, x + vCellW/8, y + vCellHt/2));
      push_back (DrawableLine (x, y + vCellH, x + vCellW/8, y + vCellHt/2));
      push_back (DrawableLine (x + vCellW/8, y + vCellHt/2, x + vCellW, y + vCellHt/2));
      break;
    }
    break;
  
  case STATE:
    switch (last.type) {
    default:
      if (value.text != last.text) {
	push_back (DrawableLine (x, y + vCellHsep, x + vCellW/4, y + vCellH));
	push_back (DrawableLine (x, y + vCellH, x + vCellW/4, y + vCellHsep));
	push_back (DrawableLine (x + vCellW/4, y + vCellHsep, x + vCellW, y + vCellHsep));
	push_back (DrawableLine (x + vCellW/4, y + vCellH, x + vCellW, y + vCellH));
	push_text (x + vCellW/4, y + vCellHtxt, value.text);
      }
      else {
	push_back (DrawableLine (x, y + vCellHsep, x + vCellW, y + vCellHsep));
	push_back (DrawableLine (x, y + vCellH, x + vCellW, y + vCellH));
      }
      break;

    case ZERO:
    case TICK:
    case PULSE:
      push_back (DrawableLine (x, y + vCellH, x + vCellW/4, y + vCellHsep));
      push_back (DrawableLine (x + vCellW/4, y + vCellHsep, x + vCellW, y + vCellHsep));
      push_back (DrawableLine (x, y + vCellH, x + vCellW, y + vCellH));
      push_text (x + vCellW/4, y + vCellHtxt, value.text);
      break;
    
    case ONE:
      push_back (DrawableLine (x, y + vCellHsep, x + vCellW/4, y + vCellH));
      push_back (DrawableLine (x + vCellW/4, y + vCellH, x + vCellW, y + vCellH));
      push_back (DrawableLine (x, y + vCellHsep, x + vCellW, y + vCellHsep));
      push_text (x + vCellW/4, y + vCellHtxt, value.text);
      break;
    
    case Z:
      push_back (DrawableLine (x, y + vCellW/4, x + vCellW/8, y + vCellH));
      push_back (DrawableLine (x, y + vCellW/4, x + vCellW/8, y + vCellHsep));
      push_back (DrawableLine (x + vCellW/8, y + vCellH, x + vCellW, y + vCellH));
      push_back (DrawableLine (x + vCellW/8, y + vCellHsep, x + vCellW, y + vCellHsep));
      push_text (x + vCellW/8, y + vCellHtxt, value.text);
      break;
    }
  }
}

// ------------------------------------------------------------

void diagram::draw_dependency (int x0, int y0, int x1, int y1) {
  list<Coordinate> shaft, head;

  push_back (DrawablePushGraphicContext ());
  push_back (DrawableStrokeColor ("blue"));

  if (x0 == x1) {
    int w = vCellW/20, h = vCellHt/6, h2 = vCellHt/10;

    if (y0 < y1) {
      y1 -= vCellHt/4;
      push_back (DrawableLine (x0, y0, x1, y1));
      push_back (DrawableFillColor ("blue"));
      head.push_back (Coordinate (x1, y1));
      head.push_back (Coordinate (x1 - w, y1 - h));
      head.push_back (Coordinate (x1, y1 - h2));
      head.push_back (Coordinate (x1 + w, y1 - h));
      push_back (DrawablePolygon (head));
    }
    else {
      y1 += vCellHt/4;
      push_back (DrawableLine (x0, y0, x1, y1));
      push_back (DrawableFillColor ("blue"));
      head.push_back (Coordinate (x1, y1));
      head.push_back (Coordinate (x1 - w, y1 + h));
      head.push_back (Coordinate (x1, y1 + h2));
      head.push_back (Coordinate (x1 + w, y1 + h));
      push_back (DrawablePolygon (head));
    }
  }
  else {
    int h = vCellHt/10, w1 = vCellW/12, w2 = vCellW/20;
    x1 -= vCellW/16;
    push_back (DrawableFillColor ("none"));
    push_back (DrawableFillOpacity (0));
    shaft.push_back (Coordinate (x0, y0));
    shaft.push_back (Coordinate ((x0 + x1) / 2, y1));
    shaft.push_back (Coordinate ((x0 + x1) / 2, y1));
    shaft.push_back (Coordinate (x1, y1));
    push_back (DrawableBezier (shaft));
    push_back (DrawableFillColor ("blue"));
    head.push_back (Coordinate (x1, y1));
    head.push_back (Coordinate (x1 - w1, y1 - h));
    head.push_back (Coordinate (x1 - w2, y1));
    head.push_back (Coordinate (x1 - w1, y1 + h));
    push_back (DrawablePolygon (head));
  }

  push_back (DrawablePopGraphicContext ());
}

// ------------------------------------------------------------

void diagram::draw_delay (int x0, int y0, int x1, int y1, int y2, 
			  const string &text) {
  list<Coordinate> head;

  push_back (DrawablePushGraphicContext ());
  push_back (DrawableStrokeColor ("blue"));

  if (x0 == x1) 
    push_back (DrawableLine (x0, y0, x1, y1));
  else {
    push_back (DrawableText (x0 + vCellWtsep, y2 - vCellHt/16, text));
    push_back (DrawableLine (x0, y0, x0, y2 + vCellHt/8));
    push_back (DrawableLine (x1, y1, x1, y2 - vCellHt/8));
    push_back (DrawableLine (x0, y2, x1, y2));
    push_back (DrawableFillColor ("blue"));
    head.push_back (Coordinate (x1, y2));
    head.push_back (Coordinate (x1 - vCellW/12, y2 - vCellHt/10));
    head.push_back (Coordinate (x1 - vCellW/20, y2));
    head.push_back (Coordinate (x1 - vCellW/12, y2 + vCellHt/10));
    push_back (DrawablePolygon (head));
  }
  push_back (DrawablePopGraphicContext ());
}

