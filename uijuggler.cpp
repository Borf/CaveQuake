#define GLM_SWIZZLE
#include <stdio.h>

#include <fstream>
// vrJuggler stuff
#include <VrLib/Kernel.h>
#include <VrLib/Application.h>
#include <VrLib/Device.h>
#include <VrLib/Log.h>

#include <glm/gtc/matrix_transform.hpp>

//#include <vpr/Util/Debug.h>
//#include <vpr/Util/Assert.h>

//#include <deprecated/Math/vjMatrix.h>
//#include <deprecated/Math/vjVec3.h>

#include <iostream>

#include "ClDataCWrap.h"
#include "vec.h"
#include "uicommon.h"
#include "rendercontext.h"
#include "globalshared.h"


#include <windows.h> // Windows API
#include <GL/gl.h>


global_shared_t *g;

void printUsage();

// use regular memory (juggler uses threads)
void *gc_malloc(size_t size) { return malloc(size); }
void *rc_malloc(size_t size) { return malloc(size); }
void gc_free(void *mem) { free(mem); }
void rc_free(void *mem) { free(mem); }

// timing
#ifdef WIN32
double gettime()
{
	return ((double)GetTickCount())/1000.0f;
}
#else
double gettime()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    return  ((double) tv.tv_sec + (double) tv.tv_usec / 1000000.0);
}
#endif


// Wrapper class to hold a copy of a context specific data structure
class ContextData
{
public:
   ContextData() : r_context(NULL)
   {
	  r_context = (r_context_t *) rc_malloc(sizeof(r_context_t));
      std::cout << "Allocated ContextData." << std::endl;
   }

   r_context_t* getData()
   { return r_context; }

private:
   r_context_t* r_context;
};


void setStatus(const char* format,...)
{
	char text[2048];
	va_list args;
	va_start (args, format);
	vsprintf (text,format, args);
	va_end (args);

	std::ofstream pFile("d:/cavestatus.txt");
	pFile.write(text, strlen(text));
	pFile.close();
}


// Actual application class
// Just derives from vjGlApp base class and implments the interface methods
// needed to call the rendering engine.
class myApp : public vrlib::Application {
private:
  // see rendercontext.h. Each rendering thread will have its own copy
	//vrj::opengl::ContextData<ContextData> contextData;
	r_context_t	contextData;

public:
	vrlib::PositionalDevice mWand;
	vrlib::DigitalDevice mButton0;
	vrlib::DigitalDevice mButton1;
	vrlib::DigitalDevice mButton2;
	vrlib::DigitalDevice mKeyPageUp;
	vrlib::DigitalDevice mKeyPageDown;
  POINT cursorPos;

  bool stop;

  double starttime, frametime, lastTime;

  double lastQuitKeyTime;
  bool lastQuitKey;

public:
	myApp()
	{
		std::cerr << std::endl;
		std::cerr << "***************************************" << std::endl;
		std::cerr << "bsp: "; //<< bsp << " pak: " << pak << std::endl;
		std::cerr << "***************************************" << std::endl;
		std::cerr << std::endl;
	  }


   // Initialize global data structures
  virtual void init() {



	  mWand.init("WandPosition");
	  mButton0.init("LeftButton");
	  mButton1.init("RightButton");
	  mButton2.init("MiddleButton");
	  mKeyPageDown.init("KeyPageDown");
	  mKeyPageUp.init("KeyPageUp");


	  stop = false;

    // load the map
    ui_init_bsp();
    printf("\n\n");

	CD_init();

    starttime = gettime();
    frametime = starttime;
    lastTime = starttime;
	lastQuitKeyTime = gettime()-10;
	lastQuitKey = false;

    g->r_setup_projection = 0;

	ui_init_gl(&contextData);

  }


virtual void postFrame()
{
	if(mKeyPageUp.getData() == vrlib::TOGGLE_ON && lastQuitKeyTime+.5 < gettime())
	{
		lastQuitKey = false;
		lastQuitKeyTime = gettime();
	}
	if(mKeyPageDown.getData() == vrlib::TOGGLE_ON && lastQuitKeyTime+.5 < gettime())
	{
		lastQuitKey = true;
		lastQuitKeyTime = gettime();
	}
	if(lastQuitKeyTime+.5 > gettime() && (
		(mKeyPageDown.getData() == vrlib::ON && lastQuitKey == false) ||
		(mKeyPageUp.getData() == vrlib::ON && lastQuitKey == true)))
	{
#ifdef NDEBUG
		SetCursorPos(cursorPos.x, cursorPos.y);
#endif
		setStatus("CaveQuake stopped...");
		Application::stop();
		return;
	}
}

  // Execute the actually drawing
	virtual void draw(const glm::mat4 &projectionMatrix, const glm::mat4 &modelviewMatrix) {
	glDepthMask(GL_TRUE);
	glClear(GL_DEPTH_BUFFER_BIT);
    r_context_t* the_context_data = &contextData;

    // Set some state that is expected
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glCullFace(GL_FRONT);
    glShadeModel(GL_SMOOTH);

	//render_clear();
    ui_display(the_context_data, frametime - starttime);
    glDisable(GL_BLEND);
  }


   glm::vec3 getRotation(glm::mat4& matrix, glm::vec3 p1, glm::vec3 p2)
  {
	p1 = glm::vec3(matrix * glm::vec4(p1,1));
	p2 = glm::vec3(matrix * glm::vec4(p2,1));

	glm::vec3 p3 = p1 - p2;

	float xrot = atan2(p3[2],p3[1]);//*57.295779513;
	float yrot = atan2(p3[0],p3[2]);//*57.295779513;
	float zrot = atan2(p3[1],p3[0]);//*57.295779513;

	return glm::vec3(xrot,yrot,zrot);
  }


  // Updates that occur immedately after device updates but before rendering
  // - Update timing
  // - Update navigation
  virtual void preFrame() {
	  if(CD_isLocal()){
		double etime;
		float wandH(0.0), wandP(0.0), wandR(0.0);

		frametime = gettime();
		etime = frametime - lastTime;
		if (etime > 0.2f)
		etime = 0.2f; 

		// move forward
		ui_key_up('w');			
		ui_key_up('s');	


		if (mButton0.getData() == vrlib::ON)
			ui_key_down('w');
		
		else if (mButton1.getData() == vrlib::ON)
			ui_key_down('s');

				// change sky
		//if (mButton2.getData() == TOGGLE_ON)
		//ui_key_down('b');

		// turn on/off collision detection
		//if (mButton1.getData() == TOGGLE_ON)
		//ui_key_down('m');

		// travel in direction wand is pointing
		//vrj::Matrix wandMat = mWand.getData();
		glm::mat4 wandMat = mWand.getData();
		
		//code johan talboom: neem een vector omhoog, draai deze terug om de Y-as, en meet dan de hoek op de Z-as voor de draaiing van de wand
		{
			glm::vec4 up = wandMat * glm::vec4(0,1,0, 0);
			glm::vec3 rot1 = getRotation(wandMat, glm::vec3(0,0,0), glm::vec3(0,0,1));
			up = glm::rotate(glm::mat4(), glm::radians(-rot1[1]), glm::vec3(0,1,0)) * up;
			wandH = glm::degrees(atan2(up[0], up[1]));

			if(wandH < 0)
				wandH = -180 - wandH;
			else
				wandH = 180 - wandH;
		}


		glm::mat4 correction_mat = glm::rotate(glm::mat4(), glm::radians(g->r_eye_az), glm::vec3(0,1,0));
		glm::vec4 eye_dir = correction_mat * wandMat * glm::vec4(0.0f, 0.0f, -1.0f, 0.0);

		// Flip to engine coordinate system
		g->r_movedir[0] = -eye_dir[2];    // +z
		g->r_movedir[1] = -eye_dir[0];    // +x
		g->r_movedir[2] = eye_dir[1];     // -y


//		g->r_eye_az += etime*1.5f;
		// Rotate the world
		const float az_scale(1.5f);
		if (fabsf(wandH) < 170.0f)
		{
			g->r_eye_az += (float)(wandH*etime*az_scale);
		}
		static double startWobble = 0;
		if(mButton2.getData() == vrlib::TOGGLE_ON)
		{
			startWobble = gettime();
		}
		
		if(startWobble == 0)
			g->r_eye_roll = 0;
		else
		{
			double diff = gettime()-startWobble;
			g->r_eye_roll = 10 * sin(diff);
			if(diff > 4*3.1415)
				startWobble = 0;
		}

		//else vjDEBUG(0,0) << "Clipping az because wandP is --> " << wandP << std::endl << vjDEBUG_FLUSH;

		ui_move(frametime - starttime);

		lastTime = frametime;
	}
	
	CD_setData(g->r_eyepos[0],g->r_eyepos[1],g->r_eyepos[2],g->r_eye_az,g->r_eye_el,g->r_eye_roll, (frametime - starttime),g->r_eyecluster, stop);
  }
  
};


int main(int argc, char **argv)
{
  // setup global data structure
  g = (global_shared_t *) malloc(sizeof(global_shared_t));
  init_global_shared(g);

  // USAGE:
  // command -c configfile1 -c configfile2 -- otherOptions
  // NOTE: The config files all have to be first then the "--" specification
  vrlib::Kernel* kernel = vrlib::Kernel::getInstance();

   for( int i = 1; i < argc; ++i )
   {
	   if(strcmp(argv[i], "--config") == 0)
	   {
			i++;
			kernel->loadConfig(argv[i]);
	   }
   }
 // Get the Juggler commands first
 //kernel->loadConfigFile("z:\\Demonstratie3PC_tracker.jconf");
 //kernel->loadConfigFile("standalone.jconf");
 //kernel->loadConfigFile("D:\\config files\sim_4walls_1overview.jconf");

	//kernel->loadConfigFile(getenv("VRJCONFIG"));
 
   // optind-1 because it will ignore elt 0 (normally app name)*/
  ui_read_args(argc, argv);
 // strcpy(g->r_help_fname, "paul/helpVR.jpg");
	
//  if(argc > 1)
	//  strcpy(g->g_bsp_fname, argv[1]);

  myApp* app = new myApp();

  kernel->setApp(app);
  kernel->start();
  TerminateProcess(GetCurrentProcess(),0);
  exit(0);
}

void printUsage()
{
   std::cout << "Usage: cq3a -c <juggler-config-file> -c <more-juggler-config> -- <cq3a config params>" << std::endl;
}

