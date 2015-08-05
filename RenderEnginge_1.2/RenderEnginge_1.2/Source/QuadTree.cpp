#include "QuadTree.h"

QuadTree::QuadTree(vector<GameObjects> objectsInScene, int nrSplits, ID3D11Device* gDevice, XMFLOAT3 boxSize){
	this->objectsInScene = objectsInScene;

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