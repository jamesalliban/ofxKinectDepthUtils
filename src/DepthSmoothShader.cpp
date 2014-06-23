//
//  DepthSmoothShader.cpp
//
//  Created by James Alliban's MBP on 30/04/2014.
//
//

#include "DepthSmoothShader.h"



void DepthSmoothShader::init()
{
    string fragShaderStr = STRINGIFY(
                                     uniform sampler2DRect tex0;
                                     
                                     uniform int blurSize;
                                     uniform float minDistForAveraging;
                                     uniform int isHorizontal;
                                     uniform float nearThreshold;
                                     uniform float farThreshold;
                                     uniform int blankDepthPixMax;
                                     uniform int isThresholdOnly;
                                     uniform int isMacMiniMeshFix;
                                     
                                     // TODO:
                                     // IS VERTEX WITHIN THRESHOLD
                                     
                                     int getDistanceFromColour(vec3 vec)
                                     {
                                         return int(255.0 * (vec.r * 255.0) + (vec.g * 255.0));
                                     }
                                     
                                     vec3 convertDistanceToColor(int dist)
                                     {
                                         // the following has been replaced as it was causing an visual bug that only appeared on the
                                         // mac mini. There were gaps in the mesh where the z positions of some vertices were offset.
                                         // This only happened where the colour of the depthImageXYZ (z-depth section) went from bright0
                                         // green to black. It seems when converting the average distance back to colour, the r colour
                                         // was often 1 less than it should be. I expect this happened on the first round of smoothing.
                                         
                                         if (isMacMiniMeshFix == 1)
                                         {
                                             return vec3((float(ceil(float(dist) / 255.0)) / 255.0) - (1.0 / 255.0),
                                                         mod(float(dist), 255.0) / 255.0,
                                                         0.0);
                                         }
                                         else
                                         {
                                             return vec3((float(dist / 255)) / 255.0,
                                                         mod(float(dist), 255.0) / 255.0,
                                                         0.0);
                                         }
                                     }
                                     
                                     bool isVertexWithinThreshold(int theDepthInQuestion)
                                     {
                                         if (theDepthInQuestion > int(nearThreshold) && theDepthInQuestion < int(farThreshold))
                                            return false;
                                         else
                                            return true;
                                     }
                                     
                                     void main()
                                     {
                                         int theDepthInQuestion = getDistanceFromColour(texture2DRect(tex0, gl_TexCoord[0].st).rgb);
                                         if (!isVertexWithinThreshold(theDepthInQuestion) || isThresholdOnly == 1)
                                         {
                                             int depthTotal = 0;
                                             depthTotal += theDepthInQuestion;
                                             int totalAveragedDepths = 1;
                                             
                                             vec2 posRT = vec2(0.0, 0.0);
                                             vec2 posLB = vec2(0.0, 0.0);
                                             vec3 colRT = vec3(0.0, 0.0, 0.0);
                                             vec3 colLB = vec3(0.0, 0.0, 0.0);
                                             int distRT = 0;
                                             int distLB = 0;
                                             int totalLBBlank = 0;
                                             int totalRTBlank = 0;
                                             
                                             for (int i = 0; i < blurSize; i++)
                                             {
                                                 // add pixels to the right / top
                                                 // pixel position
                                                 if (isHorizontal == 1)
                                                    posRT = gl_TexCoord[0].st + vec2(i, 0);
                                                 else
                                                    posRT = gl_TexCoord[0].st + vec2(0, i);
                                                 
                                                 // colour of pixel at the above position
                                                 colRT = texture2DRect(tex0, posRT).rgb;
                                                 // pixel colour converted to kinect distance
                                                 distRT = getDistanceFromColour(colRT);
                                                 // add distance to depthTotal for later averaging
                                                 
                                                 if (abs(float(distRT - theDepthInQuestion)) < minDistForAveraging)
                                                 {
                                                     depthTotal += distRT;
                                                     ++totalAveragedDepths;
                                                 }
                                                 else
                                                 {
                                                     ++totalRTBlank;
                                                     if (totalRTBlank >= blankDepthPixMax) break;
                                                 }
                                                 
                                                 
                                                 // add pixels to the left / bottom
                                                 if (isHorizontal == 1)
                                                 posLB = gl_TexCoord[0].st + vec2(i * -1, 0);
                                                 else
                                                 posLB = gl_TexCoord[0].st + vec2(0, i * -1);
                                                 
                                                 colLB = texture2DRect(tex0, posLB).rgb;
                                                 distLB = getDistanceFromColour(colLB);
                                                 
                                                 if (abs(float(distLB - theDepthInQuestion)) < minDistForAveraging)
                                                 {
                                                     depthTotal += distLB;
                                                     ++totalAveragedDepths;
                                                 }
                                                 else
                                                 {
                                                     ++totalLBBlank;
                                                     
                                                     
                                                     if (totalLBBlank >= blankDepthPixMax) break;
                                                 }
                                             }
                                             
                                             int average = int(depthTotal / totalAveragedDepths);
                                             vec3 newCol = convertDistanceToColor(average);// + 0.002);
                                             
                                             gl_FragColor	    =	vec4(newCol, 1.0);
                                             //gl_FragColor	    =	texture2DRect(tex0, gl_TexCoord[0].st);
                                             
                                         }
                                         else
                                         {
                                             gl_FragColor	    =	vec4(0.0, 0.0, 0.0, 1.0);
                                         }
                                         
                                         //gl_FragColor = texture2DRect(tex0, gl_TexCoord[0].st);
                                     }
                                );
    
    setupShaderFromSource(GL_FRAGMENT_SHADER, fragShaderStr);
    linkProgram();
}
