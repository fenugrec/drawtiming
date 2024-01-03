AM_CXXFLAGS = @MAGICKXX_CFLAGS@ -DYYDEBUG=1
AM_YFLAGS = -d

bin_PROGRAMS = drawtiming
drawtiming_SOURCES = main.cc globals.h parser.yy scanner.ll timing.cc timing.h
drawtiming_LDADD = @MAGICKXX_LIBS@

EXTRA_DIST = parser.hh
BUILT_SOURCES = parser.hh
parser.hh: parser.cc
