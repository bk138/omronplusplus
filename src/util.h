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

#ifndef UTIL_H
#define UTIL_H

#ifdef ANDROID
#include <android/log.h>
#endif

/*
exposed constants
*/
#define BIG_STRSIZE 138
#define SMALL_STRSIZE 42


/*
  exposed functions 
*/
// return random number between 0 and limit, EXCLUDING limit ...  
int ut_lRand(int limit);

void* ut_malloc(size_t sz);

void* ut_realloc(void *p, size_t newsz);

void* ut_calloc(size_t n, size_t sz);

#endif // UTIL_H
