#pragma once
#ifndef SHADOWMAP_H
#define SHADOWMAP_H
#endif

#include <vector>
#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <DirectXMathVector.inl>
#include <d3d11.h>
#include <d3dcompiler.h>
#include "GameObject.h"
#include "lights.h"
#include "SimpleMath.h"
#include "Cam.h"

using namespace DirectX::SimpleMath;
using namespace DirectX;


class ShadowMap{

private:
	//BUFFERS
	ID3D11Buffer* shadowBuffer = nullptr;
	ID3D11Buffer* gWorld = nullptr;
	
	//SHADERS
	ID3D11VertexShader* depthVertexShader = nullptr;
	ID3D11PixelShader* depthPixelShader = nullptr;

	//SHADER HELPERS
	struct DepthWorldMatrix
	{
		XMFLOAT4X4 View;
		XMFLOAT4X4 Projection;
		XMFLOAT4X4 WorldSpace;
		XMFLOAT4X4 InvWorld;
		XMFLOAT4X4 WVP;

		//Shadow matrixs
		XMFLOAT4X4 lightView;
		XMFLOAT4X4 lightProjection;
	};
	DepthWorldMatrix VertexShaderBuffer;


	ID3D11InputLayout* depthInputLayout;
private:
	//CONSTANTS
	int SHADOWMAP_WIDTH ;
	int SHADOWMAP_HEIGHT ;
	float SHADOWMAP_DEPTH;
	float SHADOWMAP_NEAR;

	//Cam for depth render
	Camera lightCam;

	//CAMERA CONSTANTS
	Vector4 camPosition2;
	Vector4 camTarget2;
	Vector4 camUp2;// Vector4(0.0f, 1.0f, 0.0f, 0.0f);;
	Vector4 DefaultForward2 = Vector4(0.0f, -1.0f, 0.0f, 0.0f);
	Vector4 DefaultRight2 = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
	Vector4 camForward2 = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
	Vector4 camRight2 = Vector4(1.0f, 0.0f, 0.0f, 0.0f);

	XMMATRIX camRotationMatrix2;
	XMMATRIX fpsCamLook2;
	XMMATRIX CamProjection2;
	XMMATRIX CamView2;

	//MATRIXES
	XMMATRIX depthProjection ,lightViewMatrix, lightworld;

	//Shadow map bulding pieces
	ID3D11Texture2D* depthMapTexture = nullptr;
	ID3D11Texture2D* deptMapBuffer = nullptr;
	ID3D11ShaderResourceView* depthMapSRV;
	ID3D11DepthStencilView* depthMapDSV;
	ID3D11RenderTargetView* depthMapRTV;
	//ViewPort
	D3D11_VIEWPORT depthVP;
public:

	ShadowMap();
	~ShadowMap();
	void CreateShadowMap(ID3D11Device* gDevice, DirLight light, XMFLOAT4 lightPos);
	//Shader Compiler
	HRESULT ShadowMap::CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob);
	//Set shadowmap constants
	//Matrixtype: 1 = Projecton , 2 = Ortographic
	void SetShadowMapConstants(int size, float Depth, float Near, int matrixType, float orthosize);

	void DrawDepthMap(std::vector<GameObjects*> Mesh, ID3D11DeviceContext* deviceContext);


	ID3D11ShaderResourceView* GetSRV(){
		return depthMapSRV;
	}

	XMMATRIX GetLightView(){
		return lightViewMatrix;
	}
	XMMATRIX GetLightProj(){
		return depthProjection;
	}

		
};
