#pragma once

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>

#include "MeshData.h"

class ModelLoader
{
public:
    void Load(String basePath, String filename);

    void ProcessNode(aiNode* node, const aiScene* scene,
        DirectX::SimpleMath::Matrix tr);

    MeshData ProcessMesh(aiMesh* mesh, const aiScene* scene);

public:
    String basePath;
    Vector<MeshData> meshes;
};

