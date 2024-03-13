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
#include "distance.h"

enum SwitchStatus { OFF, ON };

DisplaySkeleton displayer;

Skeleton* pSkeleton = NULL;
Motion* pMotion = NULL;

Fl_Window* form = NULL; // Global form 
Fl_Window* smallForm = NULL;
MouseT mouse;    // Keeping track of mouse input 
CameraT camera;  // Structure about camera setting 

SwitchStatus playButton = OFF;
SwitchStatus minusOneButton = OFF;
SwitchStatus plusOneButton = OFF;
SwitchStatus rewindButton = OFF;
SwitchStatus repeatButton = OFF;
SwitchStatus groundPlane = ON;
SwitchStatus previousPlayButtonStatus = playButton;

GLfloat groundPlaneLightHeight = 100.0;
GLint displayListGround;
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

float DisplaySkeleton::jointColours[NUMBER_JOINT_COLOURS][3] = {
  {0.0f, 1.0f, 0.0f},  // GREEN
  {1.0f, 0.0f, 0.0f},  // RED
  {0.0f, 0.0f, 1.0f}   // BLUE
};

DisplaySkeleton::DisplaySkeleton(void) {
    m_SpotJoint = -1;
    numSkeletons = 0;
    for (int skeletonIndex = 0; skeletonIndex < MAX_SKELS; skeletonIndex++) {
        m_pSkeleton[skeletonIndex] = NULL;
        m_pMotion[skeletonIndex] = NULL;
    }
}

DisplaySkeleton::~DisplaySkeleton(void) { Reset(); }

//draws the world coordinate axis NEEDS UPDATING
void DisplaySkeleton::DrawSpotJointAxis(void) {
    GLfloat axisLength = 0.5f;
    glBegin(GL_LINES);
    //draw x axis in red, y in green and z in blue
    glColor3f(1.0f, 0.2f, 0.2f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(axisLength, 0.0f, 0.0f);

    glColor3f(0.2f, 1.0f, 0.2f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, axisLength, 0.0f);

    glColor3f(0.2f, 0.2f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, axisLength);
    glEnd();
}

//build display lists for bones NEEDS UPDATING
void DisplaySkeleton::SetDisplayList(int skeletonID, Bone* bone, GLuint *pBoneList){
    GLUquadricObj* qobj;
    int numbones = m_pSkeleton[skeletonID]->numBonesInSkel(bone[0]);
    *pBoneList = glGenLists(numbones);
    qobj = gluNewQuadric();

    gluQuadricDrawStyle(qobj, GLU_FILL); //got rid of the cast to GLenum, looked like gluQuadricDrawStyle(qobj, (GLenum) GLU_SMOOTH);
    gluQuadricDrawStyle(qobj, GLU_SMOOTH); //got rid of the cast to GLenum

    float ambientFskeleton = 0.1f;
    float diffuseFskeleton = 0.9f;
    float specularFskeleton = 0.1f;

    int colourIndex = numSkeletons % NUMBER_JOINT_COLOURS;
    float jointShininess = 120.0f;
    float jointAmbient[4] = { ambientFskeleton * jointColours[colourIndex][0], ambientFskeleton * jointColours[colourIndex][1], ambientFskeleton * jointColours[colourIndex][2], 1.0 };
    float jointDiffuse[4] = { diffuseFskeleton * jointColours[colourIndex][0], diffuseFskeleton * jointColours[colourIndex][1], diffuseFskeleton * jointColours[colourIndex][2], 1.0 };
    float jointSpecular[4] = { specularFskeleton * jointColours[colourIndex][0], specularFskeleton * jointColours[colourIndex][1], specularFskeleton * jointColours[colourIndex][2], 1.0 };

    float boneColour[3] = { 1.0f, 1.0f, 1.0f };
    float boneShininess = 120.0f;
    float boneAmbient[4] = { ambientFskeleton * boneColour[0], ambientFskeleton * boneColour[1], ambientFskeleton * boneColour[2], 1.0 };
    float boneDiffuse[4] = { diffuseFskeleton * boneColour[0], diffuseFskeleton * boneColour[1], diffuseFskeleton * boneColour[2], 1.0 };
    float boneSpecular[4] = { specularFskeleton * boneColour[0], specularFskeleton * boneColour[1], specularFskeleton * boneColour[2], 1.0 };

    double jointRadius = 0.10;
    double boneRadius = 0.10;
    double sizeDifferenceJointAndBone = 0.05;

    for (int j = 0; j < numbones; j++) {
        glNewList(*pBoneList + j, GL_COMPILE);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, jointAmbient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, jointDiffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, jointSpecular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, jointShininess);
        glPushMatrix();
        glScalef(float(bone[j].aspy + sizeDifferenceJointAndBone), float(bone[j].aspy + sizeDifferenceJointAndBone), float(bone[j].aspy + sizeDifferenceJointAndBone));
        gluSphere(qobj, jointRadius, 20, 20);
        glPopMatrix();

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, boneAmbient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, boneDiffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, boneSpecular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, boneShininess);
        glPushMatrix();
        glScalef(float(bone[j].aspx), float(bone[j].aspy), 1.0f);
        gluCylinder(qobj, boneRadius, boneRadius, bone[j].length, 20, 20);

        // Two disks to close the cylinder at the bottom and the top
        gluDisk(qobj, 0.0, boneRadius, 20, 20);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, float(bone[j].length));
        gluDisk(qobj, 0.0, boneRadius, 20, 20);
        glPopMatrix();

        glPopMatrix();
        glEndList();
    }
}

/*we now define M_k = modelview matrix at the kth bone in the hierarchy, 
that stores the transformation matrix of the bone in world coordinates
We draw the k+1th bone using its local information and M_k
In the k+1th bone, we compute:
rot_parent_current - rotation matrix that takes is from the K+1 to kth local coordinate system
R_k+1 - rotation matrix fro the k+1th bone using angles from AMC file
T_k+1 - translation matrix for the k+1th node

Update relation is given by: M_k+1 = M_k * rot_parent_current * R_k+1 * T_k+1
*/
void DisplaySkeleton::DrawBone(Bone* pBone, int skelNum) {
    static double z_dir[3] = { 0.0, 0.0, 1.0 };
    double r_axis[3], theta;

    //rotate from local coorindates system of this bone to its parent
    glMultMatrixd((double*)&pBone->rot_parent_current);
    
    //draw the local coordinate system for the selected bone
    if ((renderMode == BONES_AND_LOCAL_FRAMES) && (pBone->idx == m_SpotJoint)) {
        GLint lightingStatus;
        glGetIntegerv(GL_LIGHTING, &lightingStatus);
        glDisable(GL_LIGHTING);
        DrawSpotJointAxis();
        if (lightingStatus)
            glEnable(GL_LIGHTING);
    }

    //translate AMC
    if (pBone->doftz) glTranslatef(0.0f, 0.0f, float(pBone->tz));
    if (pBone->dofty) glTranslatef(0.0f, float(pBone->ty), 0.0f);
    if (pBone->doftx) glTranslatef(float(pBone->tx), 0.0f, 0.0f);

    //rotate AMC
    if (pBone->dofrz) glRotatef(float(pBone->rz), 0.0f, 0.0f, 1.0f);
    if (pBone->dofry) glRotatef(float(pBone->ry), 0.0f, 1.0f, 0.0f);
    if (pBone->dofrx) glRotatef(float(pBone->rx), 1.0f, 0.0f, 0.0f);

    //store the current modelview matrix before adding the translation part
    glPushMatrix();

    //compute the translation from pBone to the child in its local coordinates system
    double tx = pBone->dir[0] * pBone->length;
    double ty = pBone->dir[1] * pBone->length;
    double tz = pBone->dir[2] * pBone->length;

    //use the current modelview matrix to display the current bone
    //rotate the bone from it's cannonical position
    //(elongated sphere with its major axis paralle to X axis)
    //to its correct orientation
        //there was an if statement here but the root was no longer a bone, so we just use the else below
    
    //compute the angle between the cannonical pose and the correct orientation 
    //(speciified in pBone->dir) using cross product
    //using the formula: r_axis = z_dir * pBone->dir
    v3_cross(z_dir, pBone->dir, r_axis);
    theta = get_angle(z_dir, pBone->dir, r_axis);
    glRotatef(float(theta * 180. / M_PI), float(r_axis[0]), float(r_axis[1]), float(r_axis[2]));
    glCallList(m_BoneList[skelNum] + pBone->idx);

    glPopMatrix();

    //then translate the bone depending on its length and direction
    //this corresponsds to M_k+1 = ModelviewMatrix += T_k+1
    glTranslatef(float(tx), float(ty), float(tz));
}

//traverse the hierarchy starting from the root
//every node has one child, and each node can have mutiple siblings
//the algorithm draws the current node, visits its child and then its siblings
void DisplaySkeleton::Traverse(Bone* ptr, int skelNum) {
    if (ptr != NULL) {
        glPushMatrix();
        DrawBone(ptr, skelNum);
        Traverse(ptr->child, skelNum);
        glPopMatrix();
        Traverse(ptr->sibling, skelNum);
    }
}

//draw the skeleton
void DisplaySkeleton::Render(RenderMode renderMode_) {
    renderMode = renderMode_;

    glPushMatrix();

    //there is a commented out translate here which may no longer be necessary

    //draw the skeleton starting from the root
    for (int i = 0; i < numSkeletons; i++) {
        glPushMatrix();
        double translation[3];
        m_pSkeleton[i]->GetTranslation(translation);
        double rotationAngle[3];
        m_pSkeleton[i]->GetRotationAngle(rotationAngle);

        glTranslatef(float(MOCAP_SCALE * translation[0]), float(MOCAP_SCALE * translation[1]), float(MOCAP_SCALE * translation[2]));
        glRotatef(float(rotationAngle[0]), 1.0f, 0.0f, 0.0f);
        glRotatef(float(rotationAngle[1]), 0.0f, 1.0f, 0.0f);
        glRotatef(float(rotationAngle[2]), 0.0f, 0.0f, 1.0f);
        Traverse(m_pSkeleton[i]->getRoot(), i);

        glPopMatrix();
    }
    glPopMatrix();
}

void DisplaySkeleton::LoadMotion(Motion* pMotion) {
    //always load the motion for the latest skeleton
    if (m_pMotion[numSkeletons - 1] != NULL) delete m_pMotion[numSkeletons - 1];
    m_pMotion[numSkeletons - 1] = pMotion;
}

void DisplaySkeleton::LoadSkeleton(Skeleton* pSkeleton) {
    if (numSkeletons >= MAX_SKELS) return;

    m_pSkeleton[numSkeletons] = pSkeleton;

    //create the display list for the skeleton
    //all the bones are the elongated spheres centred at (0, 0, 0)
    //the axis of elongation is the X axis
    SetDisplayList(numSkeletons, m_pSkeleton[numSkeletons]->getRoot(), &m_BoneList[numSkeletons]);
    numSkeletons++;
}

Motion* DisplaySkeleton::GetSkeletonMotion(int skeletonIndex) {
    if (skeletonIndex < 0 || skeletonIndex >= MAX_SKELS) {
        printf("Error in DisplaySkeleton::GetSkeletonMotion: index %d is illegal.\n", skeletonIndex);
        exit(0);
    }
    return m_pMotion[skeletonIndex];
}

Skeleton* DisplaySkeleton::GetSkeleton(int skeletonIndex) {
    if (skeletonIndex < 0 || skeletonIndex >= numSkeletons) {
        printf("Error in DisplaySkeleton::GetSkeleton: skeleton index %d is illegal.\n", skeletonIndex);
        exit(0);
    }
    return m_pSkeleton[skeletonIndex];
}

//NEEDS UPDATING
void DisplaySkeleton::Reset(void) {
    for (int skeletonIndex = 0; skeletonIndex < MAX_SKELS; skeletonIndex++) {
        if (m_pSkeleton[skeletonIndex != NULL]) {
            delete(m_pSkeleton[skeletonIndex]);
            glDeleteLists(m_BoneList[skeletonIndex], 1);
            m_pSkeleton[skeletonIndex] = NULL;
        }
        if (m_pMotion[skeletonIndex] != NULL) {
            delete (m_pMotion[skeletonIndex]);
            m_pMotion[skeletonIndex] = NULL;
        }
    }
    numSkeletons = 0;
}

//NEEDS UPDATING
void DisplaySkeleton::RenderWorldAxes() {
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
void DisplaySkeleton::RenderGroundPlane(double groundPlaneSize, double groundPlaneHeight, double rPlane,
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
		for (int j = 0; j < planeResolution; j++) {
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
void DisplaySkeleton::cameraView(void) {
	glTranslated(camera.tx, camera.ty, camera.tz);
	glTranslated(camera.atx, camera.aty, camera.atz);

	glRotated(-camera.tw, 0.0, 1.0, 0.0);
	glRotated(-camera.el, 1.0, 0.0, 0.0);
	glRotated(camera.az, 0.0, 1.0, 0.0);

	glTranslated(-camera.atx, -camera.aty, -camera.atz);
	glScaled(camera.zoom, camera.zoom, camera.zoom);
}

/*
#
* this function is called by Player_Gl_Window::draw(). The display is double buffered,
* and FLTK swaps buffers when the function returns. The GL context associated with this
* instance of Player_GL_Window is set to the current context by FLTK when it calls draw().
*/
//NEEDS UPDATING
void DisplaySkeleton::Redisplay() {
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
	glwindowMain->redraw();
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
	glwindowMain->redraw();
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
			postureID = displayer.GetSkeletonMotion(skeletonIndex)->getNumFrames() - 1;
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
				glwindowMain->redraw();
			}
		}
	}

	if (button == loadMotion_button) {
		if ((lastSkeleton >= 0) && (lastSkeleton >= lastMotion)) {
			char* filename = fl_file_chooser("Select filename", "*.AMC", "");
			if (filename != NULL) {
				//read motion file and create motion
				pMotion = new Motion(filename, MOCAP_SCALE, pSkeleton);
				//backup the filename
				strcpy_s(lastMotionFilename, filename);

				//set sampled motiion for display
				displayer.LoadMotion(pMotion);
				if (lastSkeleton > lastMotion) lastMotion++;

				UpdateMaxFrameNumber();
				resetPostureAccordingFrameSlider();
				frame_slider->value(currentFrameIndex);
				frame_slider->maximum((double)maxFrames);
				frame_slider->redraw();
				glwindowMain->redraw();
				Fl::flush();
			}
		} //if lastSkeleton > lastMotion
	}
	glwindowMain->redraw();
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
	glwindowMain->redraw();
}

void play_callback(Fl_Button* button, void*) {
	if (button == play_button) { minusOneButton = OFF; plusOneButton = OFF; rewindButton = OFF; playButton = ON;  rewindButton = OFF; }
	if (button == minusOne_button) { minusOneButton = ON;  plusOneButton = OFF; rewindButton = OFF; playButton = OFF; repeatButton = OFF; }
	if (button == plusOne_button) { minusOneButton = OFF; plusOneButton = ON;  rewindButton = OFF; playButton = OFF; repeatButton = OFF; }
	if (button == pause_button) { minusOneButton = OFF; plusOneButton = OFF; rewindButton = OFF; playButton = OFF; repeatButton = OFF; }
	if (button == repeat_button) { minusOneButton = OFF; plusOneButton = OFF; rewindButton = OFF; playButton = ON;  repeatButton = ON; }
	if (button == rewind_button) { minusOneButton = OFF; plusOneButton = OFF; rewindButton = ON;  playButton = OFF; repeatButton = OFF; }

	if ((previousPlayButtonStatus == OFF) && (playButton == ON))
		framesIncrementDoublePrecision = 1.0;  // Just start playing the animation, no time has been measured
}

/*
Set all skeletons to a specified frame (frameIndex)
If frameIndex is larger than the number of frames of the motion,
set the skeleton to the last frame of the motion
*/

void DisplaySkeleton::SetSkeletonsToSpecifiedFrame(int frameIndex) {
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

//need to define a new one for distance
void idle(void*) {
	if (rewindButton == ON) {
		currentFrameIndex = 0;
		currentFrameIndexDoublePrecision = 0.0;
		for (int i = 0; i < displayer.GetNumSkeletons(); i++)
		{
			if (displayer.GetSkeletonMotion(i) != NULL)
			{
				Posture* initSkeleton = displayer.GetSkeletonMotion(i)->GetPosture(0);
				displayer.GetSkeleton(i)->setPosture(*initSkeleton);
			}
		}
		rewindButton = OFF;
	}

	if (playButton == ON) {
		if (displayer.GetNumSkeletons() != 0)
		{
			currentFrameIndex++;
			if (currentFrameIndex >= maxFrames) {
				currentFrameIndex = maxFrames - 1;
				currentFrameIndexDoublePrecision = currentFrameIndex;
				playButton = OFF;  // important, especially in "recording" mode
			}
			frame_slider->value((double)currentFrameIndex + 1);

			displayer.SetSkeletonsToSpecifiedFrame(currentFrameIndex);
		}
	}  // if(playButton == ON)

	if (minusOneButton == ON)
		if (displayer.GetNumSkeletons() != 0)
		{
			currentFrameIndex--;
			if (currentFrameIndex < 0)
				currentFrameIndex = 0;
			frame_slider->value((double)currentFrameIndex + 1);

			displayer.SetSkeletonsToSpecifiedFrame(currentFrameIndex);
			minusOneButton = OFF;
		}

	if (plusOneButton == ON)
	{
		if (displayer.GetNumSkeletons() != 0)
		{
			currentFrameIndex++;
			if (currentFrameIndex >= maxFrames)
				currentFrameIndex = maxFrames - 1;
			frame_slider->value((double)currentFrameIndex + 1);

			displayer.SetSkeletonsToSpecifiedFrame(currentFrameIndex);
			plusOneButton = OFF;
		}
	}

	frame_slider->value((double)(currentFrameIndex + 1));

	previousPlayButtonStatus = playButton; // Super important updating

	glwindowMain->redraw();
}

void fslider_callback(Fl_Value_Slider* slider, long val) {
	currentFrameIndex = (int)frame_slider->value() - 1;
	currentFrameIndexDoublePrecision = currentFrameIndex;
	rewindButton = OFF;
	playButton = OFF;
	repeatButton = OFF;
	displayer.SetSkeletonsToSpecifiedFrame(currentFrameIndex);
	Fl::flush();
}

//NEEDS UPDATING
void DisplaySkeleton::GraphicsInit(DisplaySkeleton* displayer) {
	int red_bits, green_bits, blue_bits;
	struct { GLint x, y, width, height; } viewport;
	glEnable(GL_DEPTH_TEST);	/* turn on z-buffer */

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
	displayer->RenderGroundPlane(groundPlaneSize, groundPlaneHeight, groundPlaneR, groundPlaneG, groundPlaneB, groundPlaneAmbient, groundPlaneDiffuse, groundPlaneSpecular, groundPlaneShininess);
	glEndList();
}

/*
define the methods for glwindow and handle mouse events
don't make any openGL calls here as the context is not set
*/
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
	glwindowMain->redraw();

	return (handled);  // Returning one acknowledges that we handled this event
}

void Player_Gl_Window::draw() {
	draw(&displayer);
}

//Pre-written draw function, edited for the displayer
void Player_Gl_Window::draw(DisplaySkeleton* displayer) {
	// Upon setup of the window (or when Fl_Gl_Window->invalidate is called), 
	// the set of functions inside the if block are executed.
	if (!valid()) {
		displayer->GraphicsInit(displayer);
	}

	// Redisplay the screen then put the proper buffer on the screen.
	displayer->Redisplay();
}

int main() {
	char motions[][100] = { "movements\\walk.asf", 
		"movements\\walkMartial.amc",  "movements\\newWalkMartial.amc", "movements\\newAllWalk.amc"};
	char* skeletonPtr = motions[0];
	char* motionPtr = motions[1];
	char* newMotionPtr1 = motions[2];
	char* newMotionPtr2 = motions[3];
	int done = distance(skeletonPtr, motionPtr, newMotionPtr1, 100, 10, 1);
	//int done2 = distance(skeletonPtr, motionPtr, newMotionPtr2);

	//initialise form, sliders and buttons
	form = make_window();
	worldAxes_button->value(renderWorldAxes);
	frame_slider->value(1);

	//show form, and do initial draw of model
	form->show();
	glwindowMain->show(); // glwindow is initialized when the form is built

	//show individual frame
	int showMini = 0;
	if (showMini){
		smallForm = mini_window();
		smallForm->show();
		glwindowMini->show();
	}

	Fl::add_idle(idle);
	return Fl::run();
}
