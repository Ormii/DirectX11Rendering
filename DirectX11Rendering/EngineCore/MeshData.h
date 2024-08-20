#pragma once
#include "Container.h"

struct MeshData
{
    Vector<Vertex> vertices;
    Vector<uint32> indices;
    String textureFilename;
};