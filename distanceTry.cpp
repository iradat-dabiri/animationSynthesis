#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstdio>
#include <cstring>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>				
#include <fstream>
#include <cassert>
#include <string>

#include <GL/glew.h>
#include <FL/gl.h>
#include <FL/glut.H>  // GLUT for use with FLTK
#include <FL/glu.h>
#include <GLFW/glfw3.h>
#include <FL/Fl_File_Chooser.H> // file chooser for load/save
#include <FL/fl_ask.H>

#include "skeleton.h"
#include "motion.h"
#include "displaySkeleton.h"
#include "transform.h"
#include "types.h"
#include "mocapPlayer.h"   		      
#include "interface.h"  // UI framework built by FLTK (using fluid)
#include "transform.h"  // utility functions for vector and matrix transformation  
#include "displaySkeleton.h"

enum SwitchStatus { OFF, ON };

DisplaySkeleton hereDisplayer;

Skeleton* herePSkeleton = NULL;
Motion* herePMotion = NULL;

Fl_Window* hereForm = NULL; // Global form 
MouseT hereMouse;    // Keeping track of mouse input 
CameraT hereCamera;  // Structure about camera setting 

SwitchStatus hereRenderWorldAxes = ON;
SwitchStatus hereGroundPlane = ON;
SwitchStatus herePlayButton = ON;

GLfloat hereGroundPlaneLightHeight = 100.0;
GLint hereDisplayListGround;

int hereMaxFrames = 0;
int hereCurrentFrameIndex = 0;

int distance(Skeleton * pSkeleton1, Skeleton* pSkeleton2) {
	//printf("%s", pSkeleton1->getRoot());
	return 0;
}

void hereIdle(void*) {
	if (herePlayButton == ON) {
		if (hereDisplayer.GetNumSkeletons() != 0)
		{
			hereCurrentFrameIndex++;
			printf("%d", hereCurrentFrameIndex);
			if (hereCurrentFrameIndex >= hereMaxFrames) {
				herePlayButton = OFF;  // important, especially in "recording" mode
			}

			printf("%d", hereCurrentFrameIndex);
			hereDisplayer.SetSkeletonsToSpecifiedFrame(hereCurrentFrameIndex);
			printf("after setting\n");
		}
	}  // if(playButton == ON)

	glwindow->redraw();
	printf("redrawn");
}

int main2(void) {
	//initialise form, sliders and buttons
	hereForm = make_window();
	worldAxes_button->value(hereRenderWorldAxes);
	//frame_slider->value(1);

	//show form, and do initial draw of model
	hereForm->show();
	glwindow->show(); // glwindow is initialized when the form is built
		
	char motions[][20] = { "movements\\walk.asf", 
		"movements\\walk.amc"};
	char* skeletonPtr = motions[0];
	char* motionPtr = motions[1];
	//printf(skeletonPtr);
	//printf(motionPtr);
		
	if (motions) {
		if (skeletonPtr) {
			//Read skeleton from asf file
			herePSkeleton = new Skeleton(skeletonPtr, MOCAP_SCALE);

			//Set the rotations for all bones in their local coordinate system to 0
			//Set root position to (0, 0, 0)
			herePSkeleton->setBasePosture();
			hereDisplayer.LoadSkeleton(herePSkeleton);
			printf("skeleton file read\n");
		}

		if (hereDisplayer.GetNumSkeletons()) {
			if (motionPtr) {
				//read motion file and create a motion
				herePMotion = new Motion(motionPtr, MOCAP_SCALE, herePSkeleton);
				//set sampled motion for display
				hereDisplayer.LoadMotion(herePMotion);
				//tell skeleton to perform the first pose
				herePSkeleton->setPosture(*(hereDisplayer.GetSkeletonMotion(0)->GetPosture(0)));
				//set skeleton to perform the first pose
				printf("numSkel got\n");
			}
		}
	}

	Fl::add_idle(hereIdle);
	printf("%d", hereCurrentFrameIndex);
	printf("idled");
	return Fl::run();
}

int main1() {
	char skeletons[][30] = { "movements\\walk.asf",
		"movements\\martialArts.amc" };
	char* skeletonPtr1 = skeletons[0];
	char* skeletonPtr2 = skeletons[1];
	//return distance(skeletonPtr1, skeletonPtr2);
	return 0;
}