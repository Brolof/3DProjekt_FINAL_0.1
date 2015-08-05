#include "QuadTree.h"

QuadTree::QuadTree(vector<GameObjects*> objectsInScene, int nrSplits, ID3D11Device* gDevice, XMFLOAT3 boxSize){
	this->objectsInScene = objectsInScene;
	this->nrSplits = nrSplits;

	CreateQuadTree(nrSplits, XMFLOAT3(0, 0, 0), XMFLOAT3(boxSize.x, boxSize.y, boxSize.z), gDevice);
}

void QuadTree::CreateQuadTree(int nrSplits, XMFLOAT3 center, XMFLOAT3 extents, ID3D11Device* gDevice){

	BoundingBox b(center, extents);

	QuadTreeInstance lB;
	lB.CreateBBoxVBuffer(b, gDevice);
	if (nrSplits <= 0) //om det är längst ner i trädet så innebär det att denne ska innehålla gameobjects (förutsatt att det finns några där :- ))
		lB.TestContains(objectsInScene); //testa endast mot de längst ner ifall det finns några gameobjects i dem

	quadTreeBranches.push_back(lB);
	//quads.push_back(lB);

	BoundingBox children[4];

	if (nrSplits > 0){
		//CENTER ÄR JU 0!!!? FIXA DETTA
		children[0].Center = XMFLOAT3(center.x + -extents.x * 0.5f, center.y, center.z + -extents.z * 0.5);
		children[1].Center = XMFLOAT3(center.x + extents.x * 0.5f, center.y, center.z + -extents.z * 0.5f);
		children[2].Center = XMFLOAT3(center.x + -extents.x * 0.5f, center.y, center.z + extents.z *0.5f);
		children[3].Center = XMFLOAT3(center.x + extents.x * 0.5f, center.y, center.z + extents.z * 0.5f);

		children[0].Extents = XMFLOAT3(extents.x / 2, extents.y, extents.z / 2);
		children[1].Extents = XMFLOAT3(extents.x / 2, extents.y, extents.z / 2);
		children[2].Extents = XMFLOAT3(extents.x / 2, extents.y, extents.z / 2);
		children[3].Extents = XMFLOAT3(extents.x / 2, extents.y, extents.z / 2);
	}

	int newNrSplits = nrSplits - 1;
	if (nrSplits > 0){
		for each (BoundingBox child in children)
		{
			CreateQuadTree(newNrSplits, child.Center, child.Extents, gDevice);
		}
	}
}

void QuadTree::StartFrustumTest(XMMATRIX proj, XMMATRIX view){
	//frustumCheckIndex = 0;
	for (int i = 0; i < quadTreeBranches.size(); i++){
		quadTreeBranches[i].isInFrustum = false;
	}
	CheckFrustumCollision(nrSplits, 0, proj, view);
}

void QuadTree::CheckFrustumCollision(int nrSplits, int splitIndex, XMMATRIX proj, XMMATRIX view){
	BoundingFrustum f;
	f.CreateFromMatrix(f, proj);
	f.Transform(f, view);

	frustum = f;
	
	ContainmentType test = frustum.Contains(quadTreeBranches[splitIndex].bbox);
	if (test == 2 || test == 1){ //hit på boxen, contains ELLER intersects	
		quadTreeBranches[splitIndex].isInFrustum = true;
		if (nrSplits > 0){
			int newNrSplits = nrSplits - 1; //vilket steg av branches man är på
			//frustumCheckIndex += 4;
			for (int i = 0; i < 4; i++){
				if (i > 0)
					splitIndex += 4 * newNrSplits; //??
				else
					splitIndex += 1;
				CheckFrustumCollision(newNrSplits, splitIndex, proj, view);
			}
		}
		else{ //botten på trädet
			int objectIndex = 0;
			for (int y = 0; y < quadTreeBranches[splitIndex].gameObjectsToRender.size(); y++)//vad skall göras ifall den ena boxen säger att objektet ska renderas men den andra inte?
			{
				test = frustum.Contains(quadTreeBranches[splitIndex].gameObjectsToRender[y].bbox);
				if (test == 2 || test == 1){ //hit på objektets box
					quadTreeBranches[splitIndex].gameObjectsToRender[y].visibleThisFrame = true;
					quadTreeBranches[splitIndex].gameObjectsToRender[y].SetActive(true);
				}
				else if (quadTreeBranches[splitIndex].gameObjectsToRender[y].visibleThisFrame == false){ //om ingen annan box har sagt att denna ska renderas denna framen
					
					quadTreeBranches[splitIndex].gameObjectsToRender[y].SetActive(false);
					
				}
				objectIndex++;
			}
		}
	}
	else
		quadTreeBranches[splitIndex].isInFrustum = false;


	
}