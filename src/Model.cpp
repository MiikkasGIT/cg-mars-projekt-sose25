//
//  Model.cpp
//  ogl4
//
//  Created by Philipp Lensing on 21.09.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#include "Model.h"
#include "phongshader.h"
#include <list>
#include <float.h>
#include <sstream>

Model::Model() : pMeshes(NULL), MeshCount(0), pMaterials(NULL), MaterialCount(0)
{
    
}
Model::Model(const char* ModelFile, bool FitSize) : pMeshes(NULL), MeshCount(0), pMaterials(NULL), MaterialCount(0)
{
    bool ret = load(ModelFile);
    if(!ret)
        throw std::exception();
}
Model::~Model()
{
    if (pMeshes != nullptr) {
        delete[] pMeshes;
    }
    delete[] pMaterials;

    deleteNodes(&RootNode);
}

void Model::deleteNodes(Node* pNode)
{
    if(!pNode)
        return;
    for(unsigned int i=0; i<pNode->ChildCount; ++i)
        deleteNodes(&(pNode->Children[i]));
    if(pNode->ChildCount>0)
        delete [] pNode->Children;
    if(pNode->MeshCount>0)
        delete [] pNode->Meshes;
}

bool Model::load(const char* ModelFile, bool FitSize)
{
    const aiScene* pScene = aiImportFile( ModelFile,aiProcessPreset_TargetRealtime_Fast | aiProcess_TransformUVCoords );
    
    if(pScene==NULL || pScene->mNumMeshes<=0)
        return false;
    
    Filepath = ModelFile;
    Path = Filepath;
    size_t pos = Filepath.rfind('/');
    if(pos == std::string::npos)
        pos = Filepath.rfind('\\');
    if(pos !=std::string::npos)
        Path.resize(pos+1);
    
    loadMeshes(pScene, FitSize);
    loadMaterials(pScene);
    loadNodes(pScene);
    
    return true;
}


Vector Model::createVector(const aiVector3D& old) {
    return Vector(old.x, old.y, old.z);
}

void Model::loadMeshes(const aiScene* pScene, bool FitSize)
{
    if (pMeshes != nullptr) delete pMeshes;

    this->MeshCount = pScene->mNumMeshes;
    pMeshes = new Mesh[MeshCount];

    this->calcBoundingBox(pScene, BoundingBox);

    float scale = 1;
   

    // warum durch 2 nochmal ????????????????????????????
    //max minus min nehmen eigentlich
    if (FitSize) {
        Vector bBMax = BoundingBox.Max;
        Vector bBMin = BoundingBox.Min;

        float x = fmax(bBMax.X, abs(bBMin.X));
        float y = fmax(bBMax.Y, abs(bBMin.Y));
        float z = fmax(bBMax.Z, abs(bBMin.Z));

        float maxDist = fmax(fmax(x, y), z);
        scale = 5.0f / 2.0f / maxDist;
    }

    //Durch alle Meshes gehen
    for (int i = 0; i < MeshCount; i++) {
        //temporäre Variablen für die übersichtlichkeit
        aiMesh* tmpAIMesh = pScene->mMeshes[i];
        VertexBuffer* tmpVB = &pMeshes[i].VB;

        //Vertices des Meshes in den Vertexbuffer schreiben
        tmpVB->begin();
        for (int posVer = 0; posVer < tmpAIMesh->mNumVertices; posVer++) {
            //Normale umgewandelt in den Vertexbuffer schreiben
            tmpVB->addNormal(createVector(tmpAIMesh->mNormals[posVer]));

            //Texturkoordinaten in den Vertexbuffer schreiben
            for (int posTex = 0; posTex < 4; posTex++) {
                if (tmpAIMesh->HasTextureCoords(posTex)) {
                    Vector tmp = createVector(tmpAIMesh->mTextureCoords[posTex][posVer]);
                    switch (posTex)
                    {
                    case 0:
                        tmpVB->addTexcoord0(tmp.X, -tmp.Y, tmp.Z);
                        break;
                    case 1:
                        tmpVB->addTexcoord1(tmp.X, -tmp.Y, tmp.Z);
                        break;
                    case 2:
                        tmpVB->addTexcoord2(tmp.X, -tmp.Y, tmp.Z);
                        break;
                    case 3:
                        tmpVB->addTexcoord3(tmp.X, -tmp.Y, tmp.Z);
                        break;
                    default:
                        break;
                    }
                }
            }

            //Vertex hinzufügen
            tmpVB->addVertex(createVector(tmpAIMesh->mVertices[posVer]) * scale);

        }
        tmpVB->end();

        IndexBuffer* tmpIB = &pMeshes[i].IB;
        tmpIB->begin();
        //Indexbuffer füllen, durch Alle Flächen durchgehen indicies speichern
        for (int pos = 0; pos < tmpAIMesh->mNumFaces; pos++) {
            //temp Face für übersichtlichkeit
            aiFace tmpAiFace = tmpAIMesh->mFaces[pos];
            for (int ind = 0; ind < tmpAiFace.mNumIndices; ind++) {
                tmpIB->addIndex(tmpAiFace.mIndices[ind]);
            }
        }
        tmpIB->end();

        pMeshes[i].MaterialIdx = tmpAIMesh->mMaterialIndex;
    }
}




void Model::loadMaterials(const aiScene* pScene)
{
    MaterialCount = pScene->mNumMaterials;
    pMaterials = new Material[MaterialCount];

    for (int pos = 0; pos < MaterialCount; pos++) {
        aiMaterial* tmpMat = pScene->mMaterials[pos];
        aiColor3D tmpColor;

        this->pMaterials[pos].DiffTex = Texture::defaultTex();

        for (int j = 0; j < tmpMat->GetTextureCount(aiTextureType_DIFFUSE); j++) {
            aiString path;
            std::string fileFullPathDiffTex;
            tmpMat->GetTexture(aiTextureType_DIFFUSE, j, &path);

            std::stringstream ss;
            ss << Path << path.data << std::ends;
            fileFullPathDiffTex = ss.str();
            this->pMaterials[pos].DiffTex = Texture::LoadShared(fileFullPathDiffTex.c_str());
        }

        tmpMat->Get(AI_MATKEY_COLOR_AMBIENT, tmpColor);
        pMaterials[pos].AmbColor = createColor(tmpColor);

        tmpMat->Get(AI_MATKEY_COLOR_DIFFUSE, tmpColor);
        pMaterials[pos].DiffColor = createColor(tmpColor);

        tmpMat->Get(AI_MATKEY_COLOR_SPECULAR, tmpColor);
        pMaterials[pos].SpecColor = createColor(tmpColor);

        float shiny = 0.0f;
        tmpMat->Get(AI_MATKEY_SHININESS, shiny);
        pMaterials[pos].SpecExp = shiny;

    }
}

Color Model::createColor(const aiColor3D& old) {
    return Color(old.r, old.g, old.b);
}

void Model::calcBoundingBox(const aiScene* pScene, AABB& Box)
{
    Vector min(0.0f, 0.0f, 0.0f);
    Vector max(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < pScene->mNumMeshes; i++) {
        for (int j = 0; j < pScene->mMeshes[i]->mNumVertices;j++) {
            if (pScene->mMeshes[i]->mVertices[j].x < min.X) min.X = pScene->mMeshes[i]->mVertices[j].x;
            if (pScene->mMeshes[i]->mVertices[j].y < min.Y) min.Y = pScene->mMeshes[i]->mVertices[j].y;
            if (pScene->mMeshes[i]->mVertices[j].z < min.Z) min.Z = pScene->mMeshes[i]->mVertices[j].z;
            if (pScene->mMeshes[i]->mVertices[j].x > max.X) max.X = pScene->mMeshes[i]->mVertices[j].x;
            if (pScene->mMeshes[i]->mVertices[j].y > max.Y) max.Y = pScene->mMeshes[i]->mVertices[j].y;
            if (pScene->mMeshes[i]->mVertices[j].z > max.Z) max.Z = pScene->mMeshes[i]->mVertices[j].z;
        }
    }
    Box.Max = max;
    Box.Min = min;
}

void Model::loadNodes(const aiScene* pScene)
{
    deleteNodes(&RootNode);
    copyNodesRecursive(pScene->mRootNode, &RootNode);
}

void Model::copyNodesRecursive(const aiNode* paiNode, Node* pNode)
{
    pNode->Name = paiNode->mName.C_Str();
    pNode->Trans = convert(paiNode->mTransformation);
    
    if(paiNode->mNumMeshes > 0)
    {
        pNode->MeshCount = paiNode->mNumMeshes;
        pNode->Meshes = new int[pNode->MeshCount];
        for(unsigned int i=0; i<pNode->MeshCount; ++i)
            pNode->Meshes[i] = (int)paiNode->mMeshes[i];
    }
    
    if(paiNode->mNumChildren <=0)
        return;
    
    pNode->ChildCount = paiNode->mNumChildren;
    pNode->Children = new Node[pNode->ChildCount];
    for(unsigned int i=0; i<paiNode->mNumChildren; ++i)
    {
        copyNodesRecursive(paiNode->mChildren[i], &(pNode->Children[i]));
        pNode->Children[i].Parent = pNode;
    }
}

void Model::applyMaterial( unsigned int index)
{
    if(index>=MaterialCount)
        return;
    
    PhongShader* pPhong = dynamic_cast<PhongShader*>(shader());
    if(!pPhong) {
        std::cout << "Model::applyMaterial(): WARNING Invalid shader-type. Please apply PhongShader for rendering models.\n";
        return;
    }
    
    Material* pMat = &pMaterials[index];
    pPhong->ambientColor(pMat->AmbColor);
    pPhong->diffuseColor(pMat->DiffColor);
    pPhong->specularExp(pMat->SpecExp);
    pPhong->specularColor(pMat->SpecColor);
    pPhong->diffuseTexture(pMat->DiffTex);
}

void Model::draw(const BaseCamera& Cam)
{
    if(!pShader) {
        std::cout << "BaseModel::draw() no shader found" << std::endl;
        return;
    }
    pShader->modelTransform(transform());
    
    std::list<Node*> DrawNodes;
    DrawNodes.push_back(&RootNode);
    
    while(!DrawNodes.empty())
    {
        Node* pNode = DrawNodes.front();
        Matrix GlobalTransform;
        
        if(pNode->Parent != NULL)
            pNode->GlobalTrans = pNode->Parent->GlobalTrans * pNode->Trans;
        else
            pNode->GlobalTrans = transform() * pNode->Trans;
        
        pShader->modelTransform(pNode->GlobalTrans);
    
        for(unsigned int i = 0; i<pNode->MeshCount; ++i )
        {
            Mesh& mesh = pMeshes[pNode->Meshes[i]];
            mesh.VB.activate();
            mesh.IB.activate();
            applyMaterial(mesh.MaterialIdx);
            pShader->activate(Cam);
            glDrawElements(GL_TRIANGLES, mesh.IB.indexCount(), mesh.IB.indexFormat(), 0);
            mesh.IB.deactivate();
            mesh.VB.deactivate();
        }
        for(unsigned int i = 0; i<pNode->ChildCount; ++i )
            DrawNodes.push_back(&(pNode->Children[i]));
        
        DrawNodes.pop_front();
    }
}

Matrix Model::convert(const aiMatrix4x4& m)
{
    return Matrix(m.a1, m.a2, m.a3, m.a4,
                  m.b1, m.b2, m.b3, m.b4,
                  m.c1, m.c2, m.c3, m.c4,
                  m.d1, m.d2, m.d3, m.d4);
}


