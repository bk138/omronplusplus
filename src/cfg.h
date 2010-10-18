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

#ifndef CFG_H
#define CFG_H


/*
  exposed datatypes 
*/
typedef struct
{
  // sound
  char sound;
  // multithreading
  char smp;
  // benchmark
  char benchmark;
  // video 
  char opengl;
  char fullscreen;
  char statusbar;
  int screen_bpp;
  int screen_x;
  int screen_y;
  unsigned int sb_height; //this gets calculated!
  // ki 
  float autonom;
} 
systemcfg;


/*
  exposed constants
*/
#define SB_ONEROW 9





/*
  exposed functions
*/
systemcfg* cfg_load();
int cfg_save(systemcfg *cfg);



#endif
