// Copyright (c)2004-2007 by Edward Counce, All rights reserved.
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

#include "globals.h"
#ifdef HAVE_GETOPT_H
#  include <getopt.h>
#else
#  include <unistd.h>
#  define getopt_long(C,V,S,O,I) getopt(C,V,S)
#endif
using namespace std;
using namespace Magick;

#define FLAG_PAGESIZE 1
#define FLAG_SCALE 2
#define FLAG_ASPECT 4

extern FILE *yyin;
extern int yydebug;
int yyparse (void);
static void usage (void);
static void banner (void);
static void freesoft (void);

unsigned n;
timing::data data;
timing::signal_sequence deps;
string outfile;
int verbose = 0;

#ifdef HAVE_GETOPT_H

enum option_t {
    OPT_ASPECT = 0x100,
    OPT_CELL_HEIGHT,
    OPT_CELL_WIDTH,
    OPT_FONT,
    OPT_FONT_SIZE,
    OPT_HELP,
    OPT_LINE_WIDTH,
    OPT_OUTPUT,
    OPT_SCALE,
    OPT_PAGESIZE,
    OPT_VERBOSE,
    OPT_VERSION
};

struct option opts[] = {
  {"aspect", no_argument, NULL, OPT_ASPECT},
  {"cell-height", required_argument, NULL, OPT_CELL_HEIGHT},
  {"cell-width", required_argument, NULL, OPT_CELL_WIDTH},
  {"font", required_argument, NULL, OPT_FONT},
  {"font-size", required_argument, NULL, OPT_FONT_SIZE},
  {"help", no_argument, NULL, OPT_HELP},
  {"line-width", required_argument, NULL, OPT_LINE_WIDTH},
  {"output", required_argument, NULL, OPT_OUTPUT},
  {"scale", required_argument, NULL, OPT_SCALE},
  {"pagesize", required_argument, NULL, OPT_PAGESIZE},
  {"verbose", no_argument, NULL, OPT_VERBOSE},
  {"version", no_argument, NULL, OPT_VERSION},
  {0, 0, 0, 0}
};
#endif

static void render_it (timing::gc& gc, int flags,
    		       int width, int height, double scale)
{
  if (flags & FLAG_PAGESIZE)
    render (gc, data, width, height, (flags & FLAG_ASPECT));
  else
    render (gc, data, scale);
}

int main (int argc, char *argv[]) {
  int width, height;
  double scale = 1;
  int flags = 0;

  int k, c;
  while ((c = getopt_long (argc, argv, "ac:f:hl:o:p:vVw:x:", opts, &k)) != -1)
    switch (c) {
    case 'a':
    case OPT_ASPECT:
      flags |= FLAG_ASPECT;
      break;    
    case 'c':
    case OPT_CELL_HEIGHT:
      timing::vCellHt = atoi (optarg);
      break;
    case OPT_FONT:
      timing::vFont = optarg;
      break;
    case 'f':
    case OPT_FONT_SIZE:
      timing::vFontPointsize = atoi (optarg);
      break;    
    case 'h':
    case OPT_HELP:
      usage ();
      exit (1);
      break;
    case 'l':
    case OPT_LINE_WIDTH:
      timing::vLineWidth = atoi (optarg);
      break;    
    case 'o':
    case OPT_OUTPUT:
      outfile = optarg;
      break;
    case 'p':
    case OPT_PAGESIZE:
      flags |= FLAG_PAGESIZE;
      sscanf (optarg, "%dx%d", &width, &height);
      break;
    case 'x':
    case OPT_SCALE:
      flags |= FLAG_SCALE;
      scale = atof (optarg);
      break;
    case 'v':
    case OPT_VERBOSE:
      ++ verbose;
      break;
    case 'V':
    case OPT_VERSION:
      banner ();
      freesoft ();
      exit (0);
       break;
    case 'w':
    case OPT_CELL_WIDTH:
      timing::vCellW = atoi (optarg);
      break;    
    }

  if (optind >= argc) {
    usage ();
    exit (1);
  }

  if ((flags & FLAG_SCALE) && (flags & FLAG_PAGESIZE)) {
    cerr << "The pagesize and scale options are mutually exclusive" << endl;
    exit (2);
  }

  if ((flags & FLAG_SCALE) && scale <= 0) {
    cerr << "Bad scale factor (" << scale << ") given" << endl;
    exit (2);
  }

  if ((flags & FLAG_PAGESIZE) && (width <= 0 || height <= 0)) {
    cerr << "Bad page size given (" << width << " x " << height << ")" << endl;
    exit (2);
  }

  yydebug = 0;
  if (verbose > 1)
    yydebug = 1;

  try {
    for (int i = optind; i < argc; ++ i) {
      yyin = fopen (argv[i], "rt");
      if (yyin == NULL) 
	perror (argv[i]);
      else {
	if (yyparse () != 0)
	  exit (2);
	fclose (yyin);
      }
    }

    data.pad (n);
    if (verbose)
      cout << data;

    if (outfile.empty ())
      return 0;

    if (timing::postscript_gc::has_ps_ext (outfile)) {
      timing::postscript_gc gc;
      render_it (gc, flags, width, height, 1.0);

      gc.print (outfile);
    } else {
      timing::magick_gc gc;
      render_it (gc, flags, width, height, scale);

      Image img (Geometry (gc.width, gc.height), "white");
      gc.draw (img);
      img.write (outfile);
    }
  }
  catch (Magick::Exception &err) {
    cerr << "caught Magick++ exception: " << err.what () << endl;
    return 2;
  }
  catch (timing::exception &err) {
    cerr << "caught timing exception: " << err.what () << endl;
    return 2;
  }

  return 0;
}


void freesoft (void) {
  cout << "This is free software; see the source for copying conditions.  There is NO"  << endl
       << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl;
}

void banner (void) {
  cout << "drawtiming " VERSION << " + postscript patches" << endl
       << "Copyright (c) 2004-2007 by Edward Counce" << endl
       << "Copyright (c) 2006-2007 by Salvador E. Tropea" << endl
       << "Copyright (c) 2008 by Daniel Beer" << endl;
}

void usage (void) {
  banner ();
  cout << "To generate a timing diagram, write a signal description file named," << endl
       << "for example infile.txt, and run the application as shown:" << endl
       << endl
       << "    drawtiming --output outfile.gif  infile.txt" << endl
       << endl
       << "The following options are accepted:" << endl
       << endl
       << "-h" << endl
       << "--help" << endl
       << "    Show this help text." << endl
       << "-o <filename>" << endl
       << "--output <filename>" << endl
       << "    Required to produce an output image. The output format is determined" << endl
       << "    from the filename. For more details on this, consult the ImageMagick" << endl
       << "    documentation and the ImageMagick(1) man page" << endl
       << endl
       << "    In addition to the formats supported by ImageMagick, Postscript " << endl
       << "    output can be generated (this is enabled when the output filename's " << endl
       << "    extension is either \"ps\" or \"eps\")." << endl
       << "-x <float>" << endl
       << "--scale <float>" << endl
       << "    Scales the canvas size on which to render. This option has no effect" << endl
       << "    in Postscript output." << endl
       << "-p <width>x<height>" << endl
       << "--pagesize <width>x<height>" << endl
       << "    Specify the canvas size to render on." << endl
       << "-a" << endl
       << "--aspect" << endl
       << "    Maintain fixed aspect ratio if --pagesize given." << endl
       << "-v" << endl
       << "--verbose" << endl
       << "    Increases the quantity of diagnostic output." << endl
       << "-c" << endl
       << "--cell-height" << endl
       << "    Height of the signal (pixels) [48]." << endl
       << "-w" << endl
       << "--cell-width" << endl
       << "    Width of the time unit (pixels) [64]." << endl
       << "--font <name>" << endl
       << "    Font [Helvetica]" << endl
       << "-f" << endl
       << "--font-size" << endl
       << "    Font size (pt) [25]." << endl
       << "-l" << endl
       << "--line-width" << endl
       << "    Line width (pixels) [3]." << endl
       << endl
       << "Consult the drawtiming(1) man page for details." << endl;
}

