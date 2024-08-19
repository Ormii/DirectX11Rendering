#pragma once

struct TextureInfo
{
	vector<uint8_t> m_image;
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

class ResourceManager
{
public:
	void SetMeshData(const string& meshName, vector<MeshData>& meshDatas);
	bool GetMeshData(const string& meshName, vector<MeshData>& meshDatas);

	void SetTexture(const string& textureName, TextureInfo& textureInfo);
	bool GetTexture(const string& textureName, TextureInfo& textureInfo);

	void SetVsShader(const wstring& filename, VertexShaderInfo& vertexShaderInfo);
	bool GetVsShader(const wstring& filename, VertexShaderInfo& vertexShaderInfo);

	void SetPsShader(const wstring& filename, PixelShaderInfo& pixelShaderInfo);
	bool GetPsShader(const wstring& filename, PixelShaderInfo& pixelShaderInfo);

protected:
	unordered_map<string, TextureInfo> m_texturesPool;
	unordered_map<string, vector<MeshData>> m_meshDataPool;
	unordered_map<wstring, VertexShaderInfo> m_vsPool;
	unordered_map<wstring, PixelShaderInfo> m_psPool;
};

