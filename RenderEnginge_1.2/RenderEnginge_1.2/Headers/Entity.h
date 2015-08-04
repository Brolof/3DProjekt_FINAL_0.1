#pragma once
#ifndef ENTITY_H
#define ENTITY_H
#endif

#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXCollision.h>
#include <vector>


using namespace std;
using namespace DirectX;
//using namespace DirectX::TriangleTests;

class Entity{
protected:
	bool isActive; //ska vi rendera detta eller inte? jag l�gger denna h�r ifall vi tex vill kunna st�nga av ljus ox�
	bool isStatic;
	XMMATRIX rot;
	XMMATRIX scale;
	XMMATRIX pos;	

public:
	XMMATRIX world;
	XMFLOAT3 center; //xyz, vart den �r i maya v�rlden f�rh�llande till center punkten (origo)

	int indexT;


	Entity(XMFLOAT3 center, bool isActive, bool isStatic){
		this->isActive = isActive;
		this->isStatic = isStatic;
		this->center = center;
		
	
		pos = XMMatrixIdentity();
		rot = XMMatrixIdentity(); //den rotationen och skalningen som den importeras in som kommer vara standard v�rdet
		scale = XMMatrixIdentity();

		CalculateWorld();


	}
	Entity();
	~Entity(){

	}

	

	void Translate(float x, float y, float z){
		pos = XMMatrixTranslation(x, y, z);
	}

	void TranslateExact(float x, float y, float z){
		pos = XMMatrixTranslation(x, y, z);
	}

	void Rotate(XMVECTOR rotAxis, float rotValue){
		rot = XMMatrixRotationAxis(rotAxis, rotValue);
	}

	void Scale(float x, float y, float z){
		scale = XMMatrixScaling(x, y, z);
	}

	void CalculateWorld(){ //denna ska kallas innan man skickar in den i GPUn
		world = scale * rot * pos; //st�mmer detta?
	}

	void SetActive(bool a){
		isActive = a; //anv�nds ifall vi vill st�nga av detta object, tex n�r man plockar upp den
	}

	bool GetActive(){ return isActive; }
	bool GetStatic(){ return isStatic; }


	XMFLOAT3 SubXMFLOAT3(XMFLOAT3 a, XMFLOAT3 b){
		return XMFLOAT3(a.x - b.x, a.y - b.y, a.z - b.z);
	}

	bool SameXMFLOAT3(XMFLOAT3 a, XMFLOAT3 b){
		if ((a.x - b.x < 0.00001f) && (a.x - b.x > -0.00001f) && (a.y - b.y < 0.00001f) && (a.y - b.y > -0.00001f) && (a.z - b.z < 0.00001f) && (a.z - b.z > -0.00001f))
			return true;
		else return false;
	}
};