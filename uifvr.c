/* FreeVR interface
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
/* Updated 11/24/2004 by Bill Sherman for versions of FreeVR >= 0.5 */
#include <freevr.h>
#include <unistd.h>
#include <string.h>
#include <GL/gl.h>
#include <math.h>
#include "vec.h"
#include "uicommon.h"
#include "renderhud.h"
#include "rendercontext.h"
#include "globalshared.h"

global_shared_t *g;
static r_context_t *r_context = NULL;
static double *time;	/* BS: float -> double */

/* use shared memory for global shared data */
void *gc_malloc(size_t size) { return vrShmemAlloc0(size); }
void *rc_malloc(size_t size) { return malloc(size); }
void gc_free(void *mem) { vrShmemFree(mem); }
void rc_free(void *mem) { free(mem); }

static void renderProcInit();
static void display();
static void update();

int
main(int argc, char **argv)
{
    /* default shared mem arena is too small! */
    vrShmemInit(64*1024*1024);
    vrConfigure(NULL, NULL, NULL);
    /* CAVEFar = 1000.0f; */

    /* setup shared data structures */
    g = (global_shared_t *)gc_malloc(sizeof(global_shared_t));
    init_global_shared(g);
    strcpy(g->r_help_fname, "paul/helpVR.jpg");
    ui_read_args(argc, argv);

    /* load the map */
    ui_init_bsp();
    printf("\n\n");

    /* FreeVR will setup projection(s) */
    g->r_setup_projection = 0;
    g->r_calc_fps = 1;	/* BS: this doesn't seem to work! */

    time = (double *) gc_malloc(sizeof(double));
    *time = 0;

    vrFunctionSetCallback(VRFUNC_DISPLAY_INIT, vrCallbackCreate(renderProcInit, 0));
    vrFunctionSetCallback(VRFUNC_DISPLAY, vrCallbackCreate(display, 0));
    vrStart();

    while (!vrGet2switchValue(0)) {
      vrFrame();
      update();
      usleep(10000); /* 10 ms */
    }
    vrExit();

    /* ui_finish(); */
    return 0;
}

static void
renderProcInit()
{
  r_context = (r_context_t *) rc_malloc(sizeof(r_context_t));
  ui_init_gl(r_context);
}

static void
display(void)
{
  glClearDepth(1.0);
  ui_display(r_context, (double)(*time));
  hud_render(r_context);
}

static void
update()
{
  static double	lastTime = 0.0f;
  double	etime;
  float		joyx, joyy;
  vrVector	wand_point;
  double	azcos, azsin;
  vec3_t	dir;

  *time = vrCurrentSimTime();
  etime = *time - lastTime;

  if (etime > 0.2f)
    etime = 0.2f; /* cap */

  joyx = vrGetValuatorValue(0);
  joyy = vrGetValuatorValue(1);

  /* rotate CAVE */
  if (joyx > 0.3f)
    g->r_eye_az -= (joyx-0.3)*etime*50.0f;	/* BS: changed to -= */
  else if (joyx < -0.3f)			/* BS: added to make smooth on/off transition */
    g->r_eye_az -= (joyx+0.3)*etime*50.0f;	/* BS:   ditto */

  /* move forward/backward */
  if (joyy > 0.5f)       		ui_key_down('w');
  else if (joyy < -0.5f)		ui_key_down('s');
  else if (vrGet2switchValue(1))	ui_key_down('w');
  else
    ui_key_up('w');


  /* reset */
  if (vrGet2switchValue(1) && vrGet2switchValue(2) && vrGet2switchValue(3))
    ui_reset();




  /* change sky */
  if (vrGet2switchDelta(3) == 1)
    ui_key_down('b');

  /* turn on/off collision detection */
  if (vrGet2switchDelta(2) == 1)
    ui_key_down('m');

  /* travel in direction wand is pointing */
  #define WAND_SENSOR 1
  /* get the direction the wand is pointing in RW coords */
  vrVectorGetRWFrom6sensorDir(&wand_point, WAND_SENSOR, VRDIR_FORE);

  /* convert to global-world-coords */
  /*  - split the azimuth between the X & Z axes */
  azcos = cos(g->r_eye_az * M_PI / 180.0);
  azsin = sin(g->r_eye_az * M_PI / 180.0);
  dir[0] = ((-wand_point.v[VR_Z] * azcos) + ( wand_point.v[VR_X] * azsin));
  dir[1] = ((-wand_point.v[VR_X] * azcos) + (-wand_point.v[VR_Z] * azsin));
  dir[2] =    wand_point.v[VR_Y];

  g->r_movedir[0] = dir[0];
  g->r_movedir[1] = dir[1];
  g->r_movedir[2] = dir[2];

  ui_move(*time);

  lastTime = *time;
}
