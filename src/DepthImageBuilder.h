//
//  DepthImageBuilder.h
//
//  Created by James Alliban's MBP on 10/05/2014.
//
//

#pragma once

#include "ofMain.h"

class ofxKinect;
class ofxKinectDepthUtils;

class DepthImageBuilder : public ofThread
{
public:
    
    void setup(ofxKinect *_kinect, ofxKinectDepthUtils *_kinectUtils);
    void threadedFunction();
    void update();
    
    ofxKinect *kinect;
    ofxKinectDepthUtils *kinectUtils;
    bool didThreadExecute;
};