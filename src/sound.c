/* 
   This file is part of omron++, the can-do-anything visual apparaton.
 
   Copyright (C) 2006 Christian Beier <dontmind@freeshell.org>
 
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
#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include "SDL.h"
#include "SDL_thread.h"

#include "cfg.h"
#include "util.h"


/*
  internal constants
*/
#define AMPL 25000

/*
  internal functions
*/
static void snd_callback(void* userdata, Uint8* buffer, int len);
static int snd_intern_beep(void *data);
static int snd_intern_jingle();



/*
  internal variables
*/
SDL_AudioSpec des, obt;
float freq=440; //initial frequency of played sound
Sint8 init_done = 0;
Sint8 locked = 0; 


/*
  external variables
*/
// this is in main.c
extern Sint8 quit;
extern systemcfg *cfg;


int snd_init(Sint8 sndinfo)
{
  if(SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
    {
      ut_log( "WARNING: could not initialize sound subsystem:  %s\n", SDL_GetError());
      cfg->sound = 0;
      return 0;
    }
  

  des.freq   = 22050; // samplerate 
  des.format = AUDIO_S16SYS; //signed 16 bit samples
  des.channels = 1;       
  des.samples = 1024;  // buffersize  
  des.callback = snd_callback;
  des.userdata = &freq;
  
  if(SDL_OpenAudio(&des, &obt) < 0)
    {
      ut_log( "WARNING: could not open audio: %s\n", SDL_GetError());
      cfg->sound = 0;
      return 0;
    }
  
  // make sure this is called
  atexit(SDL_CloseAudio);

  init_done = 1;
  return 1;
}




void snd_printInfo()
{
  ut_log("\nSOUND INFORMATION:\n^^^^^^^^^^^^^^^^^\n");
  ut_log("samplerate: %i\n", obt.freq);
  ut_log("channels: %i\n", obt.channels); 
  ut_log("buffer size (samples): %i\n", obt.samples);
  ut_log("buffer size (bytes): %i\n", obt.size);
}





static void snd_callback(void* userdata, Uint8* buffer, int len)
{
  Uint32 i;
  float *freq =  (float*) userdata;
  Sint16* sb = (Sint16*) (buffer);
  static Uint32 x = 0;

  //(re)fill buffer with a sine sound
  for(i = 0; i < len/2; ++i)
    {
      sb[i] = (Sint16) (sin( (M_PI * 2. * x * *freq) / obt.freq) * AMPL);
      ++x;
    }
}




int snd_toggle()
{
  if(init_done) // only change if we have a running subsystem
    cfg->sound = !cfg->sound;
  
  return(cfg->sound);
}




void snd_beep(float f, float d, Sint8 nonblock)
{
  static SDL_Thread* t = NULL;

  static float t_args[2];
  t_args[0] = f;
  t_args[1] = d;

  if(init_done && cfg->sound && !locked && !quit)
    {
      if(nonblock)
	{
	  // we can be sure we don't _really_ have to wait because
	  // thread set locked to 0 
	  SDL_WaitThread(t, NULL); // clean up resources
	  locked = 1;
	  t = SDL_CreateThread(snd_intern_beep, "snd_beep", &t_args);
	}
      else
	snd_intern_beep(&t_args);
    }
}



static int snd_intern_beep(void *data)
{
  float* args = (float*) data;

  Uint32 d = (Uint32) args[1];
  freq = args[0];// tell callback function

  SDL_PauseAudio(0);
  SDL_Delay(d * 0.8);
  SDL_PauseAudio(1);
  SDL_Delay(d * 0.2); // so sounds are distinguishable

  locked = 0;
  return 0;
}




void snd_jingle(Sint8 nonblock)
{
  static SDL_Thread* t = NULL;

  if(init_done && cfg->sound && !locked && !quit)
    {
      if(nonblock)
	{
	  // we can be sure we don't _really_ have to wait because
	  // thread set locked to 0 
	  SDL_WaitThread(t, NULL); // clean up resources
	  locked = 1;
	  t = SDL_CreateThread(snd_intern_jingle, "snd_jingle", NULL);
	}
      else
	snd_intern_jingle();
    }
}



static int snd_intern_jingle()
{
  freq = 440;

  SDL_PauseAudio(0);
  SDL_Delay(400);
  SDL_PauseAudio(1);
  SDL_Delay(20);

  freq = 400;

  SDL_PauseAudio(0);
  SDL_Delay(100);
  SDL_PauseAudio(1);
  SDL_Delay(40);

  SDL_PauseAudio(0);
  SDL_Delay(100);
  SDL_PauseAudio(1);
  SDL_Delay(40);

  SDL_PauseAudio(0);
  SDL_Delay(100);
  SDL_PauseAudio(1);
  SDL_Delay(250);

  freq = 500;

  SDL_PauseAudio(0);
  SDL_Delay(300);
  SDL_PauseAudio(1);
  SDL_Delay(20);


  freq = 600;

  SDL_PauseAudio(0);
  SDL_Delay(900);
  SDL_PauseAudio(1);
  SDL_Delay(50);


  SDL_PauseAudio(1);

  locked = 0;
  return 0;
}


