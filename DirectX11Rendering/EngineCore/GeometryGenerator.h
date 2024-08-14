#pragma once

class GeometryGenerator
{
public:
    static vector<MeshData> ReadFromFile(std::string basePath,
        std::string filename);

    static MeshData MakeSquare();
    static MeshData MakeBox(const float scale = 1.0f);
    static MeshData MakeCylinder(const float bottomRadius,
        const float topRadius, float height,
        int numSlices);
    static MeshData MakeSphere(const float radius, const int numSlices,
        const int numStacks);
};

