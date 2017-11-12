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

#include "config.h"

#include "video.h"
#include "util.h"
#include "input.h"
#include "sound.h"
#include "menu.h"
#include "cfg.h"

#ifdef _MSC_VER
// disable the console window that gets started as well (https://stackoverflow.com/a/6882500/361413)
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif


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
	  ut_log("%s\n", VERSION);
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
#ifdef HAVE_OPENGL
			if ( strcmp(argv[argc-1], "-opengl") == 0 ) 
			  {                            
			    cfg->opengl = atoi(argv[argc]);                                 
			    --argc;
			  }      
			else
#endif
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
      ut_log("could not initialize SDL:  %s\n", SDL_GetError());
      exit(EXIT_FAILURE);
    }
  atexit(SDL_Quit);


  // initialize sound subsystem
  snd_init();
  snd_printInfo();

  // initialize video (and event) subsystem
  vid_init();
  vid_printInfo();
  
  // initialize input subsystem
  inp_init();
  inp_printInfo();
  

  /*
    the user supplied valid args, greetings to anyone who made it...
  */

  ut_log("\n:::  omron++ welcomes you!  :::\n\n");
  ut_log("Copyright (C) 2005-2009 Christian Beier <dontmind@freeshell.org>.\n");
  ut_log("omron++ is free software, licensed under the GPL.\n\n");
  ut_log("use 'p' to pause, 's' to take a screenshot, 'f' to toggle fullscreen mode, 'm' to (un)mute sound or 'q' to flee...\n\n");


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
  ut_log("\nUsage: %s [options]\n\navailable options:\n\n", appname);
  ut_log("   -version                  show version and exit\n");
  ut_log("   -width X                  set width\n");
  ut_log("   -height Y                 set height\n");
  ut_log("   -bpp BPP                  set bits-per-pixel\n");
  ut_log("   -statusbar 1|0            enable/disable status bar\n");
  ut_log("   -fullscreen 1|0           enable/disable fullscreen mode\n");
  ut_log("   -opengl 1|0               enable/disable opengl mode\n");
  ut_log("   -sound 1|0                enable/disable sound\n");
  ut_log("   -benchmark 1|0            enable/disable benchmark mode\n");
  ut_log("   -smp 1|0                  enable/disable multithreading\n");
  ut_log("   -auto PERCENT POSITIONS   enable auto mode\n\n");

  exit(EXIT_FAILURE);
}





