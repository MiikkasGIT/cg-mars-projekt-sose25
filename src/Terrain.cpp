#include "Terrain.h"
#include "rgbimage.h"
#include "Terrainshader.h"

Terrain::Terrain(const char* HeightMap, const char* DetailMap1, const char* DetailMap2) : Size(10,1,10)
{
    if(HeightMap && DetailMap1 && DetailMap2)
    {
        bool loaded = load( HeightMap, DetailMap1, DetailMap2);
        if(!loaded)
            throw std::exception();
    }
}

Terrain::~Terrain()
{
    
}

bool Terrain::load( const char* HeightMap, const char* DetailMap1, const char* DetailMap2)
{
    if( !HeightTex.load(HeightMap) )
        return false;
    if( !DetailTex[0].load(DetailMap1) )
        return false;
    if( !DetailTex[1].load(DetailMap2) )
        return false;
    
    // TODO: add code for creating terrain model
    
    return true;
}

void Terrain::shader( BaseShader* shader, bool deleteOnDestruction )
{
    BaseModel::shader(shader, deleteOnDestruction);
}

void Terrain::draw(const BaseCamera& Cam)
{
    applyShaderParameter();
    BaseModel::draw(Cam);
    // TODO: add code for drawing index and vertex buffers
}

void Terrain::applyShaderParameter()
{
    TerrainShader* Shader = dynamic_cast<TerrainShader*>(BaseModel::shader());
    if(!Shader)
        return;
    
    Shader->mixTex(&MixTex);
    for(int i=0; i<2; i++)
        Shader->detailTex(i,&DetailTex[i]);
    Shader->scaling(Size);
    
    // TODO: add additional parameters if needed..
}
