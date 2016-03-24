# CQ3A Makefile

# CHOOSE INTERFACE:

# VISBOX
#MAINOBJ = uivisbox.o
#MAININC = `sdl-config --cflags` -I$(HOME)/visbox
#MAINLIB = `sdl-config --libs` -L${HOME}/visbox -lbox

# CAVELIB
#MAINOBJ = uicave.o
#MAININC = -I/usr/local/CAVE/include
#MAINLIB = -L/usr/local/CAVE/lib -lcave_ogl_n32

# JUGGLER
MAINOBJ = uijuggler.o
MAININC = -I$(VJ_BASE_DIR)/include
MAINLIB = -L$(VJ_BASE_DIR)/lib$(LIBBITSUF) -lJuggler -lJuggler_utils -lJuggler_ogl -Wl,-rpath,$(VJ_BASE_DIR)/lib$(LIBBITSUF)

# FREEVR
#MAINOBJ = uifvr.o
#MAININC = -I/usr/local/freevr/src
#MAINLIB = -L/usr/local/freevr/src -lfreevr

# MPK
#MAINOBJ = uimpk.o
#MAINLIB = -lmpk -lpthread

# SDL
#MAINOBJ = uisdl.o
#MAININC = `sdl-config --cflags`
#MAINLIB = `sdl-config --libs`

# GLUT
#MAINOBJ = uiglut.o
#MAININC = 
#MAINLIB = -lglut
#MAININC = -I/local/people/prajlich/glut-3.7/include
#MAINLIB = -L/local/people/prajlich/glut-3.7/lib/glut.n32 -lglut

# GLUT (OS X)
#MAINOBJ = uiglut.o
#MAININC = -F/System/Library/Frameworks/GLUT.framework
#MAINLIB = -framework GLUT

# GLX
#MAINOBJ = uiglx.o
#MAINLIB =


OBJS = util.o bsp.o vec.o render.o tex.o lightmap.o mesh.o \
	pak.o shader.o renderback.o skybox.o entity.o md3.o \
	mapent.o uicommon.o unzip.o globalshared.o renderhud.o \
	$(MAINOBJ)

EXLIBS = -lGL -lGLU -lX11 -lXi -lXmu -lXext

LINK = $(CC)



# CHOOSE PLATFORM:

# IRIX
#INC = -Izlib-1.1.3
#LIB = -Lzlib-1.1.3
#OPT = -g -woff 1275,1183,1552
#DEFS = -DASHOCK_BIG_ENDIAN -DNO_MULTITEX -DNO_CVA
#CC = cc
# for juggler
#OPT = -ptv -w2 -O2 -gslim -woff 1552,1164,1275,1183,1515,1164,1174
#CC = CC -LANG:std
#CXX = CC -LANG:std
#LIBBITSUF = 32
#LINK = $(CXX)

# Linux
#INC = -I/usr/X11R6/include
#LIB = -L/usr/X11R6/lib
#OPT = -g -Wall
# Default requires cube map or texgen extension
#DEFS =
# Software reflection vectors - slower but better, and works on all hardware
DEFS = -DSW_REFLECTION
CC = gcc
#CXX = g++

# FreeBSD
#INC = -I/usr/X11R6/include -I/usr/local/include
#LIB = -L/usr/X11R6/lib -L/usr/local/lib
#EXLIBS = -lGL -lGLU -lX11 -lXi -lXmu -lXext -lcompat
#OPT = -g -Wall
#DEFS =
#CC = gcc
#CXX = g++

# Win32
# download win32utils.zip from CQ3A site
# it contains MSVC 6.0 project file and precompiled zlib and jpeglib

# OS X
#INC = -F/System/Library/Frameworks/OpenGL.framework -I/usr/local/include
#LIB = -L/usr/local/lib 
#EXLIBS = -framework OpenGL
#OPT = 
#DEFS = -DASHOCK_BIG_ENDIAN -DMACOSX
#CC = cc

# HP-UX
#INC = -I/opt/graphics/OpenGL/include/ -I/opt/jpeg-6/include -I/opt/zlib/include -I/opt/glut/include -I/usr/contrib/X11R6/include/X11/Xmu/
#LIB = -L/opt/graphics/OpenGL/lib/ -L/usr/contrib/X11R6/lib/ -L/opt/jpeg-6/lib -L/opt/zlib/lib
#OPT =
#DEFS = -DASHOCK_BIG_ENDIAN -DNO_MULTITEX
#CC = /opt/ansic/bin/cc

# Solaris
#INC = -I/usr/openwin/include -I/usr/local/include
#LIB = -L/usr/openwin/lib -L/usr/local/lib -lm -ldl -lrt
#OPT = -fast -native
#DEFS = -DASHOCK_BIG_ENDIAN -DNO_MULTITEX -DNO_CVA
#CC = cc
#CXX = CC


# You shouldn't have to change anything below this line
CFLAGS = $(INC) $(OPT) $(DEFS) $(MAININC)
LIBS = $(LIB) $(MAINLIB) $(EXLIBS) -lm -ljpeg -lz

all: cq3a

.c.o:
	$(CC) $(CFLAGS) -c $<

.cpp.o:
	$(CXX) $(CFLAGS) -c $<

cq3a: $(OBJS)
	$(LINK) $(OPT) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f cq3a *.o

