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


#ifndef VIDEO_H
#define VIDEO_H




/*
  exposed functions
*/

void vid_init();

void vid_setMode(int width, int height);

void vid_flip();

void vid_cls();

void vid_takeScreenshot();

void vid_printInfo();

void vid_toggleFullscreen();

SDL_Surface* vid_copySurface(SDL_Surface *src);


#endif
