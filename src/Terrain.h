#ifndef Terrain_hpp
#define Terrain_hpp

#include <vector>
#include <cmath>
#include "basemodel.h"
#include "texture.h"
#include "vertexbuffer.h"
#include "indexbuffer.h"

class Terrain : public BaseModel
{
    struct TerrainVertex {
        Vector Position;
        Vector Normal;
        float u, v;
    };

public:
    Terrain(const char* DetailMap1, const char* DetailMap2);
    virtual ~Terrain();

    // Heightmap laden (PNG/JPG) und daraus Mesh erzeugen
    bool load(const char* HeightMap, const char* DetailMap1, const char* DetailMap2, const char* MixMap);

    // Nur Detail-/Mix-Texturen laden (für prozedurale Höhen)
    bool loadDetailMix(const char* DetailMap1, const char* DetailMap2, const char* MixMap);

    // Prozedurale Erzeugung via Diamond–Square
    // size MUSS 2^k + 1 sein (z. B. 257, 513, 1025).
    bool generateDiamondSquare(int size, float roughness, unsigned int seed,
                               float worldScale = 1.0f, float heightScale = 50.0f,
                               bool wrapEdges = false);

    // Render
    virtual void shader(BaseShader* shader, bool deleteOnDestruction = false) override;
    virtual void draw(const BaseCamera& Cam) override;

    // Weltkoordinaten -> Terrainhöhe (Y in Weltkoords)
    float heightAtWorld(float xw, float zw) const;

    // Abstand von beliebiger Weltposition zur Terrainoberfläche (positiv = über Boden)
    float distanceToTerrain(const Vector& worldPos) const;

    // Getter/Setter Größe (frei verwendbar im Shader)
    const Vector& size() const { return Size; }
    void size(const Vector& s) { Size = s; }

    // Hilfsfunktion
    Vector normalCalc(const Vector& p1, const Vector& p2, const Vector& p3) const;

protected:
    void applyShaderParameter();

private:
    // --- Height Sampling State ---
    int   GridW = 0, GridH = 0;   // Grid-Auflösung (z. B. 513 x 513)
    float WorldScale = 1.0f;      // Abstand der Gridpunkte in X/Z
    float HeightScale = 1.0f;     // Y-Skalierung
    std::vector<float> Heights;   // normalisierte Höhen [0..1]

    // Lokale (Objektraum) X/Z -> Höhe (Welt-Y) via bilinearer Interpolation
    float sampleHeightLocal(float lx, float lz) const;

    // Gemeinsamer Mesh-Builder für Heightmap und DS
    void buildMeshFromHeights(const std::vector<float>& h, int width, int height,
                              float worldScale, float heightScale);

    // Diamond–Square (interner Schritt)
    inline int idx(int x, int z, int width) const { return x + z * width; }
    void dsDiamondStep(std::vector<float>& h, int size, int x, int z, int reach,
                       float amplitude, bool wrap);

    // kleine deterministische Zufallsfunktion (kein <random>)
    inline float hashNoise(int x, int z, unsigned int seed) const;

    // OpenGL Ressourcen
    VertexBuffer VB;
    IndexBuffer  IB;

    // Texturen
    Texture DetailTex[2];
    Texture MixTex;    // optional; wenn nicht gesetzt, TerrainShader sollte damit umgehen
    Texture HeightTex; // nur für Heightmap-Pfad

    // Terrain Dimensionen (frei nutzbar für Shader-Scaling)
    Vector Size = Vector(1,1,1);
};

#endif /* Terrain_hpp */
