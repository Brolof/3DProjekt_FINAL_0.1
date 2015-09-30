#pragma once
#ifndef HEIGHTMAP2_H
#define HEIGHTMAP2_H
#endif

#include <vector>
#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <iostream>
#include <fstream>
#include "WICTextureLoader.h"
#include <WICTextureLoader.h>
#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include <windowsx.h>
#include <string>
#include "SimpleMath.h"


using namespace DirectX::SimpleMath;
using namespace DirectX;
using namespace std;

class heightMap2
{



public:


	int colls;
	int rows;
	const int TEXTURE_REPEAT = 4;

	int HeíghtNumFaces;
	int HeightNumVertices;
	ID3D11Buffer* indexBuff;
	ID3D11Buffer* vertBuff = nullptr;


	struct HeightMap2Info{		// Heightmap structure
		int terrainWidth;		// Width of heightmap
		int terrainHeight;		// Height (Length) of heightmap
		XMFLOAT3 *heightMap;	// Array to store terrain's vertex positions
	};
	HeightMap2Info tempMap;


	struct Vertex	//Overloaded Vertex Structure
	{
		Vertex(){}
		Vertex(float x, float y, float z,
			float u, float v,
			float nx, float ny, float nz)
			: pos(x, y, z), texCoord(u, v), normal(nx, ny, nz){}

		XMFLOAT3 pos;
		XMFLOAT2 texCoord;
		XMFLOAT3 normal;
	};

	//funcs

	bool HeightMapLoad(char* hMap,  HeightMap2Info &hminfo);
	void createMap(ID3D11Device* dev, std::wstring tex1, std::wstring tex2, std::wstring tex3, std::wstring splat);
	void render(ID3D11Device* dev, ID3D11DeviceContext* devcon);

	heightMap2();
	~heightMap2(){}

	ID3D11Buffer* GetVertexBuffer(){
		return vertBuff;
	}

	ID3D11Buffer* GetIndexBuffer(){
		return indexBuff;
	}

	int GetFaces(){
		return HeíghtNumFaces;
	}



public:

	ID3D11ShaderResourceView* tex1shaderResourceView = nullptr;
	ID3D11ShaderResourceView* tex2shaderResourceView = nullptr;
	ID3D11ShaderResourceView* tex3shaderResourceView = nullptr;
	ID3D11ShaderResourceView* splatshaderResourceView = nullptr;

	 float hmapSize = 256;
	//Collision
	 float heights[256][256];
	float terrainSizeMultiplier = 1;
	
	float getHeightOfTerrain(float worldX, float worldZ);
	//RETURNS HEIGHT at camera position

private:


};