#pragma once

class SamplerGenerator
{
public:
	static void MakeSampler(const String samplerName,ComPtr<ID3D11SamplerState>& samplerState, function<ComPtr<ID3D11SamplerState>(ComPtr<ID3D11Device>)> func, ComPtr<ID3D11Device> device);
};

