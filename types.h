#ifndef _TYPES_H
#define _TYPES_H

// This adjusts the size of the skeleton, the default being 0.06 
// creating a skeleton of 1.7m in height
#define MOCAP_SCALE 0.06

//static const in NUM_BONES_IN_ASF_FILE = 31;
#define MAX_BONES_IN_ASF_FILE 256
#define MAX_CHAR 1024
#define MAX_SKELS 16

#define PM_MAX_FRAMES 60000

#ifndef M_PI
#define M_PI 3.14159265
#endif

enum ErrorType {
	NO_ERROR_SET = 0, BAD_OFFSET_FILE, NOT_SUPPORTED_INTERP_TYPE, BAD_INPUT_FILE
};
#endif
