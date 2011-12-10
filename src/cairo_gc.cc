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

#ifndef LITE

#include "timing.h"
#include <string>

using namespace timing;

// ------------------------------------------------------------

cairo_gc::cairo_gc (const std::string& filename) :
  gc(filename),
  surface(NULL),
  cr(NULL)
{
}

cairo_gc::~cairo_gc (void) {
  cairo_destroy(cr);
  cairo_surface_destroy(surface);
}

// ------------------------------------------------------------

void cairo_gc::uniform_stroke(){
  const double scale = (scale_x + scale_y) * 0.5;

  cairo_save(cr);
  cairo_identity_matrix(cr);
  cairo_scale(cr, scale, scale);
  cairo_set_font_size(cr, font_size);
  cairo_set_line_width (cr, stroke_width);

  cairo_stroke(cr);

  cairo_restore(cr);
}

void cairo_gc::uniform_text(const std::string &text){
  const double scale = (scale_x + scale_y) * 0.5;

  cairo_save(cr);
  cairo_identity_matrix(cr);
  cairo_scale(cr, scale, scale);
  cairo_set_font_size(cr, font_size);
  cairo_set_line_width (cr, stroke_width);

  cairo_show_text(cr, text.c_str());

  cairo_restore(cr);
}

double cairo_gc::get_label_width(const data &d) {

  double labelWidth = 0;
  cairo_text_extents_t te;

  for (signal_sequence::const_iterator i = d.sequence.begin ();
       i != d.sequence.end (); ++ i) {
    cairo_text_extents (cr, i->c_str(), &te);
    if (te.width > labelWidth)
      labelWidth = te.width;
  }

  return labelWidth;
}

void cairo_gc::bezier (const std::list<Coordinate> &points) {

  std::list<Coordinate>::const_iterator it0 = points.begin();
  std::list<Coordinate>::const_iterator it1 = points.begin(); ++it1;
  std::list<Coordinate>::const_iterator it2 = points.begin(); ++it2; ++it2;

  cairo_move_to(cr, it0->x(), it0->y());
  ++it0; ++it1; ++it2;

  while(it2 != points.end())
  {
    cairo_curve_to(cr,
      it0->x(), it0->y(),
      it1->x(), it1->y(),
      it2->x(), it2->y()
    );

    ++it0; ++it1; ++it2;
  }

  uniform_stroke();
}

// ------------------------------------------------------------

void cairo_gc::font (const std::string& name) {
  cairo_select_font_face (cr, name.c_str(), CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
}

// ------------------------------------------------------------

void cairo_gc::line (int x1, int y1, int x2, int y2) {

  cairo_move_to(cr, x1, y1);
  cairo_line_to(cr, x2, y2);
  uniform_stroke();
}

// ------------------------------------------------------------

void cairo_gc::point_size (int size) {
  cairo_set_font_size (cr, size);
  font_size = size;
}

// ------------------------------------------------------------

void cairo_gc::polygon (const std::list<Coordinate> &points)
{
  // Draw fill
  std::list<Coordinate>::const_iterator it = points.begin();

  cairo_move_to(cr, it->x(), it->y());
  while(it != points.end())
  {
    cairo_line_to(cr, it->x(), it->y());
    ++it;
  }
  cairo_close_path(cr);

  cairo_fill(cr);

  // Draw outline
  it = points.begin();
  cairo_move_to(cr, it->x(), it->y());
  while(it != points.end())
  {
    cairo_line_to(cr, it->x(), it->y());
    ++it;
  }
  cairo_close_path(cr);
  uniform_stroke();
}

// ------------------------------------------------------------
void cairo_gc::pop (void)
{
  cairo_restore(cr);
}

// ------------------------------------------------------------

void cairo_gc::push (void)
{
  cairo_save(cr);
}

// ------------------------------------------------------------

void cairo_gc::scaling (double hscale, double vscale)
{
  cairo_identity_matrix(cr);
  cairo_scale(cr, hscale, vscale);
  scale_x = hscale;
  scale_y = vscale;
}

// ------------------------------------------------------------

void cairo_gc::color (const std::string& name)
{
  double r, g, b;
  decode_color(name, &r, &g, &b);
  cairo_set_source_rgb(cr, r, g, b);
}

// ------------------------------------------------------------

void cairo_gc::set_stroke_width (int w)
{
  double ux=w, uy=w;
  cairo_device_to_user_distance (cr, &ux, &uy);
  if (ux < uy)
    ux = uy;
  cairo_set_line_width (cr, ux);

  stroke_width = w;
}

// ------------------------------------------------------------

void cairo_gc::text (int x, int y, const std::string &text)
{
  cairo_move_to(cr, x, y);
  uniform_text(text);
}

void cairo_gc::set_surface_size(double w, double h){

  if (cr != NULL){
    cairo_destroy(cr);
    cr = NULL;
  }

  if (surface != NULL){
    cairo_surface_destroy(surface);
    surface = NULL;
  }

  width = w;
  height = h;

  surface = create_surface();
  if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
    throw timing::exception();

  cr = cairo_create(surface);
  if (cairo_status(cr) != CAIRO_STATUS_SUCCESS)
    throw timing::exception();

  color("white");
  cairo_rectangle(cr, 0, 0, width, height);
  cairo_fill(cr);
}

// ------------------------------------------------------------

#ifdef CAIRO_HAS_PNG_FUNCTIONS
cairo_png_gc::cairo_png_gc(const std::string& filename) :
  cairo_gc(filename)
{
  set_surface_size(2, 2);
}
cairo_surface_t* cairo_png_gc::create_surface()
{
  return cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
}
void cairo_png_gc::finish_surface() {
  if (cairo_surface_write_to_png(surface, fname.c_str()) != CAIRO_STATUS_SUCCESS)
    throw timing::exception();
}
#endif // CAIRO_HAS_PNG_FUNCTIONS

#ifdef CAIRO_HAS_PDF_SURFACE
cairo_pdf_gc::cairo_pdf_gc(const std::string& filename) :
  cairo_gc(filename)
{
  set_surface_size(2, 2);
}
cairo_surface_t* cairo_pdf_gc::create_surface()
{
  return cairo_pdf_surface_create(fname.c_str(), width, height);
}
void cairo_pdf_gc::finish_surface(){
}

#endif // CAIRO_HAS_PDF_SURFACE

#ifdef CAIRO_HAS_SVG_SURFACE
cairo_svg_gc::cairo_svg_gc(const std::string& filename) :
  cairo_gc(filename)
{
  set_surface_size(2, 2);
}
cairo_surface_t* cairo_svg_gc::create_surface()
{
  return cairo_svg_surface_create(fname.c_str(), width, height);
}
void cairo_svg_gc::finish_surface(){
}
#endif // CAIRO_HAS_SVG_SURFACE

#ifdef CAIRO_HAS_PS_SURFACE
cairo_ps_gc::cairo_ps_gc(const std::string& filename) :
  cairo_gc(filename)
{
  set_surface_size(2, 2);
}
cairo_surface_t* cairo_ps_gc::create_surface()
{
  return cairo_ps_surface_create(fname.c_str(), width, height);
}
void cairo_ps_gc::finish_surface(){
}

cairo_eps_gc::cairo_eps_gc(const std::string& filename) :
  cairo_gc(filename)
{
  set_surface_size(2, 2);
}
cairo_surface_t* cairo_eps_gc::create_surface()
{
  cairo_surface_t* surf = cairo_ps_surface_create(fname.c_str(), width, height);
  cairo_ps_surface_set_eps(surf, 1);
  return surf;
}
void cairo_eps_gc::finish_surface(){
}
#endif // CAIRO_HAS_PS_SURFACE

#endif /* ! LITE */

