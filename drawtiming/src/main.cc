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

#include "globals.h"
#ifdef HAVE_GETOPT_H
#  include <getopt.h>
#else
#  include <unistd.h>
#  define getopt_long(C,V,S,O,I) getopt(C,V,S)
#endif
using namespace std;
using namespace Magick;

extern FILE *yyin;
extern int yydebug;
int yyparse (void);
static void usage (void);

unsigned n;
timing::data data;
timing::signal_sequence deps;
timing::diagram diagram;
string outfile;
int verbose = 0;

#ifdef HAVE_GETOPT_H
struct option opts[] = {
  {"help", no_argument, NULL, 'h'},
  {"output", required_argument, NULL, 'o'},
  {"scale", required_argument, NULL, 'x'},
  {"verbose", no_argument, NULL, 'v'},
  {0, 0, 0, 0}
};
#endif

int main (int argc, char *argv[]) {

  int k, c;
  while ((c = getopt_long (argc, argv, "ho:vx:", opts, &k)) != -1)
    switch (c) {
    case 'h':
      usage ();
      exit (1);
      break;
    case 'o':
      outfile = optarg;
      break;
    case 'x':
      diagram.scale = atof (optarg);
      break;
    case 'v':
      ++ verbose;
      break;
    }

  if (optind >= argc) {
    usage ();
    exit (1);
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

    diagram.render (data);

    Image img (Geometry (diagram.width, diagram.height), "white");
    img.draw (diagram);
    img.write (outfile);
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


void usage (void) {
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
       << "-o filename" << endl
       << "--output filename" << endl
       << "    Required to produce an output image. The output format is determined" << endl
       << "    from the filename. For more details on this, consult the ImageMagick" << endl
       << "    documentation and the ImageMagick(1) man page" << endl
       << "-x float" << endl
       << "--scale float" << endl
       << "    Scales the canvas size on which to render." << endl
       << "-v" << endl
       << "--verbose" << endl
       << "    Increases the quantity of diagnostic output." << endl
       << endl
       << "Consult the drawtiming(1) man page for details." << endl;
}
