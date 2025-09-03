//
//  LinePlaneModel.hpp
//  ogl4
//
//  Created by Philipp Lensing on 19.09.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#ifndef LinePlaneModel_hpp
#define LinePlaneModel_hpp

#include <stdio.h>
#include "basemodel.h"
#include "vertexbuffer.h"

class LinePlaneModel : public BaseModel
{
public:
    LinePlaneModel( float DimX, float DimZ, int NumSegX, int NumSegZ );
    virtual ~LinePlaneModel() {}
    virtual void draw(const BaseCamera& Cam);
protected:
    VertexBuffer VB;
};

#endif /* LinePlaneModel_hpp */
