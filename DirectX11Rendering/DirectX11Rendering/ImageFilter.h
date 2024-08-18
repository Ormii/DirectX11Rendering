#pragma once

struct ImageFilterPixelConstantBufferData
{
	float dx;
	float dy;
	float threshold;
	float strength;
};

class ImageFilter
{
public:
	
	void Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
		const wstring& prefixVertex, const wstring& prefixPixel, float width, float height);
	void Update(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, float dt);
	void Render(ComPtr<ID3D11DeviceContext> &context);

public:
	void SetShaderResources(vector<ComPtr<ID3D11ShaderResourceView>> shaderRes);
	void SetRenderTargets(vector<ComPtr<ID3D11RenderTargetView>> renderTargets);

	ImageFilterPixelConstantBufferData& GetPixelConstantBufferData() { return m_pixelConstantBufferData; }

	ComPtr<ID3D11ShaderResourceView>& GetShaderResourceView() {return m_shaderResouceView;}
	ComPtr<ID3D11RenderTargetView>& GetRenderTargetView() { return m_renderTargetView; }
	bool& GetIsNotUseThreshold() { return m_IsNotUseThreshold; }

	ComPtr<ID3D11ShaderResourceView> m_shaderResouceView;
	ComPtr<ID3D11RenderTargetView> m_renderTargetView;


protected:
	ImageFilterPixelConstantBufferData m_pixelConstantBufferData;

protected:
	shared_ptr<Mesh> m_mesh;

	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader> m_pixelShader;
	ComPtr<ID3D11InputLayout> m_inputLayout;

	D3D11_VIEWPORT	m_viewport;

	ComPtr<ID3D11SamplerState>	m_samplerState;
	ComPtr<ID3D11RasterizerState> m_rasterizerState;

protected:
	vector<ID3D11ShaderResourceView*> m_shaderResources;
	vector<ID3D11RenderTargetView*> m_renderTargets;

	bool m_IsNotUseThreshold = false;
};