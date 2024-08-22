#include "Common.hlsli"

cbuffer BasicVertexConstantBuffer : register(b0)
{
    matrix model;
    matrix invTranspose;
    matrix view;
    matrix projection;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.posModel = input.posModel;
    output.posWorld = mul(float4(input.posModel, 1.0f), model).xyz;
    output.normalModel = input.normalModel;
    output.texcoord = input.texcoord;
    
    return output;
}
