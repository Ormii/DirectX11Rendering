#include "pch.h"
#include "ResourceManager.h"

void ResourceManager::SetMeshData(const String& meshName, Vector<MeshData>& meshDatas)
{
	if (m_meshDataPool.find(meshName) != m_meshDataPool.end())
		return;

	m_meshDataPool.insert({ meshName, meshDatas });
}

bool ResourceManager::GetMeshData(const String& meshName, Vector<MeshData>& meshDatas)
{
	if (m_meshDataPool.find(meshName) == m_meshDataPool.end())
		return false;

	meshDatas = m_meshDataPool[meshName];

	return true;
}

void ResourceManager::SetTexture(const String& textureName, TextureInfo& textureInfo)
{
	if (m_texturesPool.find(textureName) != m_texturesPool.end())
		return;

	m_texturesPool.insert({ textureName, textureInfo });
}

bool ResourceManager::GetTexture(const String& textureName, TextureInfo& textureInfo)
{
	if (m_texturesPool.find(textureName) == m_texturesPool.end())
		return false;

	textureInfo = m_texturesPool[textureName];

	return true;
}

void ResourceManager::SetVsShader(const WString& filename, VertexShaderInfo& vertexShaderInfo)
{
	if (m_vsPool.find(filename) != m_vsPool.end())
		return;

	m_vsPool.insert({ filename, vertexShaderInfo });
}

bool ResourceManager::GetVsShader(const WString& filename, VertexShaderInfo& vertexShaderInfo)
{
	if (m_vsPool.find(filename) == m_vsPool.end())
		return false;

	vertexShaderInfo = m_vsPool[filename];
	return true;
}

void ResourceManager::SetPsShader(const WString& filename, PixelShaderInfo& pixelShaderInfo)
{
	if (m_psPool.find(filename) != m_psPool.end())
		return;

	m_psPool.insert({ filename, pixelShaderInfo });
}

bool ResourceManager::GetPsShader(const WString& filename, PixelShaderInfo& pixelShaderInfo)
{
	if (m_psPool.find(filename) == m_psPool.end())
		return false;

	pixelShaderInfo = m_psPool[filename];
	return true;
}

void ResourceManager::SetSamplerState(const String& samplerName, SamplerStateInfo& samplerStateInfo)
{
	if (m_samplerPool.find(samplerName) == m_samplerPool.end())
		return;

	m_samplerPool.insert({ samplerName, samplerStateInfo });
}

bool ResourceManager::GetSamplerState(const String& samplerName, SamplerStateInfo& samplerStateInfo)
{
	if (m_samplerPool.find(samplerName) == m_samplerPool.end())
		return false;

	samplerStateInfo = m_samplerPool[samplerName];
	return true;
}

