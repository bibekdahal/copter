cbuffer psConstantBuffer : register(b0)
{
	float4 material;
};

Texture2D mTexture : register(t0);
SamplerState mSampler : register(s0);

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 tex : TEXCOORD0;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float3 lightDirection = normalize(float3(-1, -2, 1));
    float lightMagnitude = 0.8f * saturate(dot( normalize(input.normal), -lightDirection)) + 0.2f;

	return mTexture.Sample(mSampler, input.tex) * float4((float3)lightMagnitude, 1.0f) * material;
}
