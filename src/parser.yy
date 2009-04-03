/* Copyright (c)2004 by Edward Counce, All rights reserved. 
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
*/

%{
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include "globals.h"
void yyerror (const char *s);
extern int yylineno;
int yylex (void);
%}

%token SYMBOL STRING CAUSE DELAY

%%

input: 
timeslice
| input timeslice;

timeslice:
'.' { ++ n; }
| statements '.' { deps.clear (); ++ n; }

statements:
statement { $$ = $1; deps.push_back ($1); }
| statements ',' statement { $$ = $3; deps.push_back ($3); }
| statements ';' statement { $$ = $3; deps.clear (); deps.push_back ($3); }
| statements CAUSE statement { $$ = $3; data.add_dependencies ($3, deps); 
    deps.clear (); deps.push_back ($3); }
| statements DELAY statement { $$ = $3; data.add_delay ($3, $1, $2); }

statement:
SYMBOL '=' SYMBOL { $$ = $1; data.set_value ($1, n, timing::sigvalue ($3)); }
| SYMBOL '=' STRING { $$ = $1; data.set_value ($1, n, timing::sigvalue ($3, timing::STATE)); }
| SYMBOL { $$ = $1; };

%%

void yyerror (const char *s) {
  std::cerr << yylineno << ": " << s << std::endl;
}
