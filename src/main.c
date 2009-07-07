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


#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "SDL_thread.h"

#include "video.h"
#include "util.h"
#include "input.h"
#include "sound.h"
#include "menu.h"
#include "config.h"


#define VERSION "1.0.0-pre"




/*
  internal functions:
*/
static void usage(char *appname);



/*
  internal variables
*/

// global marker indicating wish to exit
Sint8 quit;

// global cfg struct ptr
systemcfg *cfg;




int main(int argc, char *argv[])
{
  Sint8 startauto = 0;
  int nr_pos = 2;
  Sint8 vidinfo = 0, sndinfo = 0;


  /*
    load config
  */
  cfg = cfg_load();
  

  /*
    get args ...
  */
  while ( argc > 1 ) 
    {
      --argc;
      if ( strcmp(argv[argc], "-version") == 0 ) 
	{
	  printf("%s\n", VERSION);
	  exit(EXIT_SUCCESS);
	}
      else
	if ( strcmp(argv[argc-1], "-statusbar") == 0 ) 
	  {
	    cfg->statusbar = atoi(argv[argc]);                                 
	    --argc;
	  }
	else 
	  if ( strcmp(argv[argc-1], "-fullscreen") == 0 ) 
	    {                            
		cfg->fullscreen = atoi(argv[argc]);                                 
		--argc;
	    }      
	  else 
	      if ( strcmp(argv[argc], "-vidinfo") == 0 ) 
		vidinfo = 1;
	      else 
		if ( strcmp(argv[argc-1], "-bpp") == 0 ) 
		  {                            
		    cfg->screen_bpp = atoi(argv[argc]);                                 
		    --argc;
		  }      
		else
		  if ( strcmp(argv[argc-1], "-width") == 0 ) 
		    {                            
		      cfg->screen_x = atoi(argv[argc]);
		      if(cfg->screen_x < 200)    
			cfg->screen_x = 200;
		      --argc;
		    }      
		  else
		    if ( strcmp(argv[argc-1], "-height") == 0 ) 
		      {                            
			cfg->screen_y = atoi(argv[argc]);
			if(cfg->screen_y < 200)    
			  cfg->screen_y = 200;
			--argc;
		      }
		    else
		      if ( strcmp(argv[argc-1], "-sound") == 0 ) 
			{                            
			  cfg->sound = atoi(argv[argc]);                                 
			  --argc;
			}      
		      else
			if ( strcmp(argv[argc], "-sndinfo") == 0 ) 
			  sndinfo = 1;
			else
			  if ( strcmp(argv[argc-1], "-benchmark") == 0 ) 
			    {                            
			      cfg->benchmark = atoi(argv[argc]);
			      --argc;
			    }      
			  else
			    if ( strcmp(argv[argc-1], "-smp") == 0 ) 
			      {                            
				cfg->smp = atoi(argv[argc]);                                 
				--argc;
			      }      
			    else
			      if ( strcmp(argv[argc-1], "-opengl") == 0 ) 
				{                            
				  cfg->opengl = atoi(argv[argc]);                                 
				  --argc;
				}      
			      else
				// now for options that take two args
				if(argc > 2)
				  {
				    if(strcmp(argv[argc-2], "-auto") == 0 ) 
				      {                            
					startauto = 1;
					cfg->autonom = strtod(argv[argc-1], NULL);               
					if(cfg->autonom <= 0 || cfg->autonom > 100) // crap entered
					  usage(argv[0]);
				      
					nr_pos = strtod(argv[argc], NULL); 
					if(!(nr_pos == 2 || nr_pos == 4))// crap entered
					  usage(argv[0]);
				      
					argc -= 2;
				      }
				    else
				      usage(argv[0]);
				  }
				else 
				  usage(argv[0]);
    }


  /*
    init ...
  */

  // initialize SDL
  if(SDL_Init(0) < 0) 
    {
      fprintf(stderr, "could not initialize SDL:  %s\n", SDL_GetError());
      exit(EXIT_FAILURE);
    }
  atexit(SDL_Quit);


  // initialize sound subsystem
  snd_init();
  if (sndinfo) 
    snd_printInfo();

  // initialize video (and event) subsystem
  vid_init();
  if (vidinfo) // what did we get ?
    vid_printInfo();
  

  // initialize input subsystem
  inp_init();
  

  /*
    the user supplied valid args, greetings to anyone who made it...
  */

  printf("\n:::  omron++ welcomes you!  :::\n\n");
  printf("Copyright (C) 2005-2009 Christian Beier <dontmind@freeshell.org>.\n");
  printf("omron++ is free software, licensed under the GPL.\n\n");
  printf("use 'p' to pause, 's' to take a screenshot, 'f' to toggle fullscreen mode, 'm' to (un)mute sound or 'q' to flee...\n\n");


  /* 
     and finally start ... 
  */
  
  mn_intro();

  if(startauto)
    {
      mn_start_auto(nr_pos);
      quit = 1; // when we return, we have to tell other subsystems to shut down
    }
  else
    mn_main();

    
  /*
    exit to os ...
  */
  cfg_save(cfg);

  return EXIT_SUCCESS;
}






void usage(char *appname)
{
  printf("\nUsage: %s [options]\n\navailable options:\n\n", appname);
  printf("   -version                  show version and exit\n");
  printf("   -vidinfo                  show video info\n");
  printf("   -sndinfo                  show sound info\n");
  printf("   -width X                  set width\n");
  printf("   -height Y                 set height\n");
  printf("   -bpp BPP                  set bits-per-pixel\n");
  printf("   -statusbar 1|0            enable/disable status bar\n");
  printf("   -fullscreen 1|0           enable/disable fullscreen mode\n");
  printf("   -opengl 1|0               enable/disable opengl mode\n");
  printf("   -sndinfo                  show sound info\n");
  printf("   -sound 1|0                enable/disable sound\n");
  printf("   -benchmark 1|0            enable/disable benchmark mode\n");
  printf("   -smp 1|0                  enable/disable multithreading\n");
  printf("   -auto PERCENT POSITIONS   enable auto mode\n\n");

  exit(EXIT_FAILURE);
}





