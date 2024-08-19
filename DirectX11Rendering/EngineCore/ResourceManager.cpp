#include "pch.h"
#include "ResourceManager.h"

void ResourceManager::SetMeshData(const string& meshName, vector<MeshData>& meshDatas)
{
	if (m_meshDataPool.find(meshName) != m_meshDataPool.end())
		return;

	m_meshDataPool.insert({ meshName, meshDatas });
}

bool ResourceManager::GetMeshData(const string& meshName, vector<MeshData>& meshDatas)
{
	if (m_meshDataPool.find(meshName) == m_meshDataPool.end())
		return false;

	meshDatas = m_meshDataPool[meshName];

	return true;
}

void ResourceManager::SetTexture(const string& textureName, TextureInfo& textureInfo)
{
	if (m_texturesPool.find(textureName) != m_texturesPool.end())
		return;

	m_texturesPool.insert({ textureName, textureInfo });
}

bool ResourceManager::GetTexture(const string& textureName, TextureInfo& textureInfo)
{
	if (m_texturesPool.find(textureName) == m_texturesPool.end())
		return false;

	textureInfo = m_texturesPool[textureName];

	return true;
}

void ResourceManager::SetVsShader(const wstring& filename, VertexShaderInfo& vertexShaderInfo)
{
	if (m_vsPool.find(filename) != m_vsPool.end())
		return;

	m_vsPool.insert({ filename, vertexShaderInfo });
}

bool ResourceManager::GetVsShader(const wstring& filename, VertexShaderInfo& vertexShaderInfo)
{
	if (m_vsPool.find(filename) == m_vsPool.end())
		return false;

	vertexShaderInfo = m_vsPool[filename];
	return true;
}

void ResourceManager::SetPsShader(const wstring& filename, PixelShaderInfo& pixelShaderInfo)
{
	if (m_psPool.find(filename) != m_psPool.end())
		return;

	m_psPool.insert({ filename, pixelShaderInfo });
}

bool ResourceManager::GetPsShader(const wstring& filename, PixelShaderInfo& pixelShaderInfo)
{
	if (m_psPool.find(filename) == m_psPool.end())
		return false;

	pixelShaderInfo = m_psPool[filename];
	return true;
}

