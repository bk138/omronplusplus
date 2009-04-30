/* 
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


#include <string.h>
#include "SDL.h"

#include "input.h"
#include "video.h"
#include "sound.h"
#include "menu.h"
#include "config.h"



/*
  external variables
*/

// this is in main.c
extern Sint8 quit;
extern systemcfg *cfg;

// this in ki.c
extern char do_return[4];


/*
  internal variables
*/
// input "buffer" and marker
SDLKey input;
SDL_mutex *input_lock;

/*
  internal functions:
*/
static int inp_filterEvents(const SDL_Event *event);
static int inp_handleKeyboard();





void inp_init()
{
  // setup event filter
  SDL_SetEventFilter(inp_filterEvents);

  input_lock = SDL_CreateMutex();

  // and start keyboard handler thread
  SDL_CreateThread(inp_handleKeyboard, NULL);
}





int inp_filterEvents(const SDL_Event *event)
{
  switch (event->type) 
    {
      // This is important!  Queue it if we want to quit. 
    case SDL_QUIT: // not handled by keyb handler, this can be the window close button
      quit = 1;
      printf("\nbye!\n");
      return 1;

      // keyboard events are queued as well
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      return 1;

      // drop all other events 
    default:
      return 0;
    }
}



/*
  get key events, act on them and save them in global input var
*/
int inp_handleKeyboard()
{
  SDL_Event events[20];
  int i, found;

  while(!quit) 
    {
      // we're only interested in key events
      found = SDL_PeepEvents(events, 20, SDL_GETEVENT, SDL_KEYDOWNMASK|SDL_KEYUPMASK);

      for(i=0; i<found; ++i) 
	{
	  if(events[i].key.type == SDL_KEYDOWN)
	    {
	      switch( events[i].key.keysym.sym ) 
		{
		case SDLK_m: 
		  if(snd_toggle())
		    mn_msg("SOUND ON");
		  else
		    mn_msg("SOUND OFF");
		  break;
		case SDLK_q:
		  quit = 1;
		  printf("\nbye!\n");
		  break;
		case SDLK_s: 
		  vid_takeScreenshot();
		  mn_msg("SCREENSHOT TAKEN");
		  break;
		case SDLK_1: 
		case SDLK_KP1: 
		  do_return[0] = 1;
		  break;
		case SDLK_2: 
		case SDLK_KP2: 
		  do_return[1] = 1;
		  break;
		case SDLK_3: 
		case SDLK_KP3:
		  do_return[2] = 1; 
		  break;
		case SDLK_4: 
		case SDLK_KP4:
		  do_return[3] = 1; 
		  break;
		  
		default:
		  break;
		} 
	  	     
	      // always save id of pressed key	  
	      SDL_LockMutex(input_lock);    // lock out inp_checkInput()
	      input = events[i].key.keysym.sym;
	      SDL_UnlockMutex(input_lock);
	    }

	  if(events[i].key.type == SDL_KEYUP)
	    {
	      switch( events[i].key.keysym.sym ) 
		{
		case SDLK_1: 
		case SDLK_KP1: 
		  do_return[0] = 0; 
		  break;
		case SDLK_2: 
		case SDLK_KP2: 
		  do_return[1] = 0; 
		  break;
		case SDLK_3: 
		case SDLK_KP3: 
		  do_return[2] = 0; 
		  break;
		case SDLK_4: 
		case SDLK_KP4: 
		  do_return[3] = 0; 
		  break;
		  
		default:
		  break;		} 

	    }
	}
  
      // Give up some CPU to allow events to arrive 
      SDL_Delay(10);

    }
  
  return 0;
}



/*
  read key from global input var
*/
SDLKey inp_checkInput()
{
  SDLKey retval; 

#ifndef unix
  // with win32 there's no extra 'event gather'-thread
  // and - more importantly - it _must_ gather events from the _main_ thread!
  SDL_PumpEvents();  
#endif
  
  // lock out keyb thread
  SDL_LockMutex(input_lock);

  retval = input; // get input
  if(input != SDLK_UNKNOWN) // if it was something usable
    input = SDLK_UNKNOWN;   // indicate that we used it!

  // functions that can't be called by the keyb thread
  if(retval == SDLK_f)        // win32 and opengl mode do not suppport switching
    vid_toggleFullscreen();   // video modes from another thread


  SDL_UnlockMutex(input_lock);

 
  return retval;
}





/*
  append char corresponding to k to string
  by now reads only numbers ...
*/
int inp_textIn(char *string, int limit, SDLKey k)
{
  --limit;

  switch(k) 
    {
    case SDLK_0:
    case SDLK_KP0:
      if (strlen(string) < limit)
	strcat(string,"0");
      break;
    case SDLK_1:
    case SDLK_KP1:
      if (strlen(string) < limit)
	strcat(string,"1");
      break;
    case SDLK_2:
    case SDLK_KP2:
      if (strlen(string) < limit)
	strcat(string,"2");
      break;
    case SDLK_3:
    case SDLK_KP3:
      if (strlen(string) < limit)
	strcat(string,"3");
      break;
    case SDLK_4:
    case SDLK_KP4:
      if (strlen(string) < limit)
	strcat(string,"4");
      break;
    case SDLK_5:
    case SDLK_KP5:
      if (strlen(string) < limit)
	strcat(string,"5");
      break;
    case SDLK_6:
    case SDLK_KP6:
      if (strlen(string) < limit)
	strcat(string,"6");
      break;
    case SDLK_7:
    case SDLK_KP7:
      if (strlen(string) < limit)
	strcat(string,"7");
      break;
    case SDLK_8:
    case SDLK_KP8:
      if (strlen(string) < limit)
	strcat(string,"8");
      break;
    case SDLK_9:
    case SDLK_KP9:
      if (strlen(string) < limit)
	strcat(string,"9");
      break;
  
    case SDLK_BACKSPACE:
      string[strlen(string)-1] = '\0';
      break;

    default:
      break;
    } 
 
  return strlen(string);
} 



