#ifndef _SKELETON_H
#define _SKELETON_H

#include "posture.h"

struct Bone {
	struct Bone* sibling;
	struct Bone* child;

	int idx;

	double dir[3];

	double length; 

	double axis_x, axis_y, axis_z;

	double aspx, aspy;

	int dof;
	int dofrx, dofry, dofrz;  
	int doftx, dofty, doftz;
	int doftl;
	
	char name[256];
	double rot_parent_current[4][4];

	double rx, ry, rz;
	double tx, ty, tz;
	double tl;
	int dofo[8];
};


class Skeleton {
public:
	Skeleton(char* asf_filename, double scale);
	~Skeleton();

	Bone* getRoot();
	static int getRootIndex() { return 0; }
    
	void setPosture(Posture posture);

	void setBasePosture();

	void enableAllRotationalDOFs();

	int name2idx(char*);
	char* idx2name(int);
	void GetRootPosGlobal(double rootPosGlobal[3]);
	void GetTranslation(double translation[3]);
	void GetRotationAngle(double rotationAngle[3]);
	void SetTranslationX(double tx_) { tx = tx_; }
	void SetTranslationY(double ty_) { ty = ty_; }
	void SetTranslationZ(double tz_) { tz = tz_; }
	void SetRotationAngleX(double rx_) { rx = rx_; }
	void SetRotationAngleY(double ry_) { ry = ry_; }
	void SetRotationAngleZ(double rz_) { rz = rz_; }

	int numBonesInSkel(Bone bone);
	int movBonesInSkel(Bone bone);

protected:	
	int readASFfile(char* asf_filename, double scale);

	Bone* getBone(Bone* ptr, int bIndex);

	int setChildrenAndSibling(int parent, Bone* pChild);

	void RotateBoneDirToLocalCoordSystem();

	void set_bone_shape(Bone* bone);
	void compute_rotation_parent_child(Bone* parent, Bone* child);
	void ComputeRotationToParentCoordSystem(Bone* bone);

	double m_RootPos[3];
	double tx, ty, tz;
	double rx, ry, rz;

	int NUM_BONES_IN_ASF_FILE;
	int MOV_BONES_IN_ASF_FILE;

	Bone* m_pRootBone;
	Bone  m_pBoneList[MAX_BONES_IN_ASF_FILE];

	void removeChar(char* str);
};

#endif

