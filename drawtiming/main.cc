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
// along with Foobar; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "globals.h"
#include <getopt.h>
using namespace std;
using namespace Magick;

extern FILE *yyin;
extern int yydebug;
int yyparse (void);

unsigned n;
timing::data data;
timing::siglist deps;
double scale = 1.0;
string outfile = "/dev/null";
int verbose = 0;

struct option opts[] = {
  {"output", required_argument, NULL, 'o'},
  {"scale", required_argument, NULL, 'x'},
  {"verbose", no_argument, NULL, 'v'},
  {0, 0, 0, 0}
};

int main (int argc, char *argv[]) {
  int k, c;
  while ((c = getopt_long (argc, argv, "o:vx:", opts, &k)) != -1)
    switch (c) {
    case 'o':
      outfile = optarg;
      break;
    case 'x':
      scale = atof (optarg);
      break;
    case 'v':
      ++ verbose;
      break;
    }

  if (optind >= argc)
    return 0;

  yydebug = 0;
  if (verbose > 1)
    yydebug = 1;

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

  timing::diagram diagram;
  diagram.push_back (DrawablePushGraphicContext ());
  diagram.push_back (DrawableScaling (scale, scale));
  diagram.render (data);
  diagram.push_back (DrawablePopGraphicContext ());

  Image img (Geometry ((int)(scale*diagram.width), (int)(scale*diagram.height)), "white");
  img.draw (diagram);
  img.write (outfile);

  return 0;
}
