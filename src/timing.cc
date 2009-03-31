// Copyright (c)2004 by Edward Counce, All rights reserved. 
// Copyright (c)2006-7 by Salvador E. Tropea, All rights reserved.
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include "timing.h"
#include <map>
#include <fstream>
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
// calculate the required label width

static int label_width (const data &d) {
  int labelWidth = 0;

#ifndef LITE
  TypeMetric m;
  Image img;

  img.font (vFont);
  img.fontPointsize(vFontPointsize);
  for (signal_sequence::const_iterator i = d.sequence.begin ();
       i != d.sequence.end (); ++ i) {
    img.fontTypeMetrics (*i, &m);
    if (m.textWidth () > labelWidth)
      labelWidth = (int) m.textWidth ();
  }
#else
  int m = 0;
  for (signal_sequence::const_iterator i = d.sequence.begin ();
       i != d.sequence.end (); ++ i) {
    if ((*i).size() > m)
      m = (*i).size();
  }
  labelWidth = (int)(0.7 * m * vFontPointsize);
#endif /* LITE */

  return labelWidth;
}

// ------------------------------------------------------------
// calculate the basic height and width required before scaling

static void base_size (const data &d, int &w, int &h) {

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
// add text to the diagram

static void push_text (gc &gc, double xpos, double ypos, const string &text) {
  gc.stroke_width (1);
  gc.text (int (xpos), int (ypos), text);
  gc.stroke_width (vLineWidth);
}

// ------------------------------------------------------------

static void draw_transition (gc &gc, int x, int y, const sigvalue &last,
			     const sigvalue &value) {

  switch (value.type) {
  case ZERO:
    switch (last.type) {
    default:
      gc.line (x, y + vCellH, x + vCellW, y + vCellH);
      break;

    case ONE:
      gc.line (x, y + vCellHsep, x + vCellW/4, y + vCellH);
      gc.line (x + vCellW/4, y + vCellH, x + vCellW, y + vCellH);
      break;
    
    case Z:
      gc.line (x, y + vCellHt/2, x + vCellW/4, y + vCellH);
      gc.line (x + vCellW/4, y + vCellH, x + vCellW, y + vCellH);
      break;

    case STATE:
      gc.line (x, y + vCellHsep, x + vCellW/4, y + vCellH);
      gc.line (x, y + vCellH, x + vCellW, y + vCellH);
      break;
    }
    break;

  case ONE:
    switch (last.type) {
    default:
      gc.line (x, y + vCellHsep, x + vCellW, y + vCellHsep);
      break;

    case ZERO:
    case TICK:
    case PULSE:
      gc.line (x, y + vCellH, x + vCellW/4, y + vCellHsep);
      gc.line (x + vCellW/4, y + vCellHsep, x + vCellW, y + vCellHsep);
      break;

    case Z:
      gc.line (x, y + vCellHt/2, x + vCellW/4, y + vCellHsep);
      gc.line (x + vCellW/4, y + vCellHsep, x + vCellW, y + vCellHsep);
      break;

    case STATE:
      gc.line (x, y + vCellH, x + vCellW/4, y + vCellHsep);
      gc.line (x, y + vCellHsep, x + vCellW, y + vCellHsep);
      break;
    }
    break;

  case TICK:
  case PULSE:
    switch (last.type) {
    default:
      gc.line (x, y + vCellH, x + vCellW/4, y + vCellHsep);
      gc.line (x + vCellW/4, y + vCellHsep, x + vCellW/2, y + vCellHsep);
      gc.line (x + vCellW/2, y + vCellHsep, x + vCellW*3/4, y + vCellH);
      gc.line (x + vCellW*3/4, y + vCellH, x + vCellW, y + vCellH);
      break;

    case ONE:
    case X:
      gc.line (x, y + vCellHsep, x + vCellW/2, y + vCellHsep);
      gc.line (x + vCellW/2, y + vCellHsep, x + vCellW*3/4, y + vCellH);
      gc.line (x + vCellW*3/4, y + vCellH, x + vCellW, y + vCellH);
      break;

    case Z:
      gc.line (x, y + vCellHt/2, x + vCellW/4, y + vCellHsep);
      gc.line (x + vCellW/4, y + vCellHsep, x + vCellW/2, y + vCellHsep);
      gc.line (x + vCellW/2, y + vCellHsep, x + vCellW*3/4, y + vCellH);
      gc.line (x + vCellW*3/4, y + vCellH, x + vCellW, y + vCellH);
      break;

    case STATE:
      gc.line (x, y + vCellH, x + vCellW/4, y + vCellHsep);
      gc.line (x, y + vCellHsep, x + vCellW/2, y + vCellHsep);
      gc.line (x + vCellW/2, y + vCellHsep, x + vCellW*3/4, y + vCellH);
      gc.line (x + vCellW*3/4, y + vCellH, x + vCellW, y + vCellH);
      break;
    }
    break;
  
  case UNDEF:
  case X:
    for (int i = 0; i < 4; ++ i) {
      gc.line (x+i*(vCellW/4), y + vCellH,
	       x+(i+1)*(vCellW/4), y + vCellHsep);
      gc.line (x+i*(vCellW/4), y + vCellHsep,
	       x+(i+1)*(vCellW/4), y + vCellH);
    }
    break;
  
  case Z:
    switch (last.type) {
    default:
      gc.line (x, y + vCellHt/2, x + vCellW, y + vCellHt/2);
      break;

    case ZERO:
    case TICK:
    case PULSE:
      gc.line (x, y + vCellH, x + vCellW/4, y + vCellHt/2);
      gc.line (x + vCellW/4, y + vCellHt/2, x + vCellW, y + vCellHt/2);
      break;

    case ONE:
      gc.line (x, y + vCellHsep, x + vCellW/4, y + vCellHt/2);
      gc.line (x + vCellW/4, y + vCellHt/2, x + vCellW, y + vCellHt/2);
      break;

    case STATE:
      gc.line (x, y + vCellHsep, x + vCellW/8, y + vCellHt/2);
      gc.line (x, y + vCellH, x + vCellW/8, y + vCellHt/2);
      gc.line (x + vCellW/8, y + vCellHt/2, x + vCellW, y + vCellHt/2);
      break;
    }
    break;
  
  case STATE:
    switch (last.type) {
    default:
      if (value.text != last.text) {
	gc.line (x, y + vCellHsep, x + vCellW/4, y + vCellH);
	gc.line (x, y + vCellH, x + vCellW/4, y + vCellHsep);
	gc.line (x + vCellW/4, y + vCellHsep, x + vCellW, y + vCellHsep);
	gc.line (x + vCellW/4, y + vCellH, x + vCellW, y + vCellH);
	push_text (gc, x + vCellW/4, y + vCellHtxt, value.text);
      }
      else {
	gc.line (x, y + vCellHsep, x + vCellW, y + vCellHsep);
	gc.line (x, y + vCellH, x + vCellW, y + vCellH);
      }
      break;

    case ZERO:
    case TICK:
    case PULSE:
      gc.line (x, y + vCellH, x + vCellW/4, y + vCellHsep);
      gc.line (x + vCellW/4, y + vCellHsep, x + vCellW, y + vCellHsep);
      gc.line (x, y + vCellH, x + vCellW, y + vCellH);
      push_text (gc, x + vCellW/4, y + vCellHtxt, value.text);
      break;
    
    case ONE:
      gc.line (x, y + vCellHsep, x + vCellW/4, y + vCellH);
      gc.line (x + vCellW/4, y + vCellH, x + vCellW, y + vCellH);
      gc.line (x, y + vCellHsep, x + vCellW, y + vCellHsep);
      push_text (gc, x + vCellW/4, y + vCellHtxt, value.text);
      break;
    
    case Z:
      gc.line (x, y + vCellW/4, x + vCellW/8, y + vCellH);
      gc.line (x, y + vCellW/4, x + vCellW/8, y + vCellHsep);
      gc.line (x + vCellW/8, y + vCellH, x + vCellW, y + vCellH);
      gc.line (x + vCellW/8, y + vCellHsep, x + vCellW, y + vCellHsep);
      push_text (gc, x + vCellW/8, y + vCellHtxt, value.text);
      break;
    }
  }
}

// ------------------------------------------------------------

static void draw_dependency (gc &gc, int x0, int y0, int x1, int y1) {
  list<Coordinate> shaft, head;

  gc.push ();
  gc.stroke_color ("blue");

  if (x0 == x1) {
    int w = vCellW/20, h = vCellHt/6, h2 = vCellHt/10;

    if (y0 < y1) {
      y1 -= vCellHt/4;
      gc.line (x0, y0, x1, y1);
      gc.fill_color ("blue");
      head.push_back (Coordinate (x1, y1));
      head.push_back (Coordinate (x1 - w, y1 - h));
      head.push_back (Coordinate (x1, y1 - h2));
      head.push_back (Coordinate (x1 + w, y1 - h));
      gc.polygon (head);
    }
    else {
      y1 += vCellHt/4;
      gc.line (x0, y0, x1, y1);
      gc.fill_color ("blue");
      head.push_back (Coordinate (x1, y1));
      head.push_back (Coordinate (x1 - w, y1 + h));
      head.push_back (Coordinate (x1, y1 + h2));
      head.push_back (Coordinate (x1 + w, y1 + h));
      gc.polygon (head);
    }
  }
  else {
    int h = vCellHt/10, w1 = vCellW/12, w2 = vCellW/20;
    x1 -= vCellW/16;
    gc.fill_color ("none");
    gc.fill_opacity (0);
    shaft.push_back (Coordinate (x0, y0));
    shaft.push_back (Coordinate ((x0 + x1) / 2, y1));
    shaft.push_back (Coordinate ((x0 + x1) / 2, y1));
    shaft.push_back (Coordinate (x1, y1));
    gc.bezier (shaft);
    gc.fill_color ("blue");
    head.push_back (Coordinate (x1, y1));
    head.push_back (Coordinate (x1 - w1, y1 - h));
    head.push_back (Coordinate (x1 - w2, y1));
    head.push_back (Coordinate (x1 - w1, y1 + h));
    gc.polygon (head);
  }

  gc.pop ();
}

// ------------------------------------------------------------

static void draw_delay (gc &gc, int x0, int y0, int x1, int y1, int y2,
			const string &text) {
  list<Coordinate> head;

  gc.push ();
  gc.stroke_color ("blue");

  if (x0 == x1) 
    gc.line (x0, y0, x1, y1);
  else {
    gc.text (x0 + vCellWtsep, y2 - vCellHt/16, text);
    gc.line (x0, y0, x0, y2 + vCellHt/8);
    gc.line (x1, y1, x1, y2 - vCellHt/8);
    gc.line (x0, y2, x1, y2);
    gc.fill_color ("blue");
    head.push_back (Coordinate (x1, y2));
    head.push_back (Coordinate (x1 - vCellW/12, y2 - vCellHt/10));
    head.push_back (Coordinate (x1 - vCellW/20, y2));
    head.push_back (Coordinate (x1 - vCellW/12, y2 + vCellHt/10));
    gc.polygon (head);
  }

  gc.pop ();
}

// ------------------------------------------------------------

static void render_common (gc& gc, const data &d,
    			   double hscale, double vscale) {

  gc.push ();
  gc.scaling (hscale, vscale);
  gc.font (vFont);
  gc.point_size (vFontPointsize);
  gc.stroke_width (vLineWidth);
  gc.stroke_color ("black");

  int labelWidth = label_width (d);

  // draw a "scope-like" diagram for each signal
  map<signame,int> ypos;
  int y = 0;
  for (signal_sequence::const_iterator i = d.sequence.begin ();
       i != d.sequence.end (); ++ i) {
    const sigdata &sig = d.find_signal (*i);
    push_text (gc, vCellWrm, y + vCellHtxt, *i);
    ypos[*i] = y;
    int x = labelWidth + vCellWtsep;
    sigvalue last;
    for (value_sequence::const_iterator j = sig.data.begin ();
	 j != sig.data.end (); ++ j) {
      draw_transition (gc, x, y, last, *j);
      last = *j;
      x += vCellW;
    }
    y += vCellHt + vCellHdel * sig.maxdelays;
  }

  // draw the smooth arrows indicating the triggers for signal changes
  for (list<depdata>::const_iterator i = d.dependencies.begin ();
       i != d.dependencies.end (); ++ i)
    draw_dependency (gc, labelWidth + vCellWtsep + vCellWrm + vCellW * i->n_trigger,
                     vCellHt/2 + ypos[i->trigger],
		     labelWidth + vCellWtsep + vCellWrm + vCellW * i->n_effect,
                     vCellHt/2 + ypos[i->effect]);

  // draw the timing delay annotations
  for (list<delaydata>::const_iterator i = d.delays.begin ();
       i != d.delays.end (); ++ i)
    draw_delay (gc, labelWidth + vCellWtsep + vCellWrm + vCellW * i->n_trigger,
                vCellHt/2 + ypos[i->trigger],
		labelWidth + vCellWtsep + vCellWrm + vCellW * i->n_effect,
                vCellHt/2 + ypos[i->effect],
		ypos[i->trigger] + vCellHt + vCellHdel * i->offset + vCellHtdel,
                i->text);

  gc.pop ();
}

// ------------------------------------------------------------

void timing::render (gc &gc, const data &d, double scale) {
  int base_width, base_height;
  base_size (d, base_width, base_height);

  gc.width = (int)(scale * base_width);
  gc.height = (int)(scale * base_height);

  render_common (gc, d, scale, scale);
}

// ------------------------------------------------------------

void timing::render (gc &gc, const data &d, int w, int h, bool fixAspect) {
  int base_width, base_height;
  base_size (d, base_width, base_height);

  gc.width = w;
  gc.height = h;

  double hscale = w / (double)base_width;
  double vscale = h / (double)base_height;

  if (fixAspect) {
      // to maintain aspect ratio, and fit the image:
      hscale = vscale = min (hscale, vscale);
  }

  render_common (gc, d, hscale, vscale);
}

// ------------------------------------------------------------

#ifndef LITE
magick_gc::~magick_gc (void) {
}

// ------------------------------------------------------------

void magick_gc::bezier (const std::list<Magick::Coordinate> &points) {
  drawables.push_back (DrawableBezier (points));
}

// ------------------------------------------------------------

void magick_gc::fill_color (const std::string &name) {
  drawables.push_back (DrawableFillColor (name));
}

// ------------------------------------------------------------

void magick_gc::fill_opacity (int op) {
  drawables.push_back (DrawableFillOpacity (op));
}

// ------------------------------------------------------------

void magick_gc::font (const std::string& name) {
  drawables.push_back (DrawableFont (name, AnyStyle, 100, AnyStretch));
}

// ------------------------------------------------------------

void magick_gc::line (int x1, int y1, int x2, int y2) {
  drawables.push_back (DrawableLine (x1, y1, x2, y2));
}

// ------------------------------------------------------------

void magick_gc::point_size (int size) {
  drawables.push_back (DrawablePointSize (size));
}

// ------------------------------------------------------------

void magick_gc::polygon (const std::list<Magick::Coordinate> &points)
{
  drawables.push_back (DrawablePolygon (points));
}

// ------------------------------------------------------------

void magick_gc::pop (void)
{
  drawables.push_back (DrawablePopGraphicContext ());
}

// ------------------------------------------------------------

void magick_gc::push (void)
{
  drawables.push_back (DrawablePushGraphicContext ());
}

// ------------------------------------------------------------

void magick_gc::scaling (double hscale, double vscale)
{
  drawables.push_back (DrawableScaling (hscale, vscale));
}

// ------------------------------------------------------------

void magick_gc::stroke_color (const std::string& name)
{
  drawables.push_back (DrawableStrokeColor (name));
}

// ------------------------------------------------------------

void magick_gc::stroke_width (int w)
{
  drawables.push_back (DrawableStrokeWidth (w));
}

// ------------------------------------------------------------

void magick_gc::text (int x, int y, const std::string &text)
{
  drawables.push_back (DrawableText (x, y, text));
}

// ------------------------------------------------------------

void magick_gc::draw (Magick::Image& img) const
{
  img.draw (drawables);
}

#endif /* ! LITE */

// ------------------------------------------------------------

postscript_gc::postscript_gc (void) {
}

postscript_gc::~postscript_gc (void) {
}

// ------------------------------------------------------------

void postscript_gc::bezier (const std::list<Magick::Coordinate> &points) {
  std::list<Magick::Coordinate>::const_iterator i = points.begin();

  ps_text << "newpath\n";
  ps_text << i->x () << ' ' << (height - i->y ()) << " moveto\n";
  i++;

  while (i != points.end ()) {
    ps_text << i->x () << ' ' << (height - i->y ()) << "\n";
    ++i;
  }

  ps_text << "curveto\n";
  ps_text << "stroke\n";
}

// ------------------------------------------------------------

void postscript_gc::fill_color (const std::string& name) {
  stroke_color (name);
}

// ------------------------------------------------------------

void postscript_gc::fill_opacity (int op) {
}

// ------------------------------------------------------------

void postscript_gc::font (const std::string& name) {
  ps_text << '/' << name << " findfont\n";
}

// ------------------------------------------------------------

void postscript_gc::line (int x1, int y1, int x2, int y2) {
  ps_text << "newpath\n";
  ps_text << x1 << ' ' << (height - y1) << " moveto\n";
  ps_text << x2 << ' ' << (height - y2) << " lineto\n";
  ps_text << "stroke\n";
}

// ------------------------------------------------------------

void postscript_gc::point_size (int size) {
  ps_text << size << " scalefont\nsetfont\n";
}

// ------------------------------------------------------------

void postscript_gc::polygon (const std::list<Magick::Coordinate> &points) {
  static const char *ops[] = {"stroke", "fill"};
  std::list<Magick::Coordinate>::const_iterator i;
  int j;

  for (j = 0; j < 2; j++) {
    ps_text << "newpath\n";
    i = points.begin();
    ps_text << i->x () << ' ' << (height - i->y ()) << " moveto\n";
    i++;

    while (i != points.end ()) {
      ps_text << i->x () << ' ' << (height - i->y ()) << " lineto\n";
      ++i;
    }

    ps_text << "closepath\n";
    ps_text << ops[j] << '\n';
  }
}

// ------------------------------------------------------------

void postscript_gc::pop (void) {
  ps_text << "restore\n";
}

// ------------------------------------------------------------

void postscript_gc::push (void) {
  ps_text << "save\n";
}

// ------------------------------------------------------------

void postscript_gc::scaling (double hscale, double vscale) {
  ps_text << hscale << ' ' << vscale << " scale\n";
}

// ------------------------------------------------------------

void postscript_gc::stroke_color (const std::string& name) {
  if (name == "black")
    ps_text << "0.0 0.0 0.0 setrgbcolor\n";
  else
    ps_text << "0.0 0.0 1.0 setrgbcolor\n";
}

// ------------------------------------------------------------

void postscript_gc::stroke_width (int w) {
  ps_text << w << " setlinewidth\n";
}

// ------------------------------------------------------------

void postscript_gc::text (int x, int y, const std::string& text) {
  unsigned int i;

  ps_text << "newpath\n";
  ps_text << x << ' ' << (height - y) << " moveto\n";

  ps_text << '(';
  for (i = 0; i < text.size(); i++) {
    char c = text[i];

    if (c == '(' || c == ')')
      ps_text << '\\';
    ps_text << c;
  }

  ps_text << ')' << " show\n";
}

// ------------------------------------------------------------

static std::string filename_ext(const std::string &fname)
{
  int i = fname.size () - 1;

  while (i >= 0 && fname[i] != '.')
    i--;

  return std::string (fname, i + 1);
}

// ------------------------------------------------------------

void postscript_gc::print (std::ostream& out) const {
  out << ps_text.str();
}

// ------------------------------------------------------------

void postscript_gc::print (const std::string& filename) const {
  std::ofstream out;

  out.exceptions (ofstream::failbit | ofstream::badbit);
  out.open (filename.c_str());

  std::string ext = filename_ext (filename);

  if (!strcasecmp (ext.c_str (), "eps")) {
    out << "%!PS-Adobe-3.0 EPSF-3.0\n";
    out << "%%BoundingBox: 0 0 " << width << ' ' << height << '\n';
    print (out);
  } else {
    out << "%!PS-Adobe-3.0\n";
    print (out);
    out << "showpage\n";
  }

  out << "%%EOF\n";
}

// ------------------------------------------------------------

bool postscript_gc::has_ps_ext (const std::string &filename) {
  std::string ext = filename_ext (filename);

  return !(strcasecmp (ext.c_str (), "ps") &&
    	   strcasecmp (ext.c_str (), "eps"));
}
