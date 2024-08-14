#include "Common.hlsli"

Texture2D g_texture0 : register(t0);
SamplerState g_sampler : register(s0);

cbuffer BasicPixelConstantBuffer : register(b0)
{
    float3 eyeWorld;
    bool useTexture;
    Material material;
    LightData directionalLight;
    LightData pointlight[MAX_LIGHTS];
    LightData spotlight[MAX_LIGHTS];
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float3 toEye = normalize(eyeWorld - input.posWorld);
    float3 color = g_texture0.Sample(g_sampler, input.texcoord);
    
    int i = 0;
    
    color += ComputeDirectionalLight(directionalLight, material, input.normalWorld, toEye);
    
    [loop]
    for (i = 0; i < MAX_LIGHTS; ++i)
        color += ComputePointLight(pointlight[i], material, input.posWorld, input.normalWorld, toEye);
    
    [loop]
    for (i = 0; i < MAX_LIGHTS; ++i)
        color += ComputeSpotLight(spotlight[i], material, input.posWorld, input.normalWorld, toEye);
    
    return float4(color, 1.0);
}
