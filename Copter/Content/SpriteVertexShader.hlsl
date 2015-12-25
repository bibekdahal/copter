cbuffer vsConstantBuffer : register(b0)
{
	matrix mvp;
	float2 uv;
	//float2 spacing;
};


struct VertexShaderInput
{
	float2 pos : POSITION;
	float2 tex : TEXCOORD0;
};

struct VertexShaderOutput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	float4 pos = float4(input.pos, 0.0f, 1.0f);

	output.pos = mul(pos, mvp);
	//output.pos.y = -output.pos.y;

	output.tex = input.tex + uv;

	return output;
}
