
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
    float3 color = g_texture0.Sample(g_sampler, input.texcoord).rgb;
    
    output.color = (threshold > (color.x + color.y + color.z) / 3.0f) ?
                    float4(0.0, 0.0, 0.0, 0.0) : float4(color, 1.0f);
    
    return output;
}