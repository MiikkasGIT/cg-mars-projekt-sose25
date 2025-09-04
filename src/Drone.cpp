#include "Drone.h"
#include "phongshader.h"
#include "texture.h"
#include "Terrain.h"

template <typename T>
static inline T clampv(T v, T lo, T hi) { return (v < lo) ? lo : (v > hi) ? hi : v; }

Drone::Drone(const std::string& assetDir)
: Model((assetDir + "models/Drone.FBX").c_str(), true)
{
    this->shader(new PhongShader(), true);

    // 1) FBX von cm auf m skalieren
    const float kModelScale = 0.01f;
    Matrix S; S.scale(kModelScale);
    this->transform(S * this->transform());
    m_ScaleM = S;   // speichern

    // 2) AABB EXPLIZIT mitskalieren (wichtig!)
    AABB aabb = this->BoundingBox; // Mesh-Space-AABB
    aabb.transform(S);

    // 3) Erst jetzt übernehmen
    m_LocalAABB = aabb;
    m_WorldAABB = m_LocalAABB;
    m_WorldAABB.type = kDroneType;

    Vector sz = m_LocalAABB.size();
    std::cout << "[Drone] AABB size (after scale) = ("
              << sz.X << ", " << sz.Y << ", " << sz.Z << ")\n";

    rebuildTransform();
}

Drone::~Drone() {}
static void dbg(const std::string& msg) {
    std::cout << "[Drone] " << msg << std::endl;
}
void Drone::setPosition(const Vector& worldPos)
{
    const Vector centerNow = m_WorldAABB.getCenter();
    const Vector delta     = worldPos - centerNow;
    Matrix M; M.translation(delta);
    m_WorldAABB.transform(M); // in-place
    m_Dirty = true;
}

void Drone::applySeparation(const Vector& sep)
{
    Matrix M; M.translation(sep);
    m_WorldAABB.transform(M); // in-place
    m_Dirty = true;
}

void Drone::placeOnTerrain(Terrain* terrain, float x, float z)
{
    // 1) XZ setzen (Center verschieben)
    Vector c = m_WorldAABB.getCenter();
    c.X = x; c.Z = z;
    m_WorldAABB.moveTo(c); // passt Min/Max zum neuen Center an

    // 2) Ziel-Unterkante = max Bodenhöhe der vier Bottom-Ecken + Hover + Boost
    const float targetBottomY = desiredBottomYFromTerrain(m_WorldAABB, terrain);

    // 3) aktuelle Unterkante (Min.Y) -> getCenterBottom().Y
    const float curBottomY = m_WorldAABB.getCenterBottom().Y;
    const float dy         = targetBottomY - curBottomY;
    
    dbg("placeOnTerrain: targetBottomY=" + std::to_string(targetBottomY) +
        " curBottomY=" + std::to_string(curBottomY) +
        " dy=" + std::to_string(dy));
    
    // 4) vertikal verschieben
    Matrix M; M.translation(Vector(0, dy, 0));
    m_WorldAABB.transform(M);

    // 5) sanftes Zeug resetten
    m_Yaw = 0.0f;
    m_TiltX = m_TiltZ = m_TiltXTarget = m_TiltZTarget = 0.0f;
    m_BoostActive = false; m_BoostOffset = 0.0f;

    m_Dirty = true;
    rebuildTransform();
}

void Drone::handleInput(GLFWwindow* win, float dt)
{
    // --- Maus: Yaw nur per Maus steuern ---
    double mx, my;
    glfwGetCursorPos(win, &mx, &my);
    if (!m_MouseInit) { m_LastMX = mx; m_LastMY = my; m_MouseInit = true; }
    double dx = mx - m_LastMX;
    m_LastMX = mx; m_LastMY = my;
    if (fabs(dx) < 0.2) dx = 0.0;

    const float mouseSens = 0.0035f;
    float yawDelta = static_cast<float>(-dx) * mouseSens;
    m_Yaw += yawDelta;

    // Optional: Yaw wrappen
    {
        const float TWO_PI = 6.28318530718f;
        if (m_Yaw <= -TWO_PI || m_Yaw >= TWO_PI) m_Yaw = fmodf(m_Yaw, TWO_PI);
    }

    // --- Eingaben: W/S vor-zurück, A/D strafen ---
    float inputThrottle = 0.0f; // reine Eingabe (W/S)
    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) inputThrottle += 1.0f;
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) inputThrottle -= 1.0f;

    float strafe = 0.0f;        // A/D
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) strafe -= 1.0f; // links
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) strafe += 1.0f; // rechts

    // *** Hier wird getauscht: Bewegung nutzt das invertierte Vorzeichen ***
    const float moveThrottle = -inputThrottle;

    // Richtungsvektoren aus Yaw
    const Vector fwd   (sinf(m_Yaw), 0.0f, cosf(m_Yaw));
    const Vector right (cosf(m_Yaw), 0.0f, -sinf(m_Yaw));

    // Diagonale normieren
    float norm = (moveThrottle != 0.0f && strafe != 0.0f) ? 0.70710678f : 1.0f;

    const float speed = m_MoveSpeed;
    const Vector deltaPos =
        (fwd   * (speed * moveThrottle * norm * dt)) +
        (right * (speed * strafe       * norm * dt));

    if (deltaPos.length() > 0.0f) {
        Matrix M; M.translation(deltaPos);
        m_WorldAABB.transform(M);
    }

    // --- Boost ---
    if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS && !m_BoostActive) {
        m_BoostActive = true;
        m_BoostOffset = m_BoostPower;
    }

    // --- Tilt-Ziele ---
    // Pitch bleibt an der *Eingabe* (W kippt nach vorn, S nach hinten)
    m_TiltXTarget = inputThrottle * m_MaxTilt;

    // Maus-Yaw filtern
    const float yawVel = (dt > 0.0f) ? (yawDelta / dt) : 0.0f;
    const float alpha  = 1.0f - expf(-dt * 10.0f);
    m_YawVelFiltered   = m_YawVelFiltered + (yawVel - m_YawVelFiltered) * alpha;

    // Banking: Strafe + leicht aus Maus-Yaw
    const float strafeBank = -0.6f * m_MaxTilt * strafe;
    m_TiltZTarget = clampv( (-m_YawVelFiltered * 0.02f) + strafeBank,
                            -m_MaxTilt, m_MaxTilt );

    // Tilt weich nachführen
    const float s = 1.0f - expf(-dt * m_TiltSmoothing);
    m_TiltX += (m_TiltXTarget - m_TiltX) * s;
    m_TiltZ += (m_TiltZTarget - m_TiltZ) * s;

    m_Dirty = true;
}

float Drone::sampleGroundAt(float x, float z, const Terrain* t) const
{
    return t ? t->heightAtWorld(x, z) : 0.0f;
}

// Bodenhöhe: wir nehmen die MAX-Höhe unter den 4 unteren Ecken,
// damit man beim Hang nicht „hineinschneidet“.
float Drone::desiredBottomYFromTerrain(const AABB& wbox, const Terrain* t) const {
    const Vector c = wbox.getCenter();
    const float groundY = sampleGroundAt(c.X, c.Z, t);
    return groundY + m_BaseHoverHeight + m_BoostOffset;
}

void Drone::update(float dt, Terrain* terrain)
{
    // Boost abbauen
    if (m_BoostActive) {
        m_BoostOffset -= m_BoostDecaySpeed * dt;
        if (m_BoostOffset <= 0.0f) { m_BoostOffset = 0.0f; m_BoostActive = false; }
    }

    // sanftes Nachführen auf Hover-Zielhöhe
    if (terrain) {
        const float targetBottomY  = desiredBottomYFromTerrain(m_WorldAABB, terrain);
        const float currentBottomY = m_WorldAABB.getCenterBottom().Y;
        float dy = (targetBottomY - currentBottomY) * m_HeightFollowSpeed * dt;

        const float maxStep = 20.0f * dt; // „Dämpfer“ gegen harte Sprünge
        dy = clampv(dy, -maxStep, maxStep);

        if (fabsf(dy) > 1e-5f) {
            Matrix M; M.translation(Vector(0, dy, 0));
            m_WorldAABB.transform(M);
            m_Dirty = true;
        }
    }

    if (m_Dirty) rebuildTransform();
}

void Drone::rebuildTransform()
{
    const Vector center = m_WorldAABB.getCenter();

    Matrix T, R, tiltX, tiltZ;
    T.translation(center);
    R.rotationY(m_Yaw);
    tiltX.rotationX(m_TiltX);
    tiltZ.rotationZ(m_TiltZ);

    this->transform(T * R * (tiltX * tiltZ) * m_ScaleM);

    m_Dirty = false;
}
