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
    
    //Skybox
    pModel = new Model(ASSET_DIRECTORY "skybox.obj", false);
    pModel->shader(new PhongShader(), true);
    Models.push_back(pModel);
   
    // --- Terrain ---
    Terrain* pTerrainLocal = new Terrain(
        ASSET_DIRECTORY "texture/mars_regolith_detail.png",
        ASSET_DIRECTORY "texture/mars_rock_detail.png"
    );
    TerrainShader* pTerrainShader = new TerrainShader(ASSET_DIRECTORY);
    pTerrainLocal->shader(pTerrainShader, true);

    const int   gridSize     = 513;
    const float roughness    = 0.66f;
    const unsigned int seed  = 4242u;
    const float worldScale   = 1.5f;
    const float heightScale  = 60.0f;
    const bool  wrapEdges    = false;

    bool ok = pTerrainLocal->generateDiamondSquare(gridSize, roughness, seed,
                                                   worldScale, heightScale, wrapEdges);
    assert(ok);

    {   // Terrain in die Mitte legen
        const float halfX = (gridSize - 1) * worldScale * 0.5f;
        const float halfZ = (gridSize - 1) * worldScale * 0.5f;
        Matrix t; t.translation(Vector(-halfX, 0.0f, -halfZ));
        pTerrainLocal->transform(t * pTerrainLocal->transform());
    }

    pTerrainShader->useMixTex(true);
    pTerrainShader->setTriplanar(0.08f, 4.0f);
    pTerrainShader->setRock(0.45f, 0.12f);
    pTerrainShader->setTint(Vector(1.06f, 0.95f, 0.90f));
    pTerrainShader->setFog(80.0f, 300.0f, Vector(0.95f, 0.95f, 1.0f));

    Models.push_back(pTerrainLocal);
    pTerrain = pTerrainLocal;

    
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
