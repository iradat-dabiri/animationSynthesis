#ifndef _DISTANCE_H
#define _DISTANCE_H
#include "skeleton.h"
#include "motion.h"

void onMouse(int event, int x, int y, int flags, void* userdata);
bool arrayInArray(std::vector<std::vector<double>> arr, int size, int index[2]);
bool inArray(std::vector<std::vector<double>> arr, int size, int index);
double findDistanceRoots(vector a, vector b);

int distanceRoots(char *asf_filename, char *amc_filename, char *new_amc_filename, int length, int start);
int distance(char *asf_filename, char *amc_filename, char *new_amc_filename);

#endif