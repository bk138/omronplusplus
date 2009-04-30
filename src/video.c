/* 
   video: lowlevel video setup and blitting routines


   This file is part of omron++, the can-do-anything visual apparaton.

   Copyright (C) 2005 Christian Beier <dontmind@freeshell.org>
 
   omron++ is free software; you can redistribute it and/or modify 
   it under the terms of the GNU General Public License as published by 
   the Free Software Foundation; either version 2 of the License, or 
   (at your option) any later version. 
 
   omron++ is distributed in the hope that it will be useful, 
   but WITHOUT ANY WARRANTY; without even the implied warranty of 
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
   GNU General Public License for more details. 
 
   You should have received a copy of the GNU General Public License 
   along with this program; if not, write to the Free Software 
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 
*/


#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "SDL.h"
#include "SDL_gfxPrimitives.h"

#include "util.h"
#include "video.h"
#include "config.h"




/*
  internal variables
*/
SDL_Surface *screen;
//Uint32 videoflags =  SDL_OPENGL;
int videoflags = SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_HWACCEL | SDL_ANYFORMAT;

char drivername[SMALL_STRSIZE]; 


/*
  other stuff
*/
// these are defined in main.c
extern systemcfg *cfg;
extern Sint8 quit;





void vid_init()
{
#ifdef unix    
  if(SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD) < 0)
#else
    if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
#endif
      {
	fprintf(stderr, "ERROR: could not initialize video/event subsystem:  %s\n", SDL_GetError());
	exit(EXIT_FAILURE);
      }

  if(cfg->fullscreen)
   {
     videoflags ^= SDL_FULLSCREEN;  
     SDL_ShowCursor(SDL_DISABLE);
   }

  vid_setMode(cfg->screen_x, cfg->screen_y);

  // get name of video driver 
  SDL_VideoDriverName(drivername, SMALL_STRSIZE);

  // set window title ...
  SDL_WM_SetCaption("omron++", NULL);
}




void vid_setMode(int width, int height)
{

  cfg->screen_x = width;
  cfg->screen_y = height;

  //SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  screen = SDL_SetVideoMode(cfg->screen_x, cfg->screen_y, cfg->screen_bpp, videoflags);
  if (screen == NULL)
    {
      fprintf(stderr, "ERROR: could not open window: %s\n", SDL_GetError());
      exit(EXIT_FAILURE);
    }
}





void vid_flip()
{
  SDL_Flip(screen);
}




void vid_cls()
{
  // blacken screen...
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0)); 
  vid_flip();
}






void vid_printInfo()
{
  const SDL_VideoInfo *vidinfo = SDL_GetVideoInfo();

  printf("\nVIDEO INFORMATION:\n^^^^^^^^^^^^^^^^^\n");
  printf("using video driver: %s\n", drivername);
  printf("current display: %d bits-per-pixel.\n",vidinfo->vfmt->BitsPerPixel);
  printf("\na window manager is %savailable.\n\n", vidinfo->wm_available ? "" : "NOT " ); 
}





SDL_Surface* vid_copySurface(SDL_Surface *src)
{
  // create a surface and copy src in there
  SDL_Surface *s = SDL_CreateRGBSurface(src->flags, src->w, src->h, src->format->BitsPerPixel,
					0, 0, 0, 0);
  SDL_BlitSurface(src, NULL, s, NULL);
  return s;
}




void vid_toggleFullscreen()
{
  //toggle mouse cursor
  if(SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE)
    SDL_ShowCursor(SDL_DISABLE);
  else
    SDL_ShowCursor(SDL_ENABLE);


  // only x11 driver supports SDL's own fs toggle :-(
  if (strcmp(drivername, "x11") == 0 )
    {
      if ( SDL_WM_ToggleFullScreen(screen) == 0)
	fprintf(stderr, "NOTICE: could not toggle fullscreen mode.\n");
      videoflags ^= SDL_FULLSCREEN;
      cfg->fullscreen = !cfg->fullscreen;
    }

  else
    {
      // get a copy of current screen
      SDL_Surface *backup = vid_copySurface(screen);

      // this corrupts the pixeldata in screen ...
      videoflags ^= SDL_FULLSCREEN;
      cfg->fullscreen = !cfg->fullscreen;
      vid_setMode(cfg->screen_x, cfg->screen_y);

      // ... so restore from the backup
      SDL_BlitSurface(backup, NULL, screen, NULL);
      vid_flip();

      SDL_FreeSurface(backup);
    }
}



void vid_takeScreenshot()
{
  // should be enough ...
  char ssfilename[BIG_STRSIZE];
  char date[SMALL_STRSIZE];
  time_t t; time_t *tp = &t; 

  // get time
  time(tp);

  // convert to date-string 
  strftime(date, SMALL_STRSIZE, "%d.%m.%Y-%H.%M.%S", localtime(tp));

  snprintf(ssfilename, BIG_STRSIZE, "omron++%s.bmp", date);

  if (SDL_SaveBMP(screen, ssfilename) < 0) 
    fprintf(stderr, "NOTICE: could not take screenshot.\n");
}



















