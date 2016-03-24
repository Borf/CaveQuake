/* Aftershock 3D rendering engine
 * Copyright (C) 1999 Stephen C. Taylor
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
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "util.h"
#include "uicommon.h"
#include "renderhud.h"
#include "rendercontext.h"
#include "globalshared.h"

#define MOUSE_MASK (ButtonPressMask | ButtonReleaseMask | \
		    ButtonMotionMask | PointerMotionHintMask)
#define KEY_MASK (KeyPressMask | KeyReleaseMask)

global_shared_t *g;
static r_context_t *r_context;
static double starttime;
static Display *dpy=NULL;
static Window win;
static GLXContext ctx;

/* use regular memory */
void *gc_malloc(size_t size) { return malloc(size); }
void *rc_malloc(size_t size) { return malloc(size); }
void gc_free(void *mem) { free(mem); }
void rc_free(void *mem) { free(mem); }

/* TODO: add resize func */
static double gettime(void);
static void display(void);
static void create_window(int w, int h);
static void close_window(void);
static void mainloop(void);

int
main(int argc, char **argv)
{
    /* setup global data structures. */
    g = (global_shared_t *) gc_malloc(sizeof(global_shared_t));
    init_global_shared(g);
    ui_read_args(argc, argv);
    strcpy(g->r_help_fname, "paul/helpGL.jpg");

    /* load map */
    ui_init_bsp();
    printf("\n\n");

    create_window(g->r_viewport_w, g->r_viewport_h);

    /* setup rendering context. */
    r_context = (r_context_t *) rc_malloc(sizeof(r_context_t));
    ui_init_gl(r_context);

    starttime = gettime();
    
    XMapWindow(dpy, win);
    XFlush(dpy);
    mainloop();

    close_window();
    return 0;
}

static double
gettime(void)
{
    struct timeval tv;
    gettimeofday(&tv, (struct timezone *)NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec*0.000001;
    
}

static void
display(void)
{
    double time;

    time = gettime() - starttime;
    ui_move(time);
    ui_display(r_context, time);    
    hud_render(r_context);
    glXSwapBuffers(dpy, win);
}

static void
create_window(int w, int h)
{
    int screen;
    XSetWindowAttributes attr;
    Window root;
    unsigned long mask;
    XVisualInfo *visinfo;
    XTextProperty textprop;
    char *name[1] = {""};
    
    int attrib[] = { GLX_RGBA,
		     GLX_RED_SIZE, 1,
		     GLX_GREEN_SIZE, 1,
		     GLX_BLUE_SIZE, 1,
		     GLX_DEPTH_SIZE, 1,
		     GLX_DOUBLEBUFFER,
			None,
		     None };

    if (!(dpy=XOpenDisplay(NULL)))
	Error("Can't open display");

    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);
    visinfo = glXChooseVisual(dpy, screen, attrib);
    if (!visinfo)
	Error("Can't get visual");

    attr.background_pixel=0;
    attr.border_pixel=0;
    attr.colormap=XCreateColormap(dpy,root,visinfo->visual,AllocNone);
    attr.event_mask = MOUSE_MASK | KEY_MASK | StructureNotifyMask;
    mask=CWBackPixel|CWBorderPixel|CWColormap|CWEventMask;

    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, visinfo->depth,
			InputOutput, visinfo->visual, mask, &attr);

    XStringListToTextProperty(name, 1, &textprop);
    XSetWMName(dpy, win, &textprop);
    
    ctx = glXCreateContext(dpy, visinfo, NULL, True);
    glXMakeCurrent(dpy, win, ctx);
}

static void
close_window(void)
{
    glXDestroyContext(dpy, ctx);
}

static void
mainloop(void)
{
    XEvent ev, next;
    unsigned char key[1];
    int rc;

    for (;;)
    {
	if (XPending(dpy))
	{
	    XNextEvent(dpy, &ev);

	    switch (ev.type)
	    {
		case DestroyNotify:
		    return;
	
		case ButtonPress:
		    ui_mouse_down(ev.xbutton.x, ev.xbutton.y);
		    break;
		    
		case MotionNotify:
		{
		    int x, y;
		    
		    if (ev.xmotion.is_hint)
		    {
			int rx, ry;
			Window root, child;
			uint_t mask;
			XQueryPointer(dpy, win, &root, &child, &rx, &ry,
				      &x, &y, &mask);
		    }
		    else
		    {
			x = ev.xmotion.x;
			y = ev.xmotion.y;
		    }
		    ui_mouse_motion(x, y);
		    break;
		}

		case KeyPress:
		    rc = XLookupString(&ev.xkey, key, 1, NULL, NULL);
		    ui_key_down(key[0]);
		    break;

		case KeyRelease:
		{
		    /* Ignore key repeats (KeyRelease/KeyPress pairs) */
		    if (XPending(dpy))
		    {
			XPeekEvent(dpy, &next);
			if (next.type == KeyPress &&
			    next.xkey.keycode == ev.xkey.keycode &&
			    next.xkey.time == ev.xkey.time)
			{
			    XNextEvent(dpy, &next);
			    break;
			}
		    }
		    rc = XLookupString(&ev.xkey, key, 1, NULL, NULL);
		    ui_key_up(key[0]);
		    break;
		}
	    }
	}
	else
	    display();
    }
}


