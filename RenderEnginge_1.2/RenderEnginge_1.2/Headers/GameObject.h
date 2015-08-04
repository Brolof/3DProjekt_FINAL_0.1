#pragma once

#include "Entity.h"

struct MatInfo
{
	MatInfo()
		: Emissive(0.0f, 0.0f, 0.0f, 1.0f),
		Ambient(0.1f, 0.1f, 0.1f, 1.0f),
		Diffuse(1.0f, 1.0f, 1.0f, 1.0f),
		Specular(1.0f, 1.0f, 1.0f, 1.0f),
		SpecPow(128.0f),
		UseTexture(0){}

	MatInfo(XMFLOAT4 A, XMFLOAT4 D, XMFLOAT4 S, float SP)
	{
		Emissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		Ambient = A;
		Diffuse = D;
		Specular = S;
		SpecPow = SP;
		UseTexture = 1;
	}

	XMFLOAT4 Emissive;
	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;
	float SpecPow;
	int UseTexture;
	float Padding[2];

};

struct MaterialProperties
{
	MatInfo Material;
};

class GameObjects : public Entity{
public:
	int texIndex;
	GameObjects(ID3D11Buffer *b, XMFLOAT3 pos, bool isActive, bool isStatic) : Entity(pos, isActive, isStatic){
		this->vertexBuffer = b;

	}

	GameObjects(){}

	~GameObjects(){
		//vertexBuffer->Release();
		//indexBuffer->Release();
	}

	ID3D11Buffer* GetVertexBuffer(){
		return vertexBuffer;
	}

	void setVertexBuffer(ID3D11Buffer* VertexBufferz){
		vertexBuffer = VertexBufferz;
	}

	ID3D11Buffer* GetIndexBuffer(){
		return indexBuffer;
	}

	int nrElements;

	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *indexBuffer;

	//Material Info
	MatInfo material;


protected:
	struct Triangle{
		XMVECTOR x, y, z;
	};
	//texture
	
};


namespace MatPresets
{
	// Material Presets
	const MatInfo Emerald(
		XMFLOAT4(0.0215, 0.1745, 0.0215, 1.0),
		XMFLOAT4(0.07568, 0.61424, 0.07568, 1.0),
		XMFLOAT4(0.633, 0.727811, 0.633, 1.0),
		float(76.8f));
		
	const MatInfo Copper(
		XMFLOAT4(0.19125, 0.0735, 0.0225, 1.0),
		XMFLOAT4(0.7038, 0.27048, 0.0828, 1.0),
		XMFLOAT4(0.256777, 0.137622, 0.086014, 1.0),
		float(0.1f));
	const MatInfo Lambert(
		XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0),
		XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0),
		XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0),
		float(0.0f));

	const MatInfo BlinnBase(
		XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0),
		XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0),
		XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0),
		float(0.3f));
}