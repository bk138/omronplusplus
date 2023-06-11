/* 
   config: global system config and load/save facilities 

   This file is part of omron++, the can-do-anything visual apparaton.
 
   Copyright (C) 2007 Christian Beier <dontmind@freeshell.org>
 
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfg.h"
#include "util.h"
#include "lexwrite.h"



/*
  internal constants
*/
#define CFGFILE "omron++.cfg"

#define SCREEN "screen:", "iii"
#define FULLSCREEN "fullscreen:", "i"
#define OPENGL "opengl:", "i"
#define STATUSBAR "statusbar:", "i"
#define SMP "smp:", "i"
#define BENCHMARK "benchmark:", "i"
#define SOUND "sound:", "i"
#define AUTONOM "autonom:", "f"


/*                                                                              
  internal functions                                                            
*/ 

// this one determines the full path to the config file
static char* cfg_fullpath();





systemcfg* cfg_load()
{
  systemcfg *cfg;
  cfg = ut_calloc(1, sizeof(systemcfg));

  /*
    first, load defaults
  */
  cfg->sound = 1;
  cfg->smp = 0;
  cfg->benchmark = 0;  
  cfg->opengl = 0;
  cfg->fullscreen = 0;
  cfg->statusbar = 1;
  cfg->screen_bpp = 16;
  cfg->screen_x = 640;
  cfg->screen_y = 480;
  cfg->autonom = 1.0;

  /*
    then, try to load something from config file
  */

  FILE *cfg_file;
  if((cfg_file = fopen(cfg_fullpath(),"r")) == NULL)
    {
      fprintf(stderr,"NOTICE: no config file found, loading defaults...\n");
      return cfg;
    }

  // get the size
  fseek(cfg_file, 0, SEEK_END);
  long size = ftell(cfg_file);
  fseek(cfg_file, 0, SEEK_SET);

  // read it in
  char *cfg_string = ut_malloc(size);
  fread(cfg_string, sizeof(char), size, cfg_file);
  if(ferror(cfg_file))
    {
      fprintf(stderr, "NOTICE: error reading config file, loading defaults...\n");
      return cfg;
    }

  // and get the values
  lw_init(cfg_string);

  lw_get(SOUND, &cfg->sound);
  lw_get(SMP, &cfg->smp);
  lw_get(BENCHMARK, &cfg->benchmark);
  lw_get(FULLSCREEN, &cfg->fullscreen); 
  lw_get(OPENGL, &cfg->opengl); 
  lw_get(STATUSBAR, &cfg->statusbar); 
  lw_get(SCREEN, &cfg->screen_x, &cfg->screen_y, &cfg->screen_bpp);
  lw_get(AUTONOM, &cfg->autonom); 

  //sanity checks
  if(cfg->screen_x < 320)    
    cfg->screen_x = 320;
  
  if(cfg->screen_y < 240)    
    cfg->screen_y = 240;

  if (cfg->screen_bpp < 8) 
    cfg->screen_bpp = 8;                                             

  if(cfg->autonom <= 0 || cfg->autonom > 100) // crap entered
    cfg->autonom = 1.0;


  // cleanup
  free(cfg_string);
  fclose(cfg_file);
  return cfg;
}






int cfg_save(systemcfg *cfg)
{
  char **handle = lw_init(NULL); // new config
  
  lw_put(SCREEN, cfg->screen_x, cfg->screen_y, cfg->screen_bpp); 
  lw_put(FULLSCREEN, cfg->fullscreen);
  lw_put(OPENGL, cfg->opengl);
  lw_put(STATUSBAR, cfg->statusbar);
  lw_put(SMP, cfg->smp);
  lw_put(BENCHMARK, cfg->benchmark);
  lw_put(SOUND, cfg->sound);
  lw_put(AUTONOM, cfg->autonom);

  char* cfg_string =  *handle; // get assembled string

  // and write it to file
  FILE *cfg_file;
  if((cfg_file = fopen(cfg_fullpath(),"w")) == NULL)
    {
      fprintf(stderr,"\nWARNING: could not write config file, your settings will not be saved...\n");
      return 0;
    }
  
  fprintf(cfg_file,"%s", cfg_string);

  free(cfg_string); // don't need it anymore
  fclose(cfg_file);
  return 1;
}




// not so nice to return char*, but at least it's static...
static char* cfg_fullpath()
{
  static char *fullpath = NULL; // to avoid leaking
  char *usrpath;
  char *filename;

  if((usrpath = getenv("XDG_CONFIG_HOME")) || (usrpath = getenv("HOME")))   // unix
    filename = "/."CFGFILE;
  else  
    if((usrpath = getenv("AppData"))) // win-nt
      filename = "\\"CFGFILE;
    else  // everything else
      {
	usrpath = ""; //!!
	filename = CFGFILE;
      }

  fullpath = ut_realloc(fullpath, strlen(usrpath) + SMALL_STRSIZE);
  *fullpath = '\0';

  sprintf(fullpath, "%s%s", usrpath, filename);
  return fullpath;
}





