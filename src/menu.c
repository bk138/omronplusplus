/* 
   menu: game menu and main drawing routines


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
#include <time.h>
#include "SDL.h"
#include "SDL_gfxPrimitives.h"
#include "SDL_rotozoom.h"

#include "menu.h"
#include "video.h"
#include "input.h"
#include "util.h"
#include "ki.h"
#include "sound.h"
#include "cfg.h"

/*
  internal constants
*/
// allows for 9 digits of user input  
#define USRINPUT_SIZE 10   

#define REFRESH 42

#define SND_F_MOVE 600
#define SND_D_MOVE 80
#define SND_F_ENTER 500
#define SND_D_ENTER 100

#ifdef ANDROID
#define HELPTEXT "Simply follow the menu promptings and enter your choices, then watch the whole thing going...\n\n\nThe MENU key on your device enters menus and confirms choices, the BACK key leaves menus. You can navigate through the menus using dpad or trackball." 
#else
#define HELPTEXT "Simply follow the menu promptings and enter your choices, then watch the whole thing going...\n\n\nKEYBOARD SHORTCUTS:\n\n\n  TAKE SCREENSHOT: s\n\n  TOGGLE FULLSCREEN: f\n\n  TOGGLE PAUSE: p\n\n  TOGGLE SOUND: m\n\n  RETREAT MODE: 1, 2, 3 or 4\n\n\nFor 'advanced' usage type 'omron++ -h' to see the possible commandline arguments. (A special one is '-auto', which runs omron++ in some kind of standalone mode, without the need for user input. Instead it randomly chooses the parameters itself. Just give it the percentage of the possible maximum values to adjust performance to your machine. Makes a nice but resourcehungry screensaver ;-)"
#endif

#define CREDITSHEADING "In order of appearance:"
#define CREDITSTEXT "Jan Wirth, for the idea. Thanks, Jan...\n\n\n\nThomas 'Dante' Gaensewig, for his extensive alpha testing...\n\n\n\nBorstel Pasieka, for his patience as well...\n\n\n\nBasti Winkler, for beta testing and suggestions...\n\n\n\nMicha Gehring, for the initial bits of sound code..."

// this is for mn_aura()
#define KILLBIAS 11





/*
  internal functions
*/
static void mn_update(Uint32 delay);
// return 0 if interrupted by ESC or quit
// return 1 if time out or ENTER
static int mn_wait(Uint32 msecs);
static void mn_bullet(Uint32 x, Uint32 y);
static void mn_prompt(Uint32 x, Uint32 y);
static void mn_hovertext(Uint32 x, Uint32 y, const char* text);
static void mn_text(Sint32 x, Sint32 y, const char *text);
// print text aligned to BoundingBox of size w,h at position bx,by
// returns number of printed lines
static int mn_textBB(Sint32 bx, Sint32 by, Uint32 w, Uint32 h, const char *text, Sint32 txtstarty);

static void mn_credits();
static void mn_start();
static void mn_start_choose_autonorm(int nr_pos);
static void mn_start_norm(int nr_pos);
static void mn_quit();
static void mn_help();
static void mn_options();
static void mn_opts_vid();
static void mn_opts_vid_modes();
static void mn_opts_snd();
static void mn_opts_game(); 

// run game session
// returns 1 on completion, 0 on break
static int mn_runSession();
// draws main battle screen, but DOES NOT flip()!
static void mn_draw();
static void mn_aura(const soldier* joe);
static void mn_showDeployment(army *a);
static void mn_showStatus();



/*
  internal global vars
*/
SDL_Surface *backdrop;
char *message;


/*
  other stuff
*/
// these are defined in main.c
extern Sint8 quit;
extern systemcfg *cfg;

// these in video.c
extern SDL_Surface *screen;

// these in ki.c
extern Uint8 nr_armies;
extern army **armies;






void mn_update(Uint32 d)
{
  if(message)
    {
      int i;
      // create a transparent backup of current screen
      SDL_Surface *orig = vid_copySurface(screen);
      SDL_SetAlpha(orig, SDL_SRCALPHA, 120);      

      // create a black transparent surface
      SDL_Surface *black = vid_copySurface(screen);
      SDL_FillRect(black, NULL, SDL_MapRGB(screen->format, 0,0,0)); 
      SDL_SetAlpha(black, SDL_SRCALPHA, 100);

      stringRGBA(black, cfg->screen_x/2 - (strlen(message)*8)/2, cfg->screen_y/2, message, 0, 200, 0, 200);     

      // blit in
      for(i=0; i<5; ++i)
	{
	  SDL_BlitSurface(black, NULL, screen, NULL);
	  vid_flip();
	  SDL_Delay(2 * REFRESH);
	}

      // let it settle
      SDL_Delay(700);

      // blit back
      for(i=0; i < 5; ++i)
	{
	  SDL_BlitSurface(orig, NULL, screen, NULL);
	  vid_flip();
	  SDL_Delay(2 * REFRESH);
	}

     
      message = NULL;

      SDL_FreeSurface(black);
      SDL_FreeSurface(orig);
    }


  vid_flip();
  SDL_Delay(d);
}




// return 0 if interrupted by any keypress or quit
// return 1 if time out
int mn_wait(Uint32 msecs)
{
  SDLKey k;

  Sint64 waitmsecs = msecs; // so we can safely go < 0
  while(waitmsecs > 0)
     {
       k = inp_checkInput();
       if(k != SDLK_UNKNOWN || quit)
	 return 0; // break

       waitmsecs -= 100;
       mn_update(100);
     }

  return 1;
}






void mn_main()
{
  Uint8 state = 0;
  
  // array of function pointers 
  void (*action[5])() = { &mn_start, &mn_help, &mn_options, &mn_credits, &mn_quit };

  SDL_EnableKeyRepeat(300,150);

  // get a copy of this nice supernova screen from intro
  backdrop = vid_copySurface(screen);
  SDL_FillRect(backdrop, NULL, SDL_MapRGB(screen->format, 0,0,0)); 
  SDL_Surface *tmp = vid_copySurface(screen);
  SDL_SetAlpha(tmp, SDL_SRCALPHA, 170);
  SDL_BlitSurface(tmp, NULL, backdrop, NULL);
   
  while (!quit)
    {
    
      // blit backdrop on a black screen...
      Uint32 d = SDL_GetTicks(); // this takes a lot of time ...
      SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0)); 
      SDL_BlitSurface(backdrop, NULL, screen, NULL);
      d = SDL_GetTicks() - d ;
      
      mn_bullet(cfg->screen_x/2 -35, (cfg->screen_y/10)*(3+state));

      mn_text(cfg->screen_x/2 -24, cfg->screen_y/10 * 3, "START");
      mn_text(cfg->screen_x/2 -24, cfg->screen_y/10 * 4, "HELP");
      mn_text(cfg->screen_x/2 -24, cfg->screen_y/10 * 5, "OPTIONS");
      mn_text(cfg->screen_x/2 -24, cfg->screen_y/10 * 6, "CREDITS");
      mn_text(cfg->screen_x/2 -24, cfg->screen_y/10 * 7, "QUIT");

    
      if(d > REFRESH) // compensate for the delay here
	mn_update(0);
      else
	mn_update(REFRESH - d);


      switch(inp_checkInput())
	{
	case SDLK_UP:
	  snd_beep(SND_F_MOVE,SND_D_MOVE, 1);
	  state = state == 0 ? 4 : state-1;
	  break;
	case SDLK_DOWN:
	  snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
	  state = state == 4 ? 0 : state+1;
	  break;    
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
	  snd_beep(SND_F_ENTER, SND_D_ENTER, 1);
	  action[state]();
	  break;    
#ifdef ANDROID
	case SDLK_ESCAPE:
	  snd_beep(SND_F_ENTER, SND_D_ENTER, 1);
	  mn_quit();
	  break;
#endif
	default:
	  break;
	}
    }
}




void mn_start()
{
  Uint8 state = 0;
  
  while (!quit)
    {
      // blacken backbuffer
      SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0)); 

      mn_bullet(cfg->screen_x/2 -54, (cfg->screen_y/10)*(3+state));

      mn_text(cfg->screen_x/2 -43, cfg->screen_y/10 * 3, "2 DROPZONES");
      mn_text(cfg->screen_x/2 -43, cfg->screen_y/10 * 4, "4 DROPZONES");
     
      mn_update(REFRESH);

      switch(inp_checkInput())
	{
	case SDLK_UP:
	  snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
	  state = state == 0 ? 1 : state-1;
	  break;
	case SDLK_DOWN:
	  snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
	  state = state == 1 ? 0 : state+1;
	  break;    
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
	  snd_beep(SND_F_ENTER, SND_D_ENTER, 1);
	  switch(state)
	    {
	    case 0:
	      mn_start_choose_autonorm(2);
	      break;
	    case 1:
	      mn_start_choose_autonorm(4);
	      break;
	    default:
	      break;
	    }

	  break;     
  	case SDLK_ESCAPE:
	  return;
	  break;
	default:
	  break;
	}
    }   
}





void mn_start_choose_autonorm(int nr_pos)
{
  Uint8 state = 0;
  
  // array of function pointers 
  void (*action[2])() = { &mn_start_norm, &mn_start_auto };

  while(!quit)
    {
      // blacken backbuffer
      SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0)); 

      mn_bullet(cfg->screen_x/2 -52, (cfg->screen_y/10)*(3+state));

      mn_text(cfg->screen_x/2 -41, cfg->screen_y/10 * 3, "NORMAL MODE");
      mn_text(cfg->screen_x/2 -35, cfg->screen_y/10 * 4, "AUTO MODE");
     
      mn_update(REFRESH);

      switch(inp_checkInput())
	{
	case SDLK_UP:
	  snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
	  state = state == 0 ? 1 : state-1;
	  break;
	case SDLK_DOWN:
	  snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
	  state = state == 1 ? 0 : state+1;
	  break;    
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
	  snd_beep(SND_F_ENTER, SND_D_ENTER, 1);
	  action[state](nr_pos);
	  break;     
  	case SDLK_ESCAPE:
	  return;
	  break;
	default:
	  break;
	}
    }   
}





void mn_start_norm(int nr_pos)
{
  int i;
  SDLKey k;
 

  /*
    first, get allegiances (i.e colours) for every army

  */

  Sint8 state = 0;
  int done = 0;
  Sint8 a = MAX_NR_ARMIES;
  Sint8 army_at_pos[4] = {MAX_NR_ARMIES,MAX_NR_ARMIES,MAX_NR_ARMIES,MAX_NR_ARMIES};
  SDL_Color army_color;
 
  // one case for 2 dropzones, one for 4
  if(nr_pos == 2)
    {
      while (!quit && !done)
	{
	  // blacken backbuffer
	  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0)); 

	  switch(k = inp_checkInput())
	    {
	    case SDLK_UP:
	      if(state > 1)
		{
		  snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
		  state -= 2;
		}
	      break ;
	    case SDLK_DOWN:
	      if(state < 2)
		{
		  snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
		  state += 2;
		}
	      break;   
	    case SDLK_RIGHT:
	      if(state == 0)
		{
		  snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
		  state = 1;
		}
	      break;    
	    case SDLK_LEFT:
	      if(state == 1)
		{
		  snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
		  state = 0;
		}
	      break;   
	    case SDLK_RETURN:
	    case SDLK_KP_ENTER:
	      snd_beep(SND_F_ENTER, SND_D_ENTER, 1);
	      switch(state)
		{
		case 0:
		case 1:
		  if(army_at_pos[state] < MAX_NR_ARMIES)
		    ++army_at_pos[state];
		  else
		    army_at_pos[state] = 0;
		  break;
		case 2:
		case 3:
		  if(army_at_pos[0] != MAX_NR_ARMIES && 
		     army_at_pos[1] != MAX_NR_ARMIES && 
		     army_at_pos[0] != army_at_pos[1])
		    done = 1;
		  else
		    mn_msg("CHOOSE TWO DIFFERENT ARMIES!");
		default:
		  break;
		}	
	      break;     
	    case SDLK_ESCAPE:
	      return;
	      break;
	    default:
	      break;
	    } 

	
	  army_color = ki_getColorFromID(army_at_pos[0]);
	  boxRGBA(screen, cfg->screen_x*0.15, cfg->screen_y*0.3, cfg->screen_x*0.35, cfg->screen_y*0.55,
		  army_color.r, army_color.g, army_color.b, army_color.unused);
	  mn_text(cfg->screen_x * 0.25 - 8*strlen(ki_getNameFromID(army_at_pos[0]))/2, cfg->screen_y* 0.6,
		   ki_getNameFromID(army_at_pos[0]));


	  army_color = ki_getColorFromID(army_at_pos[1]);
	  boxRGBA(screen, cfg->screen_x*0.65, cfg->screen_y*0.3, cfg->screen_x*0.85, cfg->screen_y*0.55, 
		  army_color.r, army_color.g, army_color.b, army_color.unused);
	  mn_text(cfg->screen_x * 0.75 - 8*strlen(ki_getNameFromID(army_at_pos[1]))/2, cfg->screen_y* 0.6,
		   ki_getNameFromID(army_at_pos[1]));


	  mn_text(cfg->screen_x * 0.5 -8 , cfg->screen_y* 0.75, "OK");



	  // now the cursor
	  i = strlen(ki_getNameFromID(army_at_pos[state])); // get correct name length
	  switch(state)
	    {
	    case 0:
	      mn_bullet(cfg->screen_x * 0.25 - 8*i/2 - 8, cfg->screen_y * 0.6);
	      break;
	    case 1:
	      mn_bullet(cfg->screen_x * 0.75 - 8*i/2 - 8, cfg->screen_y * 0.6);
	      break;
	    case 2:
	    case 3:
	      mn_bullet(cfg->screen_x * 0.5 - 16, cfg->screen_y * 0.75);
	      break;
	    default:
	      break;
	    }
	 
     	  mn_update(REFRESH);

	} // while() end
    }
  
  else if (nr_pos == 4)
    {
      while (!quit && !done)
	{
	  // blacken backbuffer
	  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0)); 

	  switch(k = inp_checkInput())
	    {
	    case SDLK_UP:
	      if(state > 1)
		{
		  snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
		  state -= 2;
		}
	      break ;
	    case SDLK_DOWN:
	      if(state < 4)
		{
		  snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
		  state += 2;
		}
	      break;   
	    case SDLK_RIGHT:
	      if(state%2 == 0 && state < 4)
		{
		  snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
		  state += 1;
		}
	      break;    
	    case SDLK_LEFT:
	      if(state%2 == 1 && state < 4)
		{
		  snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
		  state -= 1;
		}
	      break;   
	    case SDLK_RETURN:
	    case SDLK_KP_ENTER:
	      snd_beep(SND_F_ENTER, SND_D_ENTER, 1);
	      switch(state)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		  if(army_at_pos[state] < MAX_NR_ARMIES)
		    ++army_at_pos[state];
		  else
		    army_at_pos[state] = 0;
		  break;
		case 4:
		case 5:
		  for(i=0; i < 4; ++i)
		    a = army_at_pos[i] != MAX_NR_ARMIES ?  army_at_pos[i] : a;
		  for(i=0; i < 4; ++i)
		    if(army_at_pos[i] != MAX_NR_ARMIES && army_at_pos[i] != a)
		      {
			done = 1;
			break;
		      }
		  // if not done at end of loop
		  if(!done)
		    mn_msg("CHOOSE AT LEAST TWO DIFFERENT ARMIES!");	

		  break;
		default:
		  break;
		}	
	      break;     
	    case SDLK_ESCAPE:
	      return;
	      break;
	    default:
	      break;
	    } 



	  army_color = ki_getColorFromID(army_at_pos[0]);
	  boxRGBA(screen, cfg->screen_x*0.175, cfg->screen_y*0.15, cfg->screen_x*0.375, cfg->screen_y*0.4,
		  army_color.r, army_color.g, army_color.b, army_color.unused);
	  mn_text(cfg->screen_x * 0.275 - 8*strlen(ki_getNameFromID(army_at_pos[0]))/2, cfg->screen_y* 0.425, 
		   ki_getNameFromID(army_at_pos[0]));



	  army_color = ki_getColorFromID(army_at_pos[1]);
	  boxRGBA(screen, cfg->screen_x*0.625, cfg->screen_y*0.15, cfg->screen_x*0.825, cfg->screen_y*0.4, 
		  army_color.r, army_color.g, army_color.b, army_color.unused);
	  mn_text(cfg->screen_x * 0.725 - 8*strlen(ki_getNameFromID(army_at_pos[1]))/2, cfg->screen_y* 0.425,
		   ki_getNameFromID(army_at_pos[1]));



	  army_color = ki_getColorFromID(army_at_pos[2]);
	  boxRGBA(screen, cfg->screen_x*0.175, cfg->screen_y*0.525, cfg->screen_x*0.375, cfg->screen_y*0.775,
		  army_color.r, army_color.g, army_color.b, army_color.unused);
	  mn_text(cfg->screen_x * 0.275 - 8*strlen(ki_getNameFromID(army_at_pos[2]))/2, cfg->screen_y* 0.8, 
		   ki_getNameFromID(army_at_pos[2]));



	  army_color = ki_getColorFromID(army_at_pos[3]);
	  boxRGBA(screen, cfg->screen_x*0.625, cfg->screen_y*0.525, cfg->screen_x*0.825, cfg->screen_y*0.775, 
		  army_color.r, army_color.g, army_color.b, army_color.unused);
	  mn_text(cfg->screen_x * 0.725 - 8*strlen(ki_getNameFromID(army_at_pos[3]))/2, cfg->screen_y* 0.8, 
		   ki_getNameFromID(army_at_pos[3]));



	  mn_text(cfg->screen_x * 0.5 -8 , cfg->screen_y* 0.9, "OK");



	  // now the cursor
	  i = strlen(ki_getNameFromID(army_at_pos[state])); // get correct name length
	  switch(state)
	    {
	    case 0:
	      mn_bullet(cfg->screen_x * 0.275 - 8*i/2 - 8, cfg->screen_y* 0.425); 
	      break;
	    case 1:
	      mn_bullet(cfg->screen_x * 0.725 - 8*i/2 - 8, cfg->screen_y* 0.425); 
	      break;
	    case 2:
	      mn_bullet(cfg->screen_x * 0.275 - 8*i/2 - 8, cfg->screen_y* 0.8); 
	      break; 
	    case 3:
	      mn_bullet(cfg->screen_x * 0.725 - 8*i/2 - 8, cfg->screen_y* 0.8); 
	      break;
	    case 4:
	    case 5:
	      mn_bullet(cfg->screen_x * 0.5 - 16, cfg->screen_y * 0.9);
	      break;
	    default:
	      break;
	    }

	  
	  mn_update(REFRESH);
	  
	} // while() end
    }
  else // neither 2 or 4 drop zones
    return;


  
  /*
    ok, we got the army ids for the start positions now,
    so lets configure the armies!
  */

  
  armycfg *ac = ki_preInit(nr_pos, army_at_pos);
  armycfg *c = ac; // c is just for counting through
  char count_s[USRINPUT_SIZE] = "";
  char dist_s[USRINPUT_SIZE] = "";
  char vrange_s[USRINPUT_SIZE] = "";
  char mbsz_s[USRINPUT_SIZE] = ""; 
  char tmpstr[BIG_STRSIZE] = "";
  

  // this is a cfg just for suggesting values
  armycfg *ac_sugg = ki_preInit(nr_pos, army_at_pos);
  ki_autoArmyCfg(0, 0, 1); // reset
  for(i=0; i < nr_armies; ++i)
    ki_autoArmyCfg(&ac_sugg[i], cfg->autonom, 0);
  armycfg *c_sugg = ac_sugg; // tmp ptr again


  // now the user input
  Uint32 stamp = 0;     
  for(i=0; i < nr_armies; ++i)
    {
      Sint8 army_configured = 0;
      state = 0;

      while (!quit && !army_configured)
	{
	  // get keypress every while() run
	  k = inp_checkInput();

	  if(k == SDLK_ESCAPE)
	    {
	      free(ac);
	      free(ac_sugg);
	      return;
	    }
          
	  // blacken backbuffer
	  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0)); 

	  
	  switch(state)
	    {
	    case 5: // accept or not
	      mn_text(cfg->screen_x*0.2,cfg->screen_y*0.7, "accept these orders (y/n)?");
	      switch(k) 
		{
		case SDLK_y: 
#ifdef WIN32
		case SDLK_z: // is this SDL's fault ?!? i'd suggest the usual suspects...
#endif
		  *count_s = *dist_s = *vrange_s = *mbsz_s = '\0';

		  snd_beep(SND_F_MOVE, SND_D_MOVE, 0);
		  snd_beep(SND_F_MOVE, SND_D_MOVE, 0);
		  snd_beep(SND_F_ENTER, SND_D_ENTER, 1);
		  army_configured = 1;
		  continue; // to while() head
		  break;

		case SDLK_n: 
		  *count_s = *dist_s = *vrange_s = *mbsz_s = '\0';
		  state = 0;
		  continue; // to while() head
		  break;
		default:
		  break;
		} 

	    case 4: // mobsize input
	      mn_text(cfg->screen_x*0.1,cfg->screen_y*0.6,"mobsize:");
	      mn_text((cfg->screen_x*0.1) + 10*8, cfg->screen_y*0.6, mbsz_s);
	      if(state == 4)
		{
		  snprintf(tmpstr, SMALL_STRSIZE, "WHAT ABOUT %i?", c_sugg->mbsz);
		  mn_hovertext(cfg->screen_x*0.1,cfg->screen_y*0.8, tmpstr);	

		  // text input
		  mn_prompt(cfg->screen_x*0.1 + 9*8,cfg->screen_y*0.6);
		  inp_textIn(mbsz_s, USRINPUT_SIZE, k);
		  if(strlen(mbsz_s))
		    c->mbsz = atoi(mbsz_s);

		  // arrow keys increase/decrease value
		  if(k == SDLK_UP || k == SDLK_RIGHT)
		    c->mbsz++;
		  if(k == SDLK_DOWN || k == SDLK_LEFT)
		    c->mbsz = c->mbsz == 0 ? 0 : c->mbsz-1;
		  snprintf(mbsz_s, USRINPUT_SIZE, "%d", c->mbsz);

		  if(k == SDLK_RETURN || k == SDLK_KP_ENTER)
		    {
		      if(strlen(mbsz_s))
			c->mbsz = atoi(mbsz_s);
		      else
			c->mbsz = c_sugg->mbsz;

		      if(c->mbsz < 1)
			c->mbsz = 1;

		      snd_beep(SND_F_MOVE, SND_D_MOVE, 0);
		      snd_beep(SND_F_MOVE, SND_D_MOVE, 0);
		      snd_beep(SND_F_ENTER, SND_D_ENTER, 1);
		      state = 5;
		    }
		}

	    case 31: // "vrange too far" error state
	      if(state == 31)
		{
		  mn_text(cfg->screen_x*0.2,cfg->screen_y*0.6,"VISION TOO FAR !");
		  if(stamp == 0)                                                                
		    stamp = SDL_GetTicks();   
		  if(SDL_GetTicks() - stamp > 2000)
		    {
		      *vrange_s = '\0';
		      state = 3;
		    }
		}
	      
	    case 3: // vrange input
	      mn_text(cfg->screen_x*0.1,cfg->screen_y*0.5,"range of vision:");
	      mn_text((cfg->screen_x*0.1) + 18*8, cfg->screen_y*0.5, vrange_s);
	      if(state == 3)
		{
		  snprintf(tmpstr, SMALL_STRSIZE, "TRY %i! ", c_sugg->vrange);
		  mn_hovertext(cfg->screen_x*0.1,cfg->screen_y*0.8, tmpstr);

		  // text input
		  mn_prompt(cfg->screen_x*0.1 + 17*8,cfg->screen_y*0.5);
		  inp_textIn(vrange_s, USRINPUT_SIZE, k);
		  if(strlen(vrange_s))
		    c->vrange = atoi(vrange_s);

		  // arrow keys increase/decrease value
		  if(k == SDLK_UP || k == SDLK_RIGHT)
		    c->vrange++;
		  if(k == SDLK_DOWN || k == SDLK_LEFT)
		    c->vrange = c->vrange == 0 ? 0 : c->vrange-1;
		  snprintf(vrange_s, USRINPUT_SIZE, "%d", c->vrange);
		  
		  if(k == SDLK_RETURN || k == SDLK_KP_ENTER)
		    {
		      if(strlen(vrange_s))
			c->vrange = atoi(vrange_s);
		      else
			c->vrange = c_sugg->vrange;

		      if(c->vrange < 1)
			c->vrange = 1;
		      //  try vrange
		      if(ki_testArmyCfg(c))
			{
			  snd_beep(SND_F_MOVE, SND_D_MOVE, 0);
			  snd_beep(SND_F_MOVE, SND_D_MOVE, 0);
			  snd_beep(SND_F_ENTER, SND_D_ENTER, 1);
			  state = 4;
			}
		      else
			{
			  stamp = 0; // zero timestamp for state 31
			  state = 31;
			  continue; // to while() head
			}
		    }
		}

	    case 21: // "does not fit" error state
	      if(state == 21)
		{
		  mn_text(cfg->screen_x*0.2,cfg->screen_y*0.5,"THIS WON'T FIT !");
		  if(stamp == 0)                                                                
		    stamp = SDL_GetTicks();   
		  if(SDL_GetTicks() - stamp > 2000)
		    {
		      *count_s = *dist_s = '\0';
		      state = 1;
		    }
		}

	    case 2: // dist input
	      mn_text(cfg->screen_x*0.1,cfg->screen_y*0.4,"deployment distance:");
	      mn_text(cfg->screen_x*0.1 + 22*8, cfg->screen_y*0.4, dist_s); 
	      if(state == 2)
		{
		  snprintf(tmpstr, SMALL_STRSIZE, "WHY NOT %i? ", c_sugg->dist);
		  mn_hovertext(cfg->screen_x*0.1,cfg->screen_y*0.8, tmpstr);

		  // text input
		  mn_prompt(cfg->screen_x*0.1 + 21*8,cfg->screen_y*0.4);
		  inp_textIn(dist_s, USRINPUT_SIZE, k);
		  if(strlen(dist_s))
		    c->dist = atoi(dist_s);

		  // arrow keys increase/decrease value
		  if(k == SDLK_UP || k == SDLK_RIGHT)
		    c->dist++;
		  if(k == SDLK_DOWN || k == SDLK_LEFT)
		    c->dist = c->dist == 0 ? 0 : c->dist-1;
		  snprintf(dist_s, USRINPUT_SIZE, "%d", c->dist);

		  if(k == SDLK_RETURN || k == SDLK_KP_ENTER)
		    {
		      if(strlen(dist_s))
			c->dist = atoi(dist_s);
		      else
			c->dist = c_sugg->dist;
		   
		      // try count and dist
		      if(ki_testArmyCfg(c))
			{
			  snd_beep(SND_F_MOVE, SND_D_MOVE, 0);
			  snd_beep(SND_F_MOVE, SND_D_MOVE, 0);
			  snd_beep(SND_F_ENTER, SND_D_ENTER, 1);
			  state = 3;
			}
		      else
			{
			  stamp = 0; // zero timestamp for state 21
			  state = 21;
			  continue; // to while() head
			}


		    }
		}
	 
	    case 1: // number input
	      mn_text(cfg->screen_x*0.1,cfg->screen_y*0.3,"number of troops:");
	      mn_text(cfg->screen_x*0.1 + 19*8, cfg->screen_y*0.3, count_s); 
	      if(state == 1)
		{
		  snprintf(tmpstr, SMALL_STRSIZE, "MAYBE %i, COMMANDER? ", c_sugg->count);
		  mn_hovertext(cfg->screen_x*0.1,cfg->screen_y*0.8, tmpstr);

		  // text input
		  mn_prompt(cfg->screen_x*0.1 + 18*8, cfg->screen_y*0.3);
		  inp_textIn(count_s, USRINPUT_SIZE, k);
		  if(strlen(count_s))
		    c->count = atoi(count_s);

		  // arrow keys increase/decrease value
		  if(k == SDLK_UP || k == SDLK_RIGHT)
		    c->count++;
		  if(k == SDLK_DOWN || k == SDLK_LEFT)
		    c->count = c->count == 0 ? 0 : c->count-1;
		  snprintf(count_s, USRINPUT_SIZE, "%d", c->count);

		  if(k == SDLK_RETURN || k == SDLK_KP_ENTER)
		    {
		      if(strlen(count_s))
			c->count = atoi(count_s);
		      else
			c->count = c_sugg->count;
		   		      
		      snd_beep(SND_F_MOVE, SND_D_MOVE, 0);
		      snd_beep(SND_F_MOVE, SND_D_MOVE, 0);
		      snd_beep(SND_F_ENTER, SND_D_ENTER, 1);		      
		      state = 2;
		    }
		}

	    case 0: // heading
	      mn_text(cfg->screen_x*0.05, cfg->screen_y*0.05, "TROOP DEPLOYMENT OF");
	      snprintf(tmpstr, SMALL_STRSIZE, "%s ARMY - %s", c->pos_str, ki_getNameFromID(c->id));
	      mn_text(cfg->screen_x*0.05, cfg->screen_y*0.1, tmpstr);

	      if(state == 0)
		{
		  mn_hovertext(cfg->screen_x*0.1,cfg->screen_y*0.8, "WELCOME!");	
		  if(stamp == 0)                                                                
		    stamp = SDL_GetTicks();   
		  if(SDL_GetTicks() - stamp > 1000)
		    state = 1;
		}
	    }

	  mn_update(REFRESH);

	} // while() end  
      
      ++c;
      ++c_sugg;

    } // for() end


  if(quit) // could be we quit in dialogue 
    return; 

  // init ki subsystem with the now filled armycfg
  ki_init(ac);

  free(ac);
  free(ac_sugg);  // this as well

  // and go!
  int complete = mn_runSession();

  
  Sint8 goahead = 0;
  while(complete && !quit && !goahead)
    {
      SDL_Delay(20);
      switch(inp_checkInput(200))
	{
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
	  goahead = 1;
	  break;
	default:
	  break;
	}
    }
  
  // cleanup before leaving
  ki_shutdown();
}





void mn_start_auto(int nr_pos)
{
  char tmpstr[BIG_STRSIZE];

  Sint8 army_at_pos[4];
  
   
  while(1)
    {
      /*
	first, choose army colors
      */

      if(nr_pos == 2)
	{
	  army_at_pos[0] = ut_lRand(MAX_NR_ARMIES);
	  while((army_at_pos[1] = ut_lRand(MAX_NR_ARMIES)) == army_at_pos[0]);
	}
      else
	if(nr_pos == 4)
	  {
	    Sint8 done = 0;
	    while(!done)
	      {
		// fill em, also allow none
		army_at_pos[0] = ut_lRand(MAX_NR_ARMIES+1);
		army_at_pos[1] = ut_lRand(MAX_NR_ARMIES+1);
		army_at_pos[2] = ut_lRand(MAX_NR_ARMIES+1);
		army_at_pos[3] = ut_lRand(MAX_NR_ARMIES+1);

		int i;
		for(i = 0; i < 4; ++i)
		  if(army_at_pos[i] < MAX_NR_ARMIES)
		    break; // i is the first position where there is an army
		while(i<3)
		  {
		    ++i;
		    if(army_at_pos[i] < MAX_NR_ARMIES)
		      done = 1;
		  }
	      }
	  }
	else 
	  return; //FAIL 

      /*
	then, prepare configs
      */
      armycfg *ac = ki_preInit(nr_pos, army_at_pos);

      vid_cls();
      mn_text(cfg->screen_x/2-40, cfg->screen_y/2, "CALCULATING");
      vid_flip();

      // automagically fill the armycfg
      int i;
      ki_autoArmyCfg(0, 0, 1); // reset
      for(i=0; i < nr_armies; ++i)
	ki_autoArmyCfg(&ac[i], cfg->autonom, 0);

      if(nr_pos == 4)
	{
	  vid_cls();
	  snprintf(tmpstr, BIG_STRSIZE, "%i ARMIES FIGHT FOR WORLD DOMINATION!", nr_armies);
	  mn_text(cfg->screen_x/2 - 8*strlen(tmpstr)/2, cfg->screen_y/2, tmpstr);
	  mn_wait(3000);
	}


      /*
	then, init ki subsystem with the now filled cfg
      */
      ki_init(ac);
      free(ac);


      // and go!
      int complete = mn_runSession();

      // cleanup 
      ki_shutdown();

      if(complete && !quit)
	{
	  mn_wait(3000); // wait 3 secs to see who won

	  Sint8 i=5;
	  while(i > 0 && !quit)
	    {
	      if(inp_checkInput() == SDLK_ESCAPE)
		return;

	      if(i>1)
		snprintf(tmpstr, BIG_STRSIZE, "CONTINUING IN %i SECONDS.", i);
	      else
		{
		  snprintf(tmpstr, BIG_STRSIZE, "CONTINUING IN %i SECOND..", i);
		  vid_cls();
		}
	      mn_msg(tmpstr);
	      mn_wait(1000);
	      --i;
	    }
	}
      else
	return;
    }
}





void mn_help()
{
  int y = 0;
  
  SDL_EnableKeyRepeat(200,50);

  while (!quit)
    {
      // blacken backbuffer
      SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0)); 

      int ln =  mn_textBB(cfg->screen_x * 0.1, cfg->screen_y * 0.1 , cfg->screen_x *0.8, cfg->screen_y * 0.8, HELPTEXT, y);

      // wrap around
      if(y > 1.1 * cfg->screen_y)
	y = -8 * ln;
      if(y < -8 * ln)
	y = 1.1 * cfg->screen_y;

      mn_update(REFRESH);

      switch(inp_checkInput())
	{
	case SDLK_UP:
	  y-=8;
	  break;
	case SDLK_DOWN:
	  y+=8;
	  break;    
	
	case SDLK_ESCAPE:
	  SDL_EnableKeyRepeat(300,150);
	  return;
	  break;
	  
	default:
	  break;
	}
    }
}




void mn_options()
{
  Uint8 state = 0;
  
  // array of function pointers 
  void (*action[3])() = { &mn_opts_vid, &mn_opts_snd, &mn_opts_game };

  while (!quit)
    {
      // blacken backbuffer
      SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0)); 

      mn_bullet(cfg->screen_x/2 -35, (cfg->screen_y/10)*(3+state));

      mn_text(cfg->screen_x/2 -24, cfg->screen_y/10 * 3, "VIDEO");
      mn_text(cfg->screen_x/2 -24, cfg->screen_y/10 * 4, "SOUND");
      mn_text(cfg->screen_x/2 -24, cfg->screen_y/10 * 5, "GAME");
     
      mn_update(REFRESH);

      switch(inp_checkInput())
	{
	case SDLK_UP:
	  snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
	  state = state == 0 ? 2 : state-1;
	  break;
	case SDLK_DOWN:
	  snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
	  state = state == 2 ? 0 : state+1;
	  break;    
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
	  snd_beep(SND_F_ENTER, SND_D_ENTER, 1);
	  action[state]();
	  break;     
	case SDLK_ESCAPE:
	  return;
	  break;
	default:
	  break;
	}
    }
}



void mn_opts_vid()
{
  Uint8 state = 0;
 
  char onoff[BIG_STRSIZE];
 
  while (!quit)
    {
      Uint8 last_state = 0;
      // blacken backbuffer
      SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0)); 

      mn_bullet(cfg->screen_x/2 -65, (cfg->screen_y/10)*(3+state));

      snprintf(onoff, BIG_STRSIZE, "STATUS BAR: %s", cfg->statusbar ? "ON":"OFF");
      mn_text(cfg->screen_x/2 -54, cfg->screen_y/10 * 3, onoff);
     
#ifndef ANDROID
      ++last_state;
      mn_text(cfg->screen_x/2 -54, cfg->screen_y/10 * 4, "VIDEO MODES...");

      ++last_state;
      snprintf(onoff, BIG_STRSIZE, "FULLSCREEN: %s", screen->flags & SDL_FULLSCREEN ? "ON":"OFF");
      mn_text(cfg->screen_x/2 -54, cfg->screen_y/10 * 5, onoff);
#endif

#ifdef HAVE_OPENGL
      ++last_state;
      snprintf(onoff, BIG_STRSIZE, "OPENGL MODE: %s", cfg->opengl ? "ON":"OFF");
      mn_text(cfg->screen_x/2 -54, cfg->screen_y/10 * 6, onoff);
#endif
     
      mn_update(REFRESH);


      switch(inp_checkInput())
	{
	case SDLK_UP:
	  snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
	  state = state == 0 ? last_state : state-1;
	  break;
	case SDLK_DOWN:
	  snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
	  state = state == last_state ? 0 : state+1;
	  break;    
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
	  snd_beep(SND_F_ENTER, SND_D_ENTER, 1);
	  switch(state)
	    {
	    case 0:
	      cfg->statusbar = !cfg->statusbar;
	      break;
	    case 1:
	      mn_opts_vid_modes();
	      break;
	    case 2:
	      vid_toggleFullscreen();
	      break;
	    case 3:
	      cfg->opengl = !cfg->opengl;
	      vid_init();	    
	      break;
	    default:
	      break;
	    }
	  break;     
	case SDLK_ESCAPE:
	  return;
	  break;
	default:
	  break;
	}
    }
}



void mn_opts_vid_modes()
{
  Uint8 state = 0;
  Sint32 x_orig = cfg->screen_x, y_orig = cfg->screen_y;

  while (!quit)
    {
      // blacken backbuffer
      SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0)); 

      mn_bullet(cfg->screen_x/2 -32-11, (cfg->screen_y/10)*(3+state));

      mn_text(cfg->screen_x/2 -32, cfg->screen_y/10 * 3, "400x300");
      mn_text(cfg->screen_x/2 -32, cfg->screen_y/10 * 4, "512x384");
      mn_text(cfg->screen_x/2 -32, cfg->screen_y/10 * 5, "640x480");
      mn_text(cfg->screen_x/2 -32, cfg->screen_y/10 * 6, "800x600");
      mn_text(cfg->screen_x/2 -36, cfg->screen_y/10 * 7, "1024x768");

      mn_update(REFRESH);


      switch(inp_checkInput())
	{
	case SDLK_UP:
	  snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
	  state = state == 0 ? 4 : state-1;
	  break;
	case SDLK_DOWN:
	  snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
	  state = state == 4 ? 0 : state+1;
	  break;    
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
	  snd_beep(SND_F_ENTER, SND_D_ENTER, 1);
	  switch(state)
	    {
	    case 0:
	      vid_setMode(400,300);
	      break;
	    case 1:
	      vid_setMode(512,384);
	      break;
	    case 2:
	      vid_setMode(640,480);
	      break; 
	    case 3:
	      vid_setMode(800,600);
	      break; 
	    case 4:
	      vid_setMode(1024,768);
	      break; 
	    default:
	      break;
	    }
	  break;     
	case SDLK_ESCAPE:
	  backdrop = zoomSurface(backdrop, (double)cfg->screen_x/x_orig,(double)cfg->screen_y/y_orig, 0);
	  return;
	  break;
	default:
	  break;
	}
    }

}




void mn_opts_snd()
{
  char onoff[BIG_STRSIZE];
 
  while(!quit)
    {
      // blacken backbuffer
      SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0)); 

      mn_bullet(cfg->screen_x/2 -49, (cfg->screen_y/10)*3);
      snprintf(onoff, BIG_STRSIZE, "SOUND: %s", cfg->sound ? "ON":"OFF");
      mn_text(cfg->screen_x/2 -38, cfg->screen_y/10 * 3, onoff);
     
      mn_update(REFRESH);

      switch(inp_checkInput())
	{
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
	  snd_beep(SND_F_ENTER, SND_D_ENTER, 1);
	  snd_toggle();
	  break;     
	case SDLK_ESCAPE:
	  return;
	  break;
	default:
	  break;
	}
    } 
    
}




void mn_opts_game()
{
  Uint8 state = 0;
  char usrinput[5];
  char onoff[BIG_STRSIZE];
  SDLKey k;
  float in;
  int locked = 0;
 
  snprintf(usrinput, 5, "%.1f", cfg->autonom);
  if(usrinput[3] == '.')
    usrinput[3] = '\0';

  while (!quit)
    {
      // blacken backbuffer
      SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0)); 

     
      mn_text(cfg->screen_x/2 -84, cfg->screen_y/10 * 3, "AUTO PERCENTAGE: ");
      mn_text(cfg->screen_x/2 + 60, cfg->screen_y/10 * 3, usrinput); 

      snprintf(onoff, BIG_STRSIZE, "MULTI THREADING: %s", cfg->smp ? "ON":"OFF");
      mn_text(cfg->screen_x/2 -84, cfg->screen_y/10 * 4, onoff);

      snprintf(onoff, BIG_STRSIZE, "BENCHMARKING: %s", cfg->benchmark ? "ON":"OFF");
      mn_text(cfg->screen_x/2 -84, cfg->screen_y/10 * 5, onoff);
  
      if(!locked)
	mn_bullet(cfg->screen_x/2 -84-11, (cfg->screen_y/10)*(3+state));

      mn_update(REFRESH);


      switch(k = inp_checkInput())
	{
	case SDLK_UP:
	  if(!locked)
	    {
	      snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
	      state = state == 0 ? 2 : state-1;
	    }
	  break;
	case SDLK_DOWN:
	  if(!locked)
	    {
	      snd_beep(SND_F_MOVE, SND_D_MOVE, 1);
	      state = state == 2 ? 0 : state+1;
	    }
	  break;    
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
	  switch(state)
	    {
	    case 0:
	      if(locked)
		{
		  in = strtod(usrinput, NULL);    
		  if(in <= 0 || in > 100) // crap entered
		    usrinput[0] = '\0';
		  else
		    {
		      cfg->autonom = in;
		      locked = 0;
		    }
		}
	      else
		{
		  snd_beep(SND_F_ENTER, SND_D_ENTER, 1);
		  locked = 1; 
		  usrinput[0] = '\0';
		  break;
		}
	      break;
	    case 1:
	      snd_beep(SND_F_ENTER, SND_D_ENTER, 1);
	      cfg->smp = !cfg->smp;
	      break;
	    case 2:
	      snd_beep(SND_F_ENTER, SND_D_ENTER, 1);
	      cfg->benchmark = !cfg->benchmark;
	      break;
	    default:
	      break;
	    }
	  break;     
	case SDLK_ESCAPE:
	  return;
	  break;
	default:
	  if(state == 0)
	    if(locked)
	      {
		mn_prompt(cfg->screen_x/2 + 52, cfg->screen_y/10 * 3);
		inp_textIn(usrinput,4, k); 	  
	      }
	  break;
	}
    }
}





void mn_credits()
{
  Sint32 y = cfg->screen_y + 20;

  SDL_EnableKeyRepeat(100,50);

  while (!quit)
    {
      // blacken backbuffer
      SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0)); 
      int ln = mn_textBB(cfg->screen_x*0.01, cfg->screen_y*0.01, cfg->screen_x*0.95 , cfg->screen_y*0.95, CREDITSHEADING,y);
      ln += mn_textBB(cfg->screen_x*0.10, cfg->screen_y*0.01+40, cfg->screen_x*0.8 , cfg->screen_y*0.8, CREDITSTEXT,y);

      // wrap around
      if(y > 1.1 * cfg->screen_y)
	y = -8 * ln;
      if(y < -8 * ln)
	y = 1.1 * cfg->screen_y;

      mn_update(REFRESH);

      //autoscroll
      --y;


      switch(inp_checkInput())
	{
	case SDLK_UP:
	  y-=8;
	  break;
	case SDLK_DOWN:
	  y+=8;
	  break;    
	
	case SDLK_ESCAPE:
	  SDL_EnableKeyRepeat(300,150);
	  return;
	  break;
	  
	default:
	  break;
	}
    }
}




void mn_quit()
{
  quit = 1;
}




// run game session
// returns 1 on completion, 0 on break
int mn_runSession()
{
  Uint8 first, i, j, pause=0;
  char tmpstr[SMALL_STRSIZE];  


  // start the action !
  // i.e. let the dice decide who may begin
  first = ut_lRand(nr_armies);

  vid_cls();
  snprintf(tmpstr, SMALL_STRSIZE, "%s BEGINS", armies[first]->name);
  mn_text(cfg->screen_x/2-40, cfg->screen_y/2, tmpstr);
  mn_wait(1000);
  vid_cls(); 


  i = 0;
  j = first;
  while(i < nr_armies)  //start with first, then draw others
    {
      mn_showDeployment(armies[j]);

      if(j == nr_armies-1)
	j = 0; //wrap around
      else 
	++j;

      ++i;
    }


  mn_wait(3000);
 

  // the main battle loop
  while(!quit)
    {
      switch(inp_checkInput())
	{
	case SDLK_p:
	case SDLK_PAUSE:
	  if((pause = !pause))
	    mn_msg("PAUSED");
	  break;
	case SDLK_ESCAPE:
	  return 0;
	  break;
	default:
	  break;
	  ;
	}


      if(!pause)
	{
	  Uint32 t1 = SDL_GetTicks();

	  i = 0;
	  j = first;
	  while(i < nr_armies)
	    {
	      ki_makeMove(armies[j]);
	      
	      if(j == nr_armies-1)
		j = 0; //wrap around
	      else 
		++j;

	      ++i;
	    }
	  
	  // show the moves
	  mn_draw(); 
	  mn_update(0);

	  if(cfg->benchmark)
	    {
	      Uint32 t2 = SDL_GetTicks();
	      printf("Moving all armies took %i ms.\n", t2-t1);
	    }

	  
	  // only one allegiance left?
	  Sint8 winner = FREE;
	  Sint8 only_one_left = 1;
	  for(i = 0; i < nr_armies; ++i)
	    if(armies[i]->count != armies[i]->nr_dead) // find one who is alive
	      {
		winner = armies[i]->id;
		break;
	      }
	  for(i = 0; i < nr_armies; ++i) // another one who is alive?
	    if(armies[i]->count != armies[i]->nr_dead && armies[i]->id != winner) 
	      {
		only_one_left = 0;
		break;
	      }
	  
	  if(only_one_left)
	    {
	      snprintf(tmpstr, SMALL_STRSIZE, "%s WINS!", ki_getNameFromID(winner));
	      mn_text(cfg->screen_x/2- 8*strlen(tmpstr)/2, cfg->screen_y/2, tmpstr);
	      mn_update(0);
	      return 1;
	    }
	}
      // we are paused, so don't eat up all cpu
      else
	mn_update(REFRESH);
    }

  return 0;
}








void mn_bullet(Uint32 x, Uint32 y)
{
  static Uint8 c = 0;
  static Uint8 rad = 0;
  static Uint8 r = 255;
  static Uint8 b = 255;


  // increase radius every x steps
  if(c % 2 == 0)
    {
      if(rad == 4)
	rad = 0;
      else
	++rad;
    }


  // only change colours every x steps
  if(c % 3 == 0)
    {
      r = ut_lRand(255);
      b = ut_lRand(255);
    }

  ++c;
 
  filledCircleRGBA(screen, x, y+3, rad, r, 200, b, 150);
}




void mn_hovertext(Uint32 x, Uint32 y, const char* text)
{
  static Uint8 c = 0;
  static Uint8 alpha = 200;
  static Sint8 d = 20;

  if(c % 3 == 0) 
    {
      if(alpha >= 200 || alpha <= 100)
	d = -d;
      alpha += d;
    }
    
  stringRGBA(screen, x, y, text, 0, 200, 0, alpha);

  ++c;
}



void mn_text(Sint32 x, Sint32 y, const char *text)
{
  stringRGBA(screen, x, y, text, 0, 200, 0, 200);
}





// print text aligned to BoundingBox of size w,h at position bx,by
int mn_textBB(Sint32 bx, Sint32 by, Uint32 w, Uint32 h, const char *text, Sint32 txtstarty)
{
  Sint32 y = by + txtstarty;
  Uint32 wordlen = 0, linelen = 0, lineno = 0;
  const Uint32 maxlinelen = w / 8;
  
  char linebuf[maxlinelen + 1];
  
  Sint8 nl = 0;
  Sint8 done = 0;

  char *p;
  char *wordstart = (char*)text;
  char *linestart = (char*)text;
  char *txtpos = (char*)text;

  /*
    SDL_Rect bb = {bx, by, w, h};
    SDL_FillRect(screen, &bb, SDL_MapRGB(screen->format, 0,0,200));
  */

  while(!done)
    {
      /*
	handle newline,space and null
      */
      switch(*txtpos)  
	{
	case '\n':
	  nl = 1;    // this is handled later on (we have to flush the linebuffer...)
	  wordstart = ++txtpos;
	  break;
	case '\0':
	  done = 1;  // set done, but flush linebuffer...
	  break;
	case ' ':
	  ++linelen;
	  wordstart = ++txtpos;
	  continue;
	default:
	  break;
	}
    
      /*
	handle other chars, i.e. words
      */

      // we need to know the word length beforehand
      p = wordstart;
      while(*p != ' ' && *p != '\n' && *p != '\0')
	++p;
      wordlen = p - wordstart;
      
    
      if(!nl && !done && linelen + wordlen <= maxlinelen) // word still fits in this line
	{
	  linelen += wordlen;
	  txtpos += wordlen;
	  continue; // at txtpos
	}
      else   // we have to print sth
	{
	  
	  if(!nl && wordlen > maxlinelen) // special case: word too long for one single line
	    {
	      if(linelen == 0) // linebuffer empty
		{
		  linelen = maxlinelen; 
		  txtpos += linelen;
		}
	    }

	  if(y >= by && y+8 <= by+h)
	    {
	      strncpy(linebuf, linestart, linelen);
	      linebuf[linelen] = '\0';
	      mn_text(bx, y, linebuf);
	    }

	  nl = 0; // just in case nl was set
	  y+=8;   // LF
	  ++lineno; 
	  linelen = 0;
	  linestart = wordstart = txtpos;
	}
    }
  return lineno;
}





void mn_prompt(Uint32 x, Uint32 y)
{
  static Uint8 c = 0;

  if(c < 5)
    mn_text(x, y, ">");

  if(c > 10)
    c = 0;

  ++c;
}




void mn_intro()
{
  Uint8 i;
  Uint32 r;

  // get the rng going !
  srand(time(NULL));

 
  /*
    black hole sun
  */
  for(i=1; i< 11; ++i)
    {
      // clear backbuffer
      SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0)); 

      for (r=1; r < (cfg->screen_y*0.6); ++r)
	{
	  if (r > 50) r+=10;
	  if (r > 120) r+=5;
	  filledCircleRGBA(screen, (cfg->screen_x/2), (cfg->screen_y/2), r, 255, ut_lRand(200), ut_lRand(255), 20);
	  filledCircleRGBA(screen, (cfg->screen_x/2), (cfg->screen_y/2), r-5, 255, ut_lRand(200), ut_lRand(255), 20);
	  vid_flip();
	  if(inp_checkInput() != SDLK_UNKNOWN || quit) // some key pressed
	    return;
	}
      
    }

  /*
    logo
  */
  // O 
  aalineRGBA(screen, cfg->screen_x*0.05, cfg->screen_y*0.4, cfg->screen_x*0.15, cfg->screen_y*0.4, 0, 200, 0, 200);
  aalineRGBA(screen, cfg->screen_x*0.15, cfg->screen_y*0.4, cfg->screen_x*0.15, cfg->screen_y*0.6, 0, 200, 0, 200);
  aalineRGBA(screen, cfg->screen_x*0.15, cfg->screen_y*0.6, cfg->screen_x*0.05, cfg->screen_y*0.6, 0, 200, 0, 200);
  aalineRGBA(screen, cfg->screen_x*0.05, cfg->screen_y*0.6, cfg->screen_x*0.05, cfg->screen_y*0.4, 0, 200, 0, 200);
  
  vid_flip();
  if(! mn_wait(666))
    return;

  // M
  aalineRGBA(screen, cfg->screen_x*0.2, cfg->screen_y*0.6, cfg->screen_x*0.2, cfg->screen_y*0.4, 0, 200, 0, 200);
  aalineRGBA(screen, cfg->screen_x*0.2, cfg->screen_y*0.4, cfg->screen_x*0.25, cfg->screen_y*0.5, 0, 200, 0, 200);
  aalineRGBA(screen, cfg->screen_x*0.25, cfg->screen_y*0.5, cfg->screen_x*0.3, cfg->screen_y*0.4, 0, 200, 0, 200);
  aalineRGBA(screen, cfg->screen_x*0.3, cfg->screen_y*0.4, cfg->screen_x*0.3, cfg->screen_y*0.6, 0, 200, 0, 200);

  vid_flip();
  if(! mn_wait(666))
    return;

  
  // R
  aalineRGBA(screen, cfg->screen_x*0.35, cfg->screen_y*0.6, cfg->screen_x*0.35, cfg->screen_y*0.4, 0, 200, 0, 200);
  aalineRGBA(screen, cfg->screen_x*0.35, cfg->screen_y*0.4, cfg->screen_x*0.45, cfg->screen_y*0.45, 0, 200, 0, 200);
  aalineRGBA(screen, cfg->screen_x*0.45, cfg->screen_y*0.45, cfg->screen_x*0.35, cfg->screen_y*0.5, 0, 200, 0, 200);
  aalineRGBA(screen, cfg->screen_x*0.35, cfg->screen_y*0.5, cfg->screen_x*0.45, cfg->screen_y*0.6, 0, 200, 0, 200);
 
  vid_flip();
  if(! mn_wait(666))
    return;

  // O
  aalineRGBA(screen, cfg->screen_x*0.5, cfg->screen_y*0.4, cfg->screen_x*0.6, cfg->screen_y*0.4, 0, 200, 0, 200);
  aalineRGBA(screen, cfg->screen_x*0.6, cfg->screen_y*0.4, cfg->screen_x*0.6, cfg->screen_y*0.6, 0, 200, 0, 200);
  aalineRGBA(screen, cfg->screen_x*0.6, cfg->screen_y*0.6, cfg->screen_x*0.5, cfg->screen_y*0.6, 0, 200, 0, 200);
  aalineRGBA(screen, cfg->screen_x*0.5, cfg->screen_y*0.6, cfg->screen_x*0.5, cfg->screen_y*0.4, 0, 200, 0, 200);

  vid_flip();
  if(! mn_wait(666))
    return;


  // N
  aalineRGBA(screen, cfg->screen_x*0.65, cfg->screen_y*0.6, cfg->screen_x*0.65, cfg->screen_y*0.4, 0, 200, 0, 200);
  aalineRGBA(screen, cfg->screen_x*0.65, cfg->screen_y*0.4, cfg->screen_x*0.75, cfg->screen_y*0.6, 0, 200, 0, 200);
  aalineRGBA(screen, cfg->screen_x*0.75, cfg->screen_y*0.6, cfg->screen_x*0.75, cfg->screen_y*0.4, 0, 200, 0, 200);

  vid_flip();
  if(! mn_wait(666))
    return;

  snd_jingle(1);
  
  aalineRGBA(screen, cfg->screen_x*0.785, cfg->screen_y*0.5, cfg->screen_x*0.865, cfg->screen_y*0.5, 0, 200, 0, 200);
  aalineRGBA(screen, cfg->screen_x*0.825, cfg->screen_y*0.55, cfg->screen_x*0.825, cfg->screen_y*0.45, 0, 200, 0, 200);
  aalineRGBA(screen, cfg->screen_x*0.885, cfg->screen_y*0.5, cfg->screen_x*0.965, cfg->screen_y*0.5, 0, 200, 0, 200);
  aalineRGBA(screen, cfg->screen_x*0.925, cfg->screen_y*0.55, cfg->screen_x*0.925, cfg->screen_y*0.45, 0, 200, 0, 200);

  vid_flip();

  // finally, wait a little bit more
  if(! mn_wait(666+2000))
    return;

}




void mn_msg(const char* txt)
{
  message = (char*) txt;
}






void mn_showDeployment(army *a)
{
  SDLKey keypress; 
  Sint8 slowly = 1;
  soldier *s = a->troops;
  char status[BIG_STRSIZE];
  int drawn;
  float percent;

  // where to draw statusbar
  SDL_Rect sb = { 3, (cfg->screen_y - cfg->sb_height) + (a->index * SB_ONEROW), cfg->screen_x, 8};
 
  int i;
  for(i = 0; i < a->count; ++i)
    {
      if(quit)
	return;

      if (! s->dead)
	{
	  pixelRGBA(screen, s->x, s->y, a->color.r, a->color.g, a->color.b, 255); 
	 	  
	  if(slowly)
	    {
	      if(cfg->sb_height)
		{ // show status bar with drawn percent
		  drawn = i+1;
		  percent = a->count > 0 ? ((float) drawn / a->count) * 100 : 0;

		  snprintf(status, BIG_STRSIZE, "%i. %s%s%i%s%5.2f%s",
			   a->index + 1, a->name, ": ", drawn, " (", percent,"%)");
		  
		  SDL_FillRect(screen, &sb, SDL_MapRGB(screen->format, 0,0,0)); 
		  stringRGBA(screen, 3, (cfg->screen_y - cfg->sb_height) + (a->index * SB_ONEROW), 
			     status, 0, 200, 0, 200);
 		}
		
	      mn_update(2000 / a->count);

	      keypress = inp_checkInput();
	      if (keypress == SDLK_ESCAPE 
		  || keypress == SDLK_RETURN 
		  || keypress == SDLK_KP_ENTER)
		slowly = 0;
	    }
	}
      ++s;
    }
 
  // at last show drawn screen, just in case we bailed out of slowmo  
  if(cfg->sb_height)
    {
      snprintf(status, BIG_STRSIZE, "%i. %s%s%i (100%%)", a->index+1, a->name, ": ", a->count);

      SDL_FillRect(screen, &sb, SDL_MapRGB(screen->format, 0,0,0)); 
      stringRGBA(screen, 3, (cfg->screen_y - cfg->sb_height) + (a->index * SB_ONEROW), 
		 status, 0, 200, 0, 200);
    }
 
  mn_update(0);
}






// draws main battle screen, but DOES NOT flip()!
static void mn_draw()
{
  Uint8 i;
  Uint32 j;
  soldier *s = NULL;

  // clear backbuffer
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0)); 

  
  for(i=0; i < nr_armies; ++i)
    {
      // s becomes the first soldier of this army
      s = armies[i]->troops;  
      
      for(j = 0; j < armies[i]->count; ++j)
	{
	  if (! s->dead)
	    {
	      pixelRGBA(screen, s->x, s->y, armies[i]->color.r, armies[i]->color.g, armies[i]->color.b, 255); 
	      mn_aura(s);
	    }

	  ++s;
	}
    }

  // if we have a statusbar, then fill it ...
  if(cfg->sb_height) 
    mn_showStatus();
}




static void mn_showStatus()
{
  Uint32 nr_alive;
  float percent;
  static char status[BIG_STRSIZE];

  Uint8 i;
  for(i=0; i < nr_armies; ++i)
    {
      nr_alive = armies[i]->count - armies[i]->nr_dead;
      
      percent = armies[i]->count > 0 ? ((float) nr_alive / armies[i]->count) * 100 : 0;
     
      // construct the status strings
      snprintf(status, BIG_STRSIZE, "%i. %s%s%i%s%5.2f%s%i%s%i", 
	       i+1, armies[i]->name,": ", nr_alive, " (", percent,
	       "%)  VR:", armies[i]->vrange,"  MBSZ:", armies[i]->mbsz);
      
      // and show them (we have a 8x8 font)
      stringRGBA(screen, 3, (cfg->screen_y - cfg->sb_height) + (i * SB_ONEROW) , status, 0, 200, 0, 200);
    }

}





static void mn_aura(const soldier* joe)
{
  static Uint8 maxalpha1 = 120;
  static Uint8 maxalpha2 = 180;

  Uint32 x = joe->x;
  Uint32 y = joe->y;
  Uint32 kills = joe->kills;

  Uint8 alpha1 = kills <= KILLBIAS ? ((float) maxalpha1 / KILLBIAS)*kills : maxalpha1;
  Uint8 alpha2 = kills <= KILLBIAS ? ((float) maxalpha2 / KILLBIAS)*kills : maxalpha2;
 
  // above ...
  // no one there ?
  if (ki_whatsThere(x,y-1)->army_index == FREE)
    {
      pixelRGBA(screen, x, y-1, 255, 255, 255, alpha1);
      // here as well ?
      if (ki_whatsThere(x, y-2)->army_index == FREE)
	pixelRGBA(screen, x, y-2, 255, 255, 255, alpha2);
    }

  // right ...
  if (ki_whatsThere(x+1,y)->army_index == FREE)
    {
      pixelRGBA(screen, x+1, y, 255, 255, 255, alpha1);
      if (ki_whatsThere(x+2, y)->army_index == FREE)
	pixelRGBA(screen, x+2, y, 255, 255, 255, alpha2);
    }

  // bottom ...
  if (ki_whatsThere(x,y+1)->army_index == FREE)
    {
      pixelRGBA(screen, x, y+1, 255, 255, 255, alpha1);
      if (ki_whatsThere(x, y+2)->army_index == FREE)
	pixelRGBA(screen, x, y+2, 255, 255, 255, alpha2);
    }

  // left ...
  if (ki_whatsThere(x-1,y)->army_index == FREE)
    {
      pixelRGBA(screen, x-1, y, 255, 255, 255, alpha1);
      if (ki_whatsThere(x-2, y)->army_index == FREE)
	pixelRGBA(screen, x-2, y, 255, 255, 255, alpha2);
    }
}




