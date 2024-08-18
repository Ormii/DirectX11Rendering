Texture2D g_texture0 : register(t0);
Texture2D g_texture1 : register(t1);

SamplerState g_sampler : register(s0);

cbuffer ImageFilterPixelConstantBufferData : register(b0)
{
    float dx;
    float dy;
    float threshold;
    float strength;
}


struct CopyImageFilterPixelShaderInput
{
    float4 posWord : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

struct CopyImageFilterPixelShaderOutput
{
    float4 color : SV_TARGET;
};


CopyImageFilterPixelShaderOutput main(CopyImageFilterPixelShaderInput input)
{
    CopyImageFilterPixelShaderOutput output;
    float3 colorSrc = g_texture0.Sample(g_sampler, input.texcoord);
    float3 colorDst = g_texture1.Sample(g_sampler, input.texcoord);
    
    output.color = float4(colorSrc + colorDst * strength, 1.0);
    
    return output;
}