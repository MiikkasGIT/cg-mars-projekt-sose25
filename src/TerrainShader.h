#ifndef TerrainShader_hpp
#define TerrainShader_hpp

#include "PhongShader.h"

class TerrainShader : public PhongShader
{
public:
    enum { DETAILTEX0=0, DETAILTEX1, DETAILTEX_COUNT };

    TerrainShader(const std::string& AssetDirectory);
    ~TerrainShader() override {}
    void activate(const BaseCamera& Cam) const override;
    void deactivate() const override;

    // Texturen
    void detailTex(unsigned idx, const Texture* t) { assert(idx<DETAILTEX_COUNT); DetailTex[idx] = t; }
    void mixTex(const Texture* t) { MixTex = t; }
    void useMixTex(bool v) { UseMixTex = v; }

    // Regler
    void setTriplanar(float texScale, float triSharpness){ TexScale = texScale; TriSharpness = triSharpness; }
    void setRock(float threshold, float softness){ RockThreshold = threshold; RockSoftness = softness; }
    void setTint(const Vector& tint){ AlbedoTint = tint; }
    void setFog(float start, float end, const Vector& color){ FogStart = start; FogEnd = end; FogColor = color; }
    void scaling(const Vector& s){ Scaling = s; }

private:
    void activateTex(const Texture* pTex, GLint Loc, int slot) const;

    // Ressourcen
    const Texture* DetailTex[DETAILTEX_COUNT] = {nullptr,nullptr};
    const Texture* MixTex = nullptr;

    // Werte
    bool   UseMixTex      = true;
    float  TexScale       = 0.08f;
    float  TriSharpness   = 4.0f;
    float  RockThreshold  = 0.45f;
    float  RockSoftness   = 0.10f;
    Vector AlbedoTint     = Vector(1.06f, 0.95f, 0.90f);
    float  FogStart       = 80.0f;
    float  FogEnd         = 300.0f;
    Vector FogColor       = Vector(0.95f, 0.95f, 1.0f);
    Vector Scaling        = Vector(1,1,1);

    // Uniform-Locations
    GLint MixTexLoc=-1, DetailTexLoc[DETAILTEX_COUNT]={-1,-1}, ScalingLoc=-1;
    GLint UseMixTexLoc=-1, TexScaleLoc=-1, TriSharpnessLoc=-1;
    GLint RockThresholdLoc=-1, RockSoftnessLoc=-1, AlbedoTintLoc=-1;
    GLint FogStartLoc=-1, FogEndLoc=-1, FogColorLoc=-1;
};

#endif
