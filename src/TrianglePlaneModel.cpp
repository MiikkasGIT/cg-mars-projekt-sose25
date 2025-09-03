//
//  TrianglePlaneModel.cpp
//  ogl4
//
//  Created by Philipp Lensing on 20.09.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#include "TrianglePlaneModel.h"


TrianglePlaneModel::TrianglePlaneModel( float DimX, float DimZ, int NumSegX, int NumSegZ )
{
    ++NumSegX; ++NumSegZ;
    float startx = DimX*-0.5f;
    float startz = DimZ*-0.5f;
    float stepx = DimX/(float)(NumSegX-1);
    float stepz = DimZ/(float)(NumSegZ-1);

    // 1. setup vertex buffer
    VB.begin();
    for( int i=0; i<NumSegZ; i++)
        for( int j=0; j<NumSegX; j++)
        {
            VB.addNormal(0, 1, 0);

            float s0 = (float) j/(float)(NumSegX-1);
            float t0 = (float) i/(float)(NumSegZ-1);
            VB.addTexcoord0( s0, t0);

            float s1 = s0 * (float)NumSegX;
            float t1 = t0 * (float)NumSegZ;
            VB.addTexcoord1( s1, t1);

            float px = startx + j*stepx;
            float pz = startz + i*stepz;
            float py = 0;
            VB.addVertex(px, py, pz);
            
        }
    VB.end();
    
    // 2. setup index buffer
    IB.begin();
    for( int i=0; i<NumSegZ-1; i++)
        for( int j=0; j<NumSegX-1; j++)
        {
            // first triangle
            unsigned int idx = i*NumSegX +j;
            IB.addIndex(idx + NumSegX);
            IB.addIndex(idx+1);
            IB.addIndex(idx);
            
            // second triangle
            IB.addIndex( (idx+1) + NumSegX );
            IB.addIndex( idx+1 );
            IB.addIndex( idx + NumSegX );
        }
    IB.end();
}

void TrianglePlaneModel::draw( const BaseCamera& Cam )
{
    BaseModel::draw(Cam);
    
    VB.activate();
    IB.activate();
    
    glDrawElements(GL_TRIANGLES, IB.indexCount(), IB.indexFormat(), 0);
    
    IB.deactivate();
    VB.deactivate();
    
}