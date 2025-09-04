#pragma once
#include "model.h"
#include "Aabb.h"
#ifdef WIN32
  #include <GL/glew.h>
  #include <glfw/glfw3.h>
#else
  #define GLFW_INCLUDE_GLCOREARB
  #define GLFW_INCLUDE_GLEXT
  #include <glfw/glfw3.h>
#endif
#include <string>

class Terrain;

class Drone : public Model {
public:
    explicit Drone(const std::string& assetDir);
    virtual ~Drone();

    // einmalige Platzierung / Teleport (XZ vorgeben, Y wird „gesnappt“)
    void placeOnTerrain(Terrain* terrain, float x, float z);

    // Steuerung / Simulation
    void handleInput(GLFWwindow* win, float dt);
    void update(float dt, Terrain* terrain);

    // API
    void   setPosition(const Vector& worldPos); // setzt Center per Translation
    void   setYaw(float yaw) { m_Yaw = yaw; m_Dirty = true; }

    Vector position() const    { return m_WorldAABB.getCenter(); }
    float  yaw() const         { return m_Yaw; }
    const AABB& worldAABB()const{ return m_WorldAABB; }
    AABB  localAABB() const    { return m_LocalAABB; }

    // Hover-Höhe (Abstand Unterkante -> Terrain)
    void  setBaseHoverHeight(float h) { m_BaseHoverHeight = h; }

    // externe (z. B. Kollisionsauflösung)
    void  applySeparation(const Vector& sep);

private:
    // Helpers
    void  rebuildTransform();
    float sampleGroundAt(float x, float z, const Terrain* t) const;
    float desiredBottomYFromTerrain(const AABB& wbox, const Terrain* t) const;

    // Pose / Ausrichtung
    float  m_Yaw   = 0.0f;
    bool   m_Dirty = true;

    // Bewegung
    float  m_MoveSpeed = 20.0f;
    float  m_RotSpeed  = 2.0f;

    // Maus/Tilt
    bool   m_MouseInit = false;
    double m_LastMX = 0.0, m_LastMY = 0.0;
    float  m_TiltX = 0.0f,  m_TiltZ = 0.0f;
    float  m_TiltXTarget = 0.0f, m_TiltZTarget = 0.0f;
    float  m_TiltSmoothing = 10.0f;
    float  m_YawVelFiltered = 0.0f;
    float  m_MaxTilt = 0.30f;

    // Hover & Boost
    float  m_BaseHoverHeight   = 1.0f;  // <<— standardmäßig niedriger
    float  m_HeightFollowSpeed = 8.0f;
    float  m_BoostPower        = 5.0f;
    float  m_BoostDecaySpeed   = 3.0f;
    float  m_BoostOffset       = 0.0f;
    bool   m_BoostActive       = false;

    // AABBs
    AABB   m_LocalAABB; // Modelspace
    AABB   m_WorldAABB; // Weltspace (wird bewegt)

    // Typ für deine BVH-Logik (falls genutzt)
    static constexpr int kDroneType = 3;
};
