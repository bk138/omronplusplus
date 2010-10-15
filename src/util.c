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


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "util.h"


int ut_lRand(int limit)
{
  return limit != 0 ? rand() % limit : 0; 
}


void* ut_malloc(size_t sz)
{
  void *p;
  if ((p = malloc(sz)) == NULL)                 
    {
      fprintf(stderr, "ERROR: could not allocate memory.\n");
      exit(EXIT_FAILURE);                                                              
    }
  return p;
}



void* ut_realloc(void *p, size_t newsz)
{
  if ((p = realloc(p, newsz)) == NULL)                 
    {
      fprintf(stderr, "ERROR: could not allocate memory.\n");
      exit(EXIT_FAILURE);                                                              
    }
  return p;
}



void* ut_calloc(size_t n, size_t sz)
{
  void *p;
  if ((p = calloc(n,sz)) == NULL)                 
    {
      fprintf(stderr, "ERROR: could not allocate memory.\n");
      exit(EXIT_FAILURE);                                                              
    }   
  return p;
}



void ut_log(const char* format, ...)
{
  va_list args;

  va_start(args, format);

#ifdef ANDROID
  __android_log_vprint(ANDROID_LOG_INFO, "Omron++", format, args);
#else
  vfprintf(stderr, format, args);
  fflush(stderr);
#endif
  va_end(args);

  return;
}
