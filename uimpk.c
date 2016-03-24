/* MPSDK interface
 * Copyright (C) 2001 Stefan Eilemann
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

/*
  
   What works/is tested:
     - Stereo
     - Multipipe/frustum
     - 2D

   What is untested (works with 1 processor/1 pipe):
     - SW DPLEX [ASYNC]
     - 3D 
     - 2D async

   What does not work:
     - DB
*/


#include <mpk/mpk.h>

#include "util.h"
#include "uicommon.h"
#include "rendercontext.h"
#include "globalshared.h"

#include <X11/keysym.h>
#include <math.h>
#include <sys/time.h>

#ifndef M_PI
#define M_PI 3.1415926535
#endif

#undef DEG2RAD
#define DEG2RAD(a)  ((a)*M_PI/180.)

/* use shared memory for global shared data */
void *gc_malloc(size_t size) { return mpkMalloc(size); }
void *rc_malloc(size_t size) { return malloc(size); }
void gc_free(void *mem) { mpkFree(mem); }
void rc_free(void *mem) { free(mem); }

typedef struct
{
    int    stereo;
    int    exit;

    double starttime;
} Shared;

typedef struct _FrameData
{
    double time;

    int r_eyecluster;
    vec3_t r_eyepos;
    float r_eye_az, r_eye_el;

    struct _FrameData *next;
} FrameData;

FrameData *frameDataBuffer = NULL;
FrameData *newFrameData( Shared  * );
Shared *shared = NULL;
global_shared_t *g;

void    initSharedData();
void    initWindow( MPKWindow * );
void    initChannel( MPKChannel *c );
void    clearChannel( MPKChannel *, void * );
void    updateChannel( MPKChannel *, void * );
void    freeFrameData( MPKConfig *, void * );

void    windowMouse( MPKWindow *, MPKEvent * );
void    windowExit( MPKWindow *, MPKEvent * );
void    windowKeyboard( MPKWindow *, MPKEvent * );

double  gettime(void);

//---------------------------------------------------------------------------
//  main
//---------------------------------------------------------------------------
main( int argc, char *argv[] )
{
    char *confFile = getenv("MPK_CONFIG_FILE");

    MPKConfig *config;
    FrameData *framedata;

    mpkInit();
    initSharedData();
    if (confFile)
        config = mpkConfigLoad(confFile);
    else        
        config = mpkConfigLoad("/usr/share/Multipipe/src/configs/1-window");

    ui_read_args(argc, argv);
    if ( config == NULL )
    {
        fprintf(stderr, "Can't load config file.\n");
        exit (0);
    }

    /* load map */
    ui_init_bsp();
    g->r_setup_projection = 0;

            mpkConfigOutput( config, 0 );

    shared->stereo = ( mpkConfigGetMode(config) == MPK_STEREO_ON ? 1 : 0 );

    mpkConfigSetWindowInitCB(config,initWindow);
    mpkConfigSetChannelInitCB(config,initChannel);
    mpkConfigSetDataFreeCB( config, freeFrameData );

    mpkConfigInit( config, 0 );

    while (!shared->exit )
    {
        mpkConfigChangeMode( config, shared->stereo );

        framedata = newFrameData( shared );
        mpkConfigFrame( config, framedata );
    }

    //---------- restore MONO

    mpkConfigSetWindowInitCB(config,NULL);
    mpkConfigSetChannelInitCB(config,NULL);
    mpkConfigChangeMode( config, MPK_STEREO_OFF );

    //---------- exit & delete config

    mpkConfigExit( config );
    mpkConfigDelete( config );
}

//---------------------------------------------------------------------------
//  initSharedData
//---------------------------------------------------------------------------
void initSharedData()
{
    shared = (Shared *)mpkMalloc( sizeof(Shared) );

    shared->exit = 0;
    shared->stereo = MPK_STEREO_OFF;
    shared->starttime = gettime();

    /* setup global data structures. */
    g = (global_shared_t *) gc_malloc(sizeof(global_shared_t));
    init_global_shared(g);

}

//---------------------------------------------------------------------------
//  initWindow
//---------------------------------------------------------------------------
void initWindow( MPKWindow *w )
{
    r_context_t *r_context;

    mpkWindowSetEventCB(w,MPK_WINDOW_EVENTCB_MOUSE,windowMouse);
    mpkWindowSetEventCB(w,MPK_WINDOW_EVENTCB_BUTTON,windowMouse);
    mpkWindowSetEventCB(w,MPK_WINDOW_EVENTCB_EXIT,windowExit);
    mpkWindowSetEventCB(w,MPK_WINDOW_EVENTCB_KEYBOARD,windowKeyboard);
    mpkWindowCreate( w );

    // GL initialization
    mpkWindowApplyViewport( w );

    /* setup rendering context. */
    r_context = (r_context_t *) rc_malloc(sizeof(r_context_t));
    ui_init_gl(r_context);
    mpkWindowSetUserData(w, (void *)r_context);
 
    mpkWindowSwapBuffers(w);
}

//---------------------------------------------------------------------------
//  initChannel
//---------------------------------------------------------------------------
void initChannel( MPKChannel *c )
{
    mpkChannelSetDrawCB(c,MPK_CHANNEL_DRAWCB_CLEAR,clearChannel);
    mpkChannelSetDrawCB(c,MPK_CHANNEL_DRAWCB_UPDATE,updateChannel);
    mpkChannelSetNearFar( c, .1, 2000.0 );
}

//---------------------------------------------------------------------------
//  newFrameData
//---------------------------------------------------------------------------
FrameData *newFrameData( Shared *shared )
{
    FrameData *framedata;
    if ( frameDataBuffer == NULL )
    {
        framedata = (FrameData *) mpkMalloc( sizeof(FrameData) );
    }
    else
    {
        framedata = frameDataBuffer;
        frameDataBuffer = framedata->next;
    }
    framedata->next = NULL;

    framedata->time = gettime() - shared->starttime;

    ui_move(framedata->time);
    framedata->r_eyecluster = g->r_eyecluster;
    framedata->r_eyepos[0] = g->r_eyepos[0];
    framedata->r_eyepos[1] = g->r_eyepos[1];
    framedata->r_eyepos[2] = g->r_eyepos[2];
    framedata->r_eye_az    = g->r_eye_az;
    framedata->r_eye_el    = g->r_eye_el;

    return framedata;
}

//---------------------------------------------------------------------------
// freeFrameData
//---------------------------------------------------------------------------
void freeFrameData( MPKConfig *cfg, void *data )
{
    FrameData *framedata = (FrameData *)data;
    framedata->next = frameDataBuffer;
    frameDataBuffer = framedata;
}

//---------------------------------------------------------------------------
//  clearChannel
//---------------------------------------------------------------------------
void clearChannel( MPKChannel *c, void *data )
{
//    FrameData *framedata = (FrameData *)data;

    mpkChannelApplyBuffer( c );
    mpkChannelApplyViewport( c );

    render_clear();
}

//---------------------------------------------------------------------------
//  updateChannel
//---------------------------------------------------------------------------
void updateChannel( MPKChannel *c, void *data )
{
    FrameData *framedata = (FrameData *)data;
    MPKWindow *w = mpkChannelGetWindow(c);
    void *wdata = mpkWindowGetUserData(w);
    r_context_t *r_context = (r_context_t *)wdata;

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    mpkChannelApplyFrustum( c );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    mpkChannelApplyTransformation( c );

    mpkChannelGetRange( c, r_context->range );
    r_context->r_frametime = framedata->time;
    r_context->r_eyecluster = framedata->r_eyecluster;
    r_context->r_eyepos[0] = framedata->r_eyepos[0];
    r_context->r_eyepos[1] = framedata->r_eyepos[1];
    r_context->r_eyepos[2] = framedata->r_eyepos[2];
    r_context->r_eye_az    = framedata->r_eye_az;
    r_context->r_eye_el    = framedata->r_eye_el;

    render_draw(r_context);    
    if (r_context->range[1] == 1.0)
        hud_render(r_context);
}

//---------------------------------------------------------------------------
//  windowMouse
//---------------------------------------------------------------------------
void windowMouse( MPKWindow *w, MPKEvent *event )
{
    XEvent *ev = event->x;
    if(ev->type == ButtonPress)
    {
        ui_mouse_down(event->mouse.xref, event->mouse.yref);
    }
    if( event->button.left )
    {
        ui_mouse_motion(event->mouse.x, event->mouse.y);
    }
}

//---------------------------------------------------------------------------
//  windowExit
//---------------------------------------------------------------------------
void windowExit( MPKWindow *w, MPKEvent *event )
{
    shared->exit = GL_TRUE;
}

//---------------------------------------------------------------------------
//  windowKeyboard
//---------------------------------------------------------------------------
void windowKeyboard( MPKWindow *w, MPKEvent *event )
{
    if ( event->keyboard.state == MPK_PRESS )
    {
        ui_key_down(event->keyboard.key);
        return;
    }

    ui_key_up(event->keyboard.key);

    switch (event->keyboard.key)
    {
        case XK_A:
        case XK_a:
            shared->stereo = !shared->stereo;
            break;
    }

}

// misc functions
double gettime(void)
{
    struct timeval tv;
    gettimeofday(&tv, (struct timezone *)NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec*0.000001;
}
