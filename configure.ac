dnl Process this file with autoconf to produce a configure script.

AC_INIT(omron++, 1.0rc)


AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE



AC_CANONICAL_HOST
case $host_os in
     darwin*)
	HOST_IS_DARWIN="yes"
	;;
     *mingw32*)
	HOST_IS_WINDOWS="yes"
	;;
     *)
	HOST_IS_DARWIN="no"
	HOST_IS_WINDOWS="no"
	;;
esac



AM_CONDITIONAL(MINGW, [ test "$HOST_IS_WINDOWS" = "yes" ])
AM_CONDITIONAL(DARWIN, [ test "$HOST_IS_DARWIN" = "yes" ])



SDLCONFIG=sdl-config
AC_ARG_WITH(sdl-config,
[[  --with-sdl-config=FILE
    Use the given path to sdl-config when determining
    SDL configuration; defaults to "sdl-config"]],
[
    if test "$withval" != "yes" -a "$withval" != ""; then
        SDLCONFIG=$withval
    fi
])
AC_MSG_CHECKING([SDL version])
if sdlversion=`$SDLCONFIG --version`; then
    AC_MSG_RESULT([$sdlversion])
else
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([SDL is required. Try --with-sdl-config.])
fi

dnl check for universal sdl lib
if test "$HOST_IS_DARWIN" = "yes"; then
   AC_MSG_CHECKING([whether SDL is a universal lib])
   dnl check for both stic and dylib
   LIBSDL="`$SDLCONFIG --prefix`/lib/libSDL.a"
   if test ! -e $LIBSDL; then
      LIBSDL="`$SDLCONFIG --prefix`/lib/libSDL.dylib"
   fi
   if test ! -z "`file $LIBSDL | grep "architecture i386" 2>/dev/null`" \
        -a ! -z "`file $LIBSDL | grep "architecture ppc" 2>/dev/null`"; then
     CC="gcc -arch i386 -arch ppc"
     CPP="cpp"
     AC_MSG_RESULT([yes])
   else
     AC_MSG_RESULT([no])
   fi
fi

dnl we have this here since we may be redifining CC above
AC_ISC_POSIX
AC_HEADER_STDC
AC_PROG_CPP
AC_PROG_CC
AM_PROG_CC_STDC
AM_PROG_CC_C_O
AC_PROG_RANLIB

if test "$HOST_IS_WINDOWS" = "yes"; then
    AC_CHECK_TOOL(WINDRES, windres, windres)
    AC_SUBST(WINDRES)
fi


SDL_CFLAGS="`$SDLCONFIG --cflags`"
SDL_LIBS="`$SDLCONFIG --static-libs`"

AC_SUBST(SDL_CFLAGS)
AC_SUBST(SDL_LIBS)

dnl we prefer the static one
LIBSDLGFX="`$SDLCONFIG --prefix`/lib/libSDL_gfx.a"
if test ! -e $LIBSDLGFX; then
   LIBSDLGFX="-lSDL_gfx"
fi

dnl this is more or less a rude guess, but so what...
dnl we need this if SDL is installed in a nonstandard place (e.g. in case of MinGW)
LDFLAGS+="-L`$SDLCONFIG --prefix`/lib"
AC_CHECK_LIB(SDL_gfx,pixelRGBA,LIBS="$LIBSDLGFX $LIBS",AC_MSG_ERROR([SDL-gfx is required.]), -lSDL)


AC_OUTPUT([
Makefile
src/Makefile
])
