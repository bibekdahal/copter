#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"

namespace Copter
{
	ref class Renderer
	{
	internal:
		Renderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
	public:
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(double totalTime, double deltaTime);
		void Render();
		Windows::Foundation::Size GetRenderTargetSize()			{ return m_deviceResources->GetRenderTargetSize(); }

		unsigned AddSprite(wchar_t* fileName, float width, float height, int numCols = 1, int numRows = 1, float xSpace = 0, float ySpace = 0);
		void DrawSprite(unsigned index, float posX, float posY, unsigned img, float depth = 0.0f, float offsetX = 0.0f, float offsetY = 0.0f);
		void DrawBillboard(unsigned sprIndex, float posX, float posY, float posZ, unsigned img, float offsetX = 0.0f, float offsetY = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f);

		unsigned AddFont(wchar_t* fontName, float fontSize,
			float color_r = 0.0f, float color_g = 0.0f, float color_b = 0.0f, float color_a = 1.0f, int fontWeight = DWRITE_FONT_WEIGHT_NORMAL,
			int fontStyle = DWRITE_FONT_STYLE_NORMAL, int fontStretch = DWRITE_FONT_STRETCH_NORMAL, int textAlignment = DWRITE_TEXT_ALIGNMENT_LEADING,
			int paraAlignment = DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		void DrawText(unsigned font, Platform::String^ text, float posX, float posY, float width, float height);
		
		unsigned AddModel();
		unsigned AddMesh(unsigned model, Vertex* vertices, unsigned numVertices, unsigned* indices, unsigned numIndices, unsigned texture);
		unsigned AddTexture(unsigned model, Platform::String^ filename);
		void DrawModel(unsigned int model);

		void SetMaterial(unsigned model, float r, float g, float b, float a);
		void SetMaterial(unsigned model, unsigned mesh, float r, float g, float b, float a);

		void TranslateMesh(unsigned model, unsigned mesh, float x, float y, float z);
		void RotateMeshX(unsigned model, unsigned mesh, float Angle);
		void RotateMeshY(unsigned model, unsigned mesh, float Angle);
		void RotateMeshZ(unsigned model, unsigned mesh, float Angle);
		void ScaleMesh(unsigned model, unsigned mesh, float size);
		void ScaleMesh(unsigned model, unsigned mesh, float x, float y, float z);

		void TranslateMeshAbs(unsigned model, unsigned mesh, float x, float y, float z);
		void RotateMeshXAbs(unsigned model, unsigned mesh, float Angle);
		void RotateMeshYAbs(unsigned model, unsigned mesh, float Angle);
		void RotateMeshZAbs(unsigned model, unsigned mesh, float Angle);
		void ScaleMeshAbs(unsigned model, unsigned mesh, float size);
		void ScaleMeshAbs(unsigned model, unsigned mesh, float x, float y, float z);
		void IdentityMesh(unsigned model, unsigned mesh);

		void TranslateModel(unsigned model, float x, float y, float z);
		void RotateModelX(unsigned model, float Angle);
		void RotateModelY(unsigned model, float Angle);
		void RotateModelZ(unsigned model, float Angle);
		void ScaleModel(unsigned model, float size);
		void ScaleModel(unsigned model, float x, float y, float z);

		void TranslateModelAbs(unsigned model, float x, float y, float z);
		void RotateModelXAbs(unsigned model, float Angle);
		void RotateModelYAbs(unsigned model, float Angle);
		void RotateModelZAbs(unsigned model, float Angle);
		void ScaleModelAbs(unsigned model, float size);
		void ScaleModelAbs(unsigned model, float x, float y, float z);
		void IdentityModel(unsigned model);
        void RotateModelAbs(unsigned model, float w, float x, float y, float z);
		
		void BeginRender2D();
		void BeginRender3D();

		void SetCamEye(float x, float y, float z);
		void SetCamAt(float x, float y, float z);
		void SetCamUp(float x, float y, float z);
    internal:
        unsigned AddModel(Platform::String^ filename, btCollisionShape** shape = NULL, float scale = 1);
        unsigned AddMesh(unsigned model, Platform::String^ filename, int texture = -1, btCollisionShape** shape = NULL, float scale = 1);
    private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		bool	m_loadingComplete;

		struct Shader
		{
			Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
			Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
			Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
			Microsoft::WRL::ComPtr<ID3D11Buffer> constantVBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> constantPBuffer;
		};
		void AddShader(Shader* shd, Platform::String^ vShaderFile, Platform::String^ pShaderFile,
			const D3D11_INPUT_ELEMENT_DESC* vertexDesc, UINT numElementsVDesc, UINT cvBufferWidth, UINT cpBufferWidth);

		struct Sprite
		{
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
			Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
			float width, height, xSpace, ySpace;
			int numCols, numRows;
		};

		struct Font
		{
			Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush;
			Microsoft::WRL::ComPtr<IDWriteTextFormat> textformat;
			D2D1_COLOR_F brushColor;
		};

		struct Mesh
		{
			Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
			unsigned indexCount;
			unsigned texture;
			DirectX::XMFLOAT4X4 offset;
			DirectX::XMFLOAT4 material;
		};

		struct Model
		{
			std::vector<Mesh> meshes;
			std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textures;
			DirectX::XMFLOAT4X4 transform;
		};

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBufferSpr;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBufferBB;
		Microsoft::WRL::ComPtr<ID3D11BlendState1> m_transparency;

		DirectX::XMFLOAT4X4 m_projection2D;
		DirectX::XMFLOAT4X4 m_projection;
		DirectX::XMFLOAT4X4 m_viewBB;
		DirectX::XMFLOAT4X4 m_vp;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;

		std::vector<Sprite> m_sprites;
		std::vector<Font> m_fonts;

		Shader SpriteShader;
		Shader ThreeDShader;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> WhiteTexture;

		DirectX::XMVECTOR eye;
		DirectX::XMVECTOR at;
		DirectX::XMVECTOR up;

		std::vector<Model> m_models;
		void DrawMesh(Model* model, Mesh* mesh);


		float WIDTH_DIV_2;
		float HEIGHT_DIV_2;
		float ASPECT;
		
	};
}

