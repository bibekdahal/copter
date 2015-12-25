#include "pch.h"
#include "Renderer.h"

#include "..\Common\DirectXHelper.h"
#include "DDSTextureLoader\DDSTextureLoader.h"
#include <string>
#include <fstream>
#include "../bullet-2.82-r2704/src/BulletCollision/CollisionShapes/btShapeHull.h"

using namespace Copter;

using namespace DirectX;
using namespace Windows::Foundation;
using namespace Microsoft::WRL;

const XMVECTOR veye = XMVectorSet(1.0f, 1.0f, -2.5f, 0.0f);
const XMVECTOR vat = XMVectorSet(0.0f, 0.5f, 0.0f, 0.0f);
const XMVECTOR vup = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
Renderer::Renderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_deviceResources(deviceResources),
	eye(veye), at(vat), up(vup)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

void Renderer::CreateWindowSizeDependentResources()
{
	Size outputSize = m_deviceResources->GetOutputSize();
	Size renderTargetSize = m_deviceResources->GetRenderTargetSize();

	ASPECT = outputSize.Width / outputSize.Height;
	WIDTH_DIV_2 = outputSize.Width*0.5f;
	HEIGHT_DIV_2 = outputSize.Height*0.5f;

	float fovAngleY = 45.0f * XM_PI / 180.0f;

	/*/ This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (ASPECT < 1.0f)
	{
		fovAngleY *= 2.0f;
	}*/

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();
	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(&m_projection, XMMatrixPerspectiveFovLH(
		fovAngleY,
		ASPECT,
		0.01f,
		100.0f
		) * XMMatrixTranspose(orientationMatrix));

	float hoffset = (renderTargetSize.Height - outputSize.Height) / 2.0f;
	float woffset = (renderTargetSize.Width - outputSize.Width) / 2.0f;
	XMStoreFloat4x4(&m_projection2D,
		XMMatrixOrthographicOffCenterLH(-woffset, outputSize.Width - woffset, hoffset,
		outputSize.Height + hoffset, 0.0f, 100.0f) *
		orientationMatrix * XMMatrixScaling(1.0f, -1.0f, 1.0f)
		);

}


void Renderer::SetCamEye(float x, float y, float z)
{
	eye = XMVectorSet(x, y, z, 0.0f);
}
void Renderer::SetCamAt(float x, float y, float z)
{
	at = XMVectorSet(x, y, z, 0.0f);
}
void Renderer::SetCamUp(float x, float y, float z)
{
	up = XMVectorSet(x, y, z, 0.0f);
}

void Renderer::Update(double totalTime, double deltaTime)
{
	XMMATRIX view;
	view = XMMatrixLookAtLH(eye, at, up);
	XMMATRIX vp = view * XMLoadFloat4x4(&m_projection);
	XMStoreFloat4x4(&m_vp, vp);

	const XMFLOAT4 v(0.0f, 0.0f, 0.0f, 1.0f);
	view.r[3] = XMLoadFloat4(&v);
	XMStoreFloat4x4(&m_viewBB, XMMatrixTranspose(view));
}


void Renderer::BeginRender2D()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	context->IASetIndexBuffer(
		m_indexBufferSpr.Get(),
		DXGI_FORMAT_R16_UINT,
		0
		);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(SpriteShader.inputLayout.Get());
	context->VSSetShader(
		SpriteShader.vertexShader.Get(),
		nullptr,
		0
		);

	context->VSSetConstantBuffers(
		0,
		1,
		SpriteShader.constantVBuffer.GetAddressOf()
		);

	context->PSSetShader(
		SpriteShader.pixelShader.Get(),
		nullptr,
		0
		);

	context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());
}
void Renderer::BeginRender3D()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(ThreeDShader.inputLayout.Get());
	context->VSSetShader(
		ThreeDShader.vertexShader.Get(),
		nullptr,
		0
		);

	context->VSSetConstantBuffers(
		0,
		1,
		ThreeDShader.constantVBuffer.GetAddressOf()
		);

	context->PSSetConstantBuffers(
		0,
		1,
		ThreeDShader.constantPBuffer.GetAddressOf()
		);

	context->PSSetShader(
		ThreeDShader.pixelShader.Get(),
		nullptr,
		0
		);

	context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());
}
void Renderer::Render()
{
	if (!m_loadingComplete) return;
	auto context = m_deviceResources->GetD3DDeviceContext();
	
	// Reset the viewport to target the whole screen.
	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	
	auto d2dcontext = m_deviceResources->GetD2DDeviceContext();
	d2dcontext->BeginDraw();
	d2dcontext->SetTransform(m_deviceResources->GetOrientationTransform2D());
	
}


unsigned Renderer::AddFont(wchar_t* fontName, float fontSize,
	float color_r, float color_g, float color_b, float color_a, int fontWeight,
	int fontStyle, int fontStretch, int textAlignment,
	int paraAlignment)
{
	Font fnt;
	DX::ThrowIfFailed(
		m_deviceResources->GetDWriteFactory()->CreateTextFormat(
		fontName,                // Font family name.
		NULL,                       // Font collection (NULL sets it to use the system font collection).
		(DWRITE_FONT_WEIGHT)fontWeight,
		(DWRITE_FONT_STYLE)fontStyle,
		(DWRITE_FONT_STRETCH)fontStretch,
		fontSize,
		L"en-us",
		&fnt.textformat
		)
		);

	DX::ThrowIfFailed(
		fnt.textformat->SetTextAlignment((DWRITE_TEXT_ALIGNMENT)textAlignment)
		);
	DX::ThrowIfFailed(
		fnt.textformat->SetParagraphAlignment((DWRITE_PARAGRAPH_ALIGNMENT)paraAlignment)
		);

	fnt.brushColor = D2D1::ColorF(color_r, color_g, color_b, color_a);
	DX::ThrowIfFailed(
		m_deviceResources->GetD2DDeviceContext()->CreateSolidColorBrush(fnt.brushColor, &fnt.brush)
		);

	m_fonts.push_back(fnt);
	return m_fonts.size() - 1;
}

void Renderer::DrawText(unsigned font, Platform::String^ text, float posX, float posY, float width, float height)
{
	if (posX == -1) posX = WIDTH_DIV_2 - width / 2.0f;
	if (posY == -1) posY = HEIGHT_DIV_2 - height / 2.0f;
	m_deviceResources->GetD2DDeviceContext()->DrawText(
		text->Begin(),       
		text->Length(),
		m_fonts[font].textformat.Get(),
		D2D1::RectF(posX, posY, width + posX, height + posY),
		m_fonts[font].brush.Get()
		);
}


unsigned Renderer::AddSprite(wchar_t* fileName, float width, float height, int numCols, int numRows, float xSpace, float ySpace)
{
	ID3D11Device2* device = m_deviceResources->GetD3DDevice();

	Sprite spr;
	DX::ThrowIfFailed(CreateDDSTextureFromFile(device, fileName, nullptr, &spr.texture));
	spr.width = width;
	spr.height = height;
	spr.numCols = numCols;
	spr.numRows = numRows;
	spr.xSpace = xSpace;
	spr.ySpace = ySpace;

	float u = 1.0f / numCols, v = 1.0f / numRows;
	VertexPosTex cubeVertices[] =
	{
		{ XMFLOAT2(0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT2(0.0f, height), XMFLOAT2(0.0f, v) },
		{ XMFLOAT2(width, 0.0f), XMFLOAT2(u, 0.0f) },
		{ XMFLOAT2(width, height), XMFLOAT2(u, v) },
	};


	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	vertexBufferData.pSysMem = cubeVertices;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
	DX::ThrowIfFailed(
		device->CreateBuffer(
		&vertexBufferDesc,
		&vertexBufferData,
		&spr.vertexBuffer
		)
		);


	m_sprites.push_back(spr);
	return m_sprites.size() - 1;
}

void Renderer::DrawSprite(unsigned index, float posX, float posY, unsigned img, float depth, float offsetX, float offsetY)
{
	ID3D11DeviceContext2* deviceContext = m_deviceResources->GetD3DDeviceContext();

	if (index >= m_sprites.size()) return;
	Sprite* spr = &m_sprites[index];

	UINT stride = sizeof(VertexPosTex);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(
		0,
		1,
		spr->vertexBuffer.GetAddressOf(),
		&stride,
		&offset
		);

	deviceContext->PSSetShaderResources(0, 1, spr->texture.GetAddressOf());


	vscbuffer m_cbufferdata;
	XMStoreFloat4x4(&m_cbufferdata.mvp,
		XMMatrixTranspose(XMMatrixTranslation(posX + offsetX, posY + offsetY, depth)
		* XMLoadFloat4x4(&m_projection2D)));

	m_cbufferdata.uv.x = (float)(img % spr->numCols) / (float)spr->numCols;
	m_cbufferdata.uv.y = (float)(img / spr->numCols) / (float)spr->numRows;;

	deviceContext->UpdateSubresource(
		SpriteShader.constantVBuffer.Get(),
		0,
		NULL,
		&m_cbufferdata,
		0,
		0
		);

	deviceContext->DrawIndexed(
		6,
		0,
		0
		);

}


unsigned Renderer::AddModel(Platform::String^ filename, btCollisionShape** shape, float scale)
{
	auto device = m_deviceResources->GetD3DDevice();
	Model mdl;

    btConvexHullShape* nshape;
    btConvexHullShape convex;
    if (shape) nshape = new btConvexHullShape();    

	using namespace std;
	fstream file;
	file.open(filename->Data(), fstream::in | fstream::binary);

	unsigned num, i, knum;
	string str;
	file.read((char*)&num, sizeof(num));
	if (num>0)
	{
		for (i = 0; i<num; ++i)
		{
			ComPtr<ID3D11ShaderResourceView> texture;
			file >> str;
			if (str != "$NULL$" && str != "")
			{
				str[str.length() - 3] = 'd'; str[str.length() - 2] = 'd'; str[str.length() - 1] = 's';
				DX::ThrowIfFailed(CreateDDSTextureFromFile(device, wstring(str.begin(), str.end()).c_str(), nullptr, &texture));
			}
			mdl.textures.push_back(texture);
		}
		file.ignore();
	}

	file.read((char*)&knum, sizeof(knum));
	for (unsigned k = 0; k<knum; ++k)
	{
		Mesh msh;
		file.read((char*)&msh.texture, sizeof(unsigned int));
		file.read((char*)&num, sizeof(num));
		if (num>0)
		{
			XMFLOAT3* vertices = new XMFLOAT3[num];
			file.read((char*)vertices, sizeof(XMFLOAT3)* num);
			XMFLOAT3* normals = new XMFLOAT3[num];
			file.read((char*)normals, sizeof(XMFLOAT3)* num);
			XMFLOAT2* tcoords = new XMFLOAT2[num];
			file.read((char*)tcoords, sizeof(XMFLOAT2)*num);

			VertexPosNormTex* meshVertices = new VertexPosNormTex[num];
			for (i = 0; i<num; ++i)
			{
				meshVertices[i].pos = vertices[i];
				meshVertices[i].normal = normals[i];
				meshVertices[i].tex = tcoords[i];

                if (shape)
                    convex.addPoint(btVector3(meshVertices[i].pos.x, meshVertices[i].pos.y, meshVertices[i].pos.z)*scale);
                
			}

			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = meshVertices;
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VertexPosNormTex)*num, D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(
				device->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&msh.vertexBuffer
				)
				);

			delete[] vertices; delete[] normals; delete[] tcoords; delete[] meshVertices;
		}

		file.read((char*)&num, sizeof(num));
		if (num>0)
		{
			unsigned int* meshIndices = new unsigned int[num];
			file.read((char*)meshIndices, sizeof(unsigned int)*num);
			msh.indexCount = num;

			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
			indexBufferData.pSysMem = meshIndices;
			indexBufferData.SysMemPitch = 0;
			indexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int)*num, D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(
				device->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&msh.indexBuffer
				)
				);

			delete[] meshIndices;
		}


		XMStoreFloat4x4(&msh.offset, XMMatrixIdentity());
		msh.material = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		mdl.meshes.push_back(msh);
	}
	file.close();

	XMStoreFloat4x4(&mdl.transform, XMMatrixIdentity());

    if (shape)
    {

        btShapeHull hull(&convex);
        hull.buildHull(convex.getMargin());
        nshape = new btConvexHullShape();

        for (int i = 0; i < hull.numVertices(); i++)           
            nshape->addPoint(hull.getVertexPointer()[i], false);
        nshape->recalcLocalAabb();
        *shape = nshape;
    }

	m_models.push_back(mdl);
	return m_models.size() - 1;
}
unsigned Renderer::AddMesh(unsigned model, Platform::String^ filename, int textureId, btCollisionShape** shape, float scale)
{
	auto device = m_deviceResources->GetD3DDevice();
	Model* mdl = &m_models[model];
	unsigned mshId = mdl->meshes.size();

    btConvexHullShape* nshape;
    btConvexHullShape convex;
    if (shape) nshape = new btConvexHullShape();

	using namespace std;
	fstream file;
	file.open(filename->Data(), fstream::in | fstream::binary);

	unsigned num, i, knum, tnum;
	tnum = mdl->textures.size();

	string str;
	file.read((char*)&num, sizeof(num));
	if (num>0)
	{
		for (i = 0; i<num; ++i)
		{
			file >> str;
			if (textureId<0)
			{
				ComPtr<ID3D11ShaderResourceView> texture;
				if (str != "$NULL$" && str != "")
				{
					str[str.length() - 3] = 'd'; str[str.length() - 2] = 'd'; str[str.length() - 1] = 's';
					DX::ThrowIfFailed(CreateDDSTextureFromFile(device, wstring(str.begin(), str.end()).c_str(), nullptr, &texture));
				}
				mdl->textures.push_back(texture);
			}
		}
		file.ignore();
	}

	file.read((char*)&knum, sizeof(knum));
	for (unsigned k = 0; k<knum; ++k)
	{
		Mesh msh;
		file.read((char*)&msh.texture, sizeof(unsigned int));
		if (textureId<0)
			msh.texture += tnum;
		else
			msh.texture = textureId;

		file.read((char*)&num, sizeof(num));
		if (num>0)
		{
			XMFLOAT3* vertices = new XMFLOAT3[num];
			file.read((char*)vertices, sizeof(XMFLOAT3)* num);
			XMFLOAT3* normals = new XMFLOAT3[num];
			file.read((char*)normals, sizeof(XMFLOAT3)* num);
			XMFLOAT2* tcoords = new XMFLOAT2[num];
			file.read((char*)tcoords, sizeof(XMFLOAT2)*num);

			VertexPosNormTex* meshVertices = new VertexPosNormTex[num];
			for (i = 0; i<num; ++i)
			{
				meshVertices[i].pos = vertices[i];
				meshVertices[i].normal = normals[i];
				meshVertices[i].tex = tcoords[i];
                if (shape)
                    convex.addPoint(btVector3(meshVertices[i].pos.x, meshVertices[i].pos.y, meshVertices[i].pos.z)*scale);
			}

			D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
			vertexBufferData.pSysMem = meshVertices;
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VertexPosNormTex)*num, D3D11_BIND_VERTEX_BUFFER);
			DX::ThrowIfFailed(
				device->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&msh.vertexBuffer
				)
				);

			delete[] vertices; delete[] normals; delete[] tcoords; delete[] meshVertices;
		}

		file.read((char*)&num, sizeof(num));
		if (num>0)
		{
			unsigned int* meshIndices = new unsigned int[num];
			file.read((char*)meshIndices, sizeof(unsigned int)*num);
			msh.indexCount = num;

			D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
			indexBufferData.pSysMem = meshIndices;
			indexBufferData.SysMemPitch = 0;
			indexBufferData.SysMemSlicePitch = 0;
			CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int)*num, D3D11_BIND_INDEX_BUFFER);
			DX::ThrowIfFailed(
				device->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&msh.indexBuffer
				)
				);

			delete[] meshIndices;
		}


		XMStoreFloat4x4(&msh.offset, XMMatrixIdentity());
		msh.material = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		mdl->meshes.push_back(msh);
	}
	file.close();

    if (shape)
    {

        btShapeHull hull(&convex);
        hull.buildHull(convex.getMargin());
        nshape = new btConvexHullShape();

        for (int i = 0; i < hull.numVertices(); i++)
            nshape->addPoint(hull.getVertexPointer()[i], false);
        nshape->recalcLocalAabb();
        *shape = nshape;
    }

	return mshId;
}

unsigned Renderer::AddModel()
{
	Model mdl;
	XMStoreFloat4x4(&mdl.transform, XMMatrixIdentity());
	m_models.push_back(mdl);
	return m_models.size() - 1;
}

unsigned Renderer::AddTexture(unsigned model, Platform::String^ filename)
{
	ComPtr<ID3D11ShaderResourceView> texture;
	DX::ThrowIfFailed(CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), filename->Data(), nullptr, &texture));
	m_models[model].textures.push_back(texture);
	return m_models[model].textures.size() - 1;
}

unsigned Renderer::AddMesh(unsigned model, Vertex* vertices, unsigned numVertices, unsigned* indices, unsigned numIndices, unsigned texture)
{
	auto device = m_deviceResources->GetD3DDevice();
	Mesh msh;
	msh.texture = texture;

	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
	vertexBufferData.pSysMem = vertices;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(Vertex)*numVertices, D3D11_BIND_VERTEX_BUFFER);
	DX::ThrowIfFailed(
		device->CreateBuffer(
		&vertexBufferDesc,
		&vertexBufferData,
		&msh.vertexBuffer
		)
		);

	msh.indexCount = numIndices;

	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	indexBufferData.pSysMem = indices;
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int)*numIndices, D3D11_BIND_INDEX_BUFFER);
	DX::ThrowIfFailed(
		device->CreateBuffer(
		&indexBufferDesc,
		&indexBufferData,
		&msh.indexBuffer
		)
		);


	XMStoreFloat4x4(&msh.offset, XMMatrixIdentity());
	msh.material = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_models[model].meshes.push_back(msh);
	return m_models[model].meshes.size() - 1;
}


void Renderer::DrawBillboard(unsigned index, float posX, float posY, float posZ, unsigned img, float offsetX, float offsetY
	, float scaleX, float scaleY)
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	if (index >= m_sprites.size()) return;
	Sprite* spr = &m_sprites[index];

	context->IASetIndexBuffer(
		m_indexBufferBB.Get(),
		DXGI_FORMAT_R16_UINT,
		0
		);

	UINT stride = sizeof(VertexPosTex);
	UINT offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		spr->vertexBuffer.GetAddressOf(),
		&stride,
		&offset
		);

	context->PSSetShaderResources(0, 1, spr->texture.GetAddressOf());

	vscbuffer m_cbufferdata;

	XMFLOAT3 campos;
	XMStoreFloat3(&campos, eye);

	XMStoreFloat4x4(&m_cbufferdata.mvp, XMMatrixTranspose(
		XMMatrixTranslation(-spr->width / 2.0f, -spr->height / 2.0f, 0.0f) *
		XMMatrixScaling(1.0f / WIDTH_DIV_2, 1.0f / HEIGHT_DIV_2 / ASPECT, 1.0f) *
		XMMatrixTranslation(offsetX, offsetY, 0.0f) *
		XMMatrixScaling(scaleX, scaleY, 1.0f) *
		XMLoadFloat4x4(&m_viewBB) *
		XMMatrixTranslation(posX, posY, posZ) *
		XMLoadFloat4x4(&m_vp)
		));

	m_cbufferdata.uv.x = (float)(img % spr->numCols) / (float)spr->numCols;
	m_cbufferdata.uv.y = (float)(img / spr->numCols) / (float)spr->numRows;;

	context->UpdateSubresource(
		SpriteShader.constantVBuffer.Get(),
		0,
		NULL,
		&m_cbufferdata,
		0,
		0
		);

	context->DrawIndexed(
		6,
		0,
		0
		);

	context->IASetIndexBuffer(
		m_indexBufferSpr.Get(),
		DXGI_FORMAT_R16_UINT,
		0
		);

}

void Renderer::DrawMesh(Model* model, Mesh* mesh)
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	UINT stride = sizeof(VertexPosNormTex);
	UINT offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		mesh->vertexBuffer.GetAddressOf(),
		&stride,
		&offset
		);

	context->IASetIndexBuffer(
		mesh->indexBuffer.Get(),
		DXGI_FORMAT_R32_UINT,
		0
		);

	if (mesh->texture < model->textures.size() && model->textures[mesh->texture])
		context->PSSetShaderResources(0, 1, model->textures[mesh->texture].GetAddressOf());
	else
		context->PSSetShaderResources(0, 1, WhiteTexture.GetAddressOf());

	vbcbuffer m_cbufferdata;
	XMMATRIX mmodel = XMLoadFloat4x4(&mesh->offset) * XMLoadFloat4x4(&model->transform);

	XMStoreFloat4x4(&m_cbufferdata.mvp, XMMatrixTranspose(mmodel * XMLoadFloat4x4(&m_vp)));
	//XMStoreFloat4x4(&m_cbufferdata.m_it, XMMatrixTranspose(mmodel));

	// making the fourth row(or column) to be (0,0,0,1) cause the 3x3 part of inverse to be same as taking inverse of 3x3 part only
	const XMFLOAT4 v(0.0f, 0.0f, 0.0f, 1.0f);
	mmodel.r[3] = XMLoadFloat4(&v);
	XMStoreFloat4x4(&m_cbufferdata.m_it, XMMatrixInverse(nullptr, mmodel)); // transpose of transpose doesn't need to be taken


	context->UpdateSubresource(
		ThreeDShader.constantVBuffer.Get(),
		0,
		NULL,
		&m_cbufferdata,
		0,
		0
		);

	context->UpdateSubresource(
		ThreeDShader.constantPBuffer.Get(),
		0,
		NULL,
		&mesh->material,
		0,
		0
		);

	context->DrawIndexed(
		mesh->indexCount,
		0,
		0
		);
}

void Renderer::DrawModel(unsigned int model)
{
	for (unsigned i = 0; i<m_models[model].meshes.size(); ++i)
		DrawMesh(&m_models[model], &m_models[model].meshes[i]);
}

void Renderer::SetMaterial(unsigned model, float r, float g, float b, float a)
{
	XMFLOAT4 mat(r, g, b, a);
	Model* mdl = &m_models[model];
	for (unsigned i = 0; i < mdl->meshes.size(); ++i)
		mdl->meshes[i].material = mat;
}
void Renderer::SetMaterial(unsigned model, unsigned mesh, float r, float g, float b, float a)
{
	m_models[model].meshes[mesh].material = XMFLOAT4(r, g, b, a);
}

void Renderer::CreateDeviceDependentResources()
{
	ID3D11Device2* device = m_deviceResources->GetD3DDevice();

	const D3D11_INPUT_ELEMENT_DESC svertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	AddShader(&SpriteShader, "SpriteVertexShader.cso", "SpritePixelShader.cso", svertexDesc, ARRAYSIZE(svertexDesc), sizeof(vscbuffer), 0);
	unsigned short indicesBB[] =
	{
		0, 1, 3,
		0, 3, 2,
	};

	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	indexBufferData.pSysMem = indicesBB;
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC indexBufferDesc(sizeof(indicesBB), D3D11_BIND_INDEX_BUFFER);
	DX::ThrowIfFailed(
		device->CreateBuffer(
		&indexBufferDesc,
		&indexBufferData,
		&m_indexBufferBB
		)
		);

	unsigned short indicesSpr[] =
	{
		3, 1, 0,		//flip the indices since y-coordinates are negated for sprites
		2, 3, 0,
	};
	indexBufferData.pSysMem = indicesSpr;
	DX::ThrowIfFailed(
		device->CreateBuffer(
		&indexBufferDesc,
		&indexBufferData,
		&m_indexBufferSpr
		)
		);

	const D3D11_INPUT_ELEMENT_DESC mvertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	AddShader(&ThreeDShader, "BasicVertexShader.cso", "BasicPixelShader.cso", mvertexDesc, ARRAYSIZE(mvertexDesc), sizeof(vbcbuffer), sizeof(pbcbuffer));


	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	DX::ThrowIfFailed(
		device->CreateSamplerState(&sampDesc, &m_sampler));

	D3D11_BLEND_DESC1 BlendState;
	ZeroMemory(&BlendState, sizeof(D3D11_BLEND_DESC1));
	BlendState.RenderTarget[0].BlendEnable = TRUE;
	BlendState.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendState.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	BlendState.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendState.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendState.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState1(&BlendState, &m_transparency);

	m_deviceResources->GetD3DDeviceContext()->OMSetBlendState(m_transparency.Get(), nullptr, 0xffffffff);

	for (unsigned i = 0; i < m_fonts.size();++i)
	DX::ThrowIfFailed(
	m_deviceResources->GetD2DDeviceContext()->CreateSolidColorBrush(m_fonts[i].brushColor, &m_fonts[i].brush)
		);

	DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"white.dds", nullptr, &WhiteTexture));

	m_loadingComplete = true;
}

void Renderer::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	SpriteShader.vertexShader.Reset();
	SpriteShader.inputLayout.Reset();
	SpriteShader.pixelShader.Reset();
	SpriteShader.constantVBuffer.Reset();
	ThreeDShader.vertexShader.Reset();
	ThreeDShader.inputLayout.Reset();
	ThreeDShader.pixelShader.Reset();
	ThreeDShader.constantVBuffer.Reset();
	ThreeDShader.constantPBuffer.Reset();

	m_transparency.Reset();
	m_sampler.Reset();

	unsigned i, j;
	for (i = 0; i < m_sprites.size(); ++i)
		m_sprites[i].vertexBuffer.Reset();
	m_indexBufferSpr.Reset();
	m_indexBufferBB.Reset();

	for (i = 0; i < m_models.size(); ++i)
	{
		Model* mdl = &m_models[i];
		for (j = 0; j < mdl->meshes.size(); ++j)
		{
			mdl->meshes[j].vertexBuffer.Reset();
			mdl->meshes[j].indexBuffer.Reset();
		}
	}

	for (i = 0; i < m_fonts.size(); ++i)
		m_fonts[i].brush.Reset();
}

Platform::Array<byte>^ DX::ReadData(Platform::String^ filename)
{
	using namespace Platform;
	Array<byte>^ FileData = nullptr;

	// open the file
	std::ifstream File(filename->Data(), std::ios::in | std::ios::binary | std::ios::ate);

	// if open was successful
	if (File.is_open())
	{
		// find the length of the file
		int Length = (int)File.tellg();

		// collect the file data
		FileData = ref new Array<byte>(Length);
		File.seekg(0, std::ios::beg);
		File.read((char*)(FileData->Data), Length);
		File.close();
	}

	return FileData;
}


void Renderer::AddShader(Shader* shd, Platform::String^ vShaderFile, Platform::String^ pShaderFile,
	const D3D11_INPUT_ELEMENT_DESC* vertexDesc, UINT numElementsVDesc, UINT cvBufferWidth, UINT cpBufferWidth)
{
	ID3D11Device2* device = m_deviceResources->GetD3DDevice();

	Platform::Array<byte>^ fileData;
	fileData = DX::ReadData(vShaderFile);

	DX::ThrowIfFailed(
		device->CreateVertexShader(
		fileData->Data,
		fileData->Length,
		nullptr,
		&shd->vertexShader
		)
		);

	DX::ThrowIfFailed(
		device->CreateInputLayout(
		vertexDesc,
		numElementsVDesc,
		fileData->Data,
		fileData->Length,
		&shd->inputLayout
		)
		);


	fileData = DX::ReadData(pShaderFile);
	DX::ThrowIfFailed(
		device->CreatePixelShader(
		fileData->Data,
		fileData->Length,
		nullptr,
		&shd->pixelShader
		)
		);

	CD3D11_BUFFER_DESC constantBufferDesc(cvBufferWidth, D3D11_BIND_CONSTANT_BUFFER);
	DX::ThrowIfFailed(
		device->CreateBuffer(
		&constantBufferDesc,
		nullptr,
		&shd->constantVBuffer
		)
		);

	if (cpBufferWidth == 0) return;
	constantBufferDesc.ByteWidth = cpBufferWidth;
	DX::ThrowIfFailed(
		device->CreateBuffer(
		&constantBufferDesc,
		nullptr,
		&shd->constantPBuffer
		)
		);
}


















void Renderer::TranslateMesh(unsigned model, unsigned mesh, float x, float y, float z)
{
	XMStoreFloat4x4(&m_models[model].meshes[mesh].offset, XMLoadFloat4x4(&m_models[model].meshes[mesh].offset) *
		XMMatrixTranslation(x, y, z));
}
void Renderer::RotateMeshX(unsigned model, unsigned mesh, float Angle)
{
	XMStoreFloat4x4(&m_models[model].meshes[mesh].offset, XMLoadFloat4x4(&m_models[model].meshes[mesh].offset) *
		XMMatrixRotationX(Angle));
}
void Renderer::RotateMeshY(unsigned model, unsigned mesh, float Angle)
{
	XMStoreFloat4x4(&m_models[model].meshes[mesh].offset, XMLoadFloat4x4(&m_models[model].meshes[mesh].offset) *
		XMMatrixRotationY(Angle));
}
void Renderer::RotateMeshZ(unsigned model, unsigned mesh, float Angle)
{
	XMStoreFloat4x4(&m_models[model].meshes[mesh].offset, XMLoadFloat4x4(&m_models[model].meshes[mesh].offset) *
		XMMatrixRotationZ(Angle));
}
void Renderer::ScaleMesh(unsigned model, unsigned mesh, float size)
{
	XMStoreFloat4x4(&m_models[model].meshes[mesh].offset, XMLoadFloat4x4(&m_models[model].meshes[mesh].offset) *
		XMMatrixScaling(size, size, size));
}
void Renderer::ScaleMesh(unsigned model, unsigned mesh, float x, float y, float z)
{
	XMStoreFloat4x4(&m_models[model].meshes[mesh].offset, XMLoadFloat4x4(&m_models[model].meshes[mesh].offset) *
		XMMatrixScaling(x, y, z));
}
void Renderer::TranslateMeshAbs(unsigned model, unsigned mesh, float x, float y, float z)
{
	XMStoreFloat4x4(&m_models[model].meshes[mesh].offset, XMMatrixTranslation(x, y, z));
}
void Renderer::RotateMeshXAbs(unsigned model, unsigned mesh, float Angle)
{
	XMStoreFloat4x4(&m_models[model].meshes[mesh].offset, XMMatrixRotationX(Angle));
}
void Renderer::RotateMeshYAbs(unsigned model, unsigned mesh, float Angle)
{
	XMStoreFloat4x4(&m_models[model].meshes[mesh].offset, XMMatrixRotationY(Angle));
}
void Renderer::RotateMeshZAbs(unsigned model, unsigned mesh, float Angle)
{
	XMStoreFloat4x4(&m_models[model].meshes[mesh].offset, XMMatrixRotationZ(Angle));
}
void Renderer::ScaleMeshAbs(unsigned model, unsigned mesh, float size)
{
	XMStoreFloat4x4(&m_models[model].meshes[mesh].offset, (XMMatrixScaling(size, size, size)));
}
void Renderer::ScaleMeshAbs(unsigned model, unsigned mesh, float x, float y, float z)
{
	XMStoreFloat4x4(&m_models[model].meshes[mesh].offset, (XMMatrixScaling(x, y, z)));
}
void Renderer::IdentityMesh(unsigned model, unsigned mesh)
{
	XMStoreFloat4x4(&m_models[model].meshes[mesh].offset, (XMMatrixIdentity()));
}

void Renderer::TranslateModel(unsigned model, float x, float y, float z)
{
	XMStoreFloat4x4(&m_models[model].transform, XMLoadFloat4x4(&m_models[model].transform) *
		(XMMatrixTranslation(x, y, z)));
}
void Renderer::RotateModelX(unsigned model, float Angle)
{
	XMStoreFloat4x4(&m_models[model].transform, XMLoadFloat4x4(&m_models[model].transform) *
		(XMMatrixRotationX(Angle)));
}
void Renderer::RotateModelY(unsigned model, float Angle)
{
	XMStoreFloat4x4(&m_models[model].transform, XMLoadFloat4x4(&m_models[model].transform) *
		(XMMatrixRotationY(Angle)));
}
void Renderer::RotateModelZ(unsigned model, float Angle)
{
	XMStoreFloat4x4(&m_models[model].transform, XMLoadFloat4x4(&m_models[model].transform) *
		(XMMatrixRotationZ(Angle)));
}
void Renderer::ScaleModel(unsigned model, float size)
{
	XMStoreFloat4x4(&m_models[model].transform, XMLoadFloat4x4(&m_models[model].transform) *
		(XMMatrixScaling(size, size, size)));
}
void Renderer::ScaleModel(unsigned model, float x, float y, float z)
{
	XMStoreFloat4x4(&m_models[model].transform, XMLoadFloat4x4(&m_models[model].transform) *
		(XMMatrixScaling(x, y, z)));
}
void Renderer::TranslateModelAbs(unsigned model, float x, float y, float z)
{
	XMStoreFloat4x4(&m_models[model].transform, (XMMatrixTranslation(x, y, z)));
}
void Renderer::RotateModelXAbs(unsigned model, float Angle)
{
	XMStoreFloat4x4(&m_models[model].transform, (XMMatrixRotationX(Angle)));
}
void Renderer::RotateModelYAbs(unsigned model, float Angle)
{
	XMStoreFloat4x4(&m_models[model].transform, (XMMatrixRotationY(Angle)));
}
void Renderer::RotateModelZAbs(unsigned model, float Angle)
{
	XMStoreFloat4x4(&m_models[model].transform, (XMMatrixRotationZ(Angle)));
}
void Renderer::ScaleModelAbs(unsigned model, float size)
{
	XMStoreFloat4x4(&m_models[model].transform, (XMMatrixScaling(size, size, size)));
}
void Renderer::ScaleModelAbs(unsigned model, float x, float y, float z)
{
	XMStoreFloat4x4(&m_models[model].transform, (XMMatrixScaling(x, y, z)));
}
void Renderer::IdentityModel(unsigned model)
{
	XMStoreFloat4x4(&m_models[model].transform, XMMatrixIdentity());
}
void Renderer::RotateModelAbs(unsigned model, float w, float x, float y, float z)
{
    XMFLOAT4 rot(x, y, z, w);
    XMStoreFloat4x4(&m_models[model].transform, XMLoadFloat4x4(&m_models[model].transform) *
        XMMatrixRotationQuaternion(XMLoadFloat4(&rot)));
}
