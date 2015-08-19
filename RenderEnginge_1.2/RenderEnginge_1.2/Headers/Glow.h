#pragma once
#ifndef GLOW_H
#define GLOW_H
#endif

#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <d3d11.h>
#include <d3dcompiler.h>

class Glow{
private:
	int screen_Width;
	int screen_Height;

	struct BlurConstantStruct{
		float screenSize; //på ett håll i taget!
		float pad3[3];
	};

	struct GlowConstantStruct{
		float glowThreshHold;
		float glowValue;
		float pad2[2];

		GlowConstantStruct(){
			glowThreshHold = 0.8f;
			glowValue = 1.0f;
			for (int i = 0; i < 2; i++){
				pad2[i] = 0.0f;
			}
		}
	};
	float glowThreshHold = 0.99f;
	float glowValue = 0.0f;
	
	BlurConstantStruct horizontalConstantStruct;
	BlurConstantStruct verticalConstantStruct;
	GlowConstantStruct glowConstantStruct;
	ID3D11Buffer *horizontalConstantBuffer;
	ID3D11Buffer *verticalConstantBuffer;
	ID3D11Buffer *glowConstantBuffer;
	
	ID3D11Texture2D *renderTargetTexture; //texturen där glowen kommer ligga
	ID3D11Texture2D *tempRenderTargetTexture;
	ID3D11Texture2D *depthStencilBuffer;
	

	ID3D11InputLayout *glowInputLayout;
	ID3DBlob* glowBlob;

	ID3D11Device *gDevice;
	ID3D11DeviceContext *gDeviceContext;

	

public:
	ID3D11RenderTargetView *renderTargetView; //det man ritar på
	ID3D11RenderTargetView *tempRenderTargetView; //det man ritar på
	ID3D11ShaderResourceView *shaderResourceView; //det som skickas in till shadern vid andra passet, det som ska ritas ut
	ID3D11ShaderResourceView *tempShaderResourceView;
	ID3D11DepthStencilView *depthStencilView;
	D3D11_VIEWPORT glowViewPort;

	ID3D11VertexShader *glowVertexShader;
	ID3D11PixelShader *glowPixelShader;

	ID3D11Buffer *planeVertexBuffer; //rendera på denna vid andra passet

	Glow(){
		void CreateViewPort();
		void CreateTextures();
		void CreatePlaneAndBuffers();
	}
	Glow(ID3D11Device *gDevice, ID3D11DeviceContext *gDeviceContext, int screen_Width, int screen_Height, ID3D11VertexShader *glowVS, ID3D11PixelShader *glowPS, ID3DBlob* glowBlob){
		this->gDevice = gDevice;
		this->gDeviceContext = gDeviceContext;
		this->screen_Width = screen_Width; //dessa går att ändra
		this->screen_Height = screen_Height;
		this->glowVertexShader = glowVS;
		this->glowPixelShader = glowPS;
		this->glowBlob = glowBlob;

		CreatePlaneAndBuffers();
		CreateViewPort();
		CreateTextures();
	}
	~Glow(){}

	void CreateViewPort();
	void CreateTextures();
	void CreatePlaneAndBuffers();
	void DrawToGlowMap();
	void ApplyBlurOnGlowHorizontal(ID3D11VertexShader *VS, ID3D11PixelShader *PS);
	void ApplyBlurOnGlowVertical(ID3D11VertexShader *VS, ID3D11PixelShader *PS);
};