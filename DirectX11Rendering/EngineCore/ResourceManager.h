#pragma once

struct TextureInfo
{
	Vector<uint8_t> m_image;
	int			m_width;
	int			m_height;
	int			m_channels;
	ComPtr<ID3D11Texture2D> m_texture;
	ComPtr<ID3D11ShaderResourceView> m_shaderResView;
};

struct VertexShaderInfo
{
	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3DBlob> m_shaderBlob;
	ComPtr<ID3D11InputLayout> m_inputLayout;
};

struct PixelShaderInfo
{
	ComPtr<ID3D11PixelShader> m_pixelShader;
	ComPtr<ID3DBlob> m_shaderBlob;
};

struct SamplerStateInfo
{
	ComPtr<ID3D11SamplerState> m_samplerState;
};

class ResourceManager
{
public:
	void SetMeshData(const String& meshName, Vector<MeshData>& meshDatas);
	bool GetMeshData(const String& meshName, Vector<MeshData>& meshDatas);

	void SetTexture(const String& textureName, TextureInfo& textureInfo);
	bool GetTexture(const String& textureName, TextureInfo& textureInfo);

	void SetVsShader(const WString& filename, VertexShaderInfo& vertexShaderInfo);
	bool GetVsShader(const WString& filename, VertexShaderInfo& vertexShaderInfo);

	void SetPsShader(const WString& filename, PixelShaderInfo& pixelShaderInfo);
	bool GetPsShader(const WString& filename, PixelShaderInfo& pixelShaderInfo);

	void SetSamplerState(const String& samplerName, SamplerStateInfo& samplerStateInfo);
	bool GetSamplerState(const String& samplerName, SamplerStateInfo& samplerStateInfo);
protected:
	HashMap<String, TextureInfo> m_texturesPool;
	HashMap<String, Vector<MeshData>> m_meshDataPool;
	HashMap<WString, VertexShaderInfo> m_vsPool;
	HashMap<WString, PixelShaderInfo> m_psPool;

	HashMap<String, SamplerStateInfo> m_samplerPool;
};

