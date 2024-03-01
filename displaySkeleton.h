#ifndef _DISPLAY_SKELETON_H_
#define _DISPLAY_SKELETON_H_

#include <FL/glu.h>
#include <GL/GLU.h>
#include "skeleton.h"
#include "motion.h"

class DisplaySkeleton 
{

  //member functions
public: 
  enum RenderMode
  {
    BONES_ONLY, BONES_AND_LOCAL_FRAMES
  };
  enum JointColor
  {
    GREEN, RED, BLUE, NUMBER_JOINT_COLOURS
  };

  DisplaySkeleton();
  ~DisplaySkeleton();

  //set skeleton for display
  void LoadSkeleton(Skeleton * pSkeleton);
  //set motion for display
  void LoadMotion(Motion * pMotion);

  //display the scene (skeleton, ground plane ....)
  void Render(RenderMode renderMode);

  void SetDisplayedSpotJoint(int jointID) {m_SpotJoint = jointID;}
  int GetDisplayedSpotJoint(void) {return m_SpotJoint;}
  int GetNumSkeletons(void) {return numSkeletons;}
  Skeleton * GetSkeleton(int skeletonIndex);
  Motion * GetSkeletonMotion(int skeletonIndex);
  void RenderWorldAxes();
  void Redisplay();
  void RenderGroundPlane(double groundPlaneSize, double groundPlaneHeight, double rPlane,
	  double gPlane, double bPlane, double ambientFskeleton, double diffuseFskeleton,
	  double specularFskeleton, double shininess);
  void SetSkeletonsToSpecifiedFrame(int frameIndex);
  void GraphicsInit(DisplaySkeleton* displayer);
  void cameraView(void);

  void Reset(void);
  
protected:
   typedef unsigned int GLuint;

  RenderMode renderMode;
  // Draw a particular bone
  void DrawBone(Bone *ptr, int skelNum);
  // Draw the skeleton hierarchy
  void Traverse(Bone *ptr, int skelNum);
  // Model matrix for the shadow
  void DrawSpotJointAxis(void);
  void SetDisplayList(int skeletonID, Bone *bone, GLuint *pBoneList);

  int m_SpotJoint;		//joint whose local coordinate framework is drawn
  int numSkeletons;
  Skeleton *m_pSkeleton[MAX_SKELS];		//pointer to current skeleton
  Motion *m_pMotion[MAX_SKELS];		//pointer to current motion	
  GLuint m_BoneList[MAX_SKELS];		//display list with bones (was GLuint)

  static float jointColours[NUMBER_JOINT_COLOURS][3];
};

#endif
