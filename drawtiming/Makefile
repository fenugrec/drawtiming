# Copyright (c)2004 by Edward Counce, All rights reserved.
# This file is part of drawtiming.
#
# Drawtiming is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Drawtiming is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with drawtiming; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

CXXFLAGS := -Wall -g -MMD $(shell Magick++-config --cppflags --cxxflags) 
LDFLAGS := $(shell Magick++-config --ldflags --libs)

all: drawtiming
clean:
	rm -f *.o *.d lex.yy.c infile.tab.c infile.tab.h

drawtiming: lex.yy.o infile.tab.o main.o timing.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

lex.yy.o: lex.yy.c
	$(CXX) $(CXXFLAGS) -c $<
lex.yy.c: infile.lex infile.tab.h
	flex $<

infile.tab.o: infile.tab.c
	$(CXX) $(CXXFLAGS) -DYYDEBUG=1 -c $<
infile.tab.h infile.tab.c: infile.y
	bison -d -o infile.tab.c $<

sinclude *.d
