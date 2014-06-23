//
//  ofxKinectDepthUtils.cpp
//
//  Created by James Alliban's MBP on 30/04/2014.
//
//

#include "ofxKinectDepthUtils.h"


void ofxKinectDepthUtils::setup(ofxKinect *_kinect)
{
    kinect = _kinect;
    
    inDepthW = 640;
	inDepthH = 480;
    
    outDepthW = inDepthW * 0.5;
    outDepthH = inDepthH * 0.5;
    
    ofFbo::Settings settings2;
    settings2.width = outDepthW;
    settings2.height = outDepthH;
    settings2.internalformat = GL_RGB;
    
    depthPingpongFbo[0].allocate(settings2);
    depthPingpongFbo[1].allocate(settings2); //currDepthFbo
    currDepthFbo = 0;
    
    ofFbo::Settings settings3;
    settings3.width = outDepthW;
    settings3.height = (outDepthH) * 3;
    settings3.internalformat = GL_RGB;
    
    
    ofFbo::Settings drawSettings;
    drawSettings.width = outDepthW * 3;
    drawSettings.height = outDepthH;
    drawSettings.internalformat = GL_RGB;
    
    processedDepthXYZFbo.allocate(settings3);
    normalFbo.allocate(settings2);
    depthImageXYZ.allocate(outDepthW, outDepthH * 3, OF_IMAGE_COLOR);
	processedDepthXYZPix.allocate(outDepthW, outDepthH * 3, OF_IMAGE_COLOR);
    drawFbo.allocate(drawSettings);
    
    
    meshBlurRadius = 1; // range - 1-10
    zAveragingMaxDepth = 195; // range - 0-200
    nearThreshold = 0;
    farThreshold = 4000;
    blankDepthPixMax = 7; // range - 1-10
    isSmoothingThresholdOnly = false;
    isMacMiniMeshFix = false;
    
    isDepthSmoothingActive = true;
    
    createShaders();
    
    depthImageBuilder.setup(kinect, this);
    //depthImageBuilder.startThread(true, false); // blocking, verbose
    
    ofFbo::Settings depthTestSettings;
    depthTestSettings.width = 32;
    depthTestSettings.height = 24;
    depthTestSettings.internalformat = GL_RGB;
    depthTestFbo.allocate(depthTestSettings);
}



void ofxKinectDepthUtils::processKinectData()
{
#ifdef TARGET_WIN32
    modifiedNearThreshold = nearThreshold * windowsZModifier;
	modifiedFarThreshold = farThreshold * windowsZModifier;
#endif
#ifdef TARGET_OSX
	modifiedNearThreshold = nearThreshold;
	modifiedFarThreshold = farThreshold;
#endif
    
    
    // encode x, y and z values from kinect world coordinates into a texture
    // Uncomment the next 3 lines and comment the 4th to perform this in a thread. The only issue is that the next frame.
//    depthImageBuilder.unlock();
//    depthImageBuilder.didThreadExecute = false;
//    depthImageXYZ.setFromPixels(depthImageXYZPix);
    updateDepthImage(kinect);
    
    
    // fill the holes
    //    holeFillerShader.begin();
    //    holeFillerShader.setUniform1f("farThreshold", modifiedFarThreshold);
    //    holeFillerFBO.begin();
    //    ofClear(0.0, 0.0, 0.0);
    //    depthImageXYZ.draw(0, 0);
    //    holeFillerFBO.end();
    //    holeFillerShader.end();
    
    
    if (isDepthSmoothingActive)
    {
        currDepthFbo = 1 - currDepthFbo;
        kinectZSmoothShader.begin();
        depthPingpongFbo[currDepthFbo].begin();
        ofClear(0, 0, 0, 1);
        kinectZSmoothShader.setUniform1i("blurSize", meshBlurRadius);
        kinectZSmoothShader.setUniform1f("minDistForAveraging", zAveragingMaxDepth);
        kinectZSmoothShader.setUniform1i("isHorizontal", 1);
        kinectZSmoothShader.setUniform1f("nearThreshold", modifiedNearThreshold);
        kinectZSmoothShader.setUniform1f("farThreshold", modifiedFarThreshold);
        kinectZSmoothShader.setUniform1i("blankDepthPixMax", blankDepthPixMax);
        kinectZSmoothShader.setUniform1i("isThresholdOnly", (isSmoothingThresholdOnly) ? 1 : 0);
        kinectZSmoothShader.setUniform1i("isMacMiniMeshFix", (isMacMiniMeshFix) ? 1 : 0);
        depthImageXYZ.draw(0, 0);
        depthPingpongFbo[currDepthFbo].end();
        kinectZSmoothShader.end();
        
        currDepthFbo = 1 - currDepthFbo;
        depthPingpongFbo[currDepthFbo].begin();
        ofClear(0, 0, 0);
        kinectZSmoothShader.begin();
        kinectZSmoothShader.setUniform1i("blurSize", meshBlurRadius);
        kinectZSmoothShader.setUniform1f("minDistForAveraging", zAveragingMaxDepth);
        kinectZSmoothShader.setUniform1i("isHorizontal", 0);
        kinectZSmoothShader.setUniform1f("nearThreshold", modifiedNearThreshold);
        kinectZSmoothShader.setUniform1f("farThreshold", modifiedFarThreshold);
        kinectZSmoothShader.setUniform1i("blankDepthPixMax", blankDepthPixMax);
        kinectZSmoothShader.setUniform1i("isThresholdOnly", (isSmoothingThresholdOnly) ? 1 : 0);
        kinectZSmoothShader.setUniform1i("isMacMiniMeshFix", (isMacMiniMeshFix) ? 1 : 0);
        depthPingpongFbo[1 - currDepthFbo].draw(0, 0);
        kinectZSmoothShader.end();
        depthPingpongFbo[currDepthFbo].end();
    }
    else
    {
//        if (isDepthSmoothingActive)
        currDepthFbo = 1 - currDepthFbo;
        depthPingpongFbo[currDepthFbo].begin();
        ofClear(0, 0, 0, 1);
        depthImageXYZ.draw(0, 0);
        depthPingpongFbo[currDepthFbo].end();
    }
    
    
    // create an fbo containing x, y and smoothed z images in preparation for normal mapping
    ofSetColor(255);
    processedDepthXYZFbo.begin();
    depthImageXYZ.draw(0, -outDepthH);
    //currDepthFbo = 1 - currDepthFbo;
    depthPingpongFbo[currDepthFbo].draw(0, outDepthH * 2);
    processedDepthXYZFbo.end();
    
    processedDepthXYZFbo.getTextureReference().readToPixels(processedDepthXYZPix);
    
    isNormalMapThresholdOnly = false;
    // calculate a normal map from my custom depthImage on the GPU
    normalFbo.begin();
    normalMapShader.begin();
    normalMapShader.setUniform2f("depthSize", outDepthW, outDepthH);
    normalMapShader.setUniform1f("farThreshold", modifiedFarThreshold);
    normalMapShader.setUniform1i("isThresholdOnly", (isNormalMapThresholdOnly) ? 1 : 0);
    processedDepthXYZFbo.draw(0, 0);
    normalMapShader.end();
    normalFbo.end();
}



void ofxKinectDepthUtils::drawProcessing(int x, int y)
{
    drawFbo.begin();
    ofClear(0, 0, 0, 0);
    depthImageXYZ.draw(0, 0);
    processedDepthXYZFbo.draw(outDepthW, -outDepthH * 2);
    normalFbo.draw(outDepthW * 2, 0);
    drawFbo.end();
    drawFbo.draw(x, y);
    
    depthTestFbo.draw(0, 0, 320, 240);
}


ofImage* ofxKinectDepthUtils::getDepthXYZ()
{
    return &depthImageXYZ;
}


ofPixels* ofxKinectDepthUtils::getProcessedDepthXYZPix()
{
    return &processedDepthXYZPix;
}


float ofxKinectDepthUtils::getPercentWithinThreshold(int minThreshold, int maxThreshold)
{
    depthTestFbo.begin();
    kinect->drawDepth(0, 0, 32, 24);
    depthTestFbo.end();
    
    depthTestFbo.getTextureReference().readToPixels(depthTestPixels);
    
    int totalWithinThreshold = 0;
    for (int i = 0; i < depthTestPixels.size(); i+=3)
    {
        if (depthTestPixels[i] > minThreshold && depthTestPixels[i] < maxThreshold)
            ++totalWithinThreshold;
    }
    
    return (float)totalWithinThreshold / (float)(32 * 24);
}


ofVec3f ofxKinectDepthUtils::getProcessedVertex(int x, int y)
{
    ofColor posDataX = processedDepthXYZPix.getColor(x, y);
    ofColor posDataY = processedDepthXYZPix.getColor(x, y + outDepthH);
    ofColor posDataZ = processedDepthXYZPix.getColor(x, y + outDepthH * 2);
    
    return ofVec3f((posDataX.r * 255 + posDataX.g) * (posDataX.b - 1),
                   (posDataY.r * 255 + posDataY.g) * (posDataY.b - 1) * -1,              ////////////////////////// FLIP Y
                   (posDataZ.r * 255 + posDataZ.g) * (posDataZ.b - 1));
}



void ofxKinectDepthUtils::createShaders()
{
    kinectZSmoothShader.init();
    normalMapShader.init();
}


void ofxKinectDepthUtils::updateDepthImage(ofxKinect *kinect)
{
    int index;
    ofVec3f coordinate;
    int totalPixels = depthImageXYZ.width * (depthImageXYZ.height / 3) * 3;
    int totalPixelsDouble = totalPixels * 2;
    unsigned char* depthImagePixels = new unsigned char[(int)depthImageXYZ.getWidth() * (int)depthImageXYZ.getHeight() * 3];
//    unsigned char* depthImagePixels = depthImageXYZ.getPixels();
//    for (int i = 0; i < totalPixels * 3; i++)
//        depthImagePixels[i] = 0;
    
    //ofPixels depthImagePixels = depthImageXYZ.getPixelsRef();
    int step = inDepthH / outDepthH;
    float flippedX;
    float flipXMultiplier = (isFlippedX) ? -1.0 : 1.0;
    float flippedY;
    float flipYMultiplier = (isFlippedY) ? -1.0 : 1.0;
    
    int coordX, coordY;
    
    //for(int y = depthImageXYZ.height / 3; y > 0; y--)
    for(int y = 0; y < depthImageXYZ.height / 3; y++)
    {
        for (int x = 0; x < depthImageXYZ.width; x++)
        //for (int x = depthImageXYZ.width; x > 0; x--)
        {
            //kinectManager->kinect.getWorldCoordinateAt(x, y);
            index = (y * (depthImageXYZ.width * 3) + (x * 3));
            
            // each channel of each pixel will be run through a formular to calculate the depth in millimeters
            // r is the number of times 255 fits into kinect.getWorldCoordinateAt(x, y).z);
            // g is the remaining figure
            // b is only used for x and y. It is used as a flag for +/- numbers - 0 = - and 100 = +
            //
            // so the z vaue of 1050 would be represented as 4, 30, 0
            if (isFlippedX)
                coordX = ((-x + outDepthW) * step);
            else
                coordX = x * step;
            
            if (isFlippedY)
                coordY = ((-y + outDepthH) * step);
            else
                coordY = y * step;
                
//                coordinate = kinect->getWorldCoordinateAt(, y * step);
//            else
//                coordinate = kinect->getWorldCoordinateAt(x * step, y * step);
            
            coordinate = kinect->getWorldCoordinateAt(coordX, coordY);
            
            // z
            depthImagePixels[index] = (int)(coordinate.z / 255);
            depthImagePixels[index + 1] = (int)(coordinate.z) % 255;
            depthImagePixels[index + 2] = 0;
            
            // x
            flippedX = coordinate.x * flipXMultiplier;
            depthImagePixels[index + totalPixels] = (int)(abs(flippedX) / 255);
            depthImagePixels[index + totalPixels + 1] = (int)(abs(flippedX)) % 255;
            depthImagePixels[index + totalPixels + 2] = (flippedX < 0) ? 0 : 2;
            
            // y
            flippedY = coordinate.y * flipYMultiplier;
            depthImagePixels[index + totalPixelsDouble] = (int)(abs(flippedY) / 255);
            depthImagePixels[index + totalPixelsDouble + 1] = (int)(abs(flippedY)) % 255;
            depthImagePixels[index + totalPixelsDouble + 2] = (flippedY < 0) ? 0 : 2;
        }
    }
//    for (int i = 0; i < totalPixels * 3; i++)
//        if (ofRandomuf() < 0.4)depthImagePixels[i] = 0;
    
    //depthImageXYZ.update();
    depthImageXYZ.setFromPixels(depthImagePixels, depthImageXYZ.width, depthImageXYZ.height, OF_IMAGE_COLOR);
    
    delete depthImagePixels;
}


ofPixels ofxKinectDepthUtils::getProcessedDepthXYZPixCopy()
{
    ofPixels pix;
    pix = processedDepthXYZPix;
    return pix;
}