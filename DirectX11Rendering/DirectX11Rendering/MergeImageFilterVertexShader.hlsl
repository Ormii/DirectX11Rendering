#include "Common.hlsli"

struct MergeImageFilterPixelShaderInput
{
    float4 posWord : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

MergeImageFilterPixelShaderInput main(VertexShaderInput input)
{
    MergeImageFilterPixelShaderInput output;
    output.posWord = float4(input.posModel, 1.0f);
    output.texcoord = input.texcoord;
    
    return output;
}