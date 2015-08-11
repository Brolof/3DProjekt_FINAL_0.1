#pragma once

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
#include "RenderEngine.h"



using namespace std;

class HeightMap{
protected:
	struct Float3{
		float x, y, z;

		Float3()
		{
			x = 1;
			y = 1;
			z = 1;
		}

		Float3(float x, float y, float z){
			this->x = x;
			this->y = y;
			this->z = z;
		}

		Float3 Float3Subtract(Float3 s){
			s.x -= s.x;
			s.y -= s.y;
			s.z -= s.z;
			return s;
		}
	};

	struct Vertex{
		float x, y, z;
		float u, v;
		float alphaU, alphaV;
		float nx, ny, nz;
	};

	struct HeightMapInfo{
		int terrainWidth;
		int terrainHeight;
		Float3 *heightMap;
	};

	unsigned char* heights; //kommer innehålla ett värde (en char) för varje pixel på heightmappen, det behövs bara ett då det är gråskala - höjd

public:
	HeightMap(ID3D11Device* gDevice, ID3D11DeviceContext* gDeviceContext);
	~HeightMap(){}
	bool ImportHeightmap(char *f, wstring texture1, wstring texture2, wstring texture3, wstring splatMap){
		if (LoadHeightMap(f) == false) return false;
		if (LoadSplatMap(texture1, texture2, texture3, splatMap) == false) return false;
		return true;
	}
	bool LoadHeightMap(char* fileName);
	bool LoadSplatMap(wstring texture1, wstring texture2, wstring texture3, wstring splatMap);

	int heightElements; //till constant buffer som håller värdet till splatmappens uv stuff

	int GetNrElements(){
		return nrElements;
	}

	ID3D11Buffer* GetVertexBuffer(){
		return vertexBuffer;
	}

	ID3D11Buffer* GetIndexBuffer(){
		return indexBuffer;
	}

	ID3D11ShaderResourceView* GetTex1(){
		return tex1shaderResourceView;
	}

	ID3D11ShaderResourceView* GetTex2(){
		return tex2shaderResourceView;
	}

	ID3D11ShaderResourceView* GetTex3(){
		return tex3shaderResourceView;
	}

	ID3D11ShaderResourceView* GetSplatTex(){
		return splatshaderResourceView;
	}

	std::vector<int> GetHeights(){
		return vertexHeightsArray;
	}

	int GetGridSize(){
		return gridSize;
	}

protected:
	int numVerts;
	int numFaces;

	int nrElements;

	int gridSize;
	float heightMultiplier;

	std::vector<int> vertexHeightsArray;

	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* indexBuffer = nullptr;
	ID3D11Device* gDevice = nullptr;
	ID3D11DeviceContext* gDeviceContext = nullptr;

	ID3D11ShaderResourceView* tex1shaderResourceView = nullptr;
	ID3D11ShaderResourceView* tex2shaderResourceView = nullptr;
	ID3D11ShaderResourceView* tex3shaderResourceView = nullptr;
	ID3D11ShaderResourceView* splatshaderResourceView = nullptr;

};