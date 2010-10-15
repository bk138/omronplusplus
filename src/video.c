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


#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "SDL.h"
#include "SDL_gfxPrimitives.h"
#ifdef HAVE_OPENGL
#include "SDL_opengl.h"
#endif
#include "util.h"
#include "video.h"
#include "config.h"





/*
  internal functions
*/
#ifdef HAVE_OPENGL
static GLuint vid_loadTexture(SDL_Surface *surface, GLfloat *texcoord);
static void* vid_getGLFuncAddr(const char* p);
#endif

/*
  internal variables
*/
SDL_Surface *screen;     // the screen we paint to
#ifdef HAVE_OPENGL
SDL_Surface *gl_screen;  // in opengl mode, this gets shown
SDL_Surface *teximage;   // we blit screen to teximage, which has the right format for
GLuint texture;          // texture, which gets shown on gl_screen
#endif

int videoflags = SDL_HWSURFACE | SDL_DOUBLEBUF;

char drivername[SMALL_STRSIZE]; 

#ifdef HAVE_OPENGL
// GL function pointers
GLubyte*(APIENTRY*vid_glGetString)(GLenum);
GLenum(APIENTRY*vid_glGetError)(void);
void(APIENTRY*vid_glEnable)(GLenum);
void(APIENTRY*vid_glDisable)(GLenum);
void(APIENTRY*vid_glMatrixMode)(GLenum);
void(APIENTRY*vid_glViewport)(GLint,GLint,GLsizei,GLsizei);
void(APIENTRY*vid_glOrtho)(GLdouble,GLdouble,GLdouble,GLdouble,GLdouble,GLdouble);
void(APIENTRY*vid_glFrustum)(GLdouble,GLdouble,GLdouble,GLdouble,GLdouble,GLdouble);
void(APIENTRY*vid_glBlendFunc)(GLenum,GLenum);
void(APIENTRY*vid_glTexEnvf)(GLenum, GLenum, GLfloat);
void(APIENTRY*vid_glClearColor)(GLclampf, GLclampf, GLclampf, GLclampf);
void(APIENTRY*vid_glClear)(GLbitfield);
void(APIENTRY*vid_glBegin)(GLenum );
void(APIENTRY*vid_glEnd)(void);
void(APIENTRY*vid_glVertex2i)(GLint, GLint);
void(APIENTRY*vid_glVertex3i)(GLint, GLint, GLint);
void(APIENTRY*vid_glBindTexture)(GLenum, GLuint);
void(APIENTRY*vid_glTexCoord2f)(GLfloat, GLfloat);
void(APIENTRY*vid_glTexParameteri)(GLenum, GLenum, GLint);
void(APIENTRY*vid_glGenTextures)(GLsizei, GLuint*);
void(APIENTRY*vid_glTexImage2D)( GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum,const GLvoid *);
void(APIENTRY*vid_glLoadIdentity)( void );
void(APIENTRY*vid_glMatrixMode)(GLenum );


void(APIENTRY*vid_glScalef)(GLfloat, GLfloat, GLfloat);
void(APIENTRY*vid_glPushMatrix)(void);
void(APIENTRY*vid_glPopMatrix)(void);
void(APIENTRY*vid_glColor3f)(GLfloat,GLfloat,GLfloat );
void(APIENTRY*vid_glTranslatef)(GLfloat,GLfloat,GLfloat );
void(APIENTRY*vid_glVertex3f)(GLfloat, GLfloat, GLfloat);
void(APIENTRY*vid_glRotatef)(GLfloat, GLfloat, GLfloat, GLfloat);
#endif




/*
  other stuff
*/
// these are defined in main.c
extern systemcfg *cfg;
extern Sint8 quit;





void vid_init()
{
  // 1. reset globals
  SDL_FreeSurface(screen);
  screen = NULL;
#ifdef HAVE_OPENGL
  SDL_FreeSurface(gl_screen);
  SDL_FreeSurface(teximage);
  gl_screen = teximage = NULL;
  texture = 0;
#endif

  // 2. halt an earlier started video subsys (in this order!!)
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
  

#ifdef unix    
  if(SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD) < 0)
#else
  if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
#endif
    {
      ut_log( "ERROR: could not initialize video/event subsystem:  %s\n", SDL_GetError());
      exit(EXIT_FAILURE);
    }
  
  if(cfg->fullscreen)
   {
     videoflags ^= SDL_FULLSCREEN;  
     SDL_ShowCursor(SDL_DISABLE);
   }


  /* 
     do all opengl init stuff here
  */
  if(!cfg->opengl)
    {
      if((videoflags & SDL_OPENGL) == SDL_OPENGL)
	videoflags ^= SDL_OPENGL;
    }
#ifdef HAVE_OPENGL
  else
    {
      int rgb_size[3];
      int bpp;

      // detect the display depth
      if ( SDL_GetVideoInfo()->vfmt->BitsPerPixel <= 8 ) 
	bpp = 8;
      else 
	bpp = 16;  /* More doesn't seem to work */
		

      /* Initialize the display */
      switch (bpp) 
	{
	case 8:
	  rgb_size[0] = 3;
	  rgb_size[1] = 3;
	  rgb_size[2] = 2;
	  break;
	case 15:
	case 16:
	  rgb_size[0] = 5;
	  rgb_size[1] = 5;
	  rgb_size[2] = 5;
	  break;
	default:
	  rgb_size[0] = 8;
	  rgb_size[1] = 8;
	  rgb_size[2] = 8;
	  break;
	}

      SDL_GL_SetAttribute( SDL_GL_RED_SIZE, rgb_size[0] );
      SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, rgb_size[1] );
      SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, rgb_size[2] );
      SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
      SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

      // request a hw accelerated context
      SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
      

      /* Use the default GL library */
      if (SDL_GL_LoadLibrary(NULL) < 0)
	{
	  ut_log( "WARNING: Unable to dynamically open GL lib : %s\n",SDL_GetError());
	  cfg->opengl = 0;
	}
      else
	{
	  // finally add opengl flag 
	  videoflags |= SDL_OPENGL;
	 
	  // get function addresses
	  vid_glGetString = vid_getGLFuncAddr("glGetString");
	  vid_glEnable = vid_getGLFuncAddr("glEnable");
	  vid_glDisable = vid_getGLFuncAddr("glDisable");
	  vid_glMatrixMode = vid_getGLFuncAddr("glMatrixMode");
	  vid_glOrtho = vid_getGLFuncAddr("glOrtho");
	  vid_glFrustum = vid_getGLFuncAddr("glFrustum");
	  vid_glViewport = vid_getGLFuncAddr("glViewport");
	  vid_glBlendFunc = vid_getGLFuncAddr("glBlendFunc");
	  vid_glTexEnvf = vid_getGLFuncAddr("glTexEnvf");
	  vid_glClearColor = vid_getGLFuncAddr("glClearColor");
	  vid_glClear = vid_getGLFuncAddr("glClear");
	  vid_glGetError = vid_getGLFuncAddr("glGetError");
	  vid_glBegin = vid_getGLFuncAddr("glBegin");
	  vid_glEnd = vid_getGLFuncAddr("glEnd");
	  vid_glBindTexture = vid_getGLFuncAddr("glBindTexture");
	  vid_glVertex2i = vid_getGLFuncAddr("glVertex2i");
	  vid_glVertex3i = vid_getGLFuncAddr("glVertex3i");
	  vid_glTexCoord2f = vid_getGLFuncAddr("glTexCoord2f");
	  vid_glTexParameteri = vid_getGLFuncAddr("glTexParameteri");
	  vid_glGenTextures = vid_getGLFuncAddr("glGenTextures");
	  vid_glTexImage2D = vid_getGLFuncAddr("glTexImage2D");
	  vid_glMatrixMode = vid_getGLFuncAddr("glMatrixMode");
	  vid_glLoadIdentity = vid_getGLFuncAddr("glLoadIdentity");

	  vid_glPushMatrix = vid_getGLFuncAddr("glPushMatrix");
	  vid_glPopMatrix = vid_getGLFuncAddr("glPopMatrix");

	  vid_glColor3f = vid_getGLFuncAddr("glColor3f");
	  vid_glTranslatef = vid_getGLFuncAddr("glTranslatef");
	  vid_glScalef = vid_getGLFuncAddr("glScalef");
	  vid_glVertex3f = vid_getGLFuncAddr("glVertex3f");
	  vid_glRotatef = vid_getGLFuncAddr("glRotatef");
	}
    }
  /*
    opengl stuff end
  */
#endif

  vid_setMode(cfg->screen_x, cfg->screen_y);
 
  // get name of video driver 
  SDL_VideoDriverName(drivername, SMALL_STRSIZE);

  // set window title ...
  SDL_WM_SetCaption("omron++", NULL);
}






void vid_setMode(int width, int height)
{
  if(!cfg->opengl)
    {
      screen = SDL_SetVideoMode(width, height, cfg->screen_bpp, videoflags);
      if(!screen)
	{
	  ut_log( "ERROR: could not open window: %s\n", SDL_GetError());
	  exit(EXIT_FAILURE);
	}
    }
#ifdef HAVE_OPENGL
  else
    {
      /*
	here we have the opengl screen, but we can only use this
	with opengl drawing functions, so we have the usual 2D screen
	setup as well, to blit this on the gl screen later on...
      */
      gl_screen = SDL_SetVideoMode(width, height, cfg->screen_bpp, videoflags);
      if(!gl_screen)
	{
	  ut_log( "ERROR: could not open window: %s\n", SDL_GetError());
	  exit(EXIT_FAILURE);
	}

      SDL_FreeSurface(teximage);
      teximage = NULL; // so we get a new one in loadTexture()
      SDL_FreeSurface(screen);
      
      screen = SDL_CreateRGBSurface(SDL_SRCALPHA, width, height, cfg->screen_bpp,
				    0, 0, 0, 0);

     
      /*
	set/restore opengl context after SDL_SetVideoMode()
      */
      
      // Note, there may be other things you need to change,
      // depending on how you have your OpenGL state set up.
      vid_glEnable(GL_DEPTH_TEST);
      //vid_glDisable(GL_CULL_FACE);
      //vid_glEnable(GL_CULL_FACE);
      vid_glEnable(GL_TEXTURE_2D);

      // This allows alpha blending of 2D textures with the scene
      vid_glEnable(GL_BLEND);
      vid_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      vid_glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

      vid_glViewport(0, 0, gl_screen->w, gl_screen->h);
      
      // reset projection matrix
      vid_glMatrixMode(GL_PROJECTION);
      vid_glLoadIdentity();
      //vid_glOrtho( -2, 2, 1.5, -1.5, 2.5, 8.5 );
      vid_glFrustum( -2, 2, -1.5, 1.5, 20, 30 );

      //vid_glOrtho(0.0, (GLdouble)gl_screen->w, (GLdouble)gl_screen->h, 0.0, -20, 20);
      //vid_glFrustum(0.0, (GLdouble)gl_screen->w, (GLdouble)gl_screen->h, 0.0, 2.5, 8.5); 


      // reset modelview matrix
      vid_glMatrixMode(GL_MODELVIEW);
      vid_glLoadIdentity();
      vid_glTranslatef(0,0,-22);
      vid_glScalef(1.6, 1.6, 1);
      vid_glRotatef(10, 0, 1, 0); 
      vid_glRotatef(10, 1, 0, 0); 
    }
#endif

  // update cfg
  cfg->screen_x = width;
  cfg->screen_y = height;
}





void vid_flip()
{
  if(!cfg->opengl)
    SDL_Flip(screen);
#ifdef HAVE_OPENGL
  else
    {
      static GLfloat red = 0.1, delta = 0.02;
      static GLfloat ani_angle = 1, ani_steps = 0;
      static GLenum gl_error;
      static GLfloat texcoord[4];
      static GLfloat texMinX, texMinY, texMaxX, texMaxY;

      vid_loadTexture(screen, texcoord);

      // Make texture coordinates easy to understand 
      texMinX = texcoord[0];
      texMinY = texcoord[1];
      texMaxX = texcoord[2];
      texMaxY = texcoord[3];

      // pulsing background
      red += delta;
      if(red >= 0.5 || red <= 0.1)
	delta = -delta;
     
      vid_glClearColor( red, 0.0, 0.0, 0.7 );
      vid_glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
      vid_glMatrixMode(GL_MODELVIEW);
      //vid_glPushMatrix();
      //vid_glLoadIdentity();
      //vid_glTranslatef(0,0,-4.5);
      
      if(ani_steps < 100)
	++ani_steps;
      else
	{
	  ani_steps = 0;
	  vid_glLoadIdentity();
	  vid_glTranslatef(0,0,-22);
	  vid_glScalef(1.6, 1.6, 1);
	  vid_glRotatef(10, 0, 1, 0); 
	  vid_glRotatef(10, 1, 0, 0); 
	}




      if(ani_steps < 25) // left
	vid_glRotatef(-ani_angle, 0, 1, 0); 
      
      if(ani_steps >= 25 && ani_steps < 50) // up
	vid_glRotatef(-ani_angle, 1, 0, 0); 
      
      if(ani_steps >= 50 && ani_steps < 75) // right
	vid_glRotatef(ani_angle, 0, 1, 0.465); 
      
      if(ani_steps >= 75 && ani_steps < 100) // down
	vid_glRotatef(ani_angle, 1, 0, 0); 
	


      // Show the image on the screen
      vid_glBindTexture(GL_TEXTURE_2D, texture);
      vid_glBegin(GL_TRIANGLE_STRIP);

      vid_glTexCoord2f(texMinX, texMinY); vid_glVertex3f(-1,  1,  1 ); // upper left
      vid_glTexCoord2f(texMinX, texMaxY); vid_glVertex3f(-1, -1,  1);  // lower left
      vid_glTexCoord2f(texMaxX, texMinY); vid_glVertex3f( 1,  1,  1);  // upper right
      vid_glTexCoord2f(texMaxX, texMaxY); vid_glVertex3f( 1, -1,  1);  // lower right

      vid_glEnd();

      vid_glBegin( GL_QUADS ); 

      vid_glColor3f(0, 0,   1  );
      vid_glVertex3f( 1,  1, -1);
      vid_glVertex3f( 1,  1,  1);
      vid_glVertex3f( 1, -1,  1);
      vid_glVertex3f( 1, -1, -1);
 
      vid_glColor3f(1, 1,   0  );
      vid_glVertex3f( 1, -1, -1);
      vid_glVertex3f( 1, -1,  1);
      vid_glVertex3f(-1, -1,  1);
      vid_glVertex3f(-1, -1, -1);
 
      vid_glColor3f(0, 1, 0 );
      vid_glVertex3f(-1, -1, -1);
      vid_glVertex3f(-1, -1,  1);
      vid_glVertex3f(-1,  1,  1);
      vid_glVertex3f(-1,  1, -1);
 
      vid_glColor3f(1, 0.1, 0.8);
      vid_glVertex3f( 1,  1,  1);
      vid_glVertex3f( 1,  1, -1);
      vid_glVertex3f(-1,  1, -1);
      vid_glVertex3f(-1,  1,  1);
 
      vid_glEnd();


      //vid_glPopMatrix();


      SDL_GL_SwapBuffers();

      // Check for GL error conditions. 
      gl_error = vid_glGetError();
      if( gl_error != GL_NO_ERROR ) 
	ut_log( "WARNING: OpenGL error: %d\n", gl_error );
    }
#endif
}




void vid_cls()
{
  // blacken screen...
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0,0,0)); 
  vid_flip();
}






void vid_printInfo()
{
  const SDL_VideoInfo *vidinfo = SDL_GetVideoInfo();

  ut_log("\nVIDEO INFORMATION:\n^^^^^^^^^^^^^^^^^\n");
  ut_log("using video driver: %s\n", drivername);
  ut_log("current display: %d bits-per-pixel.\n",vidinfo->vfmt->BitsPerPixel);
  ut_log("\na window manager is %savailable.\n\n", vidinfo->wm_available ? "" : "NOT " ); 

#ifdef HAVE_OPENGL
  if(cfg->opengl)
    {
      ut_log("OpenGL Info:\n");
      ut_log("Vendor     : %s\n", vid_glGetString( GL_VENDOR ) );
      ut_log("Renderer   : %s\n", vid_glGetString( GL_RENDERER ) );
      ut_log("Version    : %s\n", vid_glGetString( GL_VERSION ) );
      int value;
      SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &value );
      ut_log("Double Buffering:      %s\n", value ? "enabled" : "off");
      SDL_GL_GetAttribute( SDL_GL_ACCELERATED_VISUAL, &value );
      ut_log("Hardware Accelaration: %s\n", value ? "enabled":"off" );
    }
#endif
}





SDL_Surface* vid_copySurface(SDL_Surface *src)
{
  // takes a surface and copies it to a new surface of the pixel format and colors of the video framebuffer,
  // suitable for fast blitting onto the display surface
  return SDL_DisplayFormat(src);
}




void vid_toggleFullscreen()
{
  //toggle mouse cursor
  if(SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE)
    SDL_ShowCursor(SDL_DISABLE);
  else
    SDL_ShowCursor(SDL_ENABLE);

  // get a copy of current screen
  SDL_Surface *backup = vid_copySurface(screen);
  
  // this corrupts the pixeldata in screen ...
  videoflags ^= SDL_FULLSCREEN;
  cfg->fullscreen = !cfg->fullscreen;
  vid_setMode(cfg->screen_x, cfg->screen_y);
  
  // ... so restore from the backup
  SDL_BlitSurface(backup, NULL, screen, NULL);

  
  SDL_FreeSurface(backup);
}



void vid_takeScreenshot()
{
  // should be enough ...
  char ssfilename[BIG_STRSIZE];
  char date[SMALL_STRSIZE];
  time_t t; time_t *tp = &t; 

  // get time
  time(tp);

  // convert to date-string 
  strftime(date, SMALL_STRSIZE, "%d.%m.%Y-%H.%M.%S", localtime(tp));

  snprintf(ssfilename, BIG_STRSIZE, "omron++%s.bmp", date);

  if (SDL_SaveBMP(screen, ssfilename) < 0) 
    ut_log( "NOTICE: could not take screenshot.\n");
}




#ifdef HAVE_OPENGL
// fills the globals texture and teximage
GLuint vid_loadTexture(SDL_Surface *surface, GLfloat *texcoord)
{
  int tex_w, tex_h;
 

  /* Use the surface width and height expanded to powers of 2 */
  tex_w = 1;
  while(tex_w < surface->w)
    tex_w <<= 1;
  tex_h = 1;
  while(tex_h < surface->h)
    tex_h <<= 1;


  texcoord[0] = 0.0f;			        /* Min X */
  texcoord[1] = 0.0f;			        /* Min Y */
  texcoord[2] = (GLfloat)surface->w / tex_w;	/* Max X */
  texcoord[3] = (GLfloat)surface->h / tex_h;	/* Max Y */

  if(!teximage)
    {
      teximage = SDL_CreateRGBSurface(
				      SDL_SWSURFACE,
				      tex_w, tex_h,
				      32, 
#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
				      0x000000FF, 
				      0x0000FF00, 
				      0x00FF0000, 
				      0xFF000000
#else
				      0xFF000000,
				      0x00FF0000, 
				      0x0000FF00, 
				      0x000000FF
#endif
				      );

      if(!teximage) 
	return 0;
    }
	

  // Copy the surface into the GL texture image 
  SDL_BlitSurface(surface, NULL, teximage, NULL);


  // Create an OpenGL texture for the image 
  if(!texture)
    vid_glGenTextures(1, &texture);

  vid_glBindTexture(GL_TEXTURE_2D, texture);
  vid_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  vid_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  vid_glTexImage2D(GL_TEXTURE_2D,
		   0,
		   GL_RGBA,
		   tex_w, tex_h,
		   0,
		   GL_RGBA,
		   GL_UNSIGNED_BYTE,
		   teximage->pixels);
	
  return texture;
}



void* vid_getGLFuncAddr(const char* p)
{
	void* f=SDL_GL_GetProcAddress(p);
	if (f)
		return f;
	else
	  {		
	    ut_log("ERROR: Unable to get OpenGL function pointer for %s\n",p);
	    exit(EXIT_FAILURE);
	  }

	return NULL;
}
#endif // HAVE_OPENGL












