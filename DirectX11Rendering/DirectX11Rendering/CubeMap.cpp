#include "pch.h"
#include "CubeMap.h"

void CubeMap::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const wchar_t* originFilename, const wchar_t* diffuseFilename, const wchar_t* specularFilename)
{
	EngineUtility::CreateCubemapTexture(device, originFilename,
		m_originResView);
	EngineUtility::CreateCubemapTexture(device, diffuseFilename, m_diffuseResView);
	EngineUtility::CreateCubemapTexture(device, specularFilename,
		m_specularResView);

	EngineUtility::CreateConstantBuffer(device, context, m_vertexConstantData, m_vertexConstantBuffer);

	MeshData cubeMeshData{};
	m_cubeMesh = std::make_shared<Mesh>();
	GeometryGenerator::MakeSphere("CubeMap",50.0f, 10, 10, cubeMeshData);
	std::reverse(cubeMeshData.indices.begin(), cubeMeshData.indices.end());

	EngineUtility::CreateVertexBuffer(device, context, cubeMeshData.vertices, m_cubeMesh->vertexBuffer);
	EngineUtility::CreateIndexBuffer(device, context, cubeMeshData.indices, m_cubeMesh->indexBuffer);

	m_cubeMesh->m_indexCount = (UINT)cubeMeshData.indices.size();

	vector<D3D11_INPUT_ELEMENT_DESC> elements =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,4*3,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,4*3+4*3,D3D11_INPUT_PER_VERTEX_DATA,0 }
	};

	EngineUtility::CreateVertexShaderAndInputLayout(device, context, L"CubeMapVertexShader.hlsl", elements, m_vertexShader, m_inputLayout);
	EngineUtility::CreatePixelShader(device, context, L"CubeMapPixelShader.hlsl", m_pixelShader);

	D3D11_SAMPLER_DESC sampleDesc{};
	sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampleDesc.MinLOD = 0;
	sampleDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&sampleDesc, m_samplerState.GetAddressOf());
}

void CubeMap::Update(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, float dt)
{
	Engine* pEngine = dynamic_cast<Engine*>(g_EngineBase);
	if (pEngine == nullptr)
		return;

	Matrix projMat = Matrix();
	if (g_bUsePerspectiveProjection)
		projMat = pEngine->GetMainCamera()->GetPersMat();
	else
		projMat = pEngine->GetMainCamera()->GetOrthMat();


	m_vertexConstantData.viewProj = projMat.Transpose() * pEngine->GetMainCamera()->GetViewMat().Transpose();

	EngineUtility::UpdateBuffer(device, context, m_vertexConstantData, m_vertexConstantBuffer);
}

void CubeMap::Render(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context)
{
	Engine* pEngine = dynamic_cast<Engine*>(g_EngineBase);
	if (pEngine == nullptr)
		return;

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	context->IASetVertexBuffers(0, 1, m_cubeMesh->vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_cubeMesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, offset);
	context->IASetInputLayout(m_inputLayout.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if (g_bUseDrawWireFrame)
		context->RSSetState(pEngine->GetWiredRasterizerState().Get());
	else
		context->RSSetState(pEngine->GetSolidRasterizerState().Get());

	context->VSSetShader(m_vertexShader.Get(), 0, 0);
	context->VSSetConstantBuffers(0, 1, m_vertexConstantBuffer.GetAddressOf());

	context->PSSetShader(m_pixelShader.Get(), 0, 0);
	context->PSSetShaderResources(0, 1, m_originResView.GetAddressOf());
	context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

	context->DrawIndexed(m_cubeMesh->m_indexCount, 0, 0);

}
