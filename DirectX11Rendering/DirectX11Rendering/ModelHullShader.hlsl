#include "Common.hlsli"

cbuffer HullConstantBuffer : register(b0)
{
    float3 eyeWorld;
    float weight;
    
    bool useLod;
};


struct HullOut
{
    float3 posModel : POSITION;
    float3 normalModel : NORMAL;
    float2 texcoord : TEXCOORD0;
};

struct PatchConstOutput
{
	float EdgeTessFactor[3]			: SV_TessFactor;
	float InsideTessFactor			: SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

PatchConstOutput CalcHSPatchConstants(
	InputPatch<VertexShaderOutput, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
    PatchConstOutput output;
    
    float3 center = (ip[0].posWorld + ip[1].posWorld + ip[2].posWorld)/3.0;
    float dist = length(eyeWorld - center);
    float distMin = 10.0;
    float distMax = 15.0;
    float tess = (useLod == true) ? weight * saturate((distMax - dist) / (distMax - distMin)) + 1 : 1.0;
    

    output.EdgeTessFactor[0] = tess;
    output.EdgeTessFactor[1] = tess;
    output.EdgeTessFactor[2] = tess;
    output.InsideTessFactor = tess;

    return output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HullOut main(
	InputPatch<VertexShaderOutput, NUM_CONTROL_POINTS> patch,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
    HullOut output;

    output.posModel = patch[i].posModel;
    output.normalModel = patch[i].normalModel;
    output.texcoord = patch[i].texcoord;

    return output;
}
