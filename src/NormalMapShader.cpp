//
//  NormalMapShader.cpp
//
//  Created by James Alliban's MBP on 08/05/2014.
//
//

#include "NormalMapShader.h"

void NormalMapShader::init()
{
    string fragShaderStr = STRINGIFY(
                                     
                                    //#extension GL_ARB_texture_rectangle : enable
                                     
                                    uniform sampler2DRect tex0;
                                    uniform float farThreshold;
                                    uniform int isThresholdOnly;
                                    uniform vec2 depthSize;
                                     
                                    float getPositionFromColour(vec4 vec)
                                    {
                                        float plusMinusMultiplier = 1.0;
                                        if (vec.b == 0.0) plusMinusMultiplier = -1.0;
                                        
                                        return ((vec.r * 255.0 * 255.0) + (vec.g * 255.0)) * plusMinusMultiplier;
                                    }
                                                                     
                                     
                                     vec3 convertDistanceToColor(int dist)
                                     {
                                        return vec3((float(dist / 255)) / 255.0,
                                                    mod(float(dist), 255.0) / 255.0,
                                                    0.0);
                                     }
                                                                     
                                     vec3 getVertex(vec2 posX, vec2 posY, vec2 posZ, vec2 offset)
                                     {
                                        return vec3(getPositionFromColour(texture2DRect(tex0, posX + offset)),
                                                    getPositionFromColour(texture2DRect(tex0, posY + offset)),
                                                    getPositionFromColour(texture2DRect(tex0, posZ + offset)));
                                     }
                                                                     
                                     vec3 calculateFaceNormal(vec3 v0, vec3 v1, vec3 v2)
                                     {
                                        // if vertex is out of bounds, return a vec3 containing special values. This vec3 will not be used for averaging.
                                         if (isThresholdOnly == 1)
                                         {
                                             if (v0.b < -farThreshold || v1.b < -farThreshold || v2.b < -farThreshold || v0.b == 0.0 || v1.b  == 0.0 || v2.b  == 0.0)
                                             {
                                                
                                                 return vec3(-99.0, -99.0, -99.0);
                                             }
                                         }
                                        
                                         vec3 va = normalize(vec3(v0 - v1));
                                         vec3 vb = normalize(vec3(v0 - v2));
                                         return normalize(cross(va, vb));
                                    }
                                                                     
                                                                     
                                    void main(void)
                                    {
                                        float texWidth = depthSize.x;
                                        float texHeight = depthSize.y;
                                        
                                        
                                        vec2 posX = gl_TexCoord[0].st;
                                        vec2 posY = gl_TexCoord[0].st + vec2(0.0, texHeight);
                                        vec2 posZ = gl_TexCoord[0].st + vec2(0.0, texHeight * 2.0);
                                        
                                        float NORMAL_OFF = 1.0;
                                        vec3 off = vec3(-NORMAL_OFF, 0, NORMAL_OFF);
                                        
                                        // get the mainVertex
                                        vec3 mainVertex = getVertex(posX, posY, posZ, off.yy);
                                        
                                        
                                        // get the surrounding vertices
                                        // 5____0____
                                        // |\   |\   |
                                        // | \  | \  |
                                        // |  \ |  \ |
                                        // 4___\|___\1
                                        // |\   |\   |
                                        // | \  | \  |
                                        // |  \ |  \ |
                                        // |___\|___\|
                                        //      3    2
                                        
                                        vec3 vertices[6];
                                        
                                        vertices[0] = getVertex(posX, posY, posZ, off.yx);
                                        vertices[1] = getVertex(posX, posY, posZ, off.zy);
                                        vertices[2] = getVertex(posX, posY, posZ, off.zz);
                                        vertices[3] = getVertex(posX, posY, posZ, off.yz);
                                        vertices[4] = getVertex(posX, posY, posZ, off.xy);
                                        vertices[5] = getVertex(posX, posY, posZ, off.xx);
                                        
                                        // calcuate the face normals
                                        //  _________
                                        // |\   |\   |
                                        // | \5 | \  |
                                        // |4 \ |0 \ |
                                        // |___\|___\|
                                        // |\   |\   |
                                        // | \3 | \ 1|
                                        // |  \ |2 \ |
                                        // |___\|___\|
                                        
                                        vec3 faceNormals[6];
                                        
                                        faceNormals[0] = calculateFaceNormal(mainVertex, vertices[0], vertices[1]);
                                        faceNormals[1] = calculateFaceNormal(mainVertex, vertices[1], vertices[2]);
                                        faceNormals[2] = calculateFaceNormal(mainVertex, vertices[2], vertices[3]);
                                        faceNormals[3] = calculateFaceNormal(mainVertex, vertices[3], vertices[4]);
                                        faceNormals[4] = calculateFaceNormal(mainVertex, vertices[4], vertices[5]);
                                        faceNormals[5] = calculateFaceNormal(mainVertex, vertices[5], vertices[0]);
                                        
                                        vec3 average = vec3(0.0, 0.0, 0.0);
                                        
                                        float faceNormalCount = 0.0;
                                        
                                        for (int i = 0; i < 6; i++)
                                        {
                                            if (faceNormals[i].r != -99.0)
                                            {
                                                average += faceNormals[i];
                                                ++faceNormalCount;
                                            }
                                        }
                                        
                                        if (faceNormalCount > 1.0)
                                            average /= faceNormalCount;
                                        
                                        average.z *= -1.0;
                                        average = ((average + 1.0) / 2.0);
                                        
                                        
                                        if (faceNormalCount == 0.0)
                                            average = vec3(0.0, 0.0, 0.0);
                                        
                                        gl_FragColor = vec4(average, 1.0);
                                    }

                                   );
    
    setupShaderFromSource(GL_FRAGMENT_SHADER, fragShaderStr);
    linkProgram();
}