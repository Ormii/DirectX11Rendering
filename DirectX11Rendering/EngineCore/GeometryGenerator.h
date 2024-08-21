#pragma once

class GeometryGenerator
{
public:
    static void ReadFromFile(String basePath,
        String filename, Vector<MeshData>& meshes);

    static void MakeSquare(const String MeshName, MeshData& meshData);
    static void MakeBox(const String MeshName, MeshData& meshData, const float scale = 1.0f);
    static void MakeCylinder(const String MeshName, const float bottomRadius,
        const float topRadius, float height,
        int numSlices, MeshData& meshData);
    static void MakeSquareGrid(const String MeshName, const int numSlices, const int numStacks,
        MeshData& meshData, const float scale = 1.0f,
        const Vector2 texScale = Vector2(1.0f));
    static void MakeFrustom(const String MeshName, const float aspect, const float fovY, const float nearZ, const float farZ, MeshData& meshData);
    static void MakeSphere(const String MeshName, const float radius, const int numSlices,
        const int numStacks, MeshData& meshData);
};

