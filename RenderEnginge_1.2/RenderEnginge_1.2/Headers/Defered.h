#pragma once
#ifndef DEFERED_H
#define DEFERED_H
#endif

#include <vector>
#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <DirectXMathVector.inl>
#include <d3d11.h>
#include <d3dcompiler.h>
#include "GameObject.h"
#include "SimpleMath.h"
#include "WICTextureLoader.h"

using namespace DirectX::SimpleMath;
using namespace DirectX;


class DeferedRenderer{

	private:
		//BUFFERS
		ID3D11Texture2D* depthBuffer = nullptr;
		ID3D11Buffer* gWorld = nullptr;
		ID3D11Buffer* gWorld2 = nullptr;


public:		//SHADERS
		ID3D11VertexShader* vertexShader = nullptr;
		ID3D11PixelShader* pixelShader = nullptr;
		ID3D11VertexShader* deferedVertexShader = nullptr;
		ID3D11PixelShader* deferedPixelShader = nullptr;
		//SHADER HELPERS
		struct WorldMatrix
		{
			XMFLOAT4X4 View;
			XMFLOAT4X4 Projection;
			XMFLOAT4X4 WorldSpace;
			XMFLOAT4X4 InvWorld;
			XMFLOAT4X4 WVP;

		};
		WorldMatrix VertexShaderBuffer;
		struct WorldMatrix2
		{
			XMFLOAT4X4 WVP;
			XMFLOAT4X4 World;
		};
		WorldMatrix2 VertexShaderBuffer2;

		int tex;

		ID3D11InputLayout* InputLayout;
		ID3D11InputLayout* InputLayout2;
	public:
		//CONSTANTS
		XMMATRIX CamView;
		XMMATRIX CamProjection;
		XMMATRIX World;

		//MATRIXES
		XMMATRIX depthProjection, lightViewMatrix, lightworld, identity;

		//Shadow map bulding pieces
		ID3D11Texture2D* textures[3];
		ID3D11Texture2D* rtvTex;
		ID3D11ShaderResourceView* SRVArray[3];
		ID3D11DepthStencilView* DSV;
		ID3D11DepthStencilView* DSV2;
		ID3D11RenderTargetView* RTVArray[3];
		ID3D11RenderTargetView* RTVFinal;
		//ViewPort
		D3D11_VIEWPORT DeferedVP;
		D3D11_VIEWPORT DeferedVP2;
		ID3D11SamplerState* sampState = nullptr;
		ID3D11SamplerState* sampState3 = nullptr;
		ID3D11ShaderResourceView** RSWArray = nullptr;
		ID3D11ShaderResourceView* NormTex = nullptr;
		vector<int> intArrayTextures;

	public:

		DeferedRenderer();
		~DeferedRenderer();
		void CreateTextureMap(ID3D11Device* gDevice, int sw, int sh, float sd, float sn, ID3D11ShaderResourceView** RSWArray, vector<int> intArrayTex);
		//Shader Compiler
		HRESULT DeferedRenderer::CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob);
	
		void DrawDefered(std::vector<GameObjects*> Mesh, ID3D11DeviceContext* deviceContext, XMMATRIX view, XMMATRIX proj, XMMATRIX world);
		void DrawDefered2(ID3D11Buffer* buff, ID3D11DeviceContext* deviceContext);


		ID3D11ShaderResourceView* GetSRV(int x){
			return SRVArray[x];
		}


	};

