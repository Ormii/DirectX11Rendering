#include "pch.h"
#include "GeometryGenerator.h"

void GeometryGenerator::ReadFromFile(String basePath, String filename, Vector<MeshData>& meshes)
{
    if (!g_ResourceManager->GetMeshData(filename, meshes))
    {
        ModelLoader modelLoader;
        modelLoader.Load(basePath, filename);
        meshes = move(modelLoader.meshes);

        // Normalize vertices
        Vector3 vmin(1000, 1000, 1000);
        Vector3 vmax(-1000, -1000, -1000);
        for (auto& mesh : meshes) {
            for (auto& v : mesh.vertices) {
                vmin.x = XMMin(vmin.x, v.position.x);
                vmin.y = XMMin(vmin.y, v.position.y);
                vmin.z = XMMin(vmin.z, v.position.z);
                vmax.x = XMMax(vmax.x, v.position.x);
                vmax.y = XMMax(vmax.y, v.position.y);
                vmax.z = XMMax(vmax.z, v.position.z);
            }
        }

        float dx = vmax.x - vmin.x, dy = vmax.y - vmin.y, dz = vmax.z - vmin.z;
        float dl = XMMax(XMMax(dx, dy), dz);
        float cx = (vmax.x + vmin.x) * 0.5f, cy = (vmax.y + vmin.y) * 0.5f,
            cz = (vmax.z + vmin.z) * 0.5f;

        for (auto& mesh : meshes) {
            for (auto& v : mesh.vertices) {
                v.position.x = (v.position.x - cx) / dl;
                v.position.y = (v.position.y - cy) / dl;
                v.position.z = (v.position.z - cz) / dl;
            }
        }

        g_ResourceManager->SetMeshData(filename, meshes);
    }
}

void GeometryGenerator::MakeSquare(const String MeshName, MeshData& meshData)
{
    Vector<MeshData> meshes(1);
    if (!g_ResourceManager->GetMeshData(MeshName, meshes))
    {
        vector<Vector3> positions;
        vector<Vector3> colors;
        vector<Vector3> normals;
        vector<Vector2> texcoords;

        positions.push_back(Vector3(-1.0f, 1.0f, 0.0f));
        positions.push_back(Vector3(1.0f, 1.0f, 0.0f));
        positions.push_back(Vector3(1.0f, -1.0f, 0.0f));
        positions.push_back(Vector3(-1.0f, -1.0f, 0.0f));
        colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
        colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
        colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
        colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, -1.0f));

        texcoords.push_back(Vector2(0.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 1.0f));

        for (size_t i = 0; i < positions.size(); i++)
        {
            Vertex v;
            v.position = positions[i];
            v.normal = normals[i];
            v.texcoord = texcoords[i];

            meshes[0].vertices.push_back(v);
        }

        meshes[0].indices =
        {
            0, 1, 2, 0, 2, 3,
        };

        g_ResourceManager->SetMeshData(MeshName, meshes);
    }

    meshData = meshes[0];
}

void GeometryGenerator::MakeBox(const String MeshName, MeshData& meshData, const float scale)
{
    Vector<MeshData> meshes(1);
    if (!g_ResourceManager->GetMeshData(MeshName, meshes))
    {
        vector<Vector3> positions;
        vector<Vector3> colors;
        vector<Vector3> normals;
        vector<Vector2> texcoords;

        // 윗면
        positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
        positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
        positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
        positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
        colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
        colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
        colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
        colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
        normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
        normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
        normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
        normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
        texcoords.push_back(Vector2(0.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 1.0f));

        // 아랫면
        positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
        positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
        positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
        positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
        colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
        colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
        colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
        colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
        normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
        normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
        normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
        normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
        texcoords.push_back(Vector2(0.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 1.0f));

        // 앞면
        positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
        positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
        positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
        positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
        colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
        colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
        colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
        colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
        texcoords.push_back(Vector2(0.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 1.0f));

        // 뒷면
        positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
        positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
        positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
        positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
        colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
        colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
        colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
        colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
        normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 1.0f));

        // 왼쪽
        positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
        positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
        positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
        positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
        colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
        colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
        colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
        colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
        normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
        normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
        normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
        normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
        texcoords.push_back(Vector2(0.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 1.0f));

        // 오른쪽
        positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
        positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
        positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
        positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
        colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
        colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
        colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
        colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
        normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
        normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
        normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
        normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
        texcoords.push_back(Vector2(0.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 0.0f));
        texcoords.push_back(Vector2(1.0f, 1.0f));
        texcoords.push_back(Vector2(0.0f, 1.0f));

        for (size_t i = 0; i < positions.size(); i++) {
            Vertex v;
            v.position = positions[i];
            v.normal = normals[i];
            v.texcoord = texcoords[i];
            meshes[0].vertices.push_back(v);
        }

        meshes[0].indices = {
            0,  1,  2,  0,  2,  3,  // 윗면
            4,  5,  6,  4,  6,  7,  // 아랫면
            8,  9,  10, 8,  10, 11, // 앞면
            12, 13, 14, 12, 14, 15, // 뒷면
            16, 17, 18, 16, 18, 19, // 왼쪽
            20, 21, 22, 20, 22, 23  // 오른쪽
        };

        g_ResourceManager->SetMeshData("Box", meshes);
    }

    meshData = meshes[0];
}

void GeometryGenerator::MakeCylinder(const String MeshName, const float bottomRadius, const float topRadius, float height, int numSlices, MeshData& meshData)
{
    Vector<MeshData> meshes(1);
    if (!g_ResourceManager->GetMeshData(MeshName, meshes))
    {
        const float dTheta = -XM_2PI / float(numSlices);

        Vector<Vertex>& vertices = meshes[0].vertices;

        // 옆면의 바닥 버텍스들 (인덱스 0 이상 numSlices 미만)
        for (int i = 0; i <= numSlices; i++) {
            Vertex v;
            v.position =
                Vector3::Transform(Vector3(bottomRadius, -0.5f * height, 0.0f),
                    Matrix::CreateRotationY(dTheta * float(i)));

            v.normal = v.position - Vector3(0.0f, -0.5f * height, 0.0f);
            v.normal.Normalize();
            v.texcoord = Vector2(float(i) / numSlices, 1.0f);

            vertices.push_back(v);
        }

        // 옆면의 맨 위 버텍스들 (인덱스 numSlices 이상 2 * numSlices 미만)
        for (int i = 0; i <= numSlices; i++) {
            Vertex v;
            v.position =
                Vector3::Transform(Vector3(topRadius, 0.5f * height, 0.0f),
                    Matrix::CreateRotationY(dTheta * float(i)));
            v.normal = v.position - Vector3(0.0f, 0.5f * height, 0.0f);
            v.normal.Normalize();
            v.texcoord = Vector2(float(i) / numSlices, 0.0f);

            vertices.push_back(v);
        }

        Vector<uint32_t>& indices = meshes[0].indices;

        for (int i = 0; i < numSlices; i++)
        {
            indices.push_back(i);
            indices.push_back(i + numSlices + 1);
            indices.push_back(i + 1 + numSlices + 1);

            indices.push_back(i);
            indices.push_back(i + 1 + numSlices + 1);
            indices.push_back(i + 1);
        }

        g_ResourceManager->SetMeshData(MeshName, meshes);
    }

    meshData = meshes[0];
}

void GeometryGenerator::MakeSphere(const String MeshName, const float radius, const int numSlices, const int numStacks, MeshData& meshData)
{
    Vector<MeshData> meshes(1);
    if (!g_ResourceManager->GetMeshData(MeshName, meshes))
    {
        const float dTheta = -XM_2PI / float(numSlices);
        const float dPhi = -XM_PI / float(numStacks);

        Vector<Vertex>& vertices = meshes[0].vertices;

        for (int j = 0; j <= numStacks; j++) {

            // 스택에 쌓일 수록 시작점을 x-y 평면에서 회전 시켜서 위로 올리는 구조
            Vector3 stackStartPoint = Vector3::Transform(
                Vector3(0.0f, -radius, 0.0f), Matrix::CreateRotationZ(dPhi * j));

            for (int i = 0; i <= numSlices; i++) {
                Vertex v;

                // 시작점을 x-z 평면에서 회전시키면서 원을 만드는 구조
                v.position = Vector3::Transform(
                    stackStartPoint, Matrix::CreateRotationY(dTheta * float(i)));

                v.normal = v.position; // 원점이 구의 중심
                v.normal.Normalize();
                v.texcoord =
                    Vector2(float(i) / numSlices, 1.0f - float(j) / numStacks);

                vertices.push_back(v);
            }
        }

        Vector<uint32_t>& indices = meshes[0].indices;

        for (int j = 0; j < numStacks; j++) {

            const int offset = (numSlices + 1) * j;

            for (int i = 0; i < numSlices; i++) {

                indices.push_back(offset + i);
                indices.push_back(offset + i + numSlices + 1);
                indices.push_back(offset + i + 1 + numSlices + 1);

                indices.push_back(offset + i);
                indices.push_back(offset + i + 1 + numSlices + 1);
                indices.push_back(offset + i + 1);
            }
        }

        g_ResourceManager->SetMeshData(MeshName, meshes);
    }

    meshData = meshes[0];
}
