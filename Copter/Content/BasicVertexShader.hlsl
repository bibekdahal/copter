cbuffer vsConstantBuffer : register(b0)
{
	float4x4 mvp;
	float4x4 m_normtrans;
};

struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 normal : NORMAL0;
	float2 tex : TEXCOORD0;
};

struct VertexShaderOutput
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 tex : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	float4 pos = float4(input.pos, 1.0f);

	pos = mul(pos, mvp);
	output.pos = pos;

    output.normal = mul(input.normal, (float3x3)m_normtrans);
	output.tex = input.tex;

	return output;
}
