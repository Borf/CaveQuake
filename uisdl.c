/* SDL interface
 * Copyright (C) 2001 Paul Rajlich
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include "SDL.h"
#else
#include <SDL/SDL.h>
#endif

#include "glinc.h"
#include "uicommon.h"
#include "renderhud.h"
#include "rendercontext.h"
#include "globalshared.h"

global_shared_t *g;
static r_context_t *r_context;
static double starttime;
static int bpp=16, done=0, warp=0;

/* use regular memory */
void *gc_malloc(size_t size) { return malloc(size); }
void *rc_malloc(size_t size) { return malloc(size); }
void gc_free(void *mem) { free(mem); }
void rc_free(void *mem) { free(mem); }

static void display(void);
void select_video();
void handle_event(SDL_Event event);

int
main(int argc, char **argv)
{
  Uint32 videoFlags = SDL_OPENGL | SDL_FULLSCREEN;
  SDL_Event event;

  /* setup global data structures. */
  g = (global_shared_t *) gc_malloc(sizeof(global_shared_t));
  init_global_shared(g);
  strcpy(g->r_help_fname, "paul/helpSDL.jpg");
  select_video();
  ui_read_args(argc, argv);

  /* load map */
  ui_init_bsp();
  printf("\n\n");

  /* Initialize SDL for video output */
  if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 ) {
      fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
      exit(1);
  }

  /* Create an OpenGL screen */
  if ( SDL_SetVideoMode(g->r_viewport_w, g->r_viewport_h, bpp, videoFlags) == NULL ) {
      fprintf(stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError());
      SDL_Quit();
      exit(2);
  }

  /* SDL_WM_GrabInput(SDL_GRAB_ON); */
  SDL_ShowCursor(0);

  /* setup rendering context. */
  r_context = (r_context_t *) rc_malloc(sizeof(r_context_t));
  ui_init_gl(r_context);

  starttime = (double) SDL_GetTicks() / 1000.0;

  /* center mouse */
  SDL_WarpMouse(g->r_viewport_w/2, g->r_viewport_h/2); /* center mouse */
  warp = 1;		

  while ( !done) {
    display();	
    while ( SDL_PollEvent(&event) )
      handle_event(event); 
  }
    
  SDL_Quit();
  return 0;
}

static void
display(void)
{
    double time;

    time = (double) SDL_GetTicks() / 1000.0 - starttime;
    ui_move(time);
    ui_display(r_context, time);
    hud_render(r_context);
    SDL_GL_SwapBuffers();
}

void select_video() {
  char c;	
  printf("\n");
  printf(" Choose video resolution or press ENTER for\n");
  printf(" default (640x480x16).\n");
  printf("\n");
  printf(" 0) 320x240x16      3) 800x600x16\n");
  printf(" 1) 400x300x16      4) 1024x768x16\n");
  printf(" 2) 640x480x16      5) 1280x1024x16\n");
  printf("\n");
  printf(" 6) 640x480x32      8) 1024x768x32\n");
  printf(" 7) 800x600x32      9) 1280x1024x32\n");
  printf("\n");
  printf(" Enter your choice: ");
  fflush(stdin);
  c = getc(stdin);
  printf("\n");
  printf(" Initializing...\n");
  if (c == '0') { g->r_viewport_w = 320;  g->r_viewport_h = 240;  bpp = 16; }
  if (c == '1') { g->r_viewport_w = 400;  g->r_viewport_h = 300;  bpp = 16; }
  if (c == '2') { g->r_viewport_w = 640;  g->r_viewport_h = 480;  bpp = 16; }
  if (c == '3') { g->r_viewport_w = 800;  g->r_viewport_h = 600;  bpp = 16; }
  if (c == '4') { g->r_viewport_w = 1024; g->r_viewport_h = 768;  bpp = 16; }
  if (c == '5') { g->r_viewport_w = 1280; g->r_viewport_h = 1024; bpp = 16; }
  if (c == '6') { g->r_viewport_w = 640;  g->r_viewport_h = 480;  bpp = 32; }
  if (c == '7') { g->r_viewport_w = 800;  g->r_viewport_h = 600;  bpp = 32; }
  if (c == '8') { g->r_viewport_w = 1024; g->r_viewport_h = 768;  bpp = 32; }
  if (c == '9') { g->r_viewport_w = 1280; g->r_viewport_h = 1024; bpp = 32; }
}


void handle_event(SDL_Event event) {
  int key;
  if ( event.type == SDL_QUIT )
    done = 1;
  if ( event.type == SDL_KEYDOWN ) {
    key = event.key.keysym.sym;	
    if ( key == SDLK_ESCAPE || key == SDLK_q) done = 1;
    else if ( key == SDLK_UP)   ui_key_down('w');
    else if ( key == SDLK_DOWN) ui_key_down('s');
    else if ( key == SDLK_r)    starttime = (double) SDL_GetTicks() / 1000.0;
    else 
      ui_key_down(key);
  }
  if ( event.type == SDL_KEYUP ) {
    key = event.key.keysym.sym;			    	
    if ( key == SDLK_ESCAPE || key == SDLK_q) done = 1;
    else if ( key == SDLK_UP)   ui_key_up('w');
    else if ( key == SDLK_DOWN) ui_key_up('s');
    else
      ui_key_up(key);
  }
  if ( event.type == SDL_MOUSEMOTION ) {
    if (!warp)
      ui_mouse_relmotion(event.motion.xrel, event.motion.yrel);
    else
      warp=0;
    if ((event.motion.x < 10) || (event.motion.x > g->r_viewport_w-10) ||
        (event.motion.y < 10) || (event.motion.y > g->r_viewport_h-10)) {
      SDL_WarpMouse(g->r_viewport_w/2, g->r_viewport_h/2); /* center mouse */
      warp = 1;					     /* set flag */	
    }
  }
}

