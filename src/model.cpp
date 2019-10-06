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

void Model::render(shared_ptr<Shader> shader0,
                   GLuint const overrideTexture) const {
    for (auto const &mesh : meshes) {
        mesh.render(shader0, overrideTexture);
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

    for(int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];

        for(int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    for(unsigned int i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); ++i) {
        aiString path;
        material->GetTexture(aiTextureType_DIFFUSE, i, &path);

        textures.push_back({
            loadTextureFromFile(path.C_Str()), path.C_Str()
        });
    }

    return Mesh(vertices, indices, textures);
}

// ///////////////////////////////////////////////////////////////////// //
