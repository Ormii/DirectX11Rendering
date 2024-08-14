#include "Common.hlsli"

cbuffer BasicVertexConstantBuffer : register(b0)
{
    matrix model;
    matrix invTranspose;
    matrix view;
    matrix projection;
};

cbuffer NormalVertexConstantBuffer : register(b1)
{
    float  scale;
    float3 dummy;
};


PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    float4 pos = float4(input.posModel, 1.0f);
    pos = mul(pos, model);
    
    float4 normal = float4(input.normalModel, 0.0f);
    output.normalWorld = mul(normal, invTranspose).xyz;
    output.normalWorld = normalize(output.normalWorld);
        
    float t = input.texcoord.x;
    pos.xyz += output.normalWorld * t * scale;
    
    output.posWorld = pos.xyz;
    
    pos = mul(pos, view);
    pos = mul(pos, projection);
    
    output.posProj = pos;
    output.texcoord = input.texcoord;
    output.color = float3(1, 1, 0.0) * (1 - t) + float3(1, 0, 0) * t;
    
    return output;
}