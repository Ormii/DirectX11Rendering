#include "pch.h"
#include "SamplerGenerator.h"

void SamplerGenerator::MakeSampler(const String samplerName, ComPtr<ID3D11SamplerState>& samplerState, function<ComPtr<ID3D11SamplerState>(ComPtr<ID3D11Device>)> func, ComPtr<ID3D11Device> device)
{
	SamplerStateInfo samplerStateInfo{};
	if (!g_ResourceManager->GetSamplerState(samplerName, samplerStateInfo))
	{
		samplerStateInfo.m_samplerState = func(device);
		g_ResourceManager->SetSamplerState(samplerName, samplerStateInfo);
	}

	samplerState = samplerStateInfo.m_samplerState;
}
