Texture2D g_texture0 : register(t0);
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
    float weight[5] = { 0.0545, 0.2442, 0.4026, 0.2442, 0.0545 };
    float3 color = float3(0.0, 0.0, 0.0);
    
    int i = 0;
    
    [loop]
    for (i = 0; i < 5; ++i)
    {
        color += weight[i] * g_texture0.Sample(g_sampler, input.texcoord + float2((i - 2) * dx, 0));
    }
    output.color = float4(color, 1.0f);
    
    return output;
}