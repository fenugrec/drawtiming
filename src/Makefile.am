AM_CXXFLAGS = @cairo_CFLAGS@ -DYYDEBUG=1
AM_YFLAGS = -d

bin_PROGRAMS = drawtiming
drawtiming_SOURCES = main.cc globals.h parser.yy scanner.ll timing.cc postscript_gc.cc cairo_gc.cc timing.h gc.h cairo_gc.h
drawtiming_LDADD = @cairo_LIBS@

EXTRA_DIST = parser.h
BUILT_SOURCES = parser.h
parser.h: parser.cc
