/* 
   lexwrite: lowlevel config file writing and parsing


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

#ifndef LEXWRITE_H
#define LEXWRITE_H

/*
  exposed functions
*/

// init lexwriter
// arg NULL for new config string
// arg chptr to append to chptr
// return ptr to (by now) assembled char*
char** lw_init(char *cfg);

// get sth, values are stored given pointers
int lw_get(const char* keyword, const char* fmt, ... );
// put sth, 
// fmt is: 'i' int,  'f' float and 's' for string
int lw_put(const char* keyword, const char* fmt, ... );





#endif
