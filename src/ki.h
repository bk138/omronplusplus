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

#ifndef KI_H
#define KI_H

/*
  exposed constants
*/
// these are used as ids for the battleground,
// BEWARE: must be negative !!!
#define FREE -1
#define NOWHERE -2

// this is what it says and also the id for 'no army'
#define MAX_NR_ARMIES 8



/*
  exposed datatypes 
*/
typedef struct
{
  Sint16 x;  // this really makes it faster than using Sint32
  Sint16 y;  // but only here, not as function args
} 
coord;


typedef struct
{
  Uint32 size;
  coord *fields;
  Uint8 dir;
} 
view;


typedef struct
{
  Sint16 x;
  Sint16 y;
  Sint16 s_x;
  Sint16 s_y;
  Sint8 dead;
  Sint8 lastdir;
  Uint32 kills;
} 
soldier;


typedef struct
{
  Sint8 id; // army identifier
  Sint8 index; // index in armies array
  SDL_Color color;
  const char* name;
  soldier *troops; // arrays of soldiers, their size and bodycount
  Uint32 count;
  Uint32 nr_dead;
  view **views;  // view in 8 directions and view range
  Uint32 vrange;
  Uint32 mbsz; //mobsize
}
army;


typedef struct
{
  Sint8 id;
  const char* pos_str;
  Uint32 sx;
  Uint32 sy;
  Uint32 min_x;
  Uint32 min_y; 
  Uint32 max_x; 
  Uint32 max_y;
  Uint32 count;
  Uint32 dist;
  Uint32 vrange;
  Uint32 mbsz;
}
armycfg;


typedef struct
{
  Sint8 army_index; // index in armies array
  Uint32 soldier_number;
} 
battleground_field;








/*
  exposed functions
*/

SDL_Color ki_getColorFromID(Sint8 id);
const char* ki_getNameFromID(Sint8 id);

// creates an array of armycfgs
// sets nr_armies
armycfg* ki_preInit(int nr_pos, const Sint8* army_at_pos);

// test count, dist, vrange parameters against pre-cfg
int ki_testArmyCfg(armycfg *ac);

// automagically fills out _one_ armycfg 
void ki_autoArmyCfg(armycfg *ac, float autonom, int reset);

// inits ki subsystem from a ptr to _all_ armycfgs
int ki_init(armycfg *ac);

void ki_shutdown();

battleground_field* ki_whatsThere(Sint32 x, Sint32 y);

Sint8 ki_makeMove(army *a);





#endif // KI_H
