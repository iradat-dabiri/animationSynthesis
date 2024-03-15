#ifndef _DISPLAY_SKELETON_H_
#define _DISPLAY_SKELETON_H_

#include <FL/glu.h>
#include <GL/GLU.h>
#include "skeleton.h"
#include "motion.h"

class DisplaySkeleton  {
  //member functions
public: 
  enum RenderMode { BONES_ONLY, BONES_AND_LOCAL_FRAMES };
  enum JointColor { GREEN, RED, BLUE, NUMBER_JOINT_COLOURS };

  DisplaySkeleton();
  ~DisplaySkeleton();

  //set skeleton for display
  void loadSkeleton(Skeleton * pSkeleton);
  //set motion for display
  void loadMotion(Motion * pMotion);

  //display the scene (skeleton, ground plane ....)
  void render(RenderMode renderMode);

  void setDisplayedSpotJoint(int jointID) {m_SpotJoint = jointID;}
  int getDisplayedSpotJoint(void) {return m_SpotJoint;}
  int getNumSkeletons(void) {return numSkeletons;}
  Skeleton * getSkeleton(int skeletonIndex);
  Motion * getSkeletonMotion(int skeletonIndex);
  void renderWorldAxes();
  void redisplay();
  void renderGroundPlane(double groundPlaneSize, double groundPlaneHeight, double rPlane,
	  double gPlane, double bPlane, double ambientFskeleton, double diffuseFskeleton,
	  double specularFskeleton, double shininess);
  void setSkeletonsToSpecifiedFrame(int frameIndex);
  void graphicsInit(DisplaySkeleton* displayer);
  void cameraView(void);

  void reset(void);
  
protected:
  RenderMode renderMode;
  // Draw a particular bone
  void drawBone(Bone *ptr, int skelNum);
  // Draw the skeleton hierarchy
  void traverse(Bone *ptr, int skelNum);
  // Model matrix for the shadow
  void drawSpotJointAxis(void);
  void setDisplayList(int skeletonID, Bone *bone, GLuint *pBoneList);

  int m_SpotJoint;		//joint whose local coordinate framework is drawn
  int numSkeletons;
  Skeleton *m_pSkeleton[MAX_SKELS];		//pointer to current skeleton
  Motion *m_pMotion[MAX_SKELS];		//pointer to current motion	
  GLuint m_BoneList[MAX_SKELS];		//display list with bones (was GLuint)

  static float jointColours[NUMBER_JOINT_COLOURS][3];
};

#endif
