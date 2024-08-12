#include "pch.h"
#include "Mesh.h"
#include "Engine.h"
#include "Camera.h"

Mesh::Mesh()
{

}

Mesh::~Mesh()
{

}

bool Mesh::Initialize()
{
	Engine* pEngine = dynamic_cast<Engine*>(g_EngineBase);
	if (pEngine == nullptr)
		return false;

	pEngine->CreateVertexBuffer(m_meshData.vertices, m_vertexBuffer);
	m_indexCount = UINT(m_meshData.indices.size());
	pEngine->CreateIndexBuffer(m_meshData.indices, m_indexBuffer);

	m_MeshVertexConstantBufferData.model = Matrix();
	m_MeshVertexConstantBufferData.view = Matrix();
	m_MeshVertexConstantBufferData.projection = Matrix();

	pEngine->CreateConstantBuffer(m_MeshVertexConstantBufferData, m_vertexConstantBuffer);
	pEngine->CreateConstantBuffer(m_MeshPixelConstantBufferData, m_pixelConstantBuffer);

	vector<D3D11_INPUT_ELEMENT_DESC> meshInputElements = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		 D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3,
		 D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3,
		 D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	pEngine->CreateVertexShaderAndInputLayout(
		L"MeshVertexShader.hlsl", meshInputElements, m_MeshVertexShader,
		m_MeshInputLayout);
	pEngine->CreatePixelShader(L"MeshPixelShader.hlsl", m_MeshPixelShader);


	return true;
}

void Mesh::Update(float dt)
{
	using namespace DirectX;

	Engine* pEngine = dynamic_cast<Engine*>(g_EngineBase);
	if (pEngine == nullptr)
		return;

	std::weak_ptr<Camera> pCamera = pEngine->GetMainCamera();
	if (pCamera.expired())
		return;

	m_MeshVertexConstantBufferData.model =
		Matrix::CreateScale(m_modelScaling) *
		Matrix::CreateRotationY(m_modelRotation.y) *
		Matrix::CreateRotationX(m_modelRotation.x) *
		Matrix::CreateRotationZ(m_modelRotation.z) *
		Matrix::CreateTranslation(m_modelTranslation);
	m_MeshVertexConstantBufferData.model =
		m_MeshVertexConstantBufferData.model.Transpose();

	m_MeshVertexConstantBufferData.view =
		Matrix::CreateRotationY(pCamera.lock()->GetRotation().y) *
		Matrix::CreateTranslation(pCamera.lock()->GetPosition());

	m_MeshPixelConstantBufferData.eyeWorld = Vector3::Transform(
		Vector3(0.0f), m_MeshVertexConstantBufferData.view.Invert());

	m_MeshVertexConstantBufferData.view =
		m_MeshVertexConstantBufferData.view.Transpose();

	const float aspect = pEngine->GetAspectRatio();
	if (g_bUsePerspectiveProjection) {
		m_MeshVertexConstantBufferData.projection = XMMatrixPerspectiveFovLH(
			XMConvertToRadians(pCamera.lock()->GetFovAngle()),
			aspect, pCamera.lock()->GetNearZ(), pCamera.lock()->GetFarZ());
	}
	else {
		m_MeshVertexConstantBufferData.projection =
			XMMatrixOrthographicOffCenterLH(-aspect, aspect, -1.0f, 1.0f,
				pCamera.lock()->GetNearZ(), pCamera.lock()->GetFarZ());
	}
	m_MeshVertexConstantBufferData.projection =
		m_MeshVertexConstantBufferData.projection.Transpose();

	pEngine->UpdateBuffer(m_MeshVertexConstantBufferData, m_vertexConstantBuffer);
	pEngine->UpdateBuffer(m_MeshPixelConstantBufferData, m_pixelConstantBuffer);
}

void Mesh::Render()
{
	Engine* pEngine = dynamic_cast<Engine*>(g_EngineBase);
	if (pEngine == nullptr)
		return;

	pEngine->GetContext()->VSSetShader(m_MeshVertexShader.Get(), 0, 0);
	pEngine->GetContext()->VSSetConstantBuffers(
		0, 1, m_vertexConstantBuffer.GetAddressOf());

	pEngine->GetContext()->PSSetConstantBuffers(
		0, 1, m_pixelConstantBuffer.GetAddressOf());
	pEngine->GetContext()->PSSetShader(m_MeshPixelShader.Get(), 0, 0);

	if (g_bUseDrawWireFrame) {
		pEngine->GetContext()->RSSetState(pEngine->GetWiredRasterizerState().Get());
	}
	else {
		pEngine->GetContext()->RSSetState(pEngine->GetSolidRasterizerState().Get());
	}

	UINT stride = sizeof(MeshVertex);
	UINT offset = 0;
	pEngine->GetContext()->IASetInputLayout(m_MeshInputLayout.Get());
	pEngine->GetContext()->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(),
		&stride, &offset);
	pEngine->GetContext()->IASetIndexBuffer(m_indexBuffer.Get(),
		DXGI_FORMAT_R16_UINT, 0);
	pEngine->GetContext()->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pEngine->GetContext()->DrawIndexed(m_indexCount, 0, 0);
}
