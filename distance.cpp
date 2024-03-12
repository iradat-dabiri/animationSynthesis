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
cv::Point clickedPoint;
double largest;

void onMouse(int event, int x, int y, int flags, void* userdata) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        clickedPoint = cv::Point(x, y);
        std::cout << "Clicked position: " << clickedPoint << std::endl;
    }
}

bool arrayInArray(std::vector<std::vector<double>> arr, int size, int index[2]) {
    for (int i = 0; i < size; ++i) {
        if (arr[i][0] == index[0] && arr[i][1] == index[1]) { return true; } //pair exists in array
    }
    return false;  // Number does not exist in the array
}

bool inArray(std::vector<std::vector<double>> arr, int size, int index) {
    for (int i = 0; i < size; ++i) {
        if (arr[i][0] == index) { return true; } // Number exists in the array
    }
    return false;  // Number does not exist in the array
}

std::vector<std::vector<double>> walk(int frames, int start, int length, std::vector<std::vector<double>> arrIndexes, std::vector<std::vector<double>> arr){
    for(int i = 0; i < length; i++){
        double lowest = largest;
        int position[2];
        position[0] = start;
        for(int j = 0; j < frames; j++){
            //if i wanted to check for the specific position, but I am checking for the frame itself
            //otherwise it can go in a circle
            int index[2] = { start, j };
            //if (!arrayInArray(motionIndexes, length, index)) {
            if (!inArray(arrIndexes, length, j)){
                if (arr[start][j] < lowest && j != start) {
                    lowest = arr[start][j];
                    position[1] = j;
                }
            }
        }
        printf("%d %d\n", position[0], position[1]);
        arrIndexes[i][0] = position[0];
        arrIndexes[i][1] = start = position[1];
    }

    return arrIndexes;
}

double findDistanceRoots(vector a, vector b) {
    return sqrt(pow(b[0]- a[0], 2) + pow(b[1] - a[1], 2) + pow(b[2] - a[2], 2));
}

std::vector<std::vector<double>> compare(int frames, std::vector<std::vector<double>> arr){
    std::ofstream outputFile("array.txt");
    for(int i = 0; i < frames; i++){
        Posture* currentPosture = aMotion->GetPosture(i);
        vector rootPos = aMotion->GetRootPos(*currentPosture);
        for(int j = 0; j < frames; j++){
            Posture* tempPosture = aMotion->GetPosture(j);
            vector tempPos = aMotion->GetRootPos(*tempPosture);
            arr[i][j] = findDistanceRoots(rootPos, tempPos);
            if (arr[i][j] > largest) {
                largest = arr[i][j];
            }
            outputFile << arr[i][j] << " ";
        }
        outputFile << std::endl;
    }
    outputFile.close();
    std::cout << "Array has been saved to 'array.txt'." << std::endl;
    return arr;
}

std::vector<std::vector<double>> normal(int frames, std::vector<std::vector<double>> arrNorm, std::vector<std::vector<double>> arr){
    std::ofstream outputFile("arrayNormalised.txt");
    // Use std::transform to divide each element in the array by the divisor
    for (int i = 0; i < frames; ++i) {
        for (int j = 0; j < frames; ++j) {
            arrNorm[i][j] = round((arr[i][j]/ largest) * 255);
            //the best matches are black, and the worst are white
            outputFile << arrNorm[i][j] << " ";
          }
        outputFile << std::endl;
     }
    outputFile.close();
    std::cout << "Array has been saved to 'arrayNormalised.txt'." << std::endl;

    return arrNorm;
}

int distanceRoots(char *asf_filename, char *amc_filename, char *new_amc_filename, int length, int start) { 
    //get skeleton
    aSkeleton = new Skeleton(asf_filename, MOCAP_SCALE);

    //get all postures
    aMotion = new Motion(amc_filename, MOCAP_SCALE, aSkeleton);
    int frames = aMotion->getNumFrames();
    //aSkeleton->setPosture(*(aMotion->GetPosture(1)));

    // Create a vector of vectors to represent the 2D array
    std::vector<std::vector<double>> matrix(frames, std::vector<double>(frames));
    //compare each posture with every other posture
    matrix = compare(frames, matrix);

    // Output the largest number
    std::cout << "The largest number in the array is: " << largest << std::endl;

    //normalise this array for image purposes
    std::vector<std::vector<double>> normalised(frames, std::vector<double>(frames));
    normalised = normal(frames, normalised, matrix);

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
    cv::setMouseCallback("Distance Image", onMouse);
    imwrite("distance.png", image);

    //you want to fill a 1D array with the frame numbers that will make up a motion
    //to fill the array, we find the smallest distance of the row, and check that the frame has not already been used
    //with that array, you will fill madeMotion with the frames of aMotion in the correct order
    std::vector<std::vector<double>> motionIndexes(length, std::vector<double>(2));
    motionIndexes = walk(frames, start, length,  motionIndexes, matrix);
    
    madeMotion = new Motion(length, aSkeleton);
    for(int i = 0; i < length; i++){
        madeMotion->SetPosture(i, *(aMotion->GetPosture(motionIndexes[i][0])));
    }
    madeMotion->writeAMCfile(new_amc_filename, MOCAP_SCALE);

    return 0;
}