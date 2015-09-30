#ifdef _WIN32 //detta här i behövs inte under kompilering
#define _CRT_SECURE_NO_DEPRECATE //så att den inte ska klaga på tex fopen() är osäker
#endif

#include "HeightMap.h"

using namespace DirectX;

HeightMap::HeightMap(ID3D11Device* gDevice, ID3D11DeviceContext* gDeviceContext){
	this->gDevice = gDevice;
	this->gDeviceContext = gDeviceContext;
	gridSize = 1;
	heightMultiplier = 0.01f; //höjd offset typ
}

bool HeightMap::LoadSplatMap(wstring texture1, wstring texture2, wstring texture3, wstring splatMap){
	//D3D11CreateTextureFromFile(gDevice, "Texture.bmp", &g_pTexture);
	HRESULT hr;
	int nChars;
	tex1shaderResourceView = nullptr;
	tex2shaderResourceView = nullptr;
	tex3shaderResourceView = nullptr;
	splatshaderResourceView = nullptr;

	//texture 1***********************************
	//const WCHAR *textureWCHAR1; //createwictexturefromfile kräver en WCHAR* och inte en char*	
	//nChars = MultiByteToWideChar(CP_ACP, 0, texture1, -1, NULL, 0);// required size, CP_ACP används vid temporära saker, sista parametern satt till 0 ger storleken som behövs för buffern
	//textureWCHAR1 = new WCHAR[nChars];
	//MultiByteToWideChar(CP_ACP, 0, texture1, -1, (LPWSTR)textureWCHAR1, nChars);
	hr = CreateWICTextureFromFile(gDevice, texture1.c_str(), nullptr, &tex1shaderResourceView);
	if (FAILED(hr)){
		hr = CreateDDSTextureFromFile(gDevice, texture1.c_str(), nullptr, &tex1shaderResourceView);
	}
	//hr = CreateDDSTextureFromFile(gDevice, L"Grass.png", nullptr, &tex1shaderResourceView);
	if (FAILED(hr)){
		return false;
	}

	//texture 2***********************************
	/*const WCHAR *textureWCHAR2;
	nChars = MultiByteToWideChar(CP_ACP, 0, texture2, -1, NULL, 0);
	textureWCHAR2 = new WCHAR[nChars];
	MultiByteToWideChar(CP_ACP, 0, texture2, -1, (LPWSTR)textureWCHAR2, nChars);*/
	hr = CreateWICTextureFromFile(gDevice, texture2.c_str(), nullptr, &tex2shaderResourceView);
	if (FAILED(hr)){
		hr = CreateDDSTextureFromFile(gDevice, texture2.c_str(), nullptr, &tex2shaderResourceView);
	}
	if (FAILED(hr)){
		return false;
	}

	//texture 3***********************************
	/*const WCHAR *textureWCHAR3;
	nChars = MultiByteToWideChar(CP_ACP, 0, texture3, -1, NULL, 0);
	textureWCHAR3 = new WCHAR[nChars];
	MultiByteToWideChar(CP_ACP, 0, texture3, -1, (LPWSTR)textureWCHAR3, nChars);*/
	hr = CreateWICTextureFromFile(gDevice, texture3.c_str(), nullptr, &tex3shaderResourceView);
	if (FAILED(hr)){
		hr = CreateDDSTextureFromFile(gDevice, texture3.c_str(), nullptr, &tex3shaderResourceView);
	}
	if (FAILED(hr)){
		return false;
	}

	//splatmap***********************************
	/*const WCHAR *textureWCHARsplat;
	nChars = MultiByteToWideChar(CP_ACP, 0, splatMap, -1, NULL, 0);
	textureWCHARsplat = new WCHAR[nChars];
	MultiByteToWideChar(CP_ACP, 0, splatMap, -1, (LPWSTR)textureWCHARsplat, nChars);*/
	hr = CreateWICTextureFromFile(gDevice, splatMap.c_str(), nullptr, &splatshaderResourceView);
	if (FAILED(hr)){
		hr = CreateDDSTextureFromFile(gDevice, splatMap.c_str(), nullptr, &splatshaderResourceView);
	}
	if (FAILED(hr)){
		return false;
	}

	this->tex1shaderResourceView = tex1shaderResourceView;
	this->tex2shaderResourceView = tex2shaderResourceView;
	this->tex3shaderResourceView = tex3shaderResourceView;
	this->splatshaderResourceView = splatshaderResourceView;

	//delete[] textureWCHAR1;
	//delete[] textureWCHAR2;
	//delete[] textureWCHAR3;
	//delete[] textureWCHARsplat;
	return true;
}

bool HeightMap::LoadHeightMap(char* fileName){ //MAPPEN MÅSTE VARA LIKA STOR PÅ BÅDA HÅLL!!!!!!
	//8 bits bild!! kan gå mellan 0-255, char är 1 byte, alltså 8 bit! unsgined char går mellan 0-255 MINST

	BITMAPFILEHEADER bitmapFH; //information about file
	BITMAPINFOHEADER bitmapIH; //information about image

	HeightMapInfo hmI;

	FILE *pFile; //förflytta sig i filen, pekar på den aktuella positionen

	pFile = fopen(fileName, "rb"); //mode: r - read, b - binary code
	if (pFile == NULL)
		return false;

	//fread - reads from a stream, (buffer, size, maxnrofitems, filestream)
	fread(&bitmapFH, sizeof(BITMAPFILEHEADER), 1, pFile); //fileheadern håller information om själva filen
	fread(&bitmapIH, sizeof(BITMAPINFOHEADER), 1, pFile); //infoheadern håller information om själva mapen, alltså texturen

	hmI.terrainWidth = bitmapIH.biWidth;
	hmI.terrainHeight = bitmapIH.biHeight;

	//int stride = hmI.terrainWidth*bitmapIH.biBitCount;  // bits per row
	//stride += 31;            // round up to next 32-bit boundary
	//stride /= 32;            // DWORDs per row
	//stride *= 4;
	//int stridePerRow = stride / hmI.terrainHeight;

	DWORD imageSize = hmI.terrainWidth * hmI.terrainHeight * 3; //rgb (*3) 24 bitars så alltså tre 3 byte per pixel
	//int imageSize = bitmapIH.biSize;
	heights = new unsigned char[imageSize]; //denna kommer hålla arrayen av chars kommer innehålla datan från bilden, 0-255 eller mer

	int facesCount = (hmI.terrainWidth - 1) * (hmI.terrainHeight - 1) * 2;//*2 pga att man får antal quads, men vill ha i tris

	//fseek(pFile, bitmapFH.bfOffBits, SEEK_SET); //placerar pFile i början på filen, fseek används för att flytta runt pekare i filer så man kan läsa därifrån
	////SEEK_SET = Beginning of file, offset från beginning of file : bitmapFH.bfOffBits.    bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	//fread(heights, 1, imageSize, pFile); //store:a alla värden i heights
	fseek(pFile, bitmapFH.bfOffBits, SEEK_SET); //seek_set är början av filen, sen säger bitmapFH.bfOffBits hur stora headersen är, så den hoppas över dem
	//fseek(pFile, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), SEEK_SET);
	// Read in the bitmap image data.
	fread(heights, 1, imageSize, pFile); //extra bytes per rad

	fclose(pFile);

	
	vertexHeightsArray.resize((hmI.terrainHeight * hmI.terrainWidth)); //den man går på!


	//per vertex
	heightElements = hmI.terrainWidth;

	int offsetColors = 0; //används för att kunna hoppa över GB

	widthWhole = (int)hmI.terrainWidth, heightWhole = (int)hmI.terrainHeight; //terrain values in nr verts
	for (int h = 0; h < hmI.terrainHeight; h++){ //kanske inte -1
		for (int w = 0; w < hmI.terrainWidth; w++){
			Vertex tempV;
			tempV.x = w * gridSize;
			tempV.y = (float)heights[offsetColors];
			tempV.y = tempV.y * heightMultiplier; //höjden			
			vertexHeightsArray[h * widthWhole + w] = tempV.y * heightMultiplier * 1000000;//fyller denna med alla höjd värden för att sedan användas när man går på terrängen
			tempV.z = -h * gridSize; //minus becuz LH och RH

			tempV.ny = 1;

			vertecies.push_back(tempV);
			offsetColors = offsetColors + 3;
			//if (((offsetColors/3)%(hmI.terrainWidth+stridePerRow)) == 0) //ignoring stride, fel
			//	offsetColors = offsetColors + 3;
		}
	}

	std::vector<int> indecies; //unsigned long
	//indecies.resize((hmI.terrainHeight - 1) * (hmI.terrainWidth - 1) * 6);
	indecies.resize((vertecies.size() * 5)); //6???

	//per face
	int inIndex = 0; //index för att lägga in värden i indecies
	nrElements = 0;

	int UIndex = 0;
	int VIndex = 0;

	//int terWidth = hmI.terrainWidth;
	//int terHeight = hmI.terrainHeight;

	for (int h = 0; h < hmI.terrainHeight - 1; h++){
		for (int w = 0; w < hmI.terrainWidth - 1; w++){

			int start = (h * widthWhole) + w;

			//top left
			indecies[inIndex] = start;
			vertecies[start].u = UIndex;
			vertecies[start].v = VIndex;
			/*vertecies[start].alphaU = (float)UIndex / terWidth;
			vertecies[start].alphaV = (float)VIndex / terHeight;*/
			inIndex++;
			nrElements++;
			//top right
			indecies[inIndex] = start + 1;
			vertecies[start + 1].u = UIndex + 1;
			vertecies[start + 1].v = VIndex;
			/*vertecies[start + 1].alphaU = (float)UIndex + 1 / terWidth;
			vertecies[start + 1].alphaV = (float)VIndex / terHeight;*/
			inIndex++;
			nrElements++;
			//bottom left
			indecies[inIndex] = start + widthWhole;
			vertecies[start + widthWhole].u = UIndex;
			vertecies[start + widthWhole].v = VIndex + 1;
			/*vertecies[start + widthWhole].alphaU = (float)UIndex / terWidth;
			vertecies[start + widthWhole].alphaV = (float)VIndex + 1 / terHeight;*/
			inIndex++;
			nrElements++;
			//bottom right
			indecies[inIndex] = start + 1 + widthWhole;
			vertecies[start + 1 + widthWhole].u = UIndex + 1;
			vertecies[start + 1 + widthWhole].v = VIndex + 1;
			/*vertecies[start + 1 + widthWhole].alphaU = (float)UIndex + 1 / terWidth;
			vertecies[start + 1 + widthWhole].alphaV = (float)VIndex + 1 / terHeight;*/
			inIndex++;
			nrElements++;
			//bottom left
			indecies[inIndex] = start + widthWhole;
			vertecies[start + widthWhole].u = UIndex;
			vertecies[start + widthWhole].v = VIndex + 1;
			/*vertecies[start + widthWhole].alphaU = (float)UIndex / terWidth;
			vertecies[start + widthWhole].alphaV = (float)VIndex + 1 / terHeight;*/
			inIndex++;
			nrElements++;
			//top right
			indecies[inIndex] = start + 1;
			vertecies[start + 1].u = UIndex + 1;
			vertecies[start + 1].v = VIndex;
			/*vertecies[start + 1].alphaU = (float)UIndex + 1 / terWidth;
			vertecies[start + 1].alphaV = (float)VIndex / terHeight;*/
			inIndex++;
			nrElements++;

			UIndex++;
		}
		VIndex++;
		UIndex = 0;
	}
	//edge1
	Float3 pos1;
	Float3 pos2;
	//edge2
	Float3 pos3;
	Float3 pos4;

	Float3 vec1;
	Float3 vec2;

	DirectX::XMVECTOR edge1;
	DirectX::XMVECTOR edge2;

	std::vector<DirectX::XMFLOAT3> normals;
	DirectX::XMFLOAT3 normal;

	for (int i = 0; i < indecies.size() - 2; i++){ //loopar igenom alla indexes och beräknar normalen genom att hämta dem vertiser som triangeln bildar normal med
		pos1.x = vertecies[indecies[i]].x;
		pos1.y = vertecies[indecies[i]].y;
		pos1.z = vertecies[indecies[i]].z;

		pos2.x = vertecies[indecies[i + 1]].x;
		pos2.y = vertecies[indecies[i + 1]].y;
		pos2.z = vertecies[indecies[i + 1]].z;

		pos3.x = vertecies[indecies[indecies[i]]].x;
		pos3.y = vertecies[indecies[indecies[i]]].y;
		pos3.z = vertecies[indecies[indecies[i]]].z;

		pos4.x = vertecies[indecies[i + 2]].x;
		pos4.y = vertecies[indecies[i + 2]].y;
		pos4.z = vertecies[indecies[i + 2]].z;

		vec1 = pos2.Float3Subtract(pos1);
		vec2 = pos4.Float3Subtract(pos3);

		edge1 = DirectX::XMVectorSet(vec1.x, vec1.y, vec1.z, 0.0f);
		edge2 = DirectX::XMVectorSet(vec2.x, vec2.y, vec2.z, 0.0f);
		DirectX::XMStoreFloat3(&normal, (DirectX::XMVector3Cross(edge1, edge2)));
		normals.push_back(normal);
	}

	//loopa igenom alla vertiser och kolla vilka plan den binds samman med
	DirectX::XMVECTOR sumNormals = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	int vertexCount = hmI.terrainHeight * hmI.terrainWidth;
	int vertexNumFacesConnected = 0;
	Float3 tempValues;
	tempValues.x = 0;
	tempValues.y = 0;
	tempValues.z = 0;
	for (int i = 0; i < vertexCount; i++){ //loopa igenom alla vertiser
		for (int tri = 0; tri < facesCount; tri++){
			try{
				if (indecies[tri * 3] == i || indecies[(tri * 3) + 1] == i || indecies[(tri * 3) + 2] == i){ //kollar ifall vertisen ligger i denna triangeln
					tempValues.x = XMVectorGetX(sumNormals) + tempValues.x;
					tempValues.y = XMVectorGetY(sumNormals) + tempValues.y;
					tempValues.z = XMVectorGetZ(sumNormals) + tempValues.z;

					sumNormals = DirectX::XMVectorSet(tempValues.x, tempValues.y, tempValues.z, 0.0f);
					vertexNumFacesConnected++;
				}
			}
			catch (...){
				OutputDebugStringA("Out of index in heightmap");
			}
		}
		//sumNormals = sumNormals / vertexNumFacesConnected; //får average värdet
		sumNormals = XMVectorSet(XMVectorGetX(sumNormals) / vertexNumFacesConnected, XMVectorGetY(sumNormals) / vertexNumFacesConnected, XMVectorGetZ(sumNormals) / vertexNumFacesConnected, 0.0f); //får average värdet
		sumNormals = XMVector3Normalize(sumNormals);
		vertecies[i].nx = XMVectorGetX(sumNormals);
		vertecies[i].ny = XMVectorGetY(sumNormals);
		vertecies[i].nz = XMVectorGetZ(sumNormals);

		sumNormals = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		vertexNumFacesConnected = 0;
	}

	D3D11_BUFFER_DESC bDesc;
	SecureZeroMemory(&bDesc, sizeof(D3D11_BUFFER_DESC));
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.Usage = D3D11_USAGE_DEFAULT;
	bDesc.ByteWidth = sizeof(Vertex)* vertecies.size();

	D3D11_BUFFER_DESC bDesc2;
	SecureZeroMemory(&bDesc2, sizeof(D3D11_BUFFER_DESC));
	bDesc2.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc2.Usage = D3D11_USAGE_DEFAULT;
	bDesc2.ByteWidth = sizeof(int)* indecies.size();

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = vertecies.data();//<--------
	gDevice->CreateBuffer(&bDesc, &data, &vertexBuffer);

	data.pSysMem = indecies.data();//<--------
	gDevice->CreateBuffer(&bDesc2, &data, &indexBuffer);


	delete[] heights;
	return true;

}

void HeightMap::GetHeightOnPosition(float x, float z, float& y){
	//x /= (float)gridSize, z /= (float)gridSize;
	z = -1 * z;
	
	

	if (x < vertecies.at(0).x || x > vertecies.at(vertecies.size() - 1).x)
		return;
	if (z < vertecies.at(0).z || z > -(vertecies.at(vertecies.size() - 1).z))
		return;
	
	float xPos = x / gridSize * widthWhole;
	float zPos = z / gridSize * heightWhole;

	int xPosI = (int)(floorf(xPos));
	int zPosI = (int)(floorf(zPos));

	if ((zPosI * widthWhole + xPosI) > 60)
		int mojs = 0;

	float quadHeight1 = vertexHeightsArray[zPosI * widthWhole + xPosI];
	y = quadHeight1 + 0.2f;
	return;
	//float quadHeight2 = vertexHeightsArray[zPos * widthWhole + (xPos + 1)];
	//float quadHeight3 = vertexHeightsArray[(zPos + 1) * widthWhole + xPos];
	//float quadHeight4 = vertexHeightsArray[(zPos + 1) * widthWhole + (xPos + 1)];


	//y = (quadHeight1 * (1 - x) + quadHeight2 * x) * (1 - z) + (quadHeight3 * (1 - x) + quadHeight4 * x) * z;
	y += 0.2f;
	
}