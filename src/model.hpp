#ifndef MODEL_H
#define MODEL_H
// //////////////////////////////////////////////////////////// Includes //
#include "shader.hpp"
#include "mesh.hpp"
#include "renderable.hpp"

#include "assimp/scene.h"

#include <string>
#include <vector>
#include <memory>

// //////////////////////////////////////////////////////// Class: Model //
class Model : public Renderable {
private:
    std::vector<Mesh> meshes;

public:
    Model(std::string const &path);

    void render(std::shared_ptr<Shader> shader,
                GLuint const overrideTexture = 0) const;
    
private:
    void loadModel(std::string const &path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat,
            aiTextureType type, std::string typeName);
};

// ///////////////////////////////////////////////////////////////////// //
#endif // MODEL_H
