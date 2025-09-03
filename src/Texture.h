//
//  Texture.h
//  RealtimeRending
//
//  Created by Philipp Lensing on 24.10.14.
//  Copyright (c) 2014 Philipp Lensing. All rights reserved.
//

#ifndef __RealtimeRending__Texture__
#define __RealtimeRending__Texture__

#include <iostream>
#include <map>

#ifdef WIN32
#include <GL/glew.h>
#include <glfw/glfw3.h>
#else
#define GLFW_INCLUDE_GLCOREARB
#define GLFW_INCLUDE_GLEXT
#include <glfw/glfw3.h>
#endif

class RGBImage;

class Texture
{
public:
    Texture();
    Texture(unsigned int width, unsigned int height, unsigned char* data);
    Texture(const char* Filename );
    Texture(const RGBImage& img);
    ~Texture();
    bool load(const char* Filename);
    bool create(unsigned int width, unsigned int height, unsigned char* data);
    bool create(const RGBImage& img);
    void activate(int slot=0) const;
    void deactivate() const;
    bool isValid() const;
    const RGBImage* getRGBImage() const;
    static Texture* defaultTex();
    static const Texture* LoadShared(const char* Filename);
    static void ReleaseShared( const Texture* pTex );
    
protected:
    void release();
    unsigned char* LoadBMP( const char* Filename, unsigned int& width, unsigned int& height );
    RGBImage* createImage( unsigned char* Data, unsigned int width, unsigned int height );
    GLuint m_TextureID;
    RGBImage* m_pImage;
    mutable int CurrentTextureUnit;
    static Texture* pDefaultTex;
    
    struct TexEntry
    {
        int RefCount;
        const Texture* pTex;
    };
    typedef std::map<std::string, TexEntry> SharedTexMap;
    static SharedTexMap SharedTextures;
    
};

#endif /* defined(__RealtimeRending__Texture__) */
