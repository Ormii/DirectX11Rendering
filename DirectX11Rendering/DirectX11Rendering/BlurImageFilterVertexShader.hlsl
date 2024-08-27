#include "Common.hlsli"

struct BlurImageFilterPixelShaderInput
{
    float4 posWord : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

BlurImageFilterPixelShaderInput main(VertexShaderInput input)
{
    BlurImageFilterPixelShaderInput output;
    output.posWord = float4(input.posModel, 1.0f);
    output.texcoord = input.texcoord;
    
    return output;
}