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

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "SDL.h"
#include "SDL_thread.h"

#include "ki.h"
#include "util.h"
#include "sound.h"
#include "cfg.h"


/*
  internal constants
*/
// the default ki_shoot() win chance in percent
#define DFLT_WIN_CHANCE 75
#define KILL_BONUS 5



/*
  internal functions:
*/
static army *ki_initArmy(armycfg *ac, Uint8 index);

static void ki_shutdownArmy(army *a);

static view **ki_initViews(Uint32 vrange);

static int ki_placeSoldiers(int *x, int *y, Uint32 min_x, Uint32 min_y, Uint32 max_x, Uint32 max_y, Sint32 dist);

static void ki_shortRangeScan(const soldier *joe);

static void ki_longRangeScan(army *a, const soldier *joe, Uint8 *choices, Uint8 choices_nr);

static int ki_scanView(void *data);

static void ki_shoot(soldier* joe, Uint8 dir);

static void ki_move(soldier* joe, Sint8 dir);

static Sint8 ki_getReturnDir(soldier* joe);



/*
  internal global variables
*/
// index of opponents 
army **armies; Uint8 nr_armies;

// indicates return mode
char do_return[4];


// scanresults, 8 slots for each army
// this hold army indices, not ids!
Sint32 *scanresult;
coord scanpos = {NOWHERE, NOWHERE};


// a two-dimensional arena in a
// one-dimensional array ...
battleground_field *battleground;
// a placeholder if we access a field that's not on the battleground
battleground_field nowhere = {NOWHERE, NOWHERE};



/*
  other stuff
*/
// these are defined in main.c
extern Sint8 quit;
extern systemcfg *cfg;


//#define DEBUG




SDL_Color ki_getColorFromID(Sint8 id)
{
  SDL_Color c = {0,0,0,255};

  switch(id)
    {
    case 0: // RED
      c.r = 255;
      break;
    case 1: // BLUE
      c.r = 100;
      c.g = 100;
      c.b = 255;
      break;
    case 2: // GREEN
      c.g = 255;
      break;
    case 3: // YELLOW
      c.r = 255;
      c.g = 255;
      break;
    case 4: // BROWN
      c.r = 140;
      c.g = 70;
      c.b = 20;
      break;
    case 5: // ORANGE
      c.r = 255;
      c.g = 145;
      break;
    case 6: // BLACK
      break;
    case 7: // WHITE
      c.r = 255;
      c.g = 255;
      c.b = 255;
      break;
    default: // NONE
      c.a = 0; // set alpha to 0
      break;
    }

  return c;
}




const char* ki_getNameFromID(Sint8 id)
{
  switch(id)
    {
    case 0: // RED
      return "RED";
      break;
    case 1: // BLUE
      return "BLUE";
      break;
    case 2: // GREEN
      return "GREEN";
      break;
    case 3: // YELLOW
      return "YELLOW";
      break;
    case 4: // BROWN
      return "BROWN";
      break;
    case 5: // ORANGE
      return "ORANGE";
      break;
    case 6: // BLACK
      return "BLACK";
      break;
    case 7: // WHITE
      return "WHITE";
      break;
    default: // NONE
      return "NONE";
      break;
    }
}




// creates an array of armycfgs
// sets nr_armies
armycfg* ki_preInit(int nr_pos, const Sint8* army_at_pos)
{
  // first, calculate number of armies
  nr_armies = 0;
  int i;
  for(i=0; i < nr_pos; ++i)
    if(army_at_pos[i] != MAX_NR_ARMIES)
      ++nr_armies;

  // then set this
  if(cfg->statusbar)
    cfg->sb_height = nr_armies * SB_ONEROW;

  // alloc memory now
  armycfg *ac = ut_calloc(nr_armies, sizeof(armycfg));
  armycfg *c = ac; // for going through the array

  if(nr_pos == 2)
    {
      // pos 0 - left
      if(army_at_pos[0] != MAX_NR_ARMIES)
	{
	  c->id = army_at_pos[0];
	  c->pos_str = "LEFT"; 
	  c->sx = cfg->screen_x*0.25;
	  c->sy = (cfg->screen_y - cfg->sb_height)*0.5;
	  c->min_x = 0; 
	  c->min_y = 0; 
	  c->max_x = (cfg->screen_x/2)-1; 
	  c->max_y = cfg->screen_y-1 - cfg->sb_height; 

	  ++c;
	}
      // pos 1 - right
      if(army_at_pos[1] != MAX_NR_ARMIES)
	{
	  c->id = army_at_pos[1];
	  c->pos_str = "RIGHT"; 
	  c->sx = cfg->screen_x*0.75;
	  c->sy = (cfg->screen_y - cfg->sb_height)*0.5;
	  c->min_x = cfg->screen_x/2; 
	  c->min_y = 0; 
	  c->max_x = cfg->screen_x-1;
	  c->max_y = cfg->screen_y-1 - cfg->sb_height;
	}
    }
  else
    if(nr_pos == 4)
      {
	// pos 0 - upper left
	if(army_at_pos[0] != MAX_NR_ARMIES)
	  {
	    c->id = army_at_pos[0];
	    c->pos_str = "UPPER LEFT"; 
	    c->sx = cfg->screen_x*0.25;
	    c->sy = (cfg->screen_y - cfg->sb_height)*0.25;
	    c->min_x = 0; 
	    c->min_y = 0; 
	    c->max_x = (cfg->screen_x/2)-1;
	    c->max_y = (cfg->screen_y-1 - cfg->sb_height) /2;
	    
	    ++c;
	  } 

	// pos 1 - upper right
	if(army_at_pos[1] != MAX_NR_ARMIES)
	  {
	    c->id = army_at_pos[1];
	    c->pos_str = "UPPER RIGHT"; 
	    c->sx = cfg->screen_x*0.75;
	    c->sy =(cfg->screen_y - cfg->sb_height)*0.25;
	    c->min_x = cfg->screen_x/2; 
	    c->min_y = 0; 
	    c->max_x = cfg->screen_x-1; 
	    c->max_y = (cfg->screen_y-1 - cfg->sb_height)/2;

	    ++c;
	  }

	// pos 2 - lower left
	if(army_at_pos[2] != MAX_NR_ARMIES)
	  {
	    c->id = army_at_pos[2];
	    c->pos_str = "LOWER LEFT"; 
	    c->sx = cfg->screen_x*0.25;
	    c->sy =(cfg->screen_y - cfg->sb_height)*0.75;
	    c->min_x =  0; 
	    c->min_y = (cfg->screen_y-1 - cfg->sb_height) /2;
	    c->max_x = (cfg->screen_x/2)-1;
	    c->max_y = (cfg->screen_y-1 - cfg->sb_height) ;

	    ++c;
	  }

	// pos 3 - lower right
	if(army_at_pos[3] != MAX_NR_ARMIES)
	  {
	    c->id = army_at_pos[3];
	    c->pos_str = "LOWER RIGHT"; 
	    c->sx = cfg->screen_x*0.75;
	    c->sy =(cfg->screen_y - cfg->sb_height)*0.75;
	    c->min_x = cfg->screen_x/2; 
	    c->min_y = (cfg->screen_y-1 - cfg->sb_height) /2; 
	    c->max_x = cfg->screen_x-1;
	    c->max_y = cfg->screen_y-1 - cfg->sb_height;
 
	  }
      }
    else // neither 2 or 3 start positions 
      {
	free(ac);
	return NULL;
      }

  // all well
  return ac;
}






// test count, dist, vrange parameters of _one_ armycfg
int ki_testArmyCfg(armycfg *ac)
{
  /*
    test count and dist
  */

  int x = ac->sx;
  int y = ac->sy;
  // init placement function
  ki_placeSoldiers(NULL, NULL, 0,0,0,0, ac->dist);
  int i;
  for (i = 0; i < ac->count; ++i)
    {
      // if the troops won't fit bail out
      if (! ki_placeSoldiers(&x, &y, ac->min_x, ac->min_y, ac->max_x, ac->max_y, -1))
	return 0;
    } 
  /*
    test vrange
  */
  Uint32 max = cfg->screen_x > cfg->screen_y ? cfg->screen_x : cfg->screen_y;
 
  if(ac->vrange >= max)
    return 0;

  /*
    all well
  */
  return 1;
}





// automagically fills out _one_ armycfg 
void ki_autoArmyCfg(armycfg *ac, float autonom, int reset)
{
  Uint64 effort, effort_lim;
  static Uint32 lastcount;

  if(reset)
   {
     lastcount = 0;
     return;
   }
  

  int good = 0;

  while(!good)
    {
      // random values out of maximum ranges
      ac->count = ut_lRand((cfg->screen_x * cfg->screen_y) / nr_armies);
      if(lastcount) 
	if(ac->count < lastcount - lastcount/5 || ac->count > lastcount + lastcount/5)
	  continue;

      ac->vrange = 1 + ut_lRand(cfg->screen_x > cfg->screen_y ? cfg->screen_x : cfg->screen_y); 

      // effort limit estimation: 
      // percentages of max count x max coords to looks at
      effort_lim = (((float)autonom / 100) * ((cfg->screen_x * cfg->screen_y) / nr_armies)) *
	(((float)autonom / 100) * 3.14 * 
         (cfg->screen_x > cfg->screen_y ? cfg->screen_x : cfg->screen_y) *
	 (cfg->screen_x > cfg->screen_y ? cfg->screen_x : cfg->screen_y));
  
      // and the current effort
      effort = ac->count * 3.14 * ac->vrange * ac->vrange; 


      // try again if chosen values exceed given percentage or count is zero
      if(effort == 0 || effort > effort_lim)
	continue;


      // to keep it interesting, mobsize is 1/4 of count, but max 138 ...
      ac->mbsz = ut_lRand(ac->count/4 < 138 ? ac->count/4 : 138) + 1; //get [1..count] value
     
      ac->dist = ut_lRand(cfg->screen_x / (2*nr_armies));
      

      /*
	try chosen values
      */
      // view range ...
      if(ki_testArmyCfg(ac))
	good = 1;
    }

  lastcount = ac->count;
}




// inits ki subsystem from a ptr to _all_ armycfgs
int ki_init(armycfg *ac)
{
  Uint32 i;
  armycfg *c = ac;

  // just to be sure...
  for(i=0; i < nr_armies; ++i)
    {
      if(!ki_testArmyCfg(c))
	return 0;
      ++c;
    }
 
  // (re)-init the rng 
  srand(time(NULL));

  // the battleground is a two-dimensional arena embedded in a one-dimensional array
  // as such: we have screen_y times an array of screen_x size
  battleground = ut_malloc((cfg->screen_x * (cfg->screen_y - cfg->sb_height)) * sizeof(battleground_field));
  // and mark everything as free...
  for(i=0; i < (cfg->screen_x * (cfg->screen_y - cfg->sb_height)); ++i)
    battleground[i].army_index = FREE;
    
  // 8 slots for each army
  scanresult = ut_malloc(8 * nr_armies * sizeof(Sint32));

  armies = ut_malloc(nr_armies * sizeof(army*));

  c = ac; 
  for(i=0; i < nr_armies; ++i)
    {
      armies[i] = ki_initArmy(c, i);
      ++c;
    }

  return 1;
}





void ki_shutdown()
{
  int i;
  for (i=0; i < nr_armies; ++i)
    ki_shutdownArmy(armies[i]);

  free(armies);
  free(scanresult);
  free(battleground);
}




static army* ki_initArmy(armycfg *ac, Uint8 index)
{
  Uint32 i;
  Sint32 x = ac->sx, y = ac->sy;
 
  soldier *s = NULL;
  army *a = NULL;

  // first, allocate mem for the army structs
  a = ut_calloc(1, sizeof(army));
  
  // and some basic setup
  a->id = ac->id;
  a->index = index;
  a->name = ki_getNameFromID(ac->id);
  a->color = ki_getColorFromID(ac->id);
  a->count = ac->count;
  a->vrange = ac->vrange;
  a->mbsz = ac->mbsz;

  /*
    try count and dist
  */
  a->troops = ut_calloc(a->count, sizeof(soldier));

  // init placement function
  ki_placeSoldiers(NULL, NULL, 0,0,0,0, ac->dist);

  // get first soldier´s address, i.e. start of troops array
  s = a->troops;

  // fill it 
  for (i = 0; i < a->count; ++i)
    {
      s->dead = 0;
      s->kills = 0;
      s->lastdir = -1;
	  
      ki_placeSoldiers(&x, &y, ac->min_x, ac->min_y, ac->max_x, ac->max_y, -1);
      s->x = x;
      s->y = y;
      s->s_x = x; // also save start position
      s->s_y = y;
      
      ++s;
    }

  // and the views
  a->views = ki_initViews(a->vrange);

 
  // now that everything was initialized successfully,
  // actually place the troops on the battleground ...
  s = a->troops; // get array start
  for(i = 0; i < a->count; ++i)
    {
      // the first addend selects the vertical line specified by the y value,
      // the second one selects the x'th member of that line ...
      battleground[s->y * cfg->screen_x + s->x].army_index = a->index;
      battleground[s->y * cfg->screen_x + s->x].soldier_number = i;
      ++s;
    }
  

  // all went well
  return a;
}





// this takes a fully (!!) initialized army
// and deallocates its memory ...
static void ki_shutdownArmy(army *a)
{
  Uint8 dir;
  
  free(a->troops);

  for(dir=0; dir<8; ++dir) 
    {
      free(a->views[dir]->fields);
      free(a->views[dir]);
    }

  free(a->views);
  
  free(a);
}






// okay, this one's a bit ugly, but it works...
// if all goes well, returns a fully initialized view object
static view **ki_initViews(Uint32 vrange)
{
  char *q;
  Sint32 x, y;
  Uint32 qs, i, size;
  Uint32 horizvert_s = 0, diagonal_s = 0;
  view **v = NULL;
  Uint32 max = cfg->screen_x > cfg->screen_y ? cfg->screen_x : cfg->screen_y;
 
  if(vrange >= max)
    return NULL;
    
  // the quadrant is qs*qs big
  // qs is vrange+1 because we have our soldier at (0,0) plus vrange fields around ...
  qs = vrange + 1;
  q = ut_malloc(qs * qs * sizeof(char));
  // then, mark everything as empty
  memset(q, 'e', qs * qs * sizeof(char));
  
  for(y=0; y < qs; ++y)
    for(x=0; x < qs; ++x)
      {
	// the inner region is the soldier itself and its
	// short-range vision, so mark this
	if(x < 2 && y < 2)
	  {
	    q[y*qs + x] = 'i';
	    q[x*qs + y] = 'i';
	  }
	else
	  {
	    /*
	      okeh: the angle we're looking for is 45°/2 = 22,5°.
	      tan(22,5°) = 0,41 = m of our linear function y=mx.
	      we mark a triangle below that with 'h' and mirror this
	      to the y-axis, marked with 'v' ...
	    */
	    if(y <= 0.414*x)
	      {
		q[y*qs + x] = 'h';
		q[x*qs + y] = 'v';
	      }
	    
	    // what's beyond the range we don't see
	    // (circle: r^2 = x^2+y^2)
	    if((x*x + y*y) > vrange*vrange)
	      q[y*qs + x] = 'x';
	  }
      }


 
  // get the size for the real coordinate arrays
  for(y=0; y < qs; ++y)
    for(x=0; x < qs; ++x)
      switch(q[y*qs + x])
	{
	case 'e':
	  ++diagonal_s;
	  break;
	case 'h':
	  ++horizvert_s;
	  break;
	default:
	  break;
	}

  // we only have the upper "half" of the horiz sector
  // (or the right half of the vertical one),
  // so double it, but then take away the stripe where y is zero
  // (make some drawings and find out yourself...)
  horizvert_s = 2*horizvert_s - (vrange > 0 ? vrange-1 : 0);
  
#ifdef DEBUG
  printf("\nDEBUG: initViews\n");
  // so, you have sth like this:
  for(y=qs-1; y > -1; y--)
    for(x=0; x < qs; ++x)
      if(x<qs-1)
	printf("%c ", q[y*qs+x]);
      else
	printf("%c\n", q[y*qs+x]);
  printf(" d: %i\n", diagonal_s);
  printf("hv: %i\n", horizvert_s);
  printf("diff: %i\n", horizvert_s-diagonal_s);  
#endif
  

  /*
    now that we have the sizes, go and allocate some mem!
  */
  
  // an array of 8 pointers to views
  v = ut_malloc(8*sizeof(view*));
  
  // the views are organised in an array[8]
  for(i=0; i < 8; ++i)
    {
      if(i%2 == 0)
	size = horizvert_s;
      else
	size = diagonal_s;

      // the view struct itself
      v[i] = ut_malloc(sizeof(view));
  
      // the fields to look at in this view
      v[i]->fields = ut_malloc(size * sizeof(coord));

      v[i]->dir = i;
      v[i]->size = size;
    } 

  /*
    ok, initialize the view-fields
  */

  // sector 0 is put together by hand
  i=0;
  
  for(y=0; y < qs; ++y)
    for(x=0; x < qs; ++x)
      if(q[y*qs + x] == 'v')
	{
	  v[0]->fields[i].x = x;
	  v[0]->fields[i].y = -y; // because 0,0 is upper left in SDL !
	  ++i;
	}
   
  for(y=0; y < qs; ++y)
    for(x=1; x < qs; ++x)
      if(q[y*qs + x] == 'v')
	{
	  v[0]->fields[i].x = -x;
	  v[0]->fields[i].y = -y; // same here	
	  ++i;
	}

  // the other horizvert ones are modified copies of sector[0]
  for(i=0; i < horizvert_s; ++i)
    {
      // beware that the y-axis is reversed !
      x = v[0]->fields[i].x;
      y = v[0]->fields[i].y;
      
      v[2]->fields[i].x = -y;
      v[2]->fields[i].y = -x;
   
      v[4]->fields[i].x = x;
      v[4]->fields[i].y = -y;

      v[6]->fields[i].x = y;
      v[6]->fields[i].y = -x;
    }
  
  
  // the diagonal ones:
  i=0;
  
  for(y=0; y < qs; ++y)
    for(x=0; x < qs; ++x)
      if(q[y*qs + x] == 'e')
	{
	  v[1]->fields[i].x = x;
	  v[1]->fields[i].y = -y; // same here again ...

	  v[3]->fields[i].x = x;
	  v[3]->fields[i].y = y;

	  v[5]->fields[i].x = -x;
	  v[5]->fields[i].y = y;

	  v[7]->fields[i].x = -x;
	  v[7]->fields[i].y = -y; // bla
	  ++i;
	}

  //cleanup
  free(q);
  
  // whoa, we did it
  return v;
}





// this places soldiers in a spiral-like way
// --> you HAVE TO init it first with the placement distance !!!
static int ki_placeSoldiers(Sint32 *x, Sint32 *y, Uint32 min_x, Uint32 min_y, Uint32 max_x, Uint32 max_y, Sint32 init)
{
  // chdir == changes of direction,
  // whereto == 0..east, 1..south, 2..west, 3..north
  static Uint8 chdir, whereto, firstrun;
  static Uint32 stepstodo, stepsdone, steps;


  // because of the staticness above we also need this for a fresh run...
  if (init >= 0)
    {
      chdir = 0;
      whereto = 0; 
      stepstodo = 1; 
      stepsdone = 0;
      steps = init+1;
      firstrun = 1;
      return 1;
    }

  // because the zeroth soldier is in the middle of the spiral
  if (firstrun)
    {
      firstrun = 0;
      return 1;
    }

  switch (whereto)
    {
    case 0: 
      (*x) += steps;
      ++stepsdone;
      break;
    case 1: 
      (*y) += steps;
      ++stepsdone;
      break;
    case 2: 
      (*x) -= steps;
      ++stepsdone;
      break;
    case 3: 
      (*y) -= steps;
      ++stepsdone;
      break;
    default:
      break;
    }


  // if the step in some direction results in an out-of-arena position,
  // we can bail out here ...
  if ((*x < min_x || *x > max_x) || (*y < min_y || *y > max_y ))
    return 0;

 
  if (stepsdone == stepstodo)
    {
      // change direction
      if(whereto == 3) 
	whereto = 0;
      else
	++whereto;

      ++chdir;

      // we have changed direction two times,
      // so increment stepstodo
      if(chdir == 2)
	{
	  chdir = 0;
	  ++stepstodo;
	}
      
      stepsdone = 0;
    }

  return 1;
}




battleground_field* ki_whatsThere(Sint32 x, Sint32 y)
{
  // some sanity checks:
  // if we're out ouf bounds, return address of the nowhere field...
  if (x < 0 || x > cfg->screen_x-1)
    return &nowhere;

  if (y < 0 || y > (cfg->screen_y - 1) - cfg->sb_height)
    return &nowhere;

  // ok? then return address of the specified field
  return &battleground[y * cfg->screen_x + x];
}





// the real guts of it all
Sint8 ki_makeMove(army *a)
{
  Sint8 army_alive=0, dir, c, choices_nr, finalchoices_nr;
  Uint32 i; 
  Uint32 nr_friends=0, nr_foes=0;
  Sint32 score, hiscore;
  soldier *s = NULL;    

  static Uint8 choices[8];
  static Uint8 finalchoices[8];

  //setup
  s = a->troops;

  
  // go!
  for(i=0; i < a->count; ++i)
    {
      if (! s->dead)
	{
	  army_alive = 1;	     
	   
	  // at first no choices
	  choices_nr = 0;

	  /*
	    scan perimeter of current soldier
	    (here we always just use the first 8 slots of scanresult,
	    each one indicates friend or foe...)
	  */
	  ki_shortRangeScan(s);

	  
	  /*
	    1. any foes in the nearest vincinity ?
	  */   
	  for (dir=0; dir<8; ++dir)
	    if (scanresult[dir] >= 0 && armies[scanresult[dir]]->id != a->id)
	      {
		choices[choices_nr] = dir;
		++choices_nr;
	      }
	      
	  // found some foes ?
	  if(choices_nr > 0)
	    {
	      // randomly choose one of the choices
	      dir = choices[ut_lRand(choices_nr)];
		    
	      // try to kill and erase from battleground (or die trying...)
	      ki_shoot(s, dir);
	      // only move if s won the fight!!!
	      if(!s->dead)
		ki_move(s, dir);
	   		  
	      // we did move, so go on with next soldier
	      ++s;
	      continue;
	    }


	  /*
	    1.5. we're out of melee, let's see if we got orders to
	    head back 
	  */
	  
	  if(do_return[a->index])
	    {
	      Sint8 retdir = ki_getReturnDir(s);
	      if(retdir >= 0 && scanresult[retdir] == FREE)
		ki_move(s, retdir); 
	      // go on with next soldier
	      ++s;
	      continue;
	    }

	  /*
	    2. ok, no foes directly next to us, 
	    but is there any possibility to move anywhere ?
	  */

	  for (dir = 0; dir < 8; ++dir)
	    if (scanresult[dir] == FREE) 
	      {
		choices[choices_nr] = dir;
		++choices_nr;
	      }

	  // found any possibilities to move ?
	  if(choices_nr > 0)
	    {
	      // first no final choices, hiscore negative
	      finalchoices_nr = 0;
	      hiscore = -1;
	      
	      /*
		scan each view sector of our soldier we could move to 
		(here we use scanresult differently: the first eight slots
		indicate the number of armies[0] in each sector, 
		the second eight slots the numbers of armies[1] in each sector,
		and so on ...
	      */
	      ki_longRangeScan(a, s, choices, choices_nr);

	     
	      for(c=0; c < choices_nr; ++c)
		{
		  dir = choices[c];
		  
		  nr_foes = 0; // several enemy armies, but no alliances (yet) ;-)

		  Sint8 idx;
		  for(idx=0; idx < nr_armies; ++idx)
		    if(armies[idx]->id != a->id)
		      nr_foes += scanresult[(idx*8)+dir];
		    else
		      nr_friends = scanresult[(idx*8)+dir];

		  if (nr_friends < a->mbsz) // mobsize at least one !
		    {
		      if(nr_foes > 0)
			if(nr_friends > 0)
			  score = (a->mbsz - nr_friends) + nr_foes;
			else
			  score = nr_foes;
		      else
			score = 0;
		    }
		  else
		    score = -nr_friends + nr_foes;

		  // if score for this direction is negative, discard this direction
		  if (score < 0)
		    continue;
	
		  if (score > hiscore)
		    {
		      hiscore = score;
		      finalchoices[0] = dir;
		      finalchoices_nr = 1;
		    }
		  else
		    if (score == hiscore)
		      {
			finalchoices[finalchoices_nr] = dir;
			++finalchoices_nr;
		      }
		}
		
	   
	      
	      if (finalchoices_nr > 0)
		{
		  dir = -1;
		  
		  // if last direction is in final choices, favor it
		  for(c=0; c < finalchoices_nr; ++c)
		    if(finalchoices[c] == s->lastdir)
		      {
			if(ut_lRand(1000) < 950) // 95% chance
			  dir = finalchoices[c];
			break;
		      }
		  
		  // if not, randomly choose one of the final choices 
		  if(dir == -1)
		    dir = finalchoices[ut_lRand(finalchoices_nr)];
		  
		  // and move there ...
		  ki_move(s, dir);
		
		}
	      // or else don't move ...
	    }
	  // no move possible for this soldier
	}
      // next one please
      ++s;
    }
  // whole army processed, return true if there are survivors
  return army_alive;
}






/*
  show how the nearest perimeter of our soldier looks like:
  .  7 0 1  
  .   \|/   
  .  6-j-2  
  .   /|\   
  .  5 4 3  
*/
static void ki_shortRangeScan(const soldier* joe)
{
  scanresult[0] = ki_whatsThere(joe->x,   joe->y-1)->army_index;
  scanresult[1] = ki_whatsThere(joe->x+1, joe->y-1)->army_index;
  scanresult[2] = ki_whatsThere(joe->x+1, joe->y  )->army_index;
  scanresult[3] = ki_whatsThere(joe->x+1, joe->y+1)->army_index;
  scanresult[4] = ki_whatsThere(joe->x,   joe->y+1)->army_index;
  scanresult[5] = ki_whatsThere(joe->x-1, joe->y+1)->army_index;
  scanresult[6] = ki_whatsThere(joe->x-1, joe->y  )->army_index;
  scanresult[7] = ki_whatsThere(joe->x-1, joe->y-1)->army_index;
}  





static void ki_longRangeScan(army *a, const soldier *joe, Uint8 *choices, Uint8 choices_nr)
{
  Uint8 i, dir;
  static SDL_Thread *scanThreads[8];

  // reset scanresults: no one in evry direction
  for(i=0; i < 8 * nr_armies; ++i)
    scanresult[i] = 0;

  // scan would reveal nothing, this makes blind armies a _lot_ faster
  if(a->vrange <= 1)
    return;


  scanpos.x = joe->x;
  scanpos.y = joe->y;

  if(!cfg->smp) // no multithreading
    {
      for(i=0; i < choices_nr; ++i)
	{
	  dir = choices[i];
	  ki_scanView(a->views[dir]);
	}
    }
  else
    {
      // fire them up
      // FIXME: start them at beginning!!
      for(i=0; i < choices_nr; ++i)
	{
	  dir = choices[i];
	  scanThreads[dir] = SDL_CreateThread(ki_scanView, "ki_scanView", a->views[dir]);
	}

  
      // and wait for each one
      for(i=0; i < choices_nr; ++i)
	{
	  dir = choices[i];
	  SDL_WaitThread(scanThreads[dir], NULL);
	}
    }

#ifdef DEBUG
  ut_log("\nDEBUG: ki_longRangeScan\n");
  ut_log("\nARMY %i - soldier nr: %i - SOLDIER pos: %i,%i\n",
	 a->id, ki_whatsThere(joe->x, joe->y)->soldier_number, joe->x, joe->y);
  for(dir=0; dir < 8; ++dir)
    ut_log("direction %i: %i of id 0\n", dir, scanresult[dir]);
  for(dir=8; dir < 16; ++dir)
    ut_log("direction %i: %i of id 1\n", dir-8, scanresult[dir]);
#endif
  
}






static int ki_scanView(void *data)
{
  Sint8 res;
  view *v = (view*)data; 

  // for every (relative !) coordinate in the sector, we add
  // the x and y values to our current position and have a look 
  // what's there ...
  Uint32 j;
  for(j=0; j < v->size; ++j)
    {

#ifdef DEBUG
      ut_log("\nDEBUG: ki_scanView\n");
      ut_log("scan pos: %i,%i\n", scanpos.x, scanpos.y);
      ut_log("sector: %i, rel %i,%i", v->dir, v->fields[j].x, v->fields[j].y );
      ut_log("\nsector: %i, pos %i,%i", v->dir, scanpos.x + v->fields[j].x, scanpos.y + v->fields[j].y );
#endif
    
      // this returns an army index for adressing armies[index]
      res = ki_whatsThere(scanpos.x + v->fields[j].x, scanpos.y + v->fields[j].y)->army_index;

      // increment scanresult according to index and direction
      if(res >= 0) // so its a valid army index
	scanresult[(res*8) + v->dir]++; 

	    
      // otherwise res is negative and thus either
      // FREE (noone there) or NOWHERE (out-of-battleground)
    }

  return 0;
}







/*
  kill whatever is in the specified direction or die trying
  i.e. set to dead and remove from battleground
*/
static void ki_shoot(soldier* joe, Uint8 dir)
{
  // get coordinates of our soldier
  Sint32 x = joe->x;
  Sint32 y = joe->y;
 
  battleground_field* field_joe = ki_whatsThere(x, y);

  // compute coordinates to shoot at
  switch(dir)
    {
    case 0:
      y--;
      break;
    case 1:
      ++x;
      y--;
      break;
    case 2:
      ++x;
      break;
    case 3:
      ++x;
      ++y;
      break;
    case 4:
      ++y;
      break;
    case 5:
      --x;
      ++y;
      break;
    case 6:
      --x;
      break;
    case 7:
      --x;
      --y;
      break;
    default:
      break;
    }


  // get field and soldier at this coordinates
  battleground_field* field_enemy = ki_whatsThere(x, y);
  soldier* enemy = &armies[field_enemy->army_index]->troops[field_enemy->soldier_number];

#ifdef DEBUG
  // make sure we really shoot at someone
  if(field_enemy->army_index < 0)
    {
      ut_log("DEBUG: ki_shoot()\n");
      ut_log("OUCH! here should be an army index to shoot at!\n");
      exit(EXIT_FAILURE);
    }
#endif


  // KILL_BONUS percent bonus for evry kill
  if(ut_lRand(100) < DFLT_WIN_CHANCE + (KILL_BONUS * (joe->kills - enemy->kills)))
    {
      // set dead the enemy
      enemy->dead = 1;
      armies[field_enemy->army_index]->nr_dead++;
      snd_beep((armies[field_enemy->army_index]->id + 1) * 200, 50, 1); // funeral sound
      
      // clear this battleground_field
      field_enemy->army_index = FREE;      
    
      // add to the score
      joe->kills++;
    }
  else
    {
      // set dead our g.i. joe
      joe->dead = 1;
      armies[field_joe->army_index]->nr_dead++;
      snd_beep((armies[field_joe->army_index]->id + 1) * 200, 50, 1); // funeral sound
      
      // clear this battleground_field
      field_joe->army_index = FREE;      
    
      // add to the score of enemy soldier
      enemy->kills++;
    }

}




static void ki_move(soldier* joe, Sint8 dir)
{
  battleground_field* field_new = NULL;

  // get current values
  Sint32 x = joe->x;
  Sint32 y = joe->y; 
  battleground_field* field_old = ki_whatsThere(x, y);
 
#ifdef DEBUG     	    
  ut_log("\nDEBUG: ki_move()\n");
  int olx = x, oly = y;
  ut_log("old field @(%i, %i) before: army: %i - number: %i\n",
	 x,y, ki_whatsThere(x, y)->army_index ,ki_whatsThere(x, y)->soldier_number);

  if(ki_whatsThere(x, y)->army_index < 0)
    {
      ut_log("OUCH! here should be an index!\n");
      exit(EXIT_FAILURE);
    }
#endif
  
  // determine new coordinates
  switch(dir)
    {
    case 0:
      --y;
      break;
    case 1:
      ++x;
      --y;
      break;
    case 2:
      ++x;
      break;
    case 3:
      ++x;
      ++y;
      break;
    case 4:
      ++y;
      break;
    case 5:
      --x;
      ++y;
      break;
    case 6:
      --x;
      break;
    case 7:
      --x;
      --y;
      break;
    default:
      break;
    }

  
  // alter position of our 'ol soldier
  joe->x = x;
  joe->y = y; 

  joe->lastdir = dir;

  
  //update battleground
  field_new = ki_whatsThere(x, y);

  field_new->army_index = field_old->army_index;
  field_new->soldier_number = field_old->soldier_number;

  field_old->army_index = FREE;      

#ifdef DEBUG     	    
  ut_log("old field @(%i, %i) after : army: %i - number: %i\n", 
	 olx,oly, ki_whatsThere(olx, oly)->army_index ,ki_whatsThere(olx, oly)->soldier_number);
  ut_log("new field @(%i, %i) : army: %i - number: %i\n", 
	 x,y, ki_whatsThere(x, y)->army_index, ki_whatsThere(x, y)->soldier_number);
#endif  

}



// returns < 0 if no dir found
static Sint8 ki_getReturnDir(soldier* joe)
{
  int dx = joe->s_x - joe->x;
  int dy = joe->s_y - joe->y;

  Sint8 dir = -1;

  if(abs(dx) > abs(dy))  // left-right
    dir = dx > 0 ? 2 : 6;
  else
    if(abs(dy) > abs(dx))// up-down
      dir = dy > 0 ? 4 : 0;
    else // same abs(), diagonal 
      {
	if(dx < 0)
	  {
	    if(dy < 0)
	      dir = 7; 
	    else
	      dir = 5;
	  }
	else
	  if(dx > 0)
	    {
	      if(dy < 0)
		dir = 1; 
	      else
		dir = 3;
	    }
	
	// dx == dy == 0
      }

  if(dir >= 0) // we have somewhere to move
    {
      int c = ut_lRand(100);
      if(c < 33)// 33% chance for a little change
	{
	  if(c%2 == 0) // and now 50-fifty
	    dir = dir == 7 ? 0 : dir+1;
	  else
	    dir = dir == 0 ? 7 : dir-1;
	}
    }

  return dir;
}
