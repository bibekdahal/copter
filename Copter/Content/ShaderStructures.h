#pragma once

#define Float4Align __declspec(align(16))
namespace Copter
{
	struct VertexPosTex
	{
		DirectX::XMFLOAT2 pos;
		DirectX::XMFLOAT2 tex;
	};

	struct VertexPosNormTex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 tex;
	};

	Float4Align struct vscbuffer
	{
		DirectX::XMFLOAT4X4 mvp;
		DirectX::XMFLOAT2 uv;
		//DirectX::XMFLOAT2 spacing;
	};

	Float4Align struct pbcbuffer
	{
		DirectX::XMFLOAT4 material;
	};

	struct vbcbuffer
	{
		DirectX::XMFLOAT4X4 mvp;
		DirectX::XMFLOAT4X4 m_it;
	};

	value struct Vertex
	{
		float posX, posY, posZ;
		float normalX, normalY, normalZ;
		float texU, texV;
	};
}