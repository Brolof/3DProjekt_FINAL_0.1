#include "GameObject.h"
#include <vector>

#pragma once

class QuadTree{
public:
	struct Float3{
		float x, y, z;
		Float3(float x, float y, float z){
			this->x = x;
			this->y = y;
			this->z = z;
		}

	};
	struct QuadTreeInstance{
		QuadTreeInstance(){}
		~QuadTreeInstance(){
			boxBuffer->Release();
			/*for (int i = 0; i < children.size; i++){
				delete children[i];
			}*/
			children.clear();
		}

		vector<GameObjects*> gameObjectsToRender;
		int splitIndex;
		//QuadTreeInstance *children = nullptr;
		vector<QuadTreeInstance*> children;
		//QuadTreeInstance children[4];
		//vector<QuadTreeInstance> children;
		BoundingBox bbox;
		ID3D11Buffer *boxBuffer = nullptr;		
		void CreateBBoxVBuffer(BoundingBox b, ID3D11Device* gDevice){
			bbox = b;
			std::vector<Float3> boxVertPoints;

			boxVertPoints.push_back(Float3(bbox.Center.x - bbox.Extents.x, bbox.Center.y - bbox.Extents.y, bbox.Center.z - bbox.Extents.z)); //0,0,0
			boxVertPoints.push_back(Float3(bbox.Center.x + bbox.Extents.x, bbox.Center.y - bbox.Extents.y, bbox.Center.z - bbox.Extents.z)); //1,0,0
			boxVertPoints.push_back(Float3(bbox.Center.x + bbox.Extents.x, bbox.Center.y + bbox.Extents.y, bbox.Center.z - bbox.Extents.z)); //1,1,0
			boxVertPoints.push_back(Float3(bbox.Center.x - bbox.Extents.x, bbox.Center.y + bbox.Extents.y, bbox.Center.z - bbox.Extents.z)); //0,1,0
			boxVertPoints.push_back(Float3(bbox.Center.x - bbox.Extents.x, bbox.Center.y - bbox.Extents.y, bbox.Center.z - bbox.Extents.z)); //0,0,0

			boxVertPoints.push_back(Float3(bbox.Center.x - bbox.Extents.x, bbox.Center.y - bbox.Extents.y, bbox.Center.z + bbox.Extents.z)); //0,0,1
			boxVertPoints.push_back(Float3(bbox.Center.x + bbox.Extents.x, bbox.Center.y - bbox.Extents.y, bbox.Center.z + bbox.Extents.z)); //1,0,1
			boxVertPoints.push_back(Float3(bbox.Center.x + bbox.Extents.x, bbox.Center.y + bbox.Extents.y, bbox.Center.z + bbox.Extents.z)); //1,1,1
			boxVertPoints.push_back(Float3(bbox.Center.x - bbox.Extents.x, bbox.Center.y + bbox.Extents.y, bbox.Center.z + bbox.Extents.z)); //0,1,1
			boxVertPoints.push_back(Float3(bbox.Center.x - bbox.Extents.x, bbox.Center.y - bbox.Extents.y, bbox.Center.z + bbox.Extents.z)); //0,0,1

			boxVertPoints.push_back(Float3(bbox.Center.x + bbox.Extents.x, bbox.Center.y - bbox.Extents.y, bbox.Center.z + bbox.Extents.z)); //1,0,1
			boxVertPoints.push_back(Float3(bbox.Center.x + bbox.Extents.x, bbox.Center.y - bbox.Extents.y, bbox.Center.z - bbox.Extents.z)); //1,0,0
			boxVertPoints.push_back(Float3(bbox.Center.x + bbox.Extents.x, bbox.Center.y + bbox.Extents.y, bbox.Center.z - bbox.Extents.z)); //1,1,0
			boxVertPoints.push_back(Float3(bbox.Center.x + bbox.Extents.x, bbox.Center.y + bbox.Extents.y, bbox.Center.z + bbox.Extents.z)); //1,1,1
			boxVertPoints.push_back(Float3(bbox.Center.x - bbox.Extents.x, bbox.Center.y + bbox.Extents.y, bbox.Center.z + bbox.Extents.z)); //0,1,1
			boxVertPoints.push_back(Float3(bbox.Center.x - bbox.Extents.x, bbox.Center.y + bbox.Extents.y, bbox.Center.z - bbox.Extents.z)); //0,1,0


			D3D11_BUFFER_DESC bDesc;
			ZeroMemory(&bDesc, sizeof(D3D11_BUFFER_DESC));
			bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bDesc.Usage = D3D11_USAGE_DEFAULT;
			bDesc.ByteWidth = sizeof(Float3) * (boxVertPoints.size());

			D3D11_SUBRESOURCE_DATA data;
			data.pSysMem = boxVertPoints.data();//<--------
			HRESULT VertexBufferChecker = gDevice->CreateBuffer(&bDesc, &data, &boxBuffer);
		}
		void TestContains(vector<GameObjects*> gameObjectsPossibleHit){ //skicka in alla gameobjects i världen, (gameObjectsInWorldSpace)

		for (int i = 0; i < gameObjectsPossibleHit.size(); i++){
			if (gameObjectsPossibleHit[i]->GetStatic() == true){
				ContainmentType test = bbox.Contains(gameObjectsPossibleHit[i]->bbox);
				if (test == 2 || test == 1){ //1 = intersects, 2 = contains, testa ifall nått gameobject ligger i just denna boxen, om den gör det så lägg till den
					gameObjectsToRender.push_back(gameObjectsPossibleHit[i]);
				}
			}
		}
		}

		bool isInFrustum; //för test
	};

	vector<GameObjects*> objectsInScene; //skickar in alla object hit
	vector<QuadTreeInstance*> quadTreeBranches; //alla uppdelningar
	int nrSplits; //hur många uppdelningar
	BoundingFrustum frustum;	

	QuadTree(){}
	QuadTree(vector<GameObjects*> objectsInScene, int nrSplits, ID3D11Device* gDevice, XMFLOAT3 boxSize);
	~QuadTree(){
		for (int i = 0; i < quadTreeBranches.size(); i++){
			delete quadTreeBranches[i];
		}
		quadTreeBranches.clear();

	}

	void CreateQuadTree(int nrSplits, XMFLOAT3 center, XMFLOAT3 extents, ID3D11Device* gDevice);
	void CreateQuadTree(int nrSplits, XMFLOAT3 center, XMFLOAT3 extents, ID3D11Device* gDevice, QuadTreeInstance *parent);

	//int frustumCheckIndex;
	void StartFrustumTest(XMMATRIX proj, XMMATRIX view);
	void CheckFrustumCollision(int nrSplits, XMMATRIX proj, XMMATRIX view); //kollar vilka branches respektive objekt som befinner sig i frustumet, sätter objekten till active = true/false
	void CheckFrustumCollision(int nrSplits, QuadTreeInstance *parent);

private:

};