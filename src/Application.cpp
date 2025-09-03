//
//  Application.cpp
//  ogl4
//
//  Created by Philipp Lensing on 16.09.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#include "Application.h"
#ifdef WIN32
#include <GL/glew.h>
#include <glfw/glfw3.h>
#define _USE_MATH_DEFINES
#include <math.h>
#else
#define GLFW_INCLUDE_GLCOREARB
#define GLFW_INCLUDE_GLEXT
#include <glfw/glfw3.h>
#endif
#include "lineplanemodel.h"
#include "triangleplanemodel.h"
#include "trianglespheremodel.h"
#include "lineboxmodel.h"
#include "triangleboxmodel.h"
#include "model.h"
#include "terrainshader.h"


#ifdef WIN32
#define ASSET_DIRECTORY "../../assets/"
#else
#define ASSET_DIRECTORY "../assets/"
#endif


Application::Application(GLFWwindow* pWin) : pWindow(pWin), Cam(pWin)
{
    BaseModel* pModel;
    
    // create LineGrid model with constant color shader
    pModel = new LinePlaneModel(10, 10, 10, 10);
    ConstantShader* pConstShader = new ConstantShader();
	pConstShader->color( Color(1,0,0));
    pModel->shader(pConstShader, true);
    // add to render list
    Models.push_back( pModel );
    
    // Exercise 1
    // uncomment the following lines for testing
    
    /*
    pModel = new Model(ASSET_DIRECTORY "skybox.obj", false);
    pModel->shader(new PhongShader(), true);
    Models.push_back(pModel);
   
    pTerrain = new Terrain();
    TerrainShader* pTerrainShader = new TerrainShader(ASSET_DIRECTORY);
    pTerrainShader->diffuseTexture(Texture::LoadShared(ASSET_DIRECTORY "grass.bmp"));
    pTerrain->shader(pTerrainShader, true);
    pTerrain->load(ASSET_DIRECTORY "heightmap.bmp", ASSET_DIRECTORY"grass.bmp", ASSET_DIRECTORY"rock.bmp");
    pTerrain->width(150);
    pTerrain->depth(150);
    pTerrain->height(15);
    Models.push_back(pTerrain);
    */
    
}
void Application::start()
{
    glEnable (GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Application::update(float dtime)
{
    // Exercise 1
    // TODO: Add keyboard & mouse input queries for terrain scaling ..
    
    Cam.update();
}

void Application::draw()
{
    // 1. clear screen
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2. setup shaders and draw models
    for( ModelList::iterator it = Models.begin(); it != Models.end(); ++it )
    {
        (*it)->draw(Cam);
    }
    
    // 3. check once per frame for opengl errors
    GLenum Error = glGetError();
    assert(Error==0);
}
void Application::end()
{
    for( ModelList::iterator it = Models.begin(); it != Models.end(); ++it )
        delete *it;
    
    Models.clear();
}