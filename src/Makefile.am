## Process this file with automake to produce Makefile.in


AM_CFLAGS =\
	 -Wall \
	 -g \
	 $(SDL_CFLAGS)

bin_PROGRAMS = omron++

omron___SOURCES = \
	cfg.c input.c  ki.c  lexwrite.c  main.c  menu.h  sound.h  util.h   video.h \
	cfg.h input.h  ki.h  lexwrite.h          menu.c  sound.c  util.c   video.c

if MINGW
omron___SOURCES += omron++.rc omron++.ico
.rc.o:
	 $(WINDRES) -o $@ $<
endif

omron___LDFLAGS = 

omron___LDADD = \
	$(SDL_LIBS)



