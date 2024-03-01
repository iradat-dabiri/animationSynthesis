#ifndef _MOTION_H
#define _MOTION_H

#include "vector.h"
#include "types.h"
#include "posture.h"
#include "skeleton.h"

class Motion {
public:
	Motion(char* amc_filename, double scale, Skeleton* pSkeleton);

	Motion(int numFrames, Skeleton* pSkeleton);
	~Motion();

	int writeAMCfile(char* filename, double scale, int forceAllJointsBe3DOF = 0);

	void SetPosturesToDefault();
	void SetPosture(int frameIndex, Posture InPosture);

	void SetRootPos(int frameIndex, vector vPos);
	vector GetRootPos(Posture InPosture);
	void SetBoneRotation(int frameIndex, int boneIndex, vector vRot);

	int getNumFrames() { return m_NumFrames; }
	Posture* GetPosture(int frameIndex);
	Skeleton* GetSkeleton() { return pSkeleton; }

protected:
	int m_NumFrames;
	Skeleton* pSkeleton;
	Posture* m_pPostures;

	int readAMCfile(char* name, double scale);
};

#endif
