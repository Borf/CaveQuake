/* CAVElib interface
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
/* #include <cave.h> */
#include <cave_ogl.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <GL/gl.h>
#include <string.h>
#include "vec.h"
#include "uicommon.h"
#include "renderhud.h"
#include "rendercontext.h"
#include "globalshared.h"

#ifdef WIN32
#define fabsf(x) fabs(x)
#endif

/* Uncomment this code if compiling against the CAVELib Win32 2.8 release */
/*
#if (CAVE_THREAD && WIN32)
#define CAVE_MAX_INDEXES CAVE_MAX_THREADS
#define CAVE_INDEX       CAVETID
#endif
*/

global_shared_t *g;
#ifndef CAVE_THREAD
static r_context_t *r_context;
#else
static r_context_t *r_context[CAVE_MAX_INDEXES];
#endif
static float *cqtime;

/* use shared memory for global shared data */
#ifndef CAVE_THREAD
void *gc_malloc(size_t size) { return CAVEMalloc(size); }
void *rc_malloc(size_t size) { return malloc(size); }
void gc_free(void *mem) { CAVEFree(mem); }
void rc_free(void *mem) { free(mem); }
#else
// use regular memory 
void *gc_malloc(size_t size) { return malloc(size); }
void *rc_malloc(size_t size) { return malloc(size); }
void gc_free(void *mem) { free(mem); }
void rc_free(void *mem) { free(mem); }
#endif

static void renderProcInit();
static void display();
static void update();

int
main(int argc, char **argv)
{
    /* default shared mem arena is only 8 MB! */
    /* CAVESetOption(CAVE_SHMEM_SIZE, 64*1024*1024); */
    CAVEConfigure(&argc, argv, NULL);
    CAVEFar = 1000.0f;

    /* setup shared data structures */
    g = (global_shared_t *) gc_malloc(sizeof(global_shared_t));
    init_global_shared(g);
    strcpy(g->r_help_fname, "paul/helpVR.jpg");
    ui_read_args(argc, argv);

    /* load the map */
    ui_init_bsp();
    printf("\n\n");

    /* CAVElib will setup projection(s) */
    g->r_setup_projection = 0;
    g->r_calc_fps = 0;

    cqtime = (float *) gc_malloc(sizeof(float));
    *cqtime = 0;

    CAVEInitApplication(renderProcInit, 0);
    CAVEDisplay(display, 0);
    CAVEInit();

    while (!CAVEgetbutton(CAVE_ESCKEY)) {
      update();
#ifndef WIN32
      usleep(10000); /* 10 ms */
#else
      Sleep(10);
#endif
    }
    CAVEExit();

    /* ui_finish(); */
    return 0;
}

static void
renderProcInit()
{
#ifndef CAVE_THREAD
  r_context = (r_context_t *) rc_malloc(sizeof(r_context_t));
  ui_init_gl(r_context);
#else
  r_context[CAVE_INDEX] = (r_context_t *) rc_malloc(sizeof(r_context_t));
  ui_init_gl(r_context[CAVE_INDEX]);
#endif
}

static void
display()
{
#ifndef CAVE_THREAD
  ui_display(r_context, *cqtime);
  if (CAVEMasterDisplay())
    hud_render(r_context);
#else
  ui_display(r_context[CAVE_INDEX], *cqtime);
  if (CAVEMasterDisplay())
    hud_render(r_context[CAVE_INDEX]);
#endif
  glDisable(GL_BLEND);
}

static void
update()
{
  static float lastTime = 0.0f;
  float etime;
  float wandO[3];
  vec3_t dir;

  *cqtime = CAVEGetTime();
  etime = *cqtime - lastTime;

  if (etime > 0.2f)
    etime = 0.2f; /* cap */

  /* rotate CAVE */
  if (fabsf(CAVE_JOYSTICK_X) > 0.3f)
    g->r_eye_az = g->r_eye_az - CAVE_JOYSTICK_X*etime*50.0f;

  /* move forward/backward */
  if (CAVE_JOYSTICK_Y > 0.5f)        ui_key_down('w');
  else if (CAVE_JOYSTICK_Y < -0.5f)  ui_key_down('s');
  else if (CAVEgetbutton(CAVE_WKEY)) ui_key_down('w');
  else if (CAVEgetbutton(CAVE_SKEY)) ui_key_down('s');
  else if (CAVEBUTTON1)              ui_key_down('w');
  else
    ui_key_up('w');

  /* reset */
  if (CAVEBUTTON2 && CAVEBUTTON3)
    ui_reset();

  /* show help page */
  if (CAVEgetbutton(CAVE_HKEY))
    ui_key_down('h');

  /* change sky */
  if (CAVEgetbutton(CAVE_BKEY) || CAVEButtonChange(3)==1)
    ui_key_down('b');

  /* turn on/off collision detection */
  if (CAVEgetbutton(CAVE_CKEY) || CAVEButtonChange(2)==1)
    ui_key_down('m');

  /* travel in direction wand is pointing */
  CAVEGetOrientation(CAVE_WAND, wandO);
  vec_point(dir, wandO[1]+g->r_eye_az, wandO[0]);
  /* note: keylook disables wand navigation */
  if (!g->g_keylook) {
    g->r_movedir[0] = dir[0];
    g->r_movedir[1] = dir[1];
    g->r_movedir[2] = dir[2];
  }

  /* SG - keyboard navigation start */
  if (g->g_keylook) {
    if (CAVEgetbutton(CAVE_PAD5)) ui_key_down('5');
    if (CAVEgetbutton(CAVE_PAD2)) ui_key_down('2');
    if (CAVEgetbutton(CAVE_PAD4)) ui_key_down('4');
    if (CAVEgetbutton(CAVE_PAD6)) ui_key_down('6');
    if (CAVEgetbutton(CAVE_PAD8)) ui_key_down('8');
    if (!CAVEgetbutton(CAVE_PAD2) && !CAVEgetbutton(CAVE_PAD8)) ui_key_up('2');
    if (!CAVEgetbutton(CAVE_PAD4) && !CAVEgetbutton(CAVE_PAD6)) ui_key_up('4');
  }
  if(CAVEgetbutton(CAVE_XKEY)) ui_key_down('x');
  /* SG - keyboard navigation end */

  g->r_fps = *CAVEFramesPerSecond;
  ui_move(*cqtime);

  lastTime = *cqtime;
}
