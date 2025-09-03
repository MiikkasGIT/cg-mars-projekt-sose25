//
//  BaseShader.cpp
//  ogl4
//
//  Created by Philipp Lensing on 19.09.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#include "BaseShader.h"

const BaseShader* BaseShader::ShaderInPipe = NULL;

BaseShader::BaseShader()
{
    ModelTransform.identity();
}

bool BaseShader::load( const char* VertexShaderFile, const char* FragmentShaderFile )
{
    unsigned int VSFileSize=0;
    unsigned int FSFileSize=0;
    char* VSFileData = NULL;
    char* FSFileData = NULL;
    
    VSFileData = loadFile(VertexShaderFile, VSFileSize);
    if( !VSFileData)
    {
        std::cout << "Unable to load shader file " << VertexShaderFile << std::endl;
        return false;
    }
    
    FSFileData = loadFile(FragmentShaderFile, FSFileSize);
    if( !FSFileData)
    {
        std::cout << "Unable to load shader file " << FragmentShaderFile << std::endl;
        return false;
    }
    
    ShaderProgram = createShaderProgram(VSFileData, FSFileData);
    
    delete [] VSFileData;
    delete [] FSFileData;
    
    return true;
}

GLuint BaseShader::createShaderProgram( const char* VScode, const char* FScode )
{
    ModelTransform.identity();
    const unsigned int LogSize = 64*1024;
    char ShaderLog[LogSize];
    GLsizei WrittenToLog=0;
    GLint Success = 0;
    
    GLuint VS = glCreateShader(GL_VERTEX_SHADER);
    GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);
    
    GLenum Error = glGetError();
    if(Error!=0)
    {
        std::cout << "Unable to create shader objects. Please ensure that the Shader is used for the first time AFTER successful creation of an OpenGL context!";
        exit(0);
    }
    
    glShaderSource(VS, 1, &VScode, NULL);
    glShaderSource(FS, 1, &FScode, NULL);
    
    glCompileShader(VS);
    glGetShaderiv(VS, GL_COMPILE_STATUS, &Success);
    if(Success==GL_FALSE)
    {
        sprintf(ShaderLog, "VS:");
        WrittenToLog+=3;
        GLsizei Written=0;
        glGetShaderInfoLog(VS, LogSize-WrittenToLog, &Written, &ShaderLog[WrittenToLog]);
        WrittenToLog+=Written;
    }

    glCompileShader(FS);
    glGetShaderiv(FS, GL_COMPILE_STATUS, &Success);
    if(Success==GL_FALSE)
    {
        sprintf(&ShaderLog[WrittenToLog], "FS:");
        WrittenToLog+=3;
        GLsizei Written=0;
        glGetShaderInfoLog(FS, LogSize-WrittenToLog, &Written, &ShaderLog[WrittenToLog]);
        WrittenToLog+=Written;
    }
    
    if( WrittenToLog > 0 )
    {
        // compilation failed
        std::cout << ShaderLog;
        exit(0);
    }
    
    ShaderProgram = glCreateProgram();
    assert(ShaderProgram);
    
    glAttachShader(ShaderProgram, VS);
    glDeleteShader(VS);
    glAttachShader(ShaderProgram, FS);
    glDeleteShader(FS);
    glLinkProgram(ShaderProgram);
    
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if(Success==GL_FALSE)
        glGetShaderInfoLog(ShaderProgram, LogSize-WrittenToLog, NULL, &ShaderLog[WrittenToLog]);

    if( WrittenToLog > 0 )
    {
        // compilation failed
        std::cout << ShaderLog;
        exit(0);
    }
    return ShaderProgram;
}

char* BaseShader::loadFile( const char* File, unsigned int& Filesize )
{
    FILE* pFile = fopen(File, "rb" );
    if(!pFile)
    {
        printf( "Unable to open file %s", File);
        return NULL;
    }
    
    fseek( pFile, 0, SEEK_END);
    Filesize = (unsigned int)ftell(pFile);
    fseek( pFile, 0, SEEK_SET);
    
    if(Filesize<=0)
        return NULL;
    
    char* pBuf = new char[Filesize+1];
    fread( pBuf, Filesize, 1, pFile);
    fclose(pFile);
    pBuf[Filesize] = 0;
    return pBuf;
    
}

void BaseShader::activate(const BaseCamera& Cam) const
{
    if(ShaderInPipe != this)
        glUseProgram(ShaderProgram);
    ShaderInPipe = this;
}


void BaseShader::deactivate() const
{
    glUseProgram(0);
}

GLint BaseShader::getParameterID(const char* ParamenterName) const
{
    return glGetUniformLocation(ShaderProgram, ParamenterName);
}

void BaseShader::setParameter( GLint ID, float Param) const
{
    glUniform1f(ID, Param);
}
void BaseShader::setParameter( GLint ID, int Param) const
{
    glUniform1i(ID, Param);
}
void BaseShader::setParameter( GLint ID, const Vector& Param) const
{
    glUniform3f(ID, Param.X, Param.Y, Param.Z);
}
void BaseShader::setParameter( GLint ID, const Color& Param) const
{
    glUniform3f(ID, Param.R, Param.G, Param.B);
}

void BaseShader::setParameter( GLint ID, const Matrix& Param) const
{
    glUniformMatrix4fv(ID, 1, GL_FALSE, Param.m);
}




