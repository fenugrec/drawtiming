BUILT_SOURCES = parser.h
EXTRA_DIST = parser.h
AM_CXXFLAGS = @MAGICK_CPPFLAGS@ @MAGICK_CXXFLAGS@ -DYYDEBUG=1
AM_YFLAGS = -d
LDADD = @MAGICK_LDFLAGS@ @MAGICK_LIBS@

bin_PROGRAMS = drawtiming
drawtiming_SOURCES = main.cc globals.h parser.yy scanner.ll timing.cc timing.h
parser.h: parser.cc
