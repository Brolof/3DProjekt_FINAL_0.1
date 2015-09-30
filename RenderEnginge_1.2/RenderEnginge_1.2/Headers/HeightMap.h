#pragma once

<<<<<<< HEAD
=======
#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H
#endif

>>>>>>> Merged
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

		Vertex(){
			x = 0;
			y = 0;
			z = 0;

			u = 0;
			v = 0;

			alphaU = 0;
			alphaV = 0;

			nx = 0;
			ny = 0;
			nz = 0;
		}

	};

	struct HeightMapInfo{
		DWORD terrainWidth;
		DWORD terrainHeight;
		//Float3 *heightMap;
	};

	unsigned char* heights; //kommer inneh�lla ett v�rde (en char) f�r varje pixel p� heightmappen, det beh�vs bara ett d� det �r gr�skala - h�jd

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

	int heightElements; //till constant buffer som h�ller v�rdet till splatmappens uv stuff

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

	std::vector<float> GetHeights(){
		return vertexHeightsArray;
	}

	void GetHeightOnPosition(float x, float z, float& y);

	int GetGridSize(){
		return gridSize;
	}

protected:
	int numVerts;
	int numFaces;

	int nrElements;

	int gridSize; //avst�ndet mellan tv� vertiser
	int widthWhole, heightWhole;
	float heightMultiplier;

	std::vector<Vertex> vertecies; //all the vertices
	std::vector<float> vertexHeightsArray; //all the vertexheights

	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* indexBuffer = nullptr;
	ID3D11Device* gDevice = nullptr;
	ID3D11DeviceContext* gDeviceContext = nullptr;

	ID3D11ShaderResourceView* tex1shaderResourceView = nullptr;
	ID3D11ShaderResourceView* tex2shaderResourceView = nullptr;
	ID3D11ShaderResourceView* tex3shaderResourceView = nullptr;
	ID3D11ShaderResourceView* splatshaderResourceView = nullptr;

};