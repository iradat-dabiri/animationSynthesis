#ifndef _POSTURE_H
#define _POSTURE_H

#include "vector.h"
#include "types.h"

// This struct holds the root and bone positions and rotation angles using Euler angles
// It takes into account DOF, setting rotation to 0 if it is not possible
// The order of the bones correspond to their ids
struct Posture {
public:
	vector root_pos;
	vector bone_rotation[MAX_BONES_IN_ASF_FILE];
	vector bone_translation[MAX_BONES_IN_ASF_FILE];
	vector bone_length[MAX_BONES_IN_ASF_FILE];
};

#endif
