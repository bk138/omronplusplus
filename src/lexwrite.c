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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "lexwrite.h"
#include "util.h"



/*
  config file "syntax", haha ...
*/
#define LW_HEADER "this is a omron++ config file\nfeel free to edit it.\n"
#define LW_MARKER "[startofconfig]\n"
#define LW_DELIM ";"




/*
  internal functions
*/

// append argc strings to dest, resizing dest if needed
// returns (maybe relocated) dest or NULL if failing, leaving dest intact
// NOTE: Unless dest is NULL, it must have been returned by an earlier 
//       call to malloc(),  calloc()  or  realloc(). 
static char *lw_append(char *dest, size_t argc, ...);


/*
  global vars
*/
const char *nullp = NULL;
char **lwptr = (char**) &nullp;
size_t startoffs;
size_t currentoffs;



char** lw_init(char *config)
{
  // we make a new lw string or use given one
  if (config == NULL)
    *lwptr = lw_append(NULL, 4, LW_HEADER, "\n", LW_MARKER, "\n");
  else
    *lwptr = strdup(config);

  // ff till marker
  char *tmpp;
  if((tmpp = strstr(*lwptr, LW_MARKER)) == NULL)
    return NULL;
  startoffs = currentoffs = tmpp - *lwptr + strlen(LW_MARKER);

  return lwptr;
}





int lw_get(const char* keyword, const char* fmt, ... )
{
  char *offsptr;
  static char *lastkeyword = ""; //none

  if(strcmp(lastkeyword, keyword) != 0) 
    {
      currentoffs = startoffs; // request of new subkey, so goto start
      lastkeyword = (char*)keyword;
    }

  // ff to current offset
  offsptr = *lwptr + currentoffs;

  // find keyword and adjust offset
  if((offsptr = strstr(offsptr, keyword)) == NULL)
    {
      ut_log( "WARNING: Could not find (another) '%s' keyword in config file.\n", keyword);
      return 0;
    }
  offsptr += strlen(keyword);
  currentoffs = offsptr - *lwptr;	
  

  // values
  char *value;
  char **asptr;
  int *aiptr;
  float *afptr;
  va_list ap;
  va_start (ap, fmt);
  int i;
  for(i=0; i < strlen(fmt); ++i)
    {
      if(i==0)
	value = strtok(strdup(offsptr), LW_DELIM);
      else
	value = strtok(NULL, LW_DELIM);

      if(value == NULL)
	{
	  ut_log("WARNING: Something went wrong while reading values of keyword '%s' in config file.\n", keyword);
	  return 0;
	}
      
      switch(fmt[i])
	{
	case 's':
	  asptr = va_arg(ap, char**);
	  *asptr = value;
	  break;
	case 'i':
	  aiptr = va_arg(ap, int*);
	  *aiptr = strtol(value, NULL, 10);
      	  break;
	case 'f':
	  afptr = va_arg(ap, float*);
	  *afptr = strtod(value, NULL);
	  break;
	default:
	  break;
	}
    }
  va_end (ap);


  // all went well ...
  return 1;
}
		




int lw_put(const char* keyword, const char* fmt, ... )
{
  // keyword
  *lwptr = lw_append(*lwptr, 1, keyword);

  // values
  const char *as;
  int ai;
  float af;
  char buf[64];
  va_list ap;
  va_start (ap, fmt);
  while(*fmt)
    {
      switch(*fmt)
	{
	case 's':
	  as = va_arg(ap, const char *);
	  *lwptr = lw_append(*lwptr, 2, as, LW_DELIM);
	  break;
	case 'i':
	  ai = va_arg(ap, int);
	  snprintf(buf, 64, "%i", ai);
	  *lwptr = lw_append(*lwptr, 2, buf, LW_DELIM);
	  break;
	case 'f':
	  af = va_arg(ap, double);
	  snprintf(buf, 64, "%f", af);
	  *lwptr = lw_append(*lwptr, 2, buf, LW_DELIM);
	  break;
	default:
	  break;
	}

      ++fmt;
    }
  va_end (ap);

  // trailing nl
  *lwptr = lw_append(*lwptr, 1, "\n");

  return 1;
}





// append argc strings to dest, resizing dest if needed
// returns (maybe relocated) dest or NULL if failing, leaving dest intact
// NOTE: Unless dest is NULL, it must have been returned by an earlier 
//       call to malloc(),  calloc()  or  realloc(). 
char *lw_append(char *dest, size_t argc, ...)
{
  va_list ap;
  char *offs, *newp;
  const char *as;
  size_t allocated;

  if(dest == NULL)
    {
      allocated = 0;
      offs = dest;  
    }
  else
    {
      allocated = strlen(dest)+1;
      offs = dest + allocated-1;  // let offs point to \0 of dest
    }


  va_start (ap, argc);
  while(argc)
    {
      argc--;
      as = va_arg(ap, const char *);

      if(as == NULL)
	continue;

      size_t as_len = strlen(as);

      // if offset+size(as) > size(dest)  , all including \0's
      if(offs + as_len+1 > dest + allocated)
	{
	  allocated = (allocated + as_len) * 2;

	  if((newp =  realloc(dest, allocated)) == NULL)
	    return NULL;
	  
	  offs = newp + (offs - dest);
	  dest = newp;
	}
      
      // copy 'as' to offset and adjust offset
      offs = memcpy(offs, as, as_len);
      offs += as_len;
    }

  // terminate dest string  
  *offs = '\0';

  // resize memory to optimal size, i.e. trim to offset
  if((newp = realloc(dest, offs+1 - dest)) != NULL)
    dest = newp;

  va_end (ap);

  return dest;
}

