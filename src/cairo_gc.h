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

#ifndef __CAIRO_GC_H
#define __CAIRO_GC_H

#ifndef LITE
#include <gc.h>
#include <cairo.h>
#include <cairo-svg.h>
#include <cairo-pdf.h>
#include <cairo-ps.h>

namespace timing {

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

}

#endif // LITE

#endif //header
