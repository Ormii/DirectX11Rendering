#pragma once

struct CubeMapConstantBufferData
{
	Matrix viewProj;
};

class CubeMap
{
public:
	void Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context
		, const wchar_t* originFilename, const wchar_t* diffuseFilename, const wchar_t* specularFilename);


	void Update(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, float dt);
	
	void Render(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context);


	ComPtr<ID3D11ShaderResourceView> GetDiffuseResView() { return m_diffuseResView; }
	ComPtr<ID3D11ShaderResourceView> GetSpecularResView() { return m_specularResView; }

private:
	shared_ptr<Mesh> m_cubeMesh;

	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader> m_pixelShader;
	ComPtr<ID3D11InputLayout> m_inputLayout;

	ComPtr<ID3D11ShaderResourceView> m_originResView;
	ComPtr<ID3D11ShaderResourceView> m_diffuseResView;
	ComPtr<ID3D11ShaderResourceView> m_specularResView;

	ComPtr<ID3D11Buffer> m_vertexConstantBuffer;
	CubeMapConstantBufferData m_vertexConstantData;

	ComPtr<ID3D11SamplerState> m_samplerState;
};

