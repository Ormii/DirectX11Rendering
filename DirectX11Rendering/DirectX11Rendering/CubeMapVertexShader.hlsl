
cbuffer CubeMapVertexConstantBuffer : register(b0)
{
    matrix model;
    matrix viewProj;
}

struct CubeMapVertexInput
{
    float3 posModel : POSITION;
    float3 normalModel : NORMAL;
    float2 texcoord : TEXCOORD0;
};

struct CubeMapPixelInput
{
    float4 posProj : SV_POSITION;
    float3 posModel : POSITION0;
    float3 posWorld : POSITION1;
    
};

CubeMapPixelInput main(CubeMapVertexInput input)
{
    CubeMapPixelInput output;
    output.posModel = input.posModel;
    output.posWorld = mul(float4(output.posModel, 1.0f), model);
    output.posProj = mul(float4(output.posModel, 1.0f), viewProj);
        
	return output;
}