/*
#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>				
#include <fstream>
#include <cassert>
#include <cmath>

#include <FL/gl.h>
#include <FL/glut.H>  // GLUT for use with FLTK
#include <FL/Fl_File_Chooser.H> // file chooser for load/save
#include <FL/fl_ask.H>

#include "mocapPlayer.h"   		      
#include "interface.h"  // UI framework built by FLTK (using fluid)
#include "transform.h"  // utility functions for vector and matrix transformation  
#include "displaySkeleton.h"

enum SwitchStatus {OFF, ON};

DisplaySkeleton displayer;

Skeleton* pSkeleton = NULL;
Motion* pMotion = NULL;

Fl_Window* form = NULL; // Global form 
MouseT mouse;    // Keeping track of mouse input 
CameraT camera;  // Structure about camera setting 

SwitchStatus playButton = OFF;
SwitchStatus rewindButton = OFF;
SwitchStatus repeatButton = OFF;
SwitchStatus groundPlane = ON;
SwitchStatus previousPlayButtonStatus = playButton;

GLfloat groundPlaneLightHeight = 100.0;
int displayListGround; //should be GLint
int lastSkeleton = -1;
int lastMotion = -1;

char lastMotionFilename[FILENAME_MAX];

SwitchStatus renderWorldAxes = ON;

const double standardFPS = 120.0;
double expectedFPS = standardFPS;
// maximum number of frames among all the motions loaded so far
int maxFrames = 0;
// Current frame and frame increment
int currentFrameIndex = 0;
double currentFrameIndexDoublePrecision = 0.0;
double framesIncrementDoublePrecision = 1.0;

//NEEDS UPDATING
static void RenderWorldAxes() {
	glBegin(GL_LINES);

	//draw x axis in red, y axis in green, z axis in blue
	glColor3f(1.0f, 0.2f, 0.2f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);

	glColor3f(0.2f, 1.0f, 0.2f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glColor3f(0.2f, 0.2f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 1.0f);

	glEnd();
}

//NEEDS UPDATING
void RenderGroundPlane(double groundPlaneSize, double groundPlaneHeight, double rPlane,
	double gPlane, double bPlane, double ambientFskeleton, double diffuseFskeleton,
	double specularFskeleton, double shininess) {
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0, 1.0);

	float planeAmbient[4] = { (float)(ambientFskeleton * rPlane), (float)(ambientFskeleton * gPlane), (float)(ambientFskeleton * bPlane), 1.0f };
	float planeDiffuse[4] = { (float)(diffuseFskeleton * rPlane), (float)(diffuseFskeleton * gPlane), (float)(diffuseFskeleton * bPlane), 1.0f };
	float planeSpecular[4] = { (float)(specularFskeleton * rPlane), (float)(specularFskeleton * gPlane), (float)(specularFskeleton * bPlane), 1.0f };
	float planeShininess = (float)shininess;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, planeAmbient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, planeDiffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, planeSpecular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, planeShininess);
	glNormal3f(0, 1, 0);
	const int planeResolution = 100;
	double planeIncrement = groundPlaneSize / planeResolution;
	for (int i = 0; i < planeResolution; i++) {
		for (int j = 0; i < planeResolution; j++) {
			float planeAmbientAct[4] = { (float)(ambientFskeleton * rPlane), (float)(ambientFskeleton * gPlane), (float)(ambientFskeleton * bPlane), 1.0f };
			float factor = (((i + j) % 2) == 0) ? 0.5f : 1.0f;
			planeAmbientAct[0] *= factor;
			planeAmbientAct[1] *= factor;
			planeAmbientAct[2] *= factor;
			planeAmbientAct[3] *= factor;
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, planeAmbientAct);
			glBegin(GL_QUADS);
			glVertex3f((float)(-groundPlaneSize / 2 + i * planeIncrement), (float)groundPlaneHeight, (float)(-groundPlaneSize / 2 + j * planeIncrement));
			glVertex3f((float)(-groundPlaneSize / 2 + i * planeIncrement), (float)groundPlaneHeight, (float)(-groundPlaneSize / 2 + (j + 1) * planeIncrement));
			glVertex3f((float)(-groundPlaneSize / 2 + (i + 1) * planeIncrement), (float)groundPlaneHeight, (float)(-groundPlaneSize / 2 + (j + 1) * planeIncrement));
			glVertex3f((float)(-groundPlaneSize / 2 + (i + 1) * planeIncrement), (float)groundPlaneHeight, (float)(-groundPlaneSize / 2 + j * planeIncrement));
			glEnd();
		}
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}

//NEEDS UPDATING
void cameraView(void) {
	glTranslated(camera.tx, camera.ty, camera.tz);
	glTranslated(camera.atx, camera.aty, camera.atz);

	glRotated(-camera.tw, 0.0, 1.0, 0.0);
	glRotated(-camera.el, 1.0, 0.0, 0.0);
	glRotated(camera.az, 0.0, 1.0, 0.0);

	glTranslated(-camera.atx, -camera.aty, -camera.atz);
	glScaled(camera.zoom, camera.zoom, camera.zoom);
}

/*
* this function is called by Player_Gl_Window::draw(). The display is double buffered,
* and FLTK swaps buffers when the function returns. The GL context associated with this
* instance of Player_GL_Window is set to the current context by FLTK when it calls draw().
*/
//NEEDS UPDATING
/*
void Redisplay() {
	//clear image buffer to black
	glClearColor(1.0, 1.0, 1.0, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); //clear image, zbuf

	glPushMatrix();  //save current transform matrix

	cameraView();

	glLineWidth(2.0);  //we'll draw background with thick lines
	
	if (renderWorldAxes == ON) {
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_FOG);
		RenderWorldAxes();  // draw a triad in the origin of the world coordinate
	}

	if (groundPlane == ON) {
		// draw_ground();
		glEnable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glCallList(displayListGround);

		glDisable(GL_LIGHTING);
		glDisable(GL_FOG);
		glLineWidth(1.0);
		glColor3f(0.1f, 0.1f, 0.1f);
		double ground[4] = { 0,1,0,0 };
		double light[4] = { 0,groundPlaneLightHeight,0,1 };
	}

	//render the skeletons
	if (displayer.GetNumSkeletons()) {
		glEnable(GL_LIGHTING);
		glDisable(GL_FOG);
		displayer.Render(DisplaySkeleton::BONES_AND_LOCAL_FRAMES);
	}

	glPopMatrix(); //restore current transformation matrix
}

void renderWorldAxes_callback(Fl_Light_Button* obj, long val) {
	renderWorldAxes = (SwitchStatus)worldAxes_button->value();
	glwindow->redraw();
}

void resetScene_callback(Fl_Button* button, void*)
{
	rewindButton = ON;
	playButton = OFF;
	repeatButton = OFF;
	lastSkeleton = -1;
	lastMotion = -1;
	displayer.Reset();
	maxFrames = 0;
	glwindow->redraw();
	framesIncrementDoublePrecision = 1.0;
	currentFrameIndex = 0;
	currentFrameIndexDoublePrecision = 0.0;
}

void resetPostureAccordingFrameSlider(void) {
	currentFrameIndex = (int)frame_slider->value() - 1;
	currentFrameIndexDoublePrecision = currentFrameIndex;

	//display
	for (int skeletonIndex = 0; skeletonIndex < displayer.GetNumSkeletons(); skeletonIndex++)
	{
		int postureID;
		if (currentFrameIndex >= displayer.GetSkeletonMotion(skeletonIndex)->getNumFrames())
			postureID = displayer.GetSkeletonMotion(skeletonIndex)->getNumFrames() -1;
		else
			postureID = currentFrameIndex;
		// Set skeleton to the first posture
		Posture* currentPosture = displayer.GetSkeletonMotion(skeletonIndex)->GetPosture(postureID);
		displayer.GetSkeleton(skeletonIndex)->setPosture(*currentPosture);
	}
}

void UpdateMaxFrameNumber(void) {
	maxFrames = 0;
	for (int skeletonIndex = 0; skeletonIndex < displayer.GetNumSkeletons(); skeletonIndex++) {
		int currentFrames = displayer.GetSkeletonMotion(skeletonIndex)->getNumFrames();
		if (currentFrames > maxFrames)
			maxFrames = currentFrames;
	}
}

void load_callback(Fl_Button* button, void*) {
	if (button == loadSkeleton_button) {
		if (lastSkeleton <= lastMotion) { //cannot load new skeleton until motion is assigned to the current skeleton
			char* filename = fl_file_chooser("Select filename", "*ASF", "");
			if (filename != NULL) {
				//read skeleton from asf file
				pSkeleton = new Skeleton(filename, MOCAP_SCALE);
				lastSkeleton++;
				//set the rotations for all bones in their local coordinate system to 0
				//set root position to (0, 0, 0)
				pSkeleton->setBasePosture();
				displayer.LoadSkeleton(pSkeleton);
				glwindow->redraw();
			}
		}

		if (button == loadMotion_button) {
			if ((lastSkeleton >= 0) && (lastSkeleton >= lastMotion)) {
				char* filename = fl_file_chooser("Select filename", "*.AMC", "");
				if (filename != NULL) {
					//read motion file and create motion
					pMotion = new Motion(filename, MOCAP_SCALE, pSkeleton);
					//backup the filename
					strcpy(lastMotionFilename, filename);

					//set sampled motiion for display
					displayer.LoadMotion(pMotion);
					if (lastSkeleton > lastMotion) lastMotion++;

					UpdateMaxFrameNumber();
					resetPostureAccordingFrameSlider();
					frame_slider->value(currentFrameIndex);
					frame_slider->maximum((double)maxFrames);
					frame_slider->redraw();
					glwindow->redraw();
					Fl::flush();
				}
			} //if lastSkeleton > lastMotion
		}
	}
	glwindow->redraw();
}

void reload_callback(Fl_Button* button, void*) {
	if (!displayer.GetNumSkeletons())
		return;

	//read motion file and create a motion
	pMotion = new Motion(lastMotionFilename, MOCAP_SCALE, pSkeleton);

	//set sampled motion for display
	displayer.LoadMotion(pMotion);

	resetPostureAccordingFrameSlider();
	UpdateMaxFrameNumber();
	frame_slider->maximum((double)maxFrames);
	frame_slider->value(currentFrameIndex);
	frame_slider->redraw();
	Fl::flush();
	glwindow->redraw();
}

void play_callback(Fl_Button* button, void*) {
	if (button == play_button) { rewindButton = OFF; playButton = ON;  rewindButton = OFF; }
	if (button == pause_button) { rewindButton = OFF; playButton = OFF; repeatButton = OFF; }
	if (button == repeat_button) { rewindButton = OFF; playButton = ON;  repeatButton = ON; }
	if (button == rewind_button) { rewindButton = ON;  playButton = OFF; repeatButton = OFF; }

	if ((previousPlayButtonStatus == OFF) && (playButton == ON))
		framesIncrementDoublePrecision = 1.0;  // Just start playing the animation, no time has been measured

}

/*
Set all skeletons to a specified frame (frameIndex)
If frameIndex is larger than the number of frames of the motion, 
set the skeleton to the last frame of the motion
*/
/*
void SetSkeletonsToSpecifiedFrame(int frameIndex) {
	if (frameIndex < 0)
	{
		printf("Error in SetSkeletonsToSpecifiedFrame: frameIndex %d is illegal.\n", frameIndex);
		exit(0);
	}
	for (int skeletonIndex = 0; skeletonIndex < displayer.GetNumSkeletons(); skeletonIndex++)
		if (displayer.GetSkeletonMotion(skeletonIndex) != NULL)
		{
			int postureID;
			if (frameIndex >= displayer.GetSkeletonMotion(skeletonIndex)->getNumFrames())
				postureID = displayer.GetSkeletonMotion(skeletonIndex)->getNumFrames() - 1;
			else
				postureID = frameIndex;
			displayer.GetSkeleton(skeletonIndex)->setPosture(*(displayer.GetSkeletonMotion(skeletonIndex)->GetPosture(postureID)));
		}
}

//I have completely skipped the screenshot function, 
//but there is an idle function which may be necessary

void fslider_callback(Fl_Value_Slider* slider, long val) {
	currentFrameIndex = (int)frame_slider->value() - 1;
	currentFrameIndexDoublePrecision = currentFrameIndex;
	rewindButton = OFF;
	playButton = OFF;
	repeatButton = OFF;
	SetSkeletonsToSpecifiedFrame(currentFrameIndex);
	Fl::flush();
}

//NEEDS UPDATING
void GraphicsInit() {
	int red_bits, green_bits, blue_bits;
	struct { GLint x, y, width, height; } viewport;
	glEnable(GL_DEPTH_TEST);	

	glGetIntegerv(GL_RED_BITS, &red_bits);
	glGetIntegerv(GL_GREEN_BITS, &green_bits);
	glGetIntegerv(GL_BLUE_BITS, &blue_bits);
	glGetIntegerv(GL_VIEWPORT, &viewport.x);
	printf("OpenGL window has %d bits red, %d green, %d blue; viewport is %dx%d\n",
		red_bits, green_bits, blue_bits, viewport.width, viewport.height);

	//setup perspective camera with OpenGL
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)viewport.width / viewport.height, 0.01, 200.0);

	//set the modelling transformations
	glMatrixMode(GL_MODELVIEW);
	//move away from center
	glTranslatef(0.0, 0.0, -5.0);
	camera.zoom = 1;
	camera.tw = 0;
	camera.el = -15;
	camera.az = -25;
	camera.atx = camera.aty = camera.atz = 0;

	//two white lights
	GLfloat light_Ka[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_Kd[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_Ks[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light0_pos[] = { 0.0, groundPlaneLightHeight, 0.0, 0.0 };
	GLfloat light1_pos[] = { 1.0, -1.0, 0.0, 0.0 };

	//lights
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_Ka);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_Kd);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_Ks);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);

	glLightfv(GL_LIGHT1, GL_AMBIENT, light_Ka);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_Kd);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_Ks);
	glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_NORMALIZE);
	glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, true);

	//screen buffer
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	double groundPlaneHeight = 0.0;
	double groundPlaneSize = 200.0;
	double groundPlaneR = 0.81;
	double groundPlaneG = 0.81;
	double groundPlaneB = 0.55;
	double groundPlaneAmbient = 0.1;
	double groundPlaneDiffuse = 0.9;
	double groundPlaneSpecular = 0.1;
	double groundPlaneShininess = 120.0;
	displayListGround = glGenLists(1);
	glNewList(displayListGround, GL_COMPILE);
	RenderGroundPlane(groundPlaneSize, groundPlaneHeight, groundPlaneR, groundPlaneG, groundPlaneB, groundPlaneAmbient, groundPlaneDiffuse, groundPlaneSpecular, groundPlaneShininess);
	glEndList();
}

/*
define the methods for glwindow and handle mouse events
don't make any openGL calls here as the context is not set
*/
/*
//NEEDS UPDATING
int Player_Gl_Window::handle(int event) {
	int handled = 1;
	static int prev_x, prev_y;
	int delta_x = 0, delta_y = 0;

	switch (event) {
	case FL_RELEASE:
		mouse.x = (Fl::event_x());
		mouse.y = (Fl::event_y());
		mouse.button = 0;
		break;
	case FL_PUSH:
		mouse.x = (Fl::event_x());
		mouse.y = (Fl::event_y());
		mouse.button = (Fl::event_button());
		break;
	case FL_DRAG:
		mouse.x = (Fl::event_x());
		mouse.y = (Fl::event_y());
		delta_x = mouse.x - prev_x;
		delta_y = mouse.y - prev_y;

		if (mouse.button == 3) {
			if (abs(delta_x) > abs(delta_y))
				camera.az += (GLdouble)(delta_x);
			else
				camera.el -= (GLdouble)(delta_y);
		}
		else if (mouse.button == 2) {
			if (abs(delta_y) > abs(delta_x)) 
				glScalef(float(1 + delta_y / 100.0), float(1 + delta_y / 100.0), float(1 + delta_y / 100.0));
		}
		else {
			if (mouse.button == 1) {
				camera.tx += (GLdouble)cos(camera.az / 180.0 * M_PI) * delta_x / 10.0;
				camera.tz += (GLdouble)sin(camera.az / 180.0 * M_PI) * delta_x / 10.0;
				camera.ty -= (GLdouble)delta_y / 10.0; //FLTK's origin is at the left_top corner

				camera.atx = -camera.tx;
				camera.aty = -camera.ty;
				camera.atz = -camera.tz;
			}
		}
		break;
	case FL_KEYBOARD:
		switch (Fl::event_key()) {
		case 'q':
		case 'Q':
		case 65307:
			exit(0);
		}
		break;
	default:
		// pass other events to the base class...
		handled = Fl_Gl_Window::handle(event);
	}

	prev_x = mouse.x;
	prev_y = mouse.y;
	glwindow->redraw();

	return (handled);  // Returning one acknowledges that we handled this event
}

//Pre-written draw function
void Player_Gl_Window::draw() {
	// Upon setup of the window (or when Fl_Gl_Window->invalidate is called), 
	// the set of functions inside the if block are executed.
	if (!valid()) {
		GraphicsInit();
	}

	// Redisplay the screen then put the proper buffer on the screen.
	Redisplay();
}

int mainOriginal(int argc, char** argv) {
	//initialise form, sliders and buttons
	form = make_window();
	worldAxes_button->value(renderWorldAxes);
	frame_slider->value(1);

	//show form, and do initial draw of model
	form->show();
	glwindow->show(); // glwindow is initialized when the form is built

	if (argc > 2) {
		char* filename;
		filename = argv[1];
		if (filename != NULL) {
			//Read skeleton from asf file
			pSkeleton = new Skeleton(filename, MOCAP_SCALE);

			//Set the rotations for all bones in their local coordinate system to 0
			//Set root position to (0, 0, 0)
			pSkeleton->setBasePosture();
			displayer.LoadSkeleton(pSkeleton);
			lastSkeleton++;
		}

		if (displayer.GetNumSkeletons()) {
			filename = argv[2];
			if (filename != NULL) {
				//read motion file and create a motion
				pMotion = new Motion(filename, MOCAP_SCALE, pSkeleton);
				//set sampled motion for dispplay
				displayer.LoadMotion(pMotion);
				lastMotion++;
				//tell skeleton to perform the first pose
				pSkeleton->setPosture(*(displayer.GetSkeletonMotion(0)->GetPosture(0)));
				//set skeleton to perform the first pose
				int currentFrames = displayer.GetSkeletonMotion(0)->getNumFrames();
				if (currentFrames > maxFrames) {
					maxFrames = currentFrames;
					frame_slider->maximum((double)maxFrames);
				}
				frame_slider->maximum((double)maxFrames);

				currentFrameIndex = 0;
			}
		}
		else printf("Load a skeleton first. \n");
		/*
		framesIncrementDoublePrecision = 1.0;            // Current frame and frame increment
		playButton = ON;
		repeatButton = OFF;
		groundPlane = ON;
		glwindow->redraw();
	}

	//don't know if this is needed
	//Fl::add_idle(idle);
	return Fl::run();
}
*/
