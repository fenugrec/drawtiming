AM_CXXFLAGS = @MAGICK_CPPFLAGS@ @MAGICK_CXXFLAGS@ @CAIROMM_CFLAGS@ -DYYDEBUG=1
AM_YFLAGS = -d

bin_PROGRAMS = drawtiming
drawtiming_SOURCES = main.cc globals.h parser.yy scanner.ll timing.cc timing.h
drawtiming_LDADD = @MAGICK_LIBS@ @CAIROMM_LIBS@
drawtiming_LDFLAGS = @MAGICK_LDFLAGS@

EXTRA_DIST = parser.h
BUILT_SOURCES = parser.h
parser.h: parser.cc
