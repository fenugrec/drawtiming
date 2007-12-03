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
#include "parser.h"
%}

%option yylineno
%option noyywrap
%x COMMENT QUOTE DELAYTEXT
SYM [A-Za-z0-9_]+

%%

<COMMENT>\n     BEGIN(INITIAL); 
<COMMENT>.*     ;

<QUOTE>\"       BEGIN(INITIAL); return STRING;
<QUOTE>\\.      yylval += yytext[1];
<QUOTE>\n       return -1;
<QUOTE>.        yylval += yytext[0];

<DELAYTEXT>>    BEGIN(INITIAL); return DELAY;
<DELAYTEXT>\\.  yylval += yytext[1];
<DELAYTEXT>\n   return -1;
<DELAYTEXT>.    yylval += yytext[0];

{SYM}(\.{SYM})* yylval = std::string (yytext, yyleng); return SYMBOL;
\"              BEGIN(QUOTE); yylval.erase ();
=>              return CAUSE;
-               BEGIN(DELAYTEXT); yylval.erase ();
#               BEGIN(COMMENT);
[\n\t ]+        ;
.               return yytext[0];
