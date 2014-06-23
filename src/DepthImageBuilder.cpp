//
//  DepthImageBuilder.cpp
//
//  Created by James Alliban's MBP on 10/05/2014.
//
//

#include "DepthImageBuilder.h"
#include "ofxKinect.h"
#include "ofxKinectDepthUtils.h"

void DepthImageBuilder::setup(ofxKinect *_kinect, ofxKinectDepthUtils *_kinectUtils)
{
    kinect = _kinect;
    kinectUtils = _kinectUtils;
    didThreadExecute = false;
}


void DepthImageBuilder::threadedFunction()
{
    while(isThreadRunning())
    {
        // lock access to the resource
        lock();
        
        update();
        
        // done with the resource
        unlock();
        
    }
    //stopThread();
}


void DepthImageBuilder::update()
{
    if (didThreadExecute) return;
    didThreadExecute = true;
    
    
    int index;
    ofVec3f coordinate;
    ofPixels* depthImgPix = kinectUtils->getProcessedDepthXYZPix();
    int depthImgXYZWidth = depthImgPix->getWidth();
    int depthImgXYZHeight = depthImgPix->getHeight();
    int totalPixels = depthImgXYZWidth * (depthImgXYZHeight / 3) * 3;
    int totalPixelsDouble = totalPixels * 2;
    int step = kinectUtils->inDepthH / kinectUtils->outDepthH;
    
    for(int y = 0; y < depthImgXYZHeight / 3; y++)
    {
        for(int x = 0; x < depthImgXYZWidth; x++)
        {
            //kinectManager->kinect.getWorldCoordinateAt(x, y);
            index = (y * (depthImgXYZWidth * 3) + (x * 3));
            //printf("index:%i \n", index);
            
            // each channel of each pixel will be run through a formular to calculate the depth in millimeters
            // r is the number of times 255 fits into kinect.getWorldCoordinateAt(x, y).z);
            // g is the remaining figure
            // b is only used for x and y. It is used as a flag for +/- numbers - 0 = - and 100 = +
            //
            // so the z vaue of 1050 would be represented as 4, 30, 0
            coordinate = kinect->getWorldCoordinateAt(x * step, y * step);
            
            
            // z
            depthImgPix->getPixels()[index] = (int)(coordinate.z / 255);
            depthImgPix->getPixels()[index + 1] = (int)(coordinate.z) % 255;
            depthImgPix->getPixels()[index + 2] = 0;
            
            // x
            depthImgPix->getPixels()[index + totalPixels] = (int)(abs(coordinate.x) / 255);
            depthImgPix->getPixels()[index + totalPixels + 1] = (int)(abs(coordinate.x)) % 255;
            depthImgPix->getPixels()[index + totalPixels + 2] = (coordinate.x < 1) ? 0 : 100;
            
            // y
            depthImgPix->getPixels()[index + totalPixelsDouble] = (int)(abs(coordinate.y) / 255);
            depthImgPix->getPixels()[index + totalPixelsDouble + 1] = (int)(abs(coordinate.y)) % 255;
            depthImgPix->getPixels()[index + totalPixelsDouble + 2] = (coordinate.y < 1) ? 0 : 100;
        }
    }
}