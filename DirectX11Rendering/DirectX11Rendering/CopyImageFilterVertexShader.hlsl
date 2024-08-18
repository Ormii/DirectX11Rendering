#include "Common.hlsli"

struct CopyImageFilterPixelShaderInput
{
    float4 posWord : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

CopyImageFilterPixelShaderInput main(VertexShaderInput input)
{
    CopyImageFilterPixelShaderInput output;
    output.posWord = float4(input.posModel, 1.0f);
    output.texcoord = input.texcoord;
    
	return output;
}