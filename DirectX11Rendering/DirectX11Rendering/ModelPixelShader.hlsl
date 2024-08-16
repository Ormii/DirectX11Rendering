#include "Common.hlsli"

Texture2D g_texture0 : register(t0);
TextureCube g_diffuseCube : register(t1);
TextureCube g_speculareCube : register(t2);
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
    float3 color = float3(0.0f, 0.0f, 0.0f);
    
    int i = 0;
    
    color += ComputeDirectionalLight(directionalLight, material, input.normalWorld, toEye);
    
    [loop]
    for (i = 0; i < MAX_LIGHTS; ++i)
        color += ComputePointLight(pointlight[i], material, input.posWorld, input.normalWorld, toEye);
    
    [loop]
    for (i = 0; i < MAX_LIGHTS; ++i)
        color += ComputeSpotLight(spotlight[i], material, input.posWorld, input.normalWorld, toEye);
    
    float4 diffuse = g_diffuseCube.Sample(g_sampler, input.normalWorld);
    float4 specular = g_speculareCube.Sample(g_sampler, reflect(-toEye, input.normalWorld));
    
    diffuse *= float4(material.diffuse, 1.0f);
    specular *= pow(abs(specular.r + specular.g + specular.b) / 3, material.shininess);
    specular *= float4(material.specular, 1.0);
    
    float4 ambient = g_texture0.Sample(g_sampler, input.texcoord) * float4(material.ambient, 1.0f)*2;
    diffuse *= g_texture0.Sample(g_sampler, input.texcoord);
    
    color = ambient + (diffuse + specular) + float4(color, 1.0f);
    
    return float4(color, 1.0);
}
