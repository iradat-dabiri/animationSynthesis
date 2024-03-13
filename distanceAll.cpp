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

Skeleton* bSkeleton = NULL;
Motion* bMotion = NULL;
Motion* madebMotion = NULL;
cv::Point clickedPointb;

int distanceDeprecated(char* asf_filename, char* amc_filename, char* new_amc_filename) {
    //get skeleton
    bSkeleton = new Skeleton(asf_filename, MOCAP_SCALE);

    //get all postures
    bMotion = new Motion(amc_filename, MOCAP_SCALE, bSkeleton);
    int frames = bMotion->getNumFrames();

    // Create a vector of vectors to represent the 2D array
    std::vector<std::vector<double>> matrix(frames, std::vector<double>(frames));
    std::ofstream outputFile("arrayAll.txt");
    //compare each posture with every other posture
    double largest = matrix[0][0]; // Initialize largest to the first element, so we can find the largest
    for (int i = 0; i < frames; i++) {
        Posture* currentSkeleton = bMotion->GetPosture(i);
        vector rootPos = bMotion->GetRootPos(*currentSkeleton);
        bSkeleton->setPosture(*currentSkeleton);
        for (int j = 0; j < frames; j++) {
            Posture* tempSkeleton = bMotion->GetPosture(j);
            vector tempPos = bMotion->GetRootPos(*tempSkeleton);
            matrix[i][j] = findDistanceRoots(rootPos, tempPos);
            for(int x = 0; x < 31; x++){
                int currentVal = currentSkeleton->bone_rotation[x].p[0] + currentSkeleton->bone_rotation[x].p[1] + currentSkeleton->bone_rotation[x].p[2];
                for(int y = 0; y < 31; y++){
                    int tempVal = tempSkeleton->bone_rotation[x].p[0] + tempSkeleton->bone_rotation[x].p[1] + tempSkeleton->bone_rotation[x].p[2];
                    int diff = abs(tempVal-currentVal);
                    matrix[i][j] += diff;
                }
            }
            if (matrix[i][j] > largest) {
                largest = matrix[i][j];
            }
            outputFile << matrix[i][j] << " ";
        }
        outputFile << std::endl;
    }
    outputFile.close();
    std::cout << "Array has been saved to 'arrayAll.txt'." << std::endl;

    // Output the largest number
    std::cout << "The largest number in the array is: " << largest << std::endl;

    std::vector<std::vector<double>> normalised(frames, std::vector<double>(frames));
    std::ofstream outputFile2("arrayAllNormalised.txt");
    // Use std::transform to divide each element in the array by the divisor
    for (int i = 0; i < frames; ++i) {
        for (int j = 0; j < frames; ++j) {
            normalised[i][j] = round((matrix[i][j] / largest) * 255);
            //the best matches are black, and the worst are white
            outputFile2 << normalised[i][j] << " ";
        }
        outputFile2 << std::endl;
    }
    outputFile2.close();
    std::cout << "Array has been saved to 'arrayAllNormalised.txt'." << std::endl;

    // Create a blank image
    Mat image(frames, frames, CV_8UC1);
    // Set pixels in the image based on the values in the array
    for (int i = 0; i < frames; ++i) {
        for (int j = 0; j < frames; ++j) {
            image.at<uchar>(i, j) = normalised[i][j];
        }
    }
    // Display the image
    //cv::setMouseCallback("Distance Image", onMouse);
    imshow("Distance Image Full", image);
    cv::setMouseCallback("Distance Image Full", onMouse);
    imwrite("distance.png", image);

    //you want to fill a 1D array with the frame numbers that will make up a motion
    //to fill the array, we find the smallest distance of the row, and check that the frame has not already been used
    //with that array, you will fill madeMotion with the frames of aMotion in the correct order
    int length = 10;
    int start = 100;
    std::vector<std::vector<double>> motionIndexes(length, std::vector<double>(2));

    for (int i = 0; i < length; i++) {
        double lowest = largest;
        int position[2];
        position[0] = start;
        for (int j = 0; j < frames; j++) {
            //if i wanted to check for the specific position, but I am checking for the frame itself
            //otherwise it can go in a circle
            int index[2] = { start, j };
            //if (!arrayInArray(motionIndexes, length, index)) {
            if (!inArray(motionIndexes, length, j)) {
                if (matrix[start][j] < lowest && j != start) {
                    lowest = matrix[start][j];
                    position[1] = j;
                }
            }
        }
        printf("%d %d\n", position[0], position[1]);
        motionIndexes[i][0] = position[0];
        motionIndexes[i][1] = start = position[1];
    }

    madebMotion = new Motion(length, bSkeleton);
    for (int i = 0; i < length; i++) {
        madebMotion->SetPosture(i, *(bMotion->GetPosture(motionIndexes[i][0])));
    }

    madebMotion->writeAMCfile(new_amc_filename, MOCAP_SCALE);

    return 0;
}