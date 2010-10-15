## Process this file with automake to produce Makefile.in


SUBDIRS = \
	src


docdir = $(datadir)/doc/omron++
doc_DATA = \
	README\
	AUTHORS\
	ChangeLog\
	NEWS

Applicationsdir = $(datadir)/applications/
Applications_DATA = src/omron++.desktop

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = src/omron++_32.xpm


EXTRA_DIST = \
	$(doc_DATA) \
	$(Applications_DATA) \
	$(pixmap_DATA)



if DARWIN
bundle_contents = Omron++.app/Contents
appbundle: src/omron++
	mkdir -p $(bundle_contents)/MacOS
	mkdir -p $(bundle_contents)/Resources
	echo "APPL????" > $(bundle_contents)/PkgInfo
	$(INSTALL_PROGRAM) $< $(bundle_contents)/MacOS/
	cp omron++.icns $(bundle_contents)/Resources
	cp README  $(bundle_contents)/Resources/README.TXT
	cp README-SDL $(bundle_contents)/Resources/README-SDL.TXT
	cp NEWS    $(bundle_contents)/Resources
	cp COPYING $(bundle_contents)/Resources
	cp AUTHORS $(bundle_contents)/Resources
	echo \
	"<?xml version="1.0" encoding="UTF-8"?>\
	<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd"\
	<plist version="1.0">\
	<dict>\
	<key>CFBundleDevelopmentRegion</key>\
	<string>English</string>\
	<key>CFBundleExecutable</key>\
	<string>omron++</string>\
	<key>CFBundleIconFile</key>\
	<string>icon.icns</string>\
	<key>CFBundleGetInfoString</key>\
	<string>Omron++ @PACKAGE_VERSION@, © Christian Beier (dontmind@freeshell.org), 2008, 2009\
	\
	... the simplistic peace-endowing warfare simulation.\</string>\
	<key>CFBundleName</key>  \
	<string>Omron++</string> \
	<key>CFBundlePackageType</key> \
	<string>APPL</string>          \
	<key>CFBundleShortVersionString</key> \
	<string>@PACKAGE_VERSION@</string> \
	<key>CFBundleVersion</key> \
	<string>@PACKAGE_VERSION@</string> \
	</dict>   \
	</plist> \
	" >  $(bundle_contents)/Info.plist
endif


if MINGW
nsis_installer: all omron++.nsi README NEWS COPYING
	cat README | unix2dos > README.TXT
	cat NEWS | unix2dos > NEWS.TXT
	cat COPYING | unix2dos > COPYING.TXT
	$(STRIP) src/omron++.exe
	makensis omron++.nsi 
endif
