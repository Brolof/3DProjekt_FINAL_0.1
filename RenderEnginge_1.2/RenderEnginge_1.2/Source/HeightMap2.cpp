#define _CRT_SECURE_NO_DEPRECATE 

#include "HeightMap2.h"



heightMap2::heightMap2(){
	vertBuff = nullptr;
	indexBuff = nullptr;
	HeíghtNumFaces = 0;
	HeightNumVertices = 0;
}
bool heightMap2::HeightMapLoad(char* hMap,HeightMap2Info &hminfo)
{
	FILE *filePtr;							// Point to the current position in the file
	BITMAPFILEHEADER bitmapFileHeader;		// Structure which stores information about file
	BITMAPINFOHEADER bitmapInfoHeader;		// Structure which stores information about image
	int imageSize, index;
	unsigned char height;

	// Open the file
	filePtr = fopen(hMap, "rb");
	if (filePtr == NULL)
		return 0;

	// Read bitmaps header
	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

	// Read the info header
	fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);

	// Get the width and height (width and length) of the image
	hminfo.terrainWidth = bitmapInfoHeader.biWidth;
	hminfo.terrainHeight = bitmapInfoHeader.biHeight;

	// Size of the image in bytes. the 3 represents RBG (byte, byte, byte) for each pixel
	imageSize = hminfo.terrainWidth * hminfo.terrainHeight * 3;

	// Initialize the array which stores the image data
	unsigned char* bitmapImage = new unsigned char[imageSize];

	// Set the file pointer to the beginning of the image data
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	// Store image data in bitmapImage
	fread(bitmapImage, 1, imageSize, filePtr);

	// Close file
	fclose(filePtr);

	// Initialize the heightMap array (stores the vertices of our terrain)
	hminfo.heightMap = new XMFLOAT3[hminfo.terrainWidth * hminfo.terrainHeight];

	// We use a greyscale image, so all 3 rgb values are the same, but we only need one for the height
	// So we use this counter to skip the next two components in the image data (we read R, then skip BG)
	int k = 0;

	// We divide the height by this number to "water down" the terrains height, otherwise the terrain will
	// appear to be "spikey" and not so smooth.
	float heightFactor = 7;

	// Read the image data into our heightMap array
	for (int j = 0; j< hminfo.terrainHeight; j++)
	{
		for (int i = 0; i< hminfo.terrainWidth; i++)
		{
			height = bitmapImage[k];

			index = (hminfo.terrainHeight * j) + i;

			hminfo.heightMap[index].x = (float)i;
			hminfo.heightMap[index].y = (float)height / heightFactor;
	
			hminfo.heightMap[index].z = (float)j;
			heights[i][j] = hminfo.heightMap[index].y;
			k += 3;
		}
	}

	tempMap = hminfo;

	delete[] bitmapImage;
	bitmapImage = 0;




	return true;
}


void heightMap2::createMap(ID3D11Device* dev, std::wstring tex1, std::wstring tex2, std::wstring tex3, std::wstring splat){

	 colls = tempMap.terrainWidth;
	 rows = tempMap.terrainHeight;
	//Create the grid
	HeightNumVertices = rows * colls;
	HeíghtNumFaces = (rows - 1)*(colls - 1) * 2;

	std::vector<Vertex> v(HeightNumVertices);

	for (DWORD i = 0; i < rows; ++i)
	{
		for (DWORD j = 0; j < colls; ++j)
		{
			v[i*colls + j].pos = tempMap.heightMap[i*colls + j];

			v[i*colls + j].pos.x = v[i*colls + j].pos.x*terrainSizeMultiplier;
			v[i*colls + j].pos.z = v[i*colls + j].pos.z*terrainSizeMultiplier;

			v[i*colls + j].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			
		}
	}



	std::vector<DWORD> indices(HeíghtNumFaces * 3);


	int incrementCount,tuCount, tvCount;
	// Initialize the tu and tv coordinate indexes.
	tuCount = 0;
	tvCount = 0;
	float incrementValue;


	// Calculate how much to increment the texture coordinates by.
	incrementValue = (float)TEXTURE_REPEAT / (float)tempMap.terrainWidth;

	// Calculate how many times to repeat the texture.
	incrementCount = tempMap.terrainWidth / TEXTURE_REPEAT;

	int k = 0;
	int texUIndex = 0;
	int texVIndex = 0;
	for (DWORD i = 0; i < rows -1; i++)
	{
		for (DWORD j = 0; j < colls - 1; j++)
		{
			indices[k] = i*colls + j;		// Bottom left of quad
			v[i*colls + j].texCoord = XMFLOAT2(texUIndex + incrementValue, texVIndex + incrementValue);

			indices[k + 1] = i*colls + j + 1;		// Bottom right of quad
			v[i*colls + j + 1].texCoord = XMFLOAT2(texUIndex + incrementValue, texVIndex + incrementValue);

			indices[k + 2] = (i + 1)*colls + j;	// Top left of quad
			v[(i + 1)*colls + j].texCoord = XMFLOAT2(texUIndex + incrementValue, texVIndex + incrementValue);


			indices[k + 3] = (i + 1)*colls + j;	// Top left of quad
			v[(i + 1)*colls + j].texCoord = XMFLOAT2(texUIndex + incrementValue, texVIndex + incrementValue);
			indices[k + 4] = i*colls + j + 1;		// Bottom right of quad
			v[i*colls + j + 1].texCoord = XMFLOAT2(texUIndex + incrementValue, texVIndex + incrementValue);

			indices[k + 5] = (i + 1)*colls + j + 1;	// Top right of quad
			v[(i + 1)*colls + j + 1].texCoord = XMFLOAT2(texUIndex + incrementValue, texVIndex + incrementValue);

			k += 6; // next quad

			texUIndex++;
		}
		texUIndex = 0;
		texVIndex++;
	}	
	

	////////////////////////Compute Normals///////////////////////////
	////Now we will compute the normals for each vertex using normal averaging
	//std::vector<XMFLOAT3> tempNormal;

	////normalized and unnormalized normals
	//XMFLOAT3 unnormalized = XMFLOAT3(0.0f, 0.0f, 0.0f);

	////Used to get vectors (sides) from the position of the verts
	//float vecX, vecY, vecZ;

	////Two edges of our triangle
	//XMVECTOR edge1 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	//XMVECTOR edge2 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	////Compute face normals
	//for (int i = 0; i < HeíghtNumFaces; ++i)
	//{
	//	//Get the vector describing one edge of our triangle (edge 0,2)
	//	vecX = v[indices[(i * 3)]].pos.x - v[indices[(i * 3) + 2]].pos.x;
	//	vecY = v[indices[(i * 3)]].pos.y - v[indices[(i * 3) + 2]].pos.y;
	//	vecZ = v[indices[(i * 3)]].pos.z - v[indices[(i * 3) + 2]].pos.z;
	//	edge1 = XMVectorSet(vecX, vecY, vecZ, 0.0f);	//Create our first edge

	//	//Get the vector describing another edge of our triangle (edge 2,1)
	//	vecX = v[indices[(i * 3) + 2]].pos.x - v[indices[(i * 3) + 1]].pos.x;
	//	vecY = v[indices[(i * 3) + 2]].pos.y - v[indices[(i * 3) + 1]].pos.y;
	//	vecZ = v[indices[(i * 3) + 2]].pos.z - v[indices[(i * 3) + 1]].pos.z;
	//	edge2 = XMVectorSet(vecX, vecY, vecZ, 0.0f);	//Create our second edge

	//	//Cross multiply the two edge vectors to get the un-normalized face normal
	//	XMStoreFloat3(&unnormalized, XMVector3Cross(edge1, edge2));
	//	tempNormal.push_back(unnormalized);			//Save unormalized normal (for normal averaging)
	//}

	////Compute vertex normals (normal Averaging)
	//XMVECTOR normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	//int facesUsing = 0;
	//float tX;
	//float tY;
	//float tZ;

	////Go through each vertex
	//for (int i = 0; i < HeightNumVertices; ++i)
	//{
	//	//Check which triangles use this vertex
	//	for (int j = 0; j < HeíghtNumFaces; ++j)
	//	{
	//		if (indices[j * 3] == i ||
	//			indices[(j * 3) + 1] == i ||
	//			indices[(j * 3) + 2] == i)
	//		{
	//			tX = XMVectorGetX(normalSum) + tempNormal[j].x;
	//			tY = XMVectorGetY(normalSum) + tempNormal[j].y;
	//			tZ = XMVectorGetZ(normalSum) + tempNormal[j].z;

	//			normalSum = XMVectorSet(tX, tY, tZ, 0.0f);	//If a face is using the vertex, add the unormalized face normal to the normalSum
	//			facesUsing++;
	//		}
	//	}

	//	//Get the actual normal by dividing the normalSum by the number of faces sharing the vertex
	//	normalSum = normalSum / facesUsing;

	//	//Normalize the normalSum vector
	//	normalSum = XMVector3Normalize(normalSum);

	//	//Store the normal in our current vertex
	//	v[i].normal.x = XMVectorGetX(normalSum);
	//	v[i].normal.y = XMVectorGetY(normalSum);
	//	v[i].normal.z = XMVectorGetZ(normalSum);

	//	//Clear normalSum and facesUsing for next vertex
	//	normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	//	facesUsing = 0;
	//}

	HRESULT hr;
	D3D11_BUFFER_DESC  indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	/************************************New Stuff****************************************************/
	indexBufferDesc.ByteWidth = sizeof(DWORD)* HeíghtNumFaces * 3;
	/*************************************************************************************************/
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	/************************************New Stuff****************************************************/
	iinitData.pSysMem = &indices[0];
	/*************************************************************************************************/
	hr = dev->CreateBuffer(&indexBufferDesc, &iinitData, &indexBuff);

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	/************************************New Stuff****************************************************/
	vertexBufferDesc.ByteWidth = sizeof(Vertex)* HeightNumVertices;
	/*************************************************************************************************/
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	/************************************New Stuff****************************************************/
	vertexBufferData.pSysMem = &v[0];
	/*************************************************************************************************/
	hr = dev->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertBuff);

	// MAKE TEXTURES

	hr = CreateWICTextureFromFile(dev, tex1.c_str(), nullptr, &tex1shaderResourceView);
	if (FAILED(hr)){
		hr = CreateDDSTextureFromFile(dev, tex1.c_str(), nullptr, &tex1shaderResourceView);
	}

	hr = CreateWICTextureFromFile(dev, tex2.c_str(), nullptr, &tex2shaderResourceView);
	if (FAILED(hr)){
		hr = CreateDDSTextureFromFile(dev, tex2.c_str(), nullptr, &tex2shaderResourceView);
	}

	hr = CreateWICTextureFromFile(dev, tex3.c_str() ,nullptr, &tex3shaderResourceView);
	if (FAILED(hr)){
		hr = CreateDDSTextureFromFile(dev, tex3.c_str(), nullptr, &tex3shaderResourceView);
	}

	hr = CreateWICTextureFromFile(dev, splat.c_str(), nullptr, &splatshaderResourceView);
	if (FAILED(hr)){
		hr = CreateDDSTextureFromFile(dev, splat.c_str(), nullptr, &splatshaderResourceView);
	}
}


void heightMap2::render(ID3D11Device* dev, ID3D11DeviceContext* devcon){


};

float barryCentric(Vector3 p1, Vector3 p2, Vector3 p3, Vector2 pos) {
	float det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
	float l1 = ((p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z)) / det;
	float l2 = ((p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z)) / det;
	float l3 = 1.0f - l1 - l2;
	return l1 * p1.y + l2 * p2.y + l3 * p3.y;
};

float heightMap2::getHeightOfTerrain(float wX, float wZ){


	float terrainX = wX - hmapSize;
	float terrainZ = (wZ)-hmapSize;
	float gridSquareSize = (hmapSize / (hmapSize * hmapSize) - 1);
		int gridX = (int) floor(terrainX/ gridSquareSize);
		int gridZ = (int) floor(terrainZ/ gridSquareSize);
		gridZ = (gridZ * -1) + hmapSize;
		gridX = (gridX * -1) + hmapSize;
		if (gridX >= (hmapSize * hmapSize) - 1 || gridZ >= (hmapSize * hmapSize) - 1 || gridX < 0 || gridZ < 0){
			return 0;
		}
		float xCoord = fmod(terrainX, gridSquareSize) / gridSquareSize;
		float zCoord = fmod(terrainZ, gridSquareSize) / gridSquareSize;
	
		float answer;
		if (xCoord <= (1 - zCoord)){
			answer = barryCentric(Vector3(0, heights[gridX][gridZ], 0), Vector3(1, heights[gridX + 1][gridZ], 0), Vector3(0, heights[gridX][gridZ + 1], 1),  Vector2(xCoord, zCoord));
		}
		else {
			answer = barryCentric( Vector3(1, heights[gridX + 1][gridZ], 0),  Vector3(1, heights[gridX + 1][gridZ + 1], 1),  Vector3(0, heights[gridX][gridZ + 1], 1),  Vector2(xCoord, zCoord));
		}
		return answer;

};

