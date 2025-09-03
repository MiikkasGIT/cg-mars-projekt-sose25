//
//  TriangleSphereModel.hpp
//  ogl4
//
//  Created by Philipp Lensing on 20.09.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#ifndef TriangleSphereModel_hpp
#define TriangleSphereModel_hpp

#include <stdio.h>
#include "basemodel.h"
#include "vertexbuffer.h"
#include "indexbuffer.h"

class TriangleSphereModel : public BaseModel
{
public:
    TriangleSphereModel( float Radius, int Stacks=18, int Slices=36 );
    virtual ~TriangleSphereModel() {}
    virtual void draw(const BaseCamera& Cam);
protected:
    VertexBuffer VB;
    IndexBuffer IB;
};


#endif /* TriangleSphereModel_hpp */
