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

#ifndef MENU_H
#define MENU_H

/*
  exposed functions 
*/
void mn_intro();
void mn_main();
void mn_start_auto(int nr_pos);
// a one-line message displayed to the user
void mn_msg(const char* txt);


#endif
