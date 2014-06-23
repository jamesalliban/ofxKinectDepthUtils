//
//  VertexBuilder.h
//
//  Created by James Alliban's MBP on 10/05/2014.
//
//

#pragma once

#include "ofMain.h"


class VertexBuilder : public ofThread
{
public:
    
    void setup();
    void initialiseRows();
    void threadedFunction();
    void update();
    
    
    vector<ofVec3f> vertices;
    
    
    bool didThreadExecute;

};