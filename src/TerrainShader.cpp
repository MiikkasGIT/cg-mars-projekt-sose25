//
//  TerrainShader.cpp
//  CGXcode
//
//  Created by Miikka Koensler on 19.07.25.
//  Copyright © 2025 Philipp Lensing. All rights reserved.
//

#include "TerrainShader.h"
#include <string>

TerrainShader::TerrainShader(const std::string& dir) : PhongShader(false) {
    std::string VS = dir + "vsterrain.glsl";
    std::string FS = dir + "fsterrain.glsl";
    if(!load(VS.c_str(), FS.c_str())) throw std::exception();

    PhongShader::assignLocations();
    specularColor(Color(0,0,0));

    MixTexLoc   = getParameterID("MixTex");
    ScalingLoc  = getParameterID("Scaling");
    DetailTexLoc[0] = getParameterID("DetailTex[0]");
    DetailTexLoc[1] = getParameterID("DetailTex[1]");

    UseMixTexLoc     = getParameterID("UseMixTex");
    TexScaleLoc      = getParameterID("TexScale");
    TriSharpnessLoc  = getParameterID("TriSharpness");
    RockThresholdLoc = getParameterID("RockThreshold");
    RockSoftnessLoc  = getParameterID("RockSoftness");
    AlbedoTintLoc    = getParameterID("AlbedoTint");
    FogStartLoc      = getParameterID("FogStart");
    FogEndLoc        = getParameterID("FogEnd");
    FogColorLoc      = getParameterID("FogColor");
}

void TerrainShader::activate(const BaseCamera& Cam) const {
    PhongShader::activate(Cam);
    int slot=0;
    activateTex(MixTex, MixTexLoc, slot++);
    for(int i=0;i<DETAILTEX_COUNT;++i) activateTex(DetailTex[i], DetailTexLoc[i], slot++);
    setParameter(ScalingLoc, Scaling);

    setParameter(UseMixTexLoc, UseMixTex ? 1 : 0);
    setParameter(TexScaleLoc, TexScale);
    setParameter(TriSharpnessLoc, TriSharpness);
    setParameter(RockThresholdLoc, RockThreshold);
    setParameter(RockSoftnessLoc, RockSoftness);
    setParameter(AlbedoTintLoc, AlbedoTint);
    setParameter(FogStartLoc, FogStart);
    setParameter(FogEndLoc, FogEnd);
    setParameter(FogColorLoc, FogColor);
}

void TerrainShader::deactivate() const {
    PhongShader::deactivate();
    for(int i=DETAILTEX_COUNT-1;i>=0;--i) if(DetailTex[i]) DetailTex[i]->deactivate();
    if(MixTex) MixTex->deactivate();
}

void TerrainShader::activateTex(const Texture* t, GLint loc, int slot) const {
    if(t && loc>=0){ t->activate(slot); setParameter(loc, slot); }
}
