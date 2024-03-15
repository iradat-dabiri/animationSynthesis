#include <stdio.h>
#include <string.h>
#include <fstream>
#include <math.h>
#include <stdlib.h>

#include "skeleton.h"
#include "motion.h"
#include "vector.h"

Motion::Motion(int numFrames_, Skeleton* pSkeleton_) {
	pSkeleton = pSkeleton_;
	m_NumFrames = numFrames_;

	//allocate postures array
	m_pPostures = new Posture[m_NumFrames];

	//Set all postures to default posture
	setPosturesToDefault();
}

Motion::Motion(char* amc_filename, double scale, Skeleton* pSkeleton_) {
	pSkeleton = pSkeleton_;
	m_NumFrames = 0;
	m_pPostures = NULL;

	int code = readAMCfile(amc_filename, scale);
	if (code < 0)
		throw 1;
}

Motion::~Motion() {
	if (m_pPostures != NULL)
		delete[] m_pPostures;
}

void Motion::setPosturesToDefault() {
	for (int frame = 0; frame < m_NumFrames; frame++) {
		//set root position to (0,0,0)
		m_pPostures[frame].root_pos.setValue(0.0, 0.0, 0.0);
		//set each bone orientation to (0,0,0)
		for (int j = 0; j < MAX_BONES_IN_ASF_FILE; j++)
			m_pPostures[frame].bone_rotation[j].setValue(0.0, 0.0, 0.0);
	}
}

//set posture at a specified frame
void Motion::setPosture(int frameIndex, Posture inPosture) {
	m_pPostures[frameIndex] = inPosture;
}

void Motion::setBoneRotation(int frameIndex, int boneIndex, vector vRot) {
	m_pPostures[frameIndex].bone_rotation[boneIndex] = vRot;
}

void Motion::setRootPos(int frameIndex, vector vPos) {
	m_pPostures[frameIndex].root_pos = vPos;
}

vector Motion::getRootPos(Posture inPosture){
	return inPosture.root_pos;
}

Posture* Motion::getPosture(int frameIndex) {
	if (frameIndex < 0 || frameIndex >= m_NumFrames) {
		printf("Error in Motion::getPosture: frame index %d is illegal.\n", frameIndex);
		printf("m_NumFrames = %d\n", m_NumFrames);
		exit(0);
	}
	//printf("%d", m_pPostures[frameIndex]);
	return &(m_pPostures[frameIndex]);
}

int Motion::readAMCfile(char* name, double scale) {
	Bone* hroot, * bone;
	bone = hroot = pSkeleton->getRoot();

	std::ifstream file(name, std::ios::in);
	if (file.fail()) return -1;

	int n = 0;
	char str[2048];

	//count the number of lines
	while (!file.eof()) {
		file.getline(str, 2048);
		if (file.eof()) break;
		if (strcmp(str, "") != 0) n++;
	}
	file.close();
	
	//compute number of frames, ignoring the header
	//there are NUM_BONES_IN_ASF_FILE -2 moving bones and 2 dummy (lhipjoiny and rhipjoint)
	int numBones = pSkeleton->numBonesInSkel(bone[0]);
	int movbones = pSkeleton->movBonesInSkel(bone[0]);
	n = (n - 3) / (movbones + 1);
	m_NumFrames = n;
	//allocate memory for state vector
	m_pPostures = new Posture[m_NumFrames];
	setPosturesToDefault();
	file.open(name);
	//process the header (add rotational DOFs if requested)
	while (1) {
		file >> str;
		
		if (strcmp(str, ":FORCE-ALL-JOINTS-BE-3DOF") == 0)
			pSkeleton->enableAllRotationalDOFs();

		if (strcmp(str, ":DEGREES") == 0)
			break;
	}

	for (int i = 0; i < m_NumFrames; i++) {
		int frame_num;
		file >> frame_num;

		for (int j = 0; j < movbones; j++) {
			//read bone name
			file >> str;

			//find the bone index corresponding to the bone name
			int bone_idx;
			for (bone_idx = 0; bone_idx < numBones; bone_idx++)
				if (strcmp(str, pSkeleton->idx2name(bone_idx)) == 0)
					break;

			m_pPostures[i].bone_rotation[bone_idx].setValue(0.0, 0.0, 0.0);
			for (int x = 0; x < bone[bone_idx].dof; x++) {
				double tmp;
				file >> tmp;
				switch (bone[bone_idx].dofo[x]) {
				case 0:
					printf("FATAL ERROR in bone %d not found %d\n", bone_idx, x);
					x = bone[bone_idx].dof;
					break;
				case 1:
					m_pPostures[i].bone_rotation[bone_idx].p[0] = tmp;
					break;
				case 2:
					m_pPostures[i].bone_rotation[bone_idx].p[1] = tmp;
					break;
				case 3:
					m_pPostures[i].bone_rotation[bone_idx].p[2] = tmp;
					break;
				case 4:
					m_pPostures[i].bone_translation[bone_idx].p[0] = tmp * scale;
					break;
				case 5:
					m_pPostures[i].bone_translation[bone_idx].p[1] = tmp * scale;
					break;
				case 6:
					m_pPostures[i].bone_translation[bone_idx].p[2] = tmp * scale;
					break;
				case 7:
					m_pPostures[i].bone_length[bone_idx].p[0] = tmp;// * scale;
					break;
				}
			}
			if (strcmp(str, "root") == 0) {
				m_pPostures[i].root_pos.p[0] = m_pPostures[i].bone_translation[0].p[0];
				m_pPostures[i].root_pos.p[1] = m_pPostures[i].bone_translation[0].p[1];
				m_pPostures[i].root_pos.p[2] = m_pPostures[i].bone_translation[0].p[2];
			}
			//read joint angles, including root orientation
		}
	}
	file.close();
	printf("%d samples in '%s' are read.\n", n, name);
	return n;
}

int Motion::writeAMCfile(char* filename, double scale, int forceAllJointsBe3DOF) {
	Bone* bone = pSkeleton->getRoot();

	std::ofstream os(filename);
	if (os.fail()) return -1;

	//header
	os << ":FULLY-SPECIFIED" << std::endl;
	if (forceAllJointsBe3DOF) os << ":FORCE-ALL-JOINTS-BE-3DOF" << std::endl;
	os << ":DEGREES" << std::endl;

	int numBones = pSkeleton->numBonesInSkel(bone[0]);

	int root = Skeleton::getRootIndex();
	for (int f = 0; f < m_NumFrames; f++) {
		os << f + 1 << std::endl;
		os << "root "
			<< m_pPostures[f].root_pos.p[0] / scale << " "
			<< m_pPostures[f].root_pos.p[1] / scale << " "
			<< m_pPostures[f].root_pos.p[2] / scale << " "
			<< m_pPostures[f].bone_rotation[root].p[0] << " "
			<< m_pPostures[f].bone_rotation[root].p[1] << " "
			<< m_pPostures[f].bone_rotation[root].p[2];

		for (int j = 2; j < numBones; j++) {
			//output bone name
			if (bone[j].dof != 0) {
				os << std::endl << pSkeleton->idx2name(j);

				//output bone rotation angles
				for (int d = 0; d < bone[j].dof; d++) {
					//traverse all DOFs, checking if they are enabled, then outputting them
					if (bone[j].dofo[d] == 1) {
						if (bone[j].dofrx == 1)
							os << " " << m_pPostures[f].bone_rotation[j].p[0];
					}
					if (bone[j].dofo[d] == 2) {
						if (bone[j].dofry == 1)
							os << " " << m_pPostures[f].bone_rotation[j].p[1];
					}
					if (bone[j].dofo[d] == 3) {
						if (bone[j].dofrz == 1)
							os << " " << m_pPostures[f].bone_rotation[j].p[2];
					}
				}
			}
		}
		os << std::endl;
	}

	os.close();
	printf("Write %d samples to '%s' \n", m_NumFrames, filename);
	return 0;
}
