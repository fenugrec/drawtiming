Summary: A command line tool for generating ideal timing diagrams.
Name: drawtiming
Version: 0.3
Release: 1
Copyright: GPL
Group: Productivity/Graphics/CAD
Source: drawtiming-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-root
BuildRequires: ImageMagick-Magick++ ImageMagick-devel ImageMagick gcc-c++ gcc gzip glibc-devel binutils make libstdc++-devel libstdc++ liblcms-devel freetype2-devel libexif XFree86-devel zlib-devel 
Requires: ImageMagick-Magick++
Prefix: /usr/local
Vendor: Edward Counce
URL: http://drawtiming.sourceforge.net
Packager: Edward Counce <ecounce@users.sourceforge.net>

%description
This project provides a command line tool for documenting hardware and
software designs through timing diagrams.  It includes a parser for reading an
intuitive ASCII signal description from an input file, and uses the
ImageMagick Magick++ API for rasterizing and outputting an image of an ideal
timing diagram.  Notation typical of timing diagrams found in Electrical
Engineering, including the signal state transitions with arrows indicating
causal relationships between signals is generated.  It is written in
C++, and has been tested on Linux, FreeBSD and Cygwin.

%prep
%setup -q 

%build
./configure --prefix=%{prefix}
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README
%doc /usr/local/man/man1/drawtiming.1.gz
/usr/local/bin/drawtiming

%changelog
* Wed Sep 22 2004 - ecounce@users.sourceforge.net
- update package to 0.3
* Wed Sep 22 2004 - ecounce@users.sourceforge.net
- initial package based on 0.2
