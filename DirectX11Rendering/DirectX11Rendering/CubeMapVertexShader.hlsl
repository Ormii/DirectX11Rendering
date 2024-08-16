
cbuffer CubeMapVertexConstantBuffer : register(b0)
{
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
    float3 posModel : POSITION;
    
};

CubeMapPixelInput main(CubeMapVertexInput input)
{
    CubeMapPixelInput output;
    output.posModel = input.posModel;
    output.posProj = mul(float4(input.posModel, 1.0f), viewProj);
        
	return output;
}