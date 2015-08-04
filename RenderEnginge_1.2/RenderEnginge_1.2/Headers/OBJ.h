#pragma once
#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <fstream>
#include <string.h>
#include <vector>
#include <string>
#include <DirectXCollision.h>

using namespace DirectX;
using namespace std;

class OBJ{

protected:

	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* indexBuffer = nullptr;
	UINT nrElements = 0;
	BoundingBox bBox;

	vector<XMFLOAT3> vertexPositions;
	vector<int> indecies;


	struct Float3{
		float x, y, z;
	};

	struct MaterialValues{
		Float3 kd;
		Float3 ks;
		Float3 ka;
	};

	MaterialValues material;

public:
	char* geometryFileNameTemp;

	OBJ(ID3D11Device* gDevice);
	~OBJ();
	void LoadObject(char* geometryFileName, char* materialFileName);

	void CleanUp();

	ID3D11Buffer** GetVertexBuffer(){
		return &vertexBuffer;
	}

	ID3D11Buffer** GetIndexBuffer(){
		return &indexBuffer;
	}

	MaterialValues GetMaterial()
	{
		return material;
	}

	UINT GetNrElements(){
		return nrElements;
	}

	char* GetName(){
		return geometryFileNameTemp;
	}
	
	BoundingBox GetStaticBBOX(){
		return bBox;
	}

	vector<int> GetIndecies(){ //kommer användas vid bla picking
		return indecies;
	}

	vector<XMFLOAT3> GetVertexPositions(){ //kommer användas vid bla picking
		return vertexPositions;
	}


protected:
	ID3D11Device* gDevice = nullptr;
	struct Int3{
		int x, y, z;
	};

	struct Float2{
		float u, v;
	};


	struct VertexPos{
		float vx, vy, vz;
				
	};
	struct VertexUV{
		float u, v;
	};

	struct VertexNor{
		float nx, ny, nz;
	};

	struct Vertex{
		float vx, vy, vz;
		float u, v;
		float nx, ny, nz;
	};

	struct FaceIndex{
		Int3 vert1, vert2, vert3;
	};


	std::vector<float> SavedVertsX;
	std::vector<float> SavedVertsY;
	std::vector<float> SavedVertsZ;
	std::vector<int> indexcount;


public:
	
	std::vector<float> GetVerticiesX()
	{
		return SavedVertsX;
	}
	std::vector<float> GetVerticiesY()
	{
		return SavedVertsY;
	}
	std::vector<float> GetVerticiesZ()
	{
		return SavedVertsZ;
	}

	std::vector<int> GetverteciesIndex()
	{
		return indecies;
	}

	void CreateAABB(std::vector<Vertex>);

};