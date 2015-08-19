#include "QuadTree.h"

QuadTree::QuadTree(vector<GameObjects*> objectsInScene, int nrSplits, ID3D11Device* gDevice, XMFLOAT3 boxSize){
	this->objectsInScene = objectsInScene;
	this->nrSplits = nrSplits;

	CreateQuadTree(nrSplits, XMFLOAT3(0, 0, 0), XMFLOAT3(boxSize.x, boxSize.y, boxSize.z), gDevice);
}

void QuadTree::CreateQuadTree(int nrSplits, XMFLOAT3 center, XMFLOAT3 extents, ID3D11Device* gDevice){

	BoundingBox b(center, extents);

	QuadTreeInstance *lB = new QuadTreeInstance();
	lB->CreateBBoxVBuffer(b, gDevice);
	lB->splitIndex = nrSplits;
	if (nrSplits <= 0) //om det är längst ner i trädet så innebär det att denne ska innehålla gameobjects (förutsatt att det finns några där :- ))
		lB->TestContains(objectsInScene); //testa endast mot de längst ner ifall det finns några gameobjects i dem

	quadTreeBranches.push_back(lB);
	//quads.push_back(lB);

	BoundingBox thisBranchren[4];

	thisBranchren[0].Center = XMFLOAT3(center.x + -extents.x * 0.5f, center.y, center.z + -extents.z * 0.5);
	thisBranchren[1].Center = XMFLOAT3(center.x + extents.x * 0.5f, center.y, center.z + -extents.z * 0.5f);
	thisBranchren[2].Center = XMFLOAT3(center.x + -extents.x * 0.5f, center.y, center.z + extents.z *0.5f);
	thisBranchren[3].Center = XMFLOAT3(center.x + extents.x * 0.5f, center.y, center.z + extents.z * 0.5f);

	thisBranchren[0].Extents = XMFLOAT3(extents.x / 2, extents.y, extents.z / 2);
	thisBranchren[1].Extents = XMFLOAT3(extents.x / 2, extents.y, extents.z / 2);
	thisBranchren[2].Extents = XMFLOAT3(extents.x / 2, extents.y, extents.z / 2);
	thisBranchren[3].Extents = XMFLOAT3(extents.x / 2, extents.y, extents.z / 2);


	int newNrSplits = nrSplits - 1;
	if (nrSplits > 0){
		for each (BoundingBox thisBranch in thisBranchren)
		{
			CreateQuadTree(newNrSplits, thisBranch.Center, thisBranch.Extents, gDevice, lB);
		}
	}
}

void QuadTree::CreateQuadTree(int nrSplits, XMFLOAT3 center, XMFLOAT3 extents, ID3D11Device* gDevice, QuadTreeInstance *thisBranch){

	BoundingBox b(center, extents);

	QuadTreeInstance *lB = new QuadTreeInstance();
	lB->CreateBBoxVBuffer(b, gDevice);
	lB->splitIndex = nrSplits;
	if (nrSplits <= 0) //om det är längst ner i trädet så innebär det att denne ska innehålla gameobjects (förutsatt att det finns några där :- ))
		lB->TestContains(objectsInScene); //testa endast mot de längst ner ifall det finns några gameobjects i dem

	thisBranch->children.push_back(lB); //släng in detta barnet i thisBranchen
	quadTreeBranches.push_back(lB);
	//quads.push_back(lB);

	BoundingBox thisBranchren[4];
	
	thisBranchren[0].Center = XMFLOAT3(center.x + -extents.x * 0.5f, center.y, center.z + -extents.z * 0.5);
	thisBranchren[1].Center = XMFLOAT3(center.x + extents.x * 0.5f, center.y, center.z + -extents.z * 0.5f);
	thisBranchren[2].Center = XMFLOAT3(center.x + -extents.x * 0.5f, center.y, center.z + extents.z *0.5f);
	thisBranchren[3].Center = XMFLOAT3(center.x + extents.x * 0.5f, center.y, center.z + extents.z * 0.5f);

	thisBranchren[0].Extents = XMFLOAT3(extents.x / 2, extents.y, extents.z / 2);
	thisBranchren[1].Extents = XMFLOAT3(extents.x / 2, extents.y, extents.z / 2);
	thisBranchren[2].Extents = XMFLOAT3(extents.x / 2, extents.y, extents.z / 2);
	thisBranchren[3].Extents = XMFLOAT3(extents.x / 2, extents.y, extents.z / 2);
	

	int newNrSplits = nrSplits - 1;
	if (nrSplits > 0){
		for each (BoundingBox thisBranch in thisBranchren)
		{
			CreateQuadTree(newNrSplits, thisBranch.Center, thisBranch.Extents, gDevice, lB);
		}
	}
}

void QuadTree::StartFrustumTest(XMMATRIX proj, XMMATRIX view){
	//frustumCheckIndex = 0;
	for (int i = 0; i < quadTreeBranches.size(); i++){
		quadTreeBranches[i]->isInFrustum = false;
	}

	for (int i = 0; i < objectsInScene.size(); i++){
		if (objectsInScene[i]->GetStatic() == true){
			objectsInScene[i]->SetActive(false);
			objectsInScene[i]->visibleThisFrame = false;
		}
	}
	CheckFrustumCollision(nrSplits, 0, proj, view);
}

//void QuadTree::CheckFrustumCollision(int nrSplits, int splitIndex, XMMATRIX proj, XMMATRIX view){
//	BoundingFrustum f;
//	f.CreateFromMatrix(f, proj);
//	f.Transform(f, view);
//
//	frustum = f;
//
//	ContainmentType test = frustum.Contains(quadTreeBranches[splitIndex]->bbox);
//	if (test == 2 || test == 1){ //hit på boxen, contains ELLER intersects	
//		quadTreeBranches[splitIndex]->isInFrustum = true;
//		if (nrSplits > 1){ //ska nog vara 0
//			int newNrSplits = nrSplits - 1; //vilket steg av branches man är på
//			//frustumCheckIndex += 4;
//			for (int i = 0; i < 4; i++){
//				if (i > 0)
//					splitIndex += 4 * newNrSplits; //??
//				else
//					splitIndex += 1;
//				CheckFrustumCollision(newNrSplits, splitIndex, proj, view);
//			}
//		}
//		else{ //botten på trädet
//			int objectIndex = 0;
//			for (int y = 0; y < quadTreeBranches[splitIndex]->gameObjectsToRender.size(); y++)//vad skall göras ifall den ena boxen säger att objektet ska renderas men den andra inte?
//			{
//				test = frustum.Contains(quadTreeBranches[splitIndex]->gameObjectsToRender[y].bbox);
//				if (test == 2 || test == 1){ //hit på objektets box
//					quadTreeBranches[splitIndex]->gameObjectsToRender[y].visibleThisFrame = true;
//					quadTreeBranches[splitIndex]->gameObjectsToRender[y].SetActive(true);
//				}
//				else if (quadTreeBranches[splitIndex]->gameObjectsToRender[y].visibleThisFrame == false){ //om ingen annan box har sagt att denna ska renderas denna framen
//
//					quadTreeBranches[splitIndex]->gameObjectsToRender[y].SetActive(false);
//
//				}
//				objectIndex++;
//			}
//		}
//	}
//	else
//		quadTreeBranches[splitIndex]->isInFrustum = false;
//
//
//
//}

void QuadTree::CheckFrustumCollision(int nrSplits, int splitIndex, XMMATRIX proj, XMMATRIX view){
	BoundingFrustum f;
	f.CreateFromMatrix(f, proj);
	f.Transform(f, view);

	frustum = f;

	ContainmentType test = frustum.Contains(quadTreeBranches[0]->bbox);
	if (test == 2 || test == 1){ //hit på boxen, contains ELLER intersects	
		quadTreeBranches[0]->isInFrustum = true;
		if (nrSplits > 1){ //ska nog vara 0
			int newNrSplits = nrSplits - 1; //vilket steg av branches man är på
			//frustumCheckIndex += 4;
			for (int i = 0; i < 4; i++){
				CheckFrustumCollision(newNrSplits, splitIndex, proj, view, quadTreeBranches[0]->children[i]);
			}
		}
		else{ //botten på trädet			
			for (int y = 0; y < quadTreeBranches[0]->gameObjectsToRender.size(); y++)//vad skall göras ifall den ena boxen säger att objektet ska renderas men den andra inte?
			{
				test = frustum.Contains(quadTreeBranches[0]->gameObjectsToRender[y]->bbox);
				if (test == 2 || test == 1){ //hit på objektets box
					quadTreeBranches[0]->gameObjectsToRender[y]->visibleThisFrame = true;
					quadTreeBranches[0]->gameObjectsToRender[y]->SetActive(true);
				}
				else if (quadTreeBranches[0]->gameObjectsToRender[y]->visibleThisFrame == false){ //om ingen annan box har sagt att denna ska renderas denna framen
					
					quadTreeBranches[0]->gameObjectsToRender[y]->SetActive(false);
					
				}				
			}
		}
	}
	else
		quadTreeBranches[splitIndex]->isInFrustum = false;


	
}

void QuadTree::CheckFrustumCollision(int nrSplits, int splitIndex, XMMATRIX proj, XMMATRIX view, QuadTreeInstance *thisBranch){
	BoundingFrustum f;
	f.CreateFromMatrix(f, proj);
	f.Transform(f, view);

	frustum = f;
	
	ContainmentType test = frustum.Contains(thisBranch->bbox);
	if (test == 2 || test == 1){ //hit på boxen, contains ELLER intersects	
		thisBranch->isInFrustum = true;
		if (nrSplits > 0){ //ska nog vara 0
			int newNrSplits = nrSplits - 1; //vilket steg av branches man är på
			//frustumCheckIndex += 4;
			for (int i = 0; i < 4; i++){				
				CheckFrustumCollision(newNrSplits, splitIndex, proj, view, thisBranch->children[i]);
			}
		}
		else{ //botten på trädet			
			for (int y = 0; y < thisBranch->gameObjectsToRender.size(); y++)//vad skall göras ifall den ena boxen säger att objektet ska renderas men den andra inte?
			{
				test = frustum.Contains(thisBranch->gameObjectsToRender[y]->bbox);
				//if (test == 2 || test == 1){ //hit på objektets box
					thisBranch->gameObjectsToRender[y]->visibleThisFrame = true;
					thisBranch->gameObjectsToRender[y]->SetActive(true);
				//}
				//else if (thisBranch->gameObjectsToRender[y].visibleThisFrame == false){ //om ingen annan box har sagt att denna ska renderas denna framen

					//thisBranch->gameObjectsToRender[y].SetActive(false);

				//}
				
			}
		}
	}
	else
		thisBranch->isInFrustum = false;



}