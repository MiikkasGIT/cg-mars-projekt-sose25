//
//  Camera.h
//  RealtimeRending
//
//  Created by Philipp Lensing on 22.10.14.
//  Copyright (c) 2014 Philipp Lensing. All rights reserved.
//

#ifndef __RealtimeRending__Camera__
#define __RealtimeRending__Camera__

#include <iostream>
#ifdef WIN32
#include <GL/glew.h>
#include <glfw/glfw3.h>
#else
#define GLFW_INCLUDE_GLCOREARB
#define GLFW_INCLUDE_GLEXT
#include <glfw/glfw3.h>
#endif
#include "vector.h"
#include "matrix.h"

class BaseCamera
{
public:
    virtual void update() = 0;
    virtual const Matrix& getViewMatrix() const  = 0;
    virtual const Matrix& getProjectionMatrix() const  = 0;
    virtual Vector position() const  = 0;
    virtual ~BaseCamera() {};
};

class SimpleCamera : public BaseCamera
{
public:
    virtual void update() {}
    virtual const Matrix& getViewMatrix() const { return View; }
    virtual const Matrix& getProjectionMatrix() const { return Proj;  }
    virtual Vector position() const { Matrix m = View; m.invert(); return m.translation(); }
    void setViewMatrix(const Matrix& m) { View = m;  }
    void setProjectionMatrix(const Matrix& m) { Proj = m; }
    virtual ~SimpleCamera() {};
protected:
    Matrix View;
    Matrix Proj;
};

class Camera : public BaseCamera
{
public:
    Camera(GLFWwindow* pWin);
    virtual ~Camera() {};

    // Abfragen
    virtual Vector position() const;
    Vector target() const;
    Vector up() const;

    // Setzen
    void setPosition( const Vector& Pos);
    void setTarget( const Vector& Target);
    void setUp( const Vector& Up);

    // Neu: sanftes Nachziehen und dynamisches FOV
    void setTargetPosition(const Vector& Target, float factor);
    void setFOV(float fovNew);

    // Eingabe (Trackball/Pan/Zoom)
    void mouseInput( int x, int y, int Button, int State);

    // Render-Matrizen
    virtual void update();
    virtual const Matrix& getViewMatrix() const;
    virtual const Matrix& getProjectionMatrix() const;

protected:
    void updateMouseInput();

    void pan( float dx, float dy);
    void zoom( float dz);
    void rotate( float x, float y );
    Vector getVSpherePos( float x, float y);
    Vector rotateAxisAngle( Vector v, Vector n, float a);

    Matrix m_ViewMatrix;
    Matrix m_ProjMatrix;
    Vector m_Position;
    Vector m_Target;
    Vector m_Up;
    Vector m_Panning;
    Vector m_Zoom;
    Vector m_Rotation;
    int m_LastMouseX;
    int m_LastMouseY;
    int WindowWidth;
    int WindowHeight;
    GLFWwindow* pWindow;

private:
    float fov = 65.0f; // in Grad, wird in update() genutzt
};

#endif /* defined(__RealtimeRending__Camera__) */
