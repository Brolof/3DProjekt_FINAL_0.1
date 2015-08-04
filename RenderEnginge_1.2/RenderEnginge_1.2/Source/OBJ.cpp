#include "OBJ.h"


OBJ::OBJ(ID3D11Device* gDevice){
	this->gDevice = gDevice;
	nrElements = 0;
}
OBJ::~OBJ(){

}

void OBJ::LoadObject(char* geometryFileName, char* materialFileName){
	std::ifstream obj_File;
	std::ifstream material_File;
	std::string line;

	std::vector<Vertex> vertecies;
	std::vector<VertexPos> verteciesPos;
	std::vector<VertexUV> verteciesUV;
	std::vector<VertexNor> verteciesNor;

	std::vector<FaceIndex> faceIndecies;
	std::vector<int> faceIndeciesInt;
	
	float x, y, z;
	float u, v;
	float nx, ny, nz;

	Int3 fx, fy, fz;

	obj_File.open(geometryFileName, std::ifstream::in);

	geometryFileNameTemp = geometryFileName;

	if(obj_File.is_open()){
		while (std::getline(obj_File, line)){
			if (line[0] == 'v' && line[1] == ' '){
				sscanf_s(&line[0], "v %f %f %f", &x, &y, &z);
				verteciesPos.push_back({ x, y, z  });
				vertexPositions.push_back(XMFLOAT3(x, y, z)); //storar alla vertexpositioner i en separat array som sedan kan användas i main

			}

			else if (line[0] == 'v' && line[1] == 't' && line[2] == ' '){
				sscanf_s(&line[0], "vt %f %f", &u, &v);
				verteciesUV.push_back({ u, v });
			}

			else if (line[0] == 'v' && line[1] == 'n' && line[2] == ' '){
				sscanf_s(&line[0], "vn %f %f %f", &nx, &ny, &nz);
				verteciesNor.push_back({ nx, ny, nz  });
			}


			else if (line[0] == 'f' && line[1] == ' '){
				sscanf_s(&line[0], "f %d/%d/%d %d/%d/%d %d/%d/%d", &fx.x, &fx.y, &fx.z, &fy.x, &fy.y, &fy.z, &fz.x, &fz.y, &fz.z);
				faceIndecies.push_back({ fx, fy, fz });
				nrElements++;

				indecies.push_back(fx.x-1); //alla indecies i en array som kommer att användas i main
				indecies.push_back(fy.x-1); //de kommer ligga en triangle i streck i arrayen [0], [1] och [2] kommer vara indecies för en triangle alltså
				indecies.push_back(fz.x-1);
			}


		}


		for each(FaceIndex i in faceIndecies){
			Vertex tempVertex;
			tempVertex.vx = verteciesPos[i.vert1.x-1].vx;
			tempVertex.vy = verteciesPos[i.vert1.x-1].vy;
			tempVertex.vz = verteciesPos[i.vert1.x-1].vz;
		
			
			//Fill array
			SavedVertsX.push_back(tempVertex.vx);
			SavedVertsY.push_back(tempVertex.vy);
			SavedVertsZ.push_back(tempVertex.vz);
			
			tempVertex.u = verteciesUV[i.vert1.y-1].u;
			tempVertex.v = verteciesUV[i.vert1.y-1].v;

			tempVertex.nx = verteciesNor[i.vert1.z-1].nx;
			tempVertex.ny = verteciesNor[i.vert1.z-1].ny;
			tempVertex.nz = verteciesNor[i.vert1.z-1].nz;

			vertecies.push_back(tempVertex);

			tempVertex = { 0 };
			tempVertex.vx = verteciesPos[i.vert2.x-1].vx;
			tempVertex.vy = verteciesPos[i.vert2.x-1].vy;
			tempVertex.vz = verteciesPos[i.vert2.x-1].vz;

			//Fill array
			SavedVertsX.push_back(tempVertex.vx);
			SavedVertsY.push_back(tempVertex.vy);
			SavedVertsZ.push_back(tempVertex.vz);

			tempVertex.u = verteciesUV[i.vert2.y-1].u;
			tempVertex.v = verteciesUV[i.vert2.y-1].v;

			tempVertex.nx = verteciesNor[i.vert2.z-1].nx;
			tempVertex.ny = verteciesNor[i.vert2.z-1].ny;
			tempVertex.nz = verteciesNor[i.vert2.z-1].nz;

			vertecies.push_back(tempVertex);

			tempVertex = { 0 };
			tempVertex.vx = verteciesPos[i.vert3.x-1].vx;
			tempVertex.vy = verteciesPos[i.vert3.x-1].vy;
			tempVertex.vz = verteciesPos[i.vert3.x-1].vz;

			//Fill array
			SavedVertsX.push_back(tempVertex.vx);
			SavedVertsY.push_back(tempVertex.vy);
			SavedVertsZ.push_back(tempVertex.vz);


			tempVertex.u = verteciesUV[i.vert3.y-1].u;
			tempVertex.v = verteciesUV[i.vert3.y-1].v;

			tempVertex.nx = verteciesNor[i.vert3.z-1].nx;
			tempVertex.ny = verteciesNor[i.vert3.z-1].ny;
			tempVertex.nz = verteciesNor[i.vert3.z-1].nz;

			vertecies.push_back(tempVertex);
		}


		
		CreateAABB(vertecies);

		D3D11_BUFFER_DESC bDesc;
		ZeroMemory(&bDesc, sizeof(D3D11_BUFFER_DESC));
		bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bDesc.Usage = D3D11_USAGE_DEFAULT;
		bDesc.ByteWidth = sizeof(Vertex)*(vertecies.size());

	

		D3D11_BUFFER_DESC bDesc2;
		ZeroMemory(&bDesc2, sizeof(D3D11_BUFFER_DESC));
		bDesc2.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bDesc2.Usage = D3D11_USAGE_DEFAULT;
		bDesc2.ByteWidth = sizeof(int)*(faceIndecies.size());


		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = vertecies.data();//<--------
		HRESULT VertexBufferChecker = gDevice->CreateBuffer(&bDesc, &data, &vertexBuffer);

		data.pSysMem = faceIndecies.data();
		HRESULT IndexBufferChecker = gDevice->CreateBuffer(&bDesc2, &data, &indexBuffer);

		obj_File.close();
	}


	Float3 kd, ks, ka;
	
	material_File.open(materialFileName, std::ifstream::in);

	if (material_File.is_open()){
		OutputDebugStringA("\n\nMat file Opened\n\n");
		while (std::getline(material_File, line)){
			OutputDebugStringA("\nFound text\n");
			if (line[0] == 'K' && line[1] == 'd'){ //STORA ELLER SMÅ BOKSTÄVER??
				sscanf_s(&line[0], "Kd &f &f &f", &kd.x, &kd.y, &kd.z);
				OutputDebugStringA("\nKd found\n");
			}
		
			
			else if (line[0] == 'K' && line[1] == 's'){
				sscanf_s(&line[0], "Ks &f &f &f", &ks.x, &ks.y, &ks.z);
				OutputDebugStringA("\nKs found\n");
			}

			else if (line[0] == 'K' && line[1] == 'a'){
				sscanf_s(&line[0], "Ka &f &f &f", &ka.x, &ka.y, &ka.z);
				OutputDebugStringA("\nKa found\n");
			}
		}
		material.kd = kd;
		material.ks = ks;
		material.ka = ka;
		material_File.close();
	}
	
}


void OBJ::CreateAABB(std::vector<Vertex> verts){
	float minX = INFINITY, minY = INFINITY, minZ = INFINITY;
	float maxX = -INFINITY, maxY = -INFINITY, maxZ = -INFINITY;

	for each (Vertex v in verts)
	{
		minX = min(minX, v.vx);
		minY = min(minY, v.vy);
		minZ = min(minZ, v.vz);

		maxX = max(maxX, v.vx);
		maxY = max(maxY, v.vy);
		maxZ = max(maxZ, v.vz);
	}
	float cX = (maxX - minX) / 2;
	float cY = (maxY - minY) / 2;
	float cZ = (maxZ - minZ) / 2;
	XMFLOAT3 center;
	center.x = cX, center.y = cY, center.z = cZ;
	XMFLOAT3 extents;
	extents.x = cX, extents.y = cY, extents.z = cZ;
	BoundingBox b(center, extents);
	bBox = b;
}

void OBJ::CleanUp(){
	vertexBuffer->Release();
	indexBuffer->Release();
}