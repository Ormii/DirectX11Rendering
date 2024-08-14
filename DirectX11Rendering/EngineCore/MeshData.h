#pragma once

struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<uint32> indices;
    std::string textureFilename;
};