#
# very simple makefile for omron++
# 
# NOTE: you need these:
#         - libSDL devel package version >= 1.2.0                                     
#         - libSDL-gfx devel package version >= 2.0.9   
#


.PHONY : clean
 
CFLAGS = $(shell sdl-config --cflags) -Wall -Winline -O2 -g
LIBS = $(shell sdl-config --libs) -lSDL_gfx


OBJS=main.o ki.o video.o util.o input.o sound.o menu.o config.o lexwrite.o
OUTFILE=omron++
PREFIX=/usr/local/


all: $(OUTFILE) 

$(OUTFILE):	$(OBJS) 	 
	$(CC) $(CFLAGS) $(OBJS) -o $(OUTFILE) $(LIBS)


# clean me up, scotty
clean:
	$(RM) $(OUTFILE) $(OBJS) *~


install: $(OUTFILE) 
	install -d $(PREFIX)/games \
		   $(PREFIX)/share/icons \
		   $(PREFIX)/share/applications
	install -m 644  omron++_32.xpm  $(PREFIX)/share/icons 
	install -m 644  omron++.desktop $(PREFIX)/share/applications
	install -m 755  $(OUTFILE) $(PREFIX)/bin



main.o:	        util.h input.h config.h video.h sound.h menu.h 
ki.o:		util.h config.h ki.h sound.h 
video.o:	util.h config.h video.h
util.o:		util.h 
input.o:	config.h video.h sound.h menu.h input.h         
sound.o:	config.h sound.h
menu.o:		util.h input.h config.h video.h ki.h sound.h menu.h 
config.o:       config.h util.h config.h lexwrite.h
lexwrite.o:	lexwrite.h

