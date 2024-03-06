#include <iostream>
#include <cmath>
#include <fstream>
#include <algorithm>
#include "skeleton.h"
#include "motion.h"
#include "distance.h"
#include "vector.h"
#include <opencv2/opencv.hpp>
using namespace cv;

Skeleton* aSkeleton = NULL;
Motion* aMotion = NULL;
Motion* madeMotion = NULL;

bool arrayInArray(std::vector<std::vector<double>> arr, int size, int index[2]) {
    for (int i = 0; i < size; ++i) {
        if (arr[i][0] == index[0] && arr[i][1] == index[1]) {
            return true;  //pair exists in array
        }
    }
    return false;  // Number does not exist in the array
}

bool inArray(std::vector<std::vector<double>> arr, int size, int index) {
    for (int i = 0; i < size; ++i) {
        if (arr[i][0] == index) {
            return true;  // Number exists in the array
        }
    }
    return false;  // Number does not exist in the array
}

void walk(int start, int length, int frames){
    //put it here
}

void jointTraverse(Bone* ptr) {
    if (ptr != NULL) {
        printf("%s\n", ptr->name);
        //figure out the maths for the points as dir only keeps the default points
        //printf("%d\n", ptr->dir[0]);
        //printf("%d\n", ptr->dir[1]);
        //printf("%d\n", ptr->dir[2]);
        jointTraverse(ptr->child);
        jointTraverse(ptr->sibling);
    }
}

void checkBasicLocations(Skeleton * skel){
    double rootPos[3];
    skel->GetRootPosGlobal(rootPos);
    printf("%d %d %d\n", rootPos[0], rootPos[1], rootPos[2]);
    jointTraverse(skel->getRoot());
}

double findDistanceRoots(vector a, vector b) {
    return sqrt(pow(b[0]- a[0], 2) + pow(b[1] - a[1], 2) + pow(b[2] - a[2], 2));
}


int distanceRoots(char *asf_filename, char *amc_filename, char *new_amc_filename) { 
    //get skeleton
    aSkeleton = new Skeleton(asf_filename, MOCAP_SCALE);
    //checkBasicLocations(aSkeleton);

    //get all postures
    aMotion = new Motion(amc_filename, MOCAP_SCALE, aSkeleton);
    int frames = aMotion->getNumFrames();
    aSkeleton->setPosture(*(aMotion->GetPosture(1)));
    //checkBasicLocations(aSkeleton);

    // Create a vector of vectors to represent the 2D array
    std::vector<std::vector<double>> matrix(frames, std::vector<double>(frames));
    std::ofstream outputFile("array.txt");
    //compare each posture with every other posture
    double largest = matrix[0][0]; // Initialize largest to the first element, so we can find the largest
    for(int i = 0; i < frames; i++){
        Posture* currentSkeleton = aMotion->GetPosture(i);
        vector rootPos = aMotion->GetRootPos(*currentSkeleton);
        /*
        for(int j = 0; j < 3; j++){
            printf("%d", rootPos[0]);
        }*/
        for(int j = 0; j < frames; j++){
            Posture* tempSkeleton = aMotion->GetPosture(j);
            vector tempPos = aMotion->GetRootPos(*tempSkeleton);
            matrix[i][j] = findDistanceRoots(rootPos, tempPos);
            if (matrix[i][j] > largest) {
                largest = matrix[i][j];
            }
            outputFile << matrix[i][j] << " ";
        }
        outputFile << std::endl;
    }
    outputFile.close();
    std::cout << "Array has been saved to 'array.txt'." << std::endl;

    // Output the largest number
    std::cout << "The largest number in the array is: " << largest << std::endl;

    std::vector<std::vector<double>> normalised(frames, std::vector<double>(frames));
    std::ofstream outputFile2("arrayNormalised.txt");
    // Use std::transform to divide each element in the array by the divisor
    for (int i = 0; i < frames; ++i) {
        for (int j = 0; j < frames; ++j) {
            normalised[i][j] = round((matrix[i][j]/ largest) * 255);
            //the best matches are black, and the worst are white
            outputFile2 << normalised[i][j] << " ";
          }
        outputFile2 << std::endl;
     }
    outputFile2.close();
    std::cout << "Array has been saved to 'arrayNormalised.txt'." << std::endl;

    // Create a blank image
    Mat image(frames, frames, CV_8UC1);
    // Set pixels in the image based on the values in the array
    for (int i = 0; i < frames; ++i) {
        for (int j = 0; j < frames; ++j) {
            image.at<uchar>(i, j) = normalised[i][j];
        }
    }
    // Display the image
    imshow("Distance Image", image);
    imwrite("distance.png", image);

    //you want to fill a 1D array with the frame numbers that will make up a motion
    //to fill the array, we find the smallest distance of the row, and check that the frame has not already been used
    //with that array, you will fill madeMotion with the frames of aMotion in the correct order
    int length = 200;
    int start =100;
    std::vector<std::vector<double>> motionIndexes(length, std::vector<double>(2));

    for(int i = 0; i < length; i++){
        double lowest = largest;
        int position[2];
        for(int j = 0; j < frames; j++){
            //if i wanted to check for the specific position, but I am checking for the frame itself
            //otherwise it can go in a circle
            int index[2] = { start, j };
            //if (!arrayInArray(motionIndexes, length, index)) {
            if (!inArray(motionIndexes, length, j)){
                if (matrix[start][j] < lowest && j != start) {
                    lowest = matrix[start][j];
                    position[0] = start;
                    position[1] = j;
                    //printf("%d", j);
                }
            }
        }
        //printf("%d\n", lowest);
        printf("%d %d\n", position[0], position[1]);
        motionIndexes[i][0] = position[0];
        motionIndexes[i][1] = start = position[1];
    }

    madeMotion = new Motion(length, aSkeleton);
    for(int i = 0; i < length; i++){
        madeMotion->SetPosture(i, *(aMotion->GetPosture(motionIndexes[i][0])));
    }

    madeMotion->writeAMCfile(new_amc_filename, MOCAP_SCALE);

    return 0;
}