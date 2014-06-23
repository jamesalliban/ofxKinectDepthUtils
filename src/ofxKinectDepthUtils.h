//
//  ofxKinectDepthUtils.h
//
//  Created by James Alliban's MBP on 30/04/2014.
//
//

#pragma once

#include "ofMain.h"
#include "ofxKinect.h"
#include "DepthSmoothShader.h"
#include "NormalMapShader.h"
#include "DepthImageBuilder.h"


class ofxKinectDepthUtils
{
public:
    void setup(ofxKinect *_kinect);
    void processKinectData();
    void drawProcessing(int x, int y);
    void loadShaders(); // when loading shaders externally
    
    ofFbo getDrawFbo() { return drawFbo; };
    
    ofPixels getProcessedDepthXYZPixCopy();
    
    float getPercentWithinThreshold(int minThreshold, int maxThreshold);
    
    bool getDepthSmoothingActive() { return isDepthSmoothingActive; };
    void setDepthSmoothingActive(bool val) { isDepthSmoothingActive = val; };
    
    float getNearThreshold() { return nearThreshold; };
    void setNearThreshold(float val) { nearThreshold = val; };
    
    float getFarThreshold() { return farThreshold; };
    void setFarThreshold(float val) { farThreshold = val; };
    
    float getMeshBlurRadius() { return meshBlurRadius; };
    void setMeshBlurRadius(float val) { meshBlurRadius = val; };
    
    float getZAveragingMaxDepth() { return zAveragingMaxDepth; };
    void setZAveragingMaxDepth(float val) { zAveragingMaxDepth = val; };
    
    float getBlankDepthPixMax() { return blankDepthPixMax; };
    void setBlankDepthPixMax(float val) { blankDepthPixMax = val; };
    
    bool getSmoothingThresholdOnly() { return isSmoothingThresholdOnly; };
    void setSmoothingThresholdOnly(bool val) { isSmoothingThresholdOnly = val; };
    
    bool getNormalMapThresholdOnly() { return isNormalMapThresholdOnly; };
    void setNormalMapThresholdOnly(bool val) { isNormalMapThresholdOnly = val; };
    
    bool getFlippedX() { return isFlippedX; };
    void setFlippedX(bool val) { isFlippedX = val; };
    
    bool getFlippedY() { return isFlippedY; };
    void setFlippedY(bool val) { isFlippedY = val; };

    
//    bool isMacMiniMeshFix;
    
    ofImage* getDepthXYZ();
    ofPixels* getProcessedDepthXYZPix();
    ofVec3f getProcessedVertex(int x, int y);
    
    
    int inDepthW, inDepthH;
    int outDepthW, outDepthH;
    
    
private:
    void createShaders();
    void updateDepthImage(ofxKinect *kinect);
    
    ofxKinect *kinect;
    
    ofFbo depthPingpongFbo[2];
    int currDepthFbo;
    ofFbo normalFbo;
    ofFbo drawFbo;
    ofFbo depthTestFbo;
    ofPixels depthTestPixels;
    
    DepthSmoothShader kinectZSmoothShader;
    NormalMapShader normalMapShader;
    
    DepthImageBuilder depthImageBuilder;
    
    ofImage depthImageXYZ;
    ofFbo processedDepthXYZFbo;
    ofPixels processedDepthXYZPix;
    
    bool isDepthSmoothingActive;
    
    float nearThreshold;
    float farThreshold;
    
    float meshBlurRadius;
    float zAveragingMaxDepth;
    float modifiedNearThreshold;
    float modifiedFarThreshold;
    float blankDepthPixMax;
    float isSmoothingThresholdOnly;
    
    bool isNormalMapThresholdOnly;
    bool isMacMiniMeshFix;
    
    bool isFlippedX;
    bool isFlippedY;
};