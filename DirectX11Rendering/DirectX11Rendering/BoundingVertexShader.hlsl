#include "Common.hlsli"

cbuffer BasicVertexConstantBuffer : register(b0)
{
    matrix model;
    matrix invTranspose;
    matrix view;
    matrix projection;
};

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    float4 pos = float4(input.posModel, 1.0f);
    pos = mul(pos, model);
    
    output.posWorld = pos.xyz;
    pos = mul(pos, view);
    pos = mul(pos, projection);
    output.posProj = pos;


    return output;
}
