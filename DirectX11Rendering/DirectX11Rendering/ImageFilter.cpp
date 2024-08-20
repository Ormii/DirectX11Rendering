#include "pch.h"
#include "ImageFilter.h"

void ImageFilter::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const WString& prefixVertex, const WString& prefixPixel, float width, float height)
{
	MeshData meshData{};
	GeometryGenerator::MakeSquare("ImageFilter", meshData);
	m_mesh = make_shared<Mesh>();
	
	EngineUtility::CreateVertexBuffer(device, context, meshData.vertices, m_mesh->vertexBuffer);
	EngineUtility::CreateIndexBuffer(device, context, meshData.indices, m_mesh->indexBuffer);

	vector<D3D11_INPUT_ELEMENT_DESC> elements =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	m_mesh->m_indexCount = (UINT)meshData.indices.size();

	EngineUtility::CreateVertexShaderAndInputLayout(device, context, prefixVertex + L"ImageFilterVertexShader.hlsl", elements, m_vertexShader, m_inputLayout);
	EngineUtility::CreatePixelShader(device, context, prefixPixel + L"ImageFilterPixelShader.hlsl", m_pixelShader);
	EngineUtility::CreateConstantBuffer(device, context, m_pixelConstantBufferData, m_mesh->pixelConstantBuffer);

	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = false;

	device->CreateRasterizerState(&rasterizerDesc, m_rasterizerState.GetAddressOf());

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, m_samplerState.GetAddressOf());

	ZeroMemory(&m_viewport, sizeof(D3D11_VIEWPORT));
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = width;
	m_viewport.Height = height;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	ComPtr<ID3D11Texture2D> texture;
	
	D3D11_TEXTURE2D_DESC txtDesc;
	ZeroMemory(&txtDesc, sizeof(txtDesc));
	txtDesc.Width = (UINT)width;
	txtDesc.Height = (UINT)height;
	txtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	txtDesc.MipLevels = 1;
	txtDesc.SampleDesc.Count = 1;
	txtDesc.ArraySize = 1;
	txtDesc.Usage = D3D11_USAGE_DEFAULT;
	txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	txtDesc.CPUAccessFlags = 0;
	txtDesc.MiscFlags = 0;

	D3D11_RENDER_TARGET_VIEW_DESC viewDesc{};
	viewDesc.Format = txtDesc.Format;
	viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MipSlice = 0;

	device->CreateTexture2D(&txtDesc, NULL, texture.GetAddressOf());
	device->CreateRenderTargetView(texture.Get(), &viewDesc, m_renderTargetView.GetAddressOf());
	device->CreateShaderResourceView(texture.Get(), nullptr, m_shaderResouceView.GetAddressOf());

	m_pixelConstantBufferData.dx = 1.0f / width;
	m_pixelConstantBufferData.dy = 1.0f / height;

	SetRenderTargets({ m_renderTargetView });
}

void ImageFilter::Update(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, float dt)
{
	EngineUtility::UpdateBuffer(device, context, m_pixelConstantBufferData, m_mesh->pixelConstantBuffer);
}

void ImageFilter::Render(ComPtr<ID3D11DeviceContext>& context)
{
	assert(m_shaderResources.size() > 0);
	assert(m_renderTargets.size() > 0);
	context->OMSetRenderTargets(UINT(m_renderTargets.size()), m_renderTargets.data(), nullptr);
	context->RSSetViewports(1, &m_viewport);
	context->RSSetState(m_rasterizerState.Get());

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_mesh->vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, offset);
	context->IASetInputLayout(m_inputLayout.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);


	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	context->PSSetShaderResources(0, UINT(m_shaderResources.size()), m_shaderResources.data());

	context->PSSetConstantBuffers(0, 1, m_mesh->pixelConstantBuffer.GetAddressOf());
	context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

	context->DrawIndexed(m_mesh->m_indexCount, 0, 0);
}


void ImageFilter::SetShaderResources(vector<ComPtr<ID3D11ShaderResourceView>> shaderRes)
{
	m_shaderResources.clear();
	for (const auto& tar : shaderRes)
		m_shaderResources.push_back(tar.Get());
}

void ImageFilter::SetRenderTargets(vector<ComPtr<ID3D11RenderTargetView>> renderTargets)
{
	m_renderTargets.clear();
	for (const auto& tar : renderTargets)
		m_renderTargets.push_back(tar.Get());
}