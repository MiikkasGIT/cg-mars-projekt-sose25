//
//  TrianglePlaneModel.hpp
//  ogl4
//
//  Created by Philipp Lensing on 20.09.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#ifndef TrianglePlaneModel_hpp
#define TrianglePlaneModel_hpp

#include <stdio.h>

#include <stdio.h>
#include "basemodel.h"
#include "constantshader.h"
#include "vertexbuffer.h"
#include "indexbuffer.h"

class TrianglePlaneModel : public BaseModel
{
public:
    TrianglePlaneModel( float DimX, float DimZ, int NumSegX, int NumSegZ );
    virtual ~TrianglePlaneModel() {}
    virtual void draw(const BaseCamera& Cam);
protected:
    VertexBuffer VB;
    IndexBuffer IB;
};

#endif /* TrianglePlaneModel_hpp */
