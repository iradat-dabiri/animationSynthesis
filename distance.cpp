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


void jointTraverse(Bone* ptr) {
    if (ptr != NULL) {
        printf("%s", ptr->name);
        printf("%d\n", ptr->dir[0]);
        printf("%d\n", ptr->dir[1]);
        printf("%d\n", ptr->dir[2]);
        hereTraverse(ptr->child);
        hereTraverse(ptr->sibling);
    }
}

void checkBasicLocations(Skeleton * skel){
    double rootPos[3];
    skel->GetRootPosGlobal(rootPos);

    printf("%d\n", rootPos[0]);
    printf("%d\n", rootPos[1]);
    printf("%d\n", rootPos[2]);

    jointTraverse(skel->getRoot());
}

double findDistanceRoots(vector a, vector b) {
    return sqrt(pow(b[0]- a[0], 2) + pow(b[1] - a[1], 2) + pow(b[2] - a[2], 2));
}


int distanceRoots(char *asf_filename, char *amc_filename) {
    
    //get skeleton
    aSkeleton = new Skeleton(asf_filename, MOCAP_SCALE);
    checkBasicLocations(aSkeleton);

    //get all postures
    aMotion = new Motion(amc_filename, MOCAP_SCALE, aSkeleton);
    int frames = aMotion->getNumFrames();
    printf("%d frames here\n", frames);

    // Create a vector of vectors to represent the 2D array
    std::vector<std::vector<double>> matrix(frames, std::vector<double>(frames));

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
        }
    }

    // Save the 2D array to a text file
    std::ofstream outputFile("array.txt");
    if (outputFile.is_open()) {
        for (int i = 0; i < frames; ++i) {
            for (int j = 0; j < frames; ++j) {
                outputFile << matrix[i][j] << " ";
            }
            outputFile << std::endl;
        }
        outputFile.close();
        std::cout << "Array has been saved to 'array.txt'." << std::endl;
    }
    else {
        std::cerr << "Unable to open file for writing." << std::endl;
    }

    // Output the largest number
    std::cout << "The largest number in the array is: " << largest << std::endl;

     std::vector<std::vector<double>> normalised(frames, std::vector<double>(frames));
    // Use std::transform to divide each element in the array by the divisor
     for (int i = 0; i < frames; ++i) {
         for (int j = 0; j < frames; ++j) {
            normalised[i][j] = round((matrix[i][j]/ largest) * 255);
            //the best matches are black, and the worst are white
          }
     }

    // Save the 2D array to a text file
    std::ofstream outputFile2("arrayNormalised.txt");
    if (outputFile2.is_open()) {
        for (int i = 0; i < frames; ++i) {
            for (int j = 0; j < frames; ++j) {
                outputFile2 << normalised[i][j] << " ";
            }
            outputFile2 << std::endl;
        }
        outputFile2.close();
        std::cout << "Array has been saved to 'array.txt'." << std::endl;
    }
    else {
        std::cerr << "Unable to open file for writing." << std::endl;
    }

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

    return 0;
}