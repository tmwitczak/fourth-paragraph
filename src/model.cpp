// //////////////////////////////////////////////////////////// Includes //
#include "model.hpp"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <exception>
#include <vector>
#include <memory>

// ////////////////////////////////////////////////////////////// Usings //
using std::exception;
using std::string;
using std::vector;
using std::shared_ptr;

using glm::vec2;
using glm::vec3;

// ///////////////////////////////////////////////////////////////////// //
GLuint loadTextureFromFile(string const &filename);

// ///////////////////////////////////////////////////////////////////// //
Model::Model(string const &path) {
    loadModel(path);
}

void Model::render(shared_ptr<Shader> shader0, int instances,
                   GLuint const overrideTexture) const {
    for (auto const &mesh : meshes) {
        mesh.render(shader0, instances, overrideTexture);
    }
}

void Model::loadModel(string const &path) {
    Assimp::Importer importer;

    aiScene const *scene = importer.ReadFile(path,
            aiProcess_Triangulate/* | aiProcess_FlipUVs*/);

    if(!scene ||
       scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
       !scene->mRootNode) {
        throw exception((string("ERROR::ASSIMP:: ") +
                string(importer.GetErrorString())).c_str());
    }

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene) {
    if (!node) {
        return;
    }
    for(unsigned int i = 0; i < node->mNumMeshes; ++i) {
        Mesh m = processMesh(scene->mMeshes[node->mMeshes[i]], scene);
        if (m.vertices.size() > 0) {
            m.setupMesh();
            meshes.push_back(m);
        }
    }
    for(unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene);
    }
}

#include <iostream>
using namespace std;

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    for(int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex;

        vertex.position = {
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        };

        vertex.normal = {
            mesh->mNormals[i].x,
            mesh->mNormals[i].y,
            mesh->mNormals[i].z
        };

        if(mesh->mTextureCoords[0]) {
            vertex.texCoords = {
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            };
        }
        else {
            vertex.texCoords = vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }
    //--------------------
    for (unsigned int i = 0 ; i < indices.size() ; i += 3) {
        Vertex& v0 = vertices[indices[i]];
        Vertex& v1 = vertices[indices[i+1]];
        Vertex& v2 = vertices[indices[i+2]];

        vec3 Edge1 = v1.position - v0.position;
        vec3 Edge2 = v2.position - v0.position;

        float DeltaU1 = v1.texCoords.x - v0.texCoords.x;
        float DeltaV1 = v1.texCoords.y - v0.texCoords.y;
        float DeltaU2 = v2.texCoords.x - v0.texCoords.x;
        float DeltaV2 = v2.texCoords.y - v0.texCoords.y;

        float f = 1.0f / (DeltaU1 * DeltaV2 - DeltaU2 * DeltaV1);

        vec3 Tangent, Bitangent;

        Tangent.x = f * (DeltaV2 * Edge1.x - DeltaV1 * Edge2.x);
        Tangent.y = f * (DeltaV2 * Edge1.y - DeltaV1 * Edge2.y);
        Tangent.z = f * (DeltaV2 * Edge1.z - DeltaV1 * Edge2.z);

        Bitangent.x = f * (-DeltaU2 * Edge1.x - DeltaU1 * Edge2.x);
        Bitangent.y = f * (-DeltaU2 * Edge1.y - DeltaU1 * Edge2.y);
        Bitangent.z = f * (-DeltaU2 * Edge1.z - DeltaU1 * Edge2.z);

        v0.tangent += Tangent;
        v1.tangent += Tangent;
        v2.tangent += Tangent;
    }

    for (unsigned int i = 0 ; i < vertices.size() ; i++) {
        vertices[i].tangent /= vertices[i].tangent.length();
    }
    //--------------------

    for(int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];

        for(int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

//    for(unsigned int i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); ++i) {
    aiString dirPath;
    material->GetTexture(aiTextureType_AMBIENT, 0, &dirPath);
    cout << dirPath.C_Str() << endl;
    textures.push_back({ loadTextureFromFile(string(dirPath.C_Str()) + "\\ao.jpg"), string(dirPath.C_Str())  + "\\ao.jpg"});
    textures.push_back({ loadTextureFromFile(string(dirPath.C_Str()) + "\\albedo.jpg"), string(dirPath.C_Str())  + "\\albedo.jpg"});
    textures.push_back({ loadTextureFromFile(string(dirPath.C_Str()) + "\\metalness.jpg"), string(dirPath.C_Str())  + "\\metalness.jpg"});
    textures.push_back({ loadTextureFromFile(string(dirPath.C_Str()) + "\\roughness.jpg"), string(dirPath.C_Str())  + "\\roughness.jpg"});
    textures.push_back({ loadTextureFromFile(string(dirPath.C_Str()) + "\\normal.jpg"), string(dirPath.C_Str())  + "\\normal.jpg"});
//    }

    return Mesh(vertices, indices, textures);
}

// ///////////////////////////////////////////////////////////////////// //
