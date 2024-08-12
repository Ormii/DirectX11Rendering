#include "pch.h"
#include "MeshGenerator.h"

MeshData MeshGenerator::MakeSquare()
{
    vector<Vector3> positions;
    vector<Vector3> normals;
    vector<Vector2> texcoords;

    positions.push_back(Vector3(-1.0f, 1.0f, 0.0f));
    positions.push_back(Vector3(1.0f, 1.0f, 0.0f));
    positions.push_back(Vector3(1.0f, -1.0f, 0.0f));
    positions.push_back(Vector3(-1.0f, -1.0f, 0.0f));

    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));

    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    MeshData meshData;
    
    for (size_t i = 0; i < positions.size(); ++i)
    {
        MeshVertex meshVertex{};
        meshVertex.pos = positions[i];
        meshVertex.normal = normals[i];
        meshVertex.texcoord = texcoords[i];

        meshData.vertices.push_back(meshVertex);
    }

    meshData.indices = { 0,1,2,0,2,3 };

    return meshData;
}
