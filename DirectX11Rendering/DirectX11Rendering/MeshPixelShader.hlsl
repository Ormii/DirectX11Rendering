#include "Common.hlsli"

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
    float3 color = float3(0.0,0.0,0.0);
    
    int i = 0;
    
    color += ComputeDirectionalLight(directionalLight, material, input.normalWorld, toEye);
    
    for (i = 0; i < MAX_LIGHTS; ++i)
        color += ComputePointLight(pointlight[i], material, input.posWorld, input.normalWorld, toEye);
    for (i = 0; i < MAX_LIGHTS; ++i)
        color += ComputeSpotLight(spotlight[i], material, input.posWorld, input.normalWorld, toEye);
    
    return float4(color, 1.0);
}
