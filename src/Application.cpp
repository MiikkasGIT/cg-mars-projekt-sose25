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
    Cam.setPosition(Vector(0.0f, 40.0f, 120.0f));
    
    // --- Skybox ---
    skybox = new Model(ASSET_DIRECTORY "skybox.obj", false);
    skybox->shader(new PhongShader(), true);
    Models.push_back(skybox);
   
    // --- Terrain ---
    Terrain* pTerrainLocal = new Terrain(
        ASSET_DIRECTORY "mars_regolith_detail.png",
        ASSET_DIRECTORY "mars_rock_detail.png"
    );

    const int   gridSize     = 513;
    const float roughness    = 0.66f;
    const unsigned int seed  = 4242u;
    const float worldScale   = 1.5f;
    const float heightScale  = 60.0f;
    const bool  wrapEdges    = false;

    bool ok = pTerrainLocal->generateDiamondSquare(gridSize, roughness, seed,
                                                   worldScale, heightScale, wrapEdges);
    TerrainShader* pTerrainShader = new TerrainShader(ASSET_DIRECTORY);
    pTerrainLocal->shader(pTerrainShader, /*deleteOnDestruction*/ true);
    pTerrainShader->setK(12);          // <<< WICHTIG: kein 0!
    pTerrainShader->scaling(Vector(1,1,1));
    assert(ok);

    {   // Terrain in die Mitte legen
        const float halfX = (gridSize - 1) * worldScale * 0.5f;
        const float halfZ = (gridSize - 1) * worldScale * 0.5f;
        Matrix t; t.translation(Vector(-halfX, 0.0f, -halfZ));
        pTerrainLocal->transform(t * pTerrainLocal->transform());
    }
    // --- Drone ---
    playerDrone = new Drone(ASSET_DIRECTORY);
    playerDrone->setBaseHoverHeight(0.9f);
    playerDrone->placeOnTerrain(pTerrain, 0.0f, 0.0f);
    Models.push_back(playerDrone);

    // Startkamera
    {
        const float dist = 8.0f;
        const float up   = 2.5f;
        const float yaw  = playerDrone->yaw();
        const Vector fwd(sinf(yaw), 0.0f, cosf(yaw));
        const Vector camTarget = playerDrone->position();
        const Vector camPos    = camTarget - fwd * dist + Vector(0, up, 0);
        Cam.setPosition(camPos);
        Cam.setTarget(camTarget + fwd * 5.0f);
        Cam.setFOV(65.0f);
    }
    
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

void Application::update(float dtime) {
    // --- Drone Eingaben + Terrain-Follow ---
    if (playerDrone) {
        playerDrone->handleInput(pWindow, dtime);
        playerDrone->update(dtime, pTerrain);
    }

    // --- Items updaten & Kollision mit der Drone testen ---
    if (playerDrone) {
        const AABB& droneBox = playerDrone->worldAABB();
    }

    // --- Follow-Cam ---
    if (playerDrone) {
        const float dist = 12.0f;
        const float up   = 4.0f;
        const float yaw  = playerDrone->yaw();
        const Vector fwd(sinf(yaw), 0.0f, cosf(yaw));
        const Vector camTarget = playerDrone->position();
        const Vector camPos    = camTarget + fwd * dist + Vector(0, up, 0);
        Cam.setPosition(camPos);
        Cam.setTarget(camTarget - fwd * 5.0f);
        Cam.setFOV(65.0f);
    }

    // --- Skybox an Kamera-Position pinnen ---
    if (skybox) {
        Matrix m = skybox->transform();
        const Vector cpos = Cam.position();
        m.m03 = cpos.X; m.m13 = cpos.Y; m.m23 = cpos.Z;
        skybox->transform(m);
    }

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
