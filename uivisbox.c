/* VisBOX interface
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
#include <stdio.h>
#include <string.h>

#include "glbox.h"

#include "uicommon.h"
#include "renderhud.h"
#include "rendercontext.h"
#include "globalshared.h"

global_shared_t *g;
static r_context_t *r_context;
static double starttime, time;

/* use regular memory */
void *gc_malloc(size_t size) { return malloc(size); }
void *rc_malloc(size_t size) { return malloc(size); }
void gc_free(void *mem) { free(mem); }
void rc_free(void *mem) { free(mem); }

static void display(void);
static void update(void);
static void keyDown(int key);
static void keyUp(int key);

int main(int argc, char **argv)
{
  /* setup global data structures. */
  g = (global_shared_t *) gc_malloc(sizeof(global_shared_t));
  init_global_shared(g);
  strcpy(g->r_help_fname, "paul/helpSDL.jpg");
  ui_read_args(argc, argv);

  g->r_setup_projection = 0;

  /* load map */
  ui_init_bsp();
  printf("\n\n");

  BOXInit();

  /* setup rendering context. */
  r_context = (r_context_t *) rc_malloc(sizeof(r_context_t));
  ui_init_gl(r_context);

  BOXSetAppCB(update);
  BOXSetRenderCB(display);

#if 0
  BOXSetKeyDownCB(ui_key_down);
  BOXSetKeyUpCB(ui_key_up);
#endif
  BOXSetKeyDownCB(keyDown);
  BOXSetKeyUpCB(keyUp);
  BOXSetMouseCB(ui_mouse_relmotion);

  starttime = BOXGetTime();
  BOXMainLoop();

  BOXExit();
  return 0;
}

static void update()
{
    float lx, ly, rx, ry, px, py;
    time = BOXGetTime() - starttime;
    ui_move(time);

    if (BOXGetWingEnabled()) {
      lx = BOXGetWingAxis(LEFT_X);
      ly = BOXGetWingAxis(LEFT_Y);
      rx = BOXGetWingAxis(RIGHT_X);
      ry = BOXGetWingAxis(RIGHT_Y);
      px = BOXGetWingAxis(PAD_X);
      py = BOXGetWingAxis(PAD_Y);

      ui_mouse_relmotion(rx*5, -ry*5);
      if (ly >  0.5 || py >  0.5) keyDown('w');
      else if (ly < -0.5 || py < -0.5) keyDown('s');
      else keyUp('w');
    }
}

static void display()
{
    ui_display(r_context, time);
    hud_render(r_context);
}

static void keyDown(int key)
{
    if (key == SDLK_UP) ui_key_down('w');
    else if (key == SDLK_DOWN) ui_key_down('s');
    else ui_key_down(key);
}

static void keyUp(int key)
{
    if (key == SDLK_UP) ui_key_up('w');
    else if (key == SDLK_DOWN) ui_key_up('s');
    else ui_key_up(key);
}
