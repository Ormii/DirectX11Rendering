#pragma once

class EngineUtility
{
public:

	static bool CreateDepthStencilBuffer(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
		ComPtr<ID3D11Texture2D>& depthStencilBuffer, ComPtr<ID3D11DepthStencilView>& depthStencilView, 
		ComPtr<ID3D11DepthStencilState>& depthStencilState, int32 screenWidth, int32 screenHeight, UINT numQualityLevels);

	static void CreateVertexShaderAndInputLayout(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const WString& filename,
		const vector<D3D11_INPUT_ELEMENT_DESC>& inputElements,
		ComPtr<ID3D11VertexShader>& vertexShader,
		ComPtr<ID3D11InputLayout>& inputLayout);

	static void CreatePixelShader(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const WString& filename, ComPtr<ID3D11PixelShader>& pixelShader);
	static void CreateIndexBuffer(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const Vector<uint32>& indices, ComPtr<ID3D11Buffer>& indexBuffer);
	static void CreateHullShader(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const WString& filename, ComPtr<ID3D11HullShader>& hullShader);
	static void CreateDomainShader(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const WString& filename, ComPtr<ID3D11DomainShader>& domainShader);
	
	template <typename T_VERTEX>
	static void CreateVertexBuffer(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const Vector<T_VERTEX>& vertices, ComPtr<ID3D11Buffer>& vertexBuffer)
	{
		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = sizeof(T_VERTEX) * (UINT)vertices.size();
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.MiscFlags = 0;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.StructureByteStride = sizeof(T_VERTEX);

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0, };
		vertexBufferData.pSysMem = vertices.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;

		device->CreateBuffer(&bufferDesc, &vertexBufferData, &vertexBuffer);
	}

	template <typename T_CONSTANT>
	static void CreateConstantBuffer(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const T_CONSTANT& constantBufferData, ComPtr<ID3D11Buffer>& constantBuffer)
	{
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.ByteWidth = sizeof(constantBufferData);
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &constantBufferData;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		device->CreateBuffer(&bufferDesc, &InitData, &constantBuffer);
	}

	template <typename T_DATA>
	static void UpdateBuffer(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, const T_DATA& bufferData, ComPtr<ID3D11Buffer>& buffer) 
	{
		lock_guard<mutex> guard(lock);
		D3D11_MAPPED_SUBRESOURCE ms;
		context->Map(buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &bufferData, sizeof(bufferData));
		context->Unmap(buffer.Get(), NULL);
	}

	static void CreateTexture(ComPtr<ID3D11Device>& device, const String filename,
		ComPtr<ID3D11Texture2D>& texture,
		ComPtr<ID3D11ShaderResourceView>& textureResourceView);

	static void
		CreateCubemapTexture(ComPtr<ID3D11Device>& device, const wchar_t* filename,
			ComPtr<ID3D11ShaderResourceView>& texResView);


private:
	static mutex lock;
};