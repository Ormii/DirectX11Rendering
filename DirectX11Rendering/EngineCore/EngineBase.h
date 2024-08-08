#pragma once

class EngineBase
{
public: 
	EngineBase();
	virtual ~EngineBase();

public:
	int Run();

public:
	virtual bool Initialize();
	virtual void Update() = 0;
	virtual void Render() = 0;

public:
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	bool InitMainWindow();
	bool InitDirectX();

protected:
	void CreateVertexShaderAndInputLayout(const wstring& filename,
		const vector<D3D11_INPUT_ELEMENT_DESC>& inputElements,
		ComPtr<ID3D11VertexShader>& vertexShader,
		ComPtr<ID3D11InputLayout>& inputLayout);
	void CreatePixelShader(const wstring& filename, ComPtr<ID3D11PixelShader>& pixelShader);
	void CreateIndexBuffer(const vector<uint16_t>& indices, ComPtr<ID3D11Buffer>& m_indexBuffer);

	template <typename T_VERTEX>
	void CreateVertexBuffer(const vector<T_VERTEX>& vertices, ComPtr<ID3D11Buffer>& vertexBuffer)
	{
		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = sizeof(T_VERTEX) * vertices.size();
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.MiscFlags = 0;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.StructureByteStride = sizeof(T_VERTEX);

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0, };
		vertexBufferData.pSysMem = vertices.size();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;

		m_device->CreateBuffer(&bufferDesc, &vertexBufferData, &vertexBuffer);
	}

	template <typename T_CONSTANT>
	void CreateConstantBuffer(const T_CONSTANT& constantBufferData, ComPtr<T_CONSTANT>& constantBuffer)
	{
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.ByteWidth = sizeof(constantBufferData);
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &constantBufferData;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		m_device->CreateBuffer(&bufferDesc, &InitData, &constantBuffer);
	}

	template <typename T_DATA>
	void UpdateBuffer(const T_DATA& bufferData, ComPtr<ID3D11Buffer>& buffer) {
		D3D11_MAPPED_SUBRESOURCE ms;
		m_context->Map(buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
		memcpy(ms.pData, &bufferData, sizeof(bufferData));
		m_context->Unmap(buffer.Get(), NULL);
	}

protected:
	int m_screenWidth;
	int m_screenHeight;
	HWND m_hwnd;

protected:
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_context;
	ComPtr<IDXGISwapChain> m_swapChain;
	ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	ComPtr<ID3D11RasterizerState> m_rasterizerState;

	ComPtr<ID3D11Texture2D> m_depthStencilBuffer;
	ComPtr<ID3D11DepthStencilView> m_depthStencilView;
	ComPtr<ID3D11DepthStencilState> m_depthStencilState;

	D3D11_VIEWPORT m_viewport;
};