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

#ifdef LITE

#include "timing.h"
#include <fstream>

using namespace std;
using namespace timing;

postscript_gc::postscript_gc (const std::string& filename) :
    gc(filename)
{
}

postscript_gc::~postscript_gc (void) {
}

void postscript_gc::finish_surface() {
  std::ofstream out;

  out.exceptions (ofstream::failbit | ofstream::badbit);
  out.open (fname.c_str());

  if (has_ext(fname, "eps")) {
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

void postscript_gc::set_surface_size(double w, double h){
  width = w;
  height = h;
}

double postscript_gc::get_label_width(const data &d) {
  double labelWidth = 0;

  int m = 0;
  for (signal_sequence::const_iterator i = d.sequence.begin ();
       i != d.sequence.end (); ++ i) {
    if ((*i).size() > m)
      m = (*i).size();
  }
  labelWidth = (int)(0.7 * m * vFontPointsize);

  return labelWidth;
}

void postscript_gc::bezier (const std::list<Coordinate> &points) {
  std::list<Coordinate>::const_iterator i = points.begin();

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

void postscript_gc::polygon (const std::list<Coordinate> &points) {
  static const char *ops[] = {"stroke", "fill"};
  std::list<Coordinate>::const_iterator i;
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

void postscript_gc::color (const std::string& name) {

  double r, g, b;
  decode_color(name, &r, &g, &b);

  ps_text << fixed << r << " " << g << " " << b << " setrgbcolor\n";
}

// ------------------------------------------------------------

void postscript_gc::set_stroke_width (int w) {
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

void postscript_gc::print (std::ostream& out) const {
  out << ps_text.str();
}

// ------------------------------------------------------------

#endif  // LITE
