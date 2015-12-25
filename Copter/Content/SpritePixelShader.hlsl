Texture2D sTexture : register(t0);
SamplerState sSampler : register(s0);

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};


float4 main(PixelShaderInput input) : SV_TARGET
{
	return sTexture.Sample(sSampler, input.tex);
}
