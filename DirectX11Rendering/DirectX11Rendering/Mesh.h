#pragma once
#include "Object.h"
#include "MeshGenerator.h"

struct MeshVertexConstantBuffer
{
	Matrix model;
	Matrix invTranspose;
	Matrix view;
	Matrix projection;
};

struct MeshPixelConstantBuffer {
	Vector3 eyeWorld;         
	bool useTexture;          
	Material material;        
	Light lights[MAX_LIGHTS]; 
};

class Mesh : public Object
{

public:
	Mesh();
	virtual	~Mesh();

public:
	virtual bool Initialize();

public:
	ComPtr<ID3D11Buffer>& GetVertexBuffer() { return m_vertexBuffer; }
	ComPtr<ID3D11Buffer>& GetIndexBuffer() { return m_indexBuffer; }
	ComPtr<ID3D11Buffer>& GetVertexConstantBuffer() { return m_vertexConstantBuffer; }
	ComPtr<ID3D11Buffer>& GetPixelConstantBuffer() { return m_pixelConstantBuffer; }

public:
	MeshData GetMeshData() { return m_meshData; }

public:
	virtual void Update();
	virtual void Render();

protected:
	Vector3 m_modelTranslation = Vector3(0.0f);
	Vector3 m_modelRotation = Vector3(-0.6f, 1.0f, 0.0f);
	Vector3 m_modelScaling = Vector3(0.5f);

	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;
	ComPtr<ID3D11Buffer> m_vertexConstantBuffer;
	ComPtr<ID3D11Buffer> m_pixelConstantBuffer;

	ComPtr<ID3D11VertexShader> m_MeshVertexShader;
	ComPtr<ID3D11InputLayout> m_MeshInputLayout;
	ComPtr<ID3D11PixelShader> m_MeshPixelShader;

	MeshVertexConstantBuffer m_MeshVertexConstantBufferData;
	MeshPixelConstantBuffer m_MeshPixelConstantBufferData;
	UINT m_indexCount = 0;

protected:
	MeshData m_meshData;
};

