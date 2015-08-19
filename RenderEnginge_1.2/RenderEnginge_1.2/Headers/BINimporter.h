#pragma once
#ifndef BINIMPORTER_H
#define BINIMPORTER_H
#endif
#include <iostream>
#include <fstream>

#include <vector>
#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <DirectXMathVector.inl>
#include <d3d11.h>
#include <d3dcompiler.h>
#include "Entity.h"
#include "GameObject.h"

using namespace DirectX;



class BINimporter{
public:
	void ImportBIN(ID3D11Device* gDevice, char* fileName);

private:
	vector<GameObjects*> binObj;



	struct FileInfo{
		int nrMeshes;
		int nrLights;
		int nrMaterials;
	};

	struct MeshInfo{
		int meshType;
		int nrAnimations;
		int nrBones;
		int nrPos;
		int nrNor;
		int nrUV;
		int nrTangents;
		int nrFaces;
		int nrTex;
		string name;
		int texIndex;
	};

	struct VertexData{
		XMFLOAT3 vertPos;
		XMFLOAT2 vertUV;
		XMFLOAT3 vertNor;
		XMFLOAT3 vertTangent;
	
	};

	struct FaceData{
		int indexPos[3];
		int indexNor[3];
		int indexUV[3];
		int indexTangent[3];
	};

	struct MaterialData{
		float kd[3];
		float ka[3];
		float ks[3];
		float trans;
	};



	struct Material{
		int byteSizeOfName;
		string name;

		XMFLOAT3 ka, kd, ks;
	};

	struct Int4{
		int x, y, z, w;
	};

	struct Float4{
		float x, y, z, w;
	};

public:
	BINimporter(){}
	~BINimporter(){}

	vector<string> texNameArray;
	vector<string> testTexNameArray;
	vector<int> indexArray;
	std::string mapName = "Textures/";




	vector<string> GettexNameArray(){
		return texNameArray;
	}
	vector<string> GettestTexNameArray(){
		return testTexNameArray;
	}
	vector<int> GetindexArray(){
		return indexArray;
	}


	vector<GameObjects*> GetObjects(){
		return binObj;
	}
};

