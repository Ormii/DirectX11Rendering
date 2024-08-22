#include "Common.hlsli"

cbuffer BasicVertexConstantBuffer : register(b0)
{
    matrix model;
    matrix invTranspose;
    matrix view;
    matrix projection;
};


struct HullOut
{
    float3 posModel : POSITION;
    float3 normalModel : NORMAL;
    float2 texcoord : TEXCOORD0;
};

struct PatchConstOutput
{
    float EdgeTessFactor[3] : SV_TessFactor;
    float InsideTessFactor : SV_InsideTessFactor;
};


#define NUM_CONTROL_POINTS 3

[domain("tri")]
PixelShaderInput main(
	PatchConstOutput input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HullOut, NUM_CONTROL_POINTS> patch)
{
    PixelShaderInput output;
    
    float3 posModel =
        domain.x * patch[0].posModel +
        domain.y * patch[1].posModel +
        domain.z * patch[2].posModel;

    float2 texCoord =
        domain.x * patch[0].texcoord +
        domain.y * patch[1].texcoord +
        domain.z * patch[2].texcoord;

    float3 normal =
        domain.x * patch[0].normalModel +
        domain.y * patch[1].normalModel +
        domain.z * patch[2].normalModel;
    
    
    float4 pos = mul(float4(posModel, 1.0f), model);
    output.posWorld = pos;
    
    pos = mul(float4(output.posWorld, 1.0f), view);
    pos = mul(pos, projection);
    
    output.posProj = pos;
    output.texcoord = texCoord;
    
    output.normalWorld = mul(float4(normal, 0.0f), invTranspose).xyz;
    output.normalWorld = normalize(output.normalWorld);
    
    return output;
}
