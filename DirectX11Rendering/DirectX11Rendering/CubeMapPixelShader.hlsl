TextureCube g_textureCube0 : register(t0);
SamplerState g_sampler : register(s0);

struct CubeMapPixelInput
{
    float4 posProj : SV_POSITION;
    float3 posModel : POSITION;
};

struct CubeMapPixelOutput
{
    float4 pixelColor;
};

CubeMapPixelOutput main(CubeMapPixelInput input) : SV_TARGET
{
    CubeMapPixelOutput output;
    output.pixelColor = g_textureCube0.Sample(g_sampler, input.posModel.xyz);
	return output;
}