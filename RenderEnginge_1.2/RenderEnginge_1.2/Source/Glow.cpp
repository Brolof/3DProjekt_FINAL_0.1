#include "Glow.h"

void Glow::CreateViewPort(){
	//D3D11_VIEWPORT glowViewPort;
	glowViewPort.Width = screen_Width;
	glowViewPort.Height = screen_Height;
	glowViewPort.MinDepth = 0.0f;
	glowViewPort.MaxDepth = 1.0f;
	glowViewPort.TopLeftX = 0;
	glowViewPort.TopLeftY = 0;
	//gDeviceContext->RSSetViewports(1, &glowViewPort);

}

void Glow::CreateTextures(){
	//ID3DBlob* pVS = nullptr;
	D3D11_INPUT_ELEMENT_DESC inputDescOnly[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	gDevice->CreateInputLayout(inputDescOnly, ARRAYSIZE(inputDescOnly), glowBlob->GetBufferPointer(), glowBlob->GetBufferSize(), &glowInputLayout);

	glowBlob->Release(); //viktigt att den görs här så den inte fuckar upp vertexshaderns skapelse i renderengine eller tvärtom

	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	///////////////////////// Map's Texture
	// Initialize the  texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	textureDesc.Width = screen_Width;
	textureDesc.Height = screen_Height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; //texturen är bunden till både ett rendertarget och en subresource
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	gDevice->CreateTexture2D(&textureDesc, NULL, &renderTargetTexture);
	gDevice->CreateTexture2D(&textureDesc, NULL, &tempRenderTargetTexture);

	/////////////////////// Render to texture's Render Target
	// Setup the description of the render target view.
	renderTargetViewDesc.Format = textureDesc.Format; //samma format som renderTargetTexturen
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	gDevice->CreateRenderTargetView(renderTargetTexture, &renderTargetViewDesc, &renderTargetView);
	gDevice->CreateRenderTargetView(tempRenderTargetTexture, &renderTargetViewDesc, &tempRenderTargetView);

	shaderResourceViewDesc.Format = textureDesc.Format;//samma format som renderTargetTexturen
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	gDevice->CreateShaderResourceView(renderTargetTexture, &shaderResourceViewDesc, &shaderResourceView);
	gDevice->CreateShaderResourceView(tempRenderTargetTexture, &shaderResourceViewDesc, &tempShaderResourceView); //temporär SRV som kan behöva användas med blur

	//DepthBuffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D11_TEXTURE2D_DESC));
	depthStencilDesc.Width = screen_Width;
	depthStencilDesc.Height = screen_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;


	gDevice->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
	gDevice->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);

}

void Glow::CreatePlaneAndBuffers(){
	struct PlaneVertex
	{
		float x, y, z;
		float u, v;

	}
	PlaneVertices[4] =
	{
		-1.0f, -1.0f, -1.0f,		//v0 
		0.0f, 1.0f,			//t0


		-1.0f, 1.0f, -1.0f,		//v1
		0.0f, 0.0f,				//t1


		1.0f, -1.0f, -1.0f,		//v2
		1.0f, 1.0f,			//t2


		1.0f, 1.0f, -1.0f,		//v3
		1.0f, 0.0f			//t3	
	};
	//PlaneVertices[4] =
	//{
	//	-0.5f, -0.5f, -0.5f,		//v0 
	//	0.0f, 1.0f,			//t0


	//	-0.5f, 0.5f, -0.5f,		//v1
	//	0.0f, 0.0f,				//t1


	//	0.5f, -0.5f, -0.5f,		//v2
	//	1.0f, 1.0f,			//t2


	//	0.5f, 0.5f, -0.5f,		//v3
	//	1.0f, 0.0f			//t3	
	//};


	// VertexBuffer description
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(PlaneVertices);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = PlaneVertices;
	gDevice->CreateBuffer(&bufferDesc, &data, &planeVertexBuffer);


	//constant buffers!!
	horizontalConstantStruct.screenSize = screen_Width;
	verticalConstantStruct.screenSize = screen_Height;

	D3D11_BUFFER_DESC blurBufferDesc;
	memset(&blurBufferDesc, 0, sizeof(blurBufferDesc));
	blurBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	blurBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	blurBufferDesc.ByteWidth = sizeof(horizontalConstantStruct);
	gDevice->CreateBuffer(&blurBufferDesc, NULL, &horizontalConstantBuffer);

	D3D11_BUFFER_DESC blurBufferDesc2;
	memset(&blurBufferDesc2, 0, sizeof(blurBufferDesc2));
	blurBufferDesc2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	blurBufferDesc2.Usage = D3D11_USAGE_DEFAULT;
	blurBufferDesc2.ByteWidth = sizeof(verticalConstantStruct);
	gDevice->CreateBuffer(&blurBufferDesc2, NULL, &verticalConstantBuffer);

	gDeviceContext->UpdateSubresource(horizontalConstantBuffer, 0, NULL, &horizontalConstantStruct, 0, 0);
	gDeviceContext->UpdateSubresource(verticalConstantBuffer, 0, NULL, &verticalConstantStruct, 0, 0);
	//glowConstantStruct.glowThreshHold = this->glowThreshHold;
	//glowConstantStruct.glowValue = this->glowValue;
	D3D11_BUFFER_DESC blurBufferDesc3;
	memset(&blurBufferDesc3, 0, sizeof(blurBufferDesc3));
	blurBufferDesc3.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	blurBufferDesc3.Usage = D3D11_USAGE_DEFAULT;
	blurBufferDesc3.ByteWidth = sizeof(glowConstantStruct);
	HRESULT kpasta = gDevice->CreateBuffer(&blurBufferDesc3, NULL, &glowConstantBuffer);
}

void Glow::DrawToGlowMap(){
	gDeviceContext->RSSetViewports(1, &glowViewPort);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	gDeviceContext->IASetInputLayout(glowInputLayout);
	gDeviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	gDeviceContext->ClearRenderTargetView(renderTargetView, clearColor);
	gDeviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	gDeviceContext->UpdateSubresource(glowConstantBuffer, 0, NULL, &glowConstantStruct, 0, 0);
	gDeviceContext->PSSetConstantBuffers(3, 1, &glowConstantBuffer);
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //ta bort denna sen kanske
	gDeviceContext->VSSetShader(glowVertexShader, nullptr, 0);
	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	gDeviceContext->PSSetShader(glowPixelShader, nullptr, 0);
}


void Glow::ApplyBlurOnGlowHorizontal(ID3D11VertexShader *VS, ID3D11PixelShader *PS){
	UINT32 vertexPosTex = 5 * sizeof(float);
	UINT offset2 = 0;
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP); //ta bort denna sen kanske

	gDeviceContext->RSSetViewports(1, &glowViewPort);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	gDeviceContext->IASetInputLayout(glowInputLayout);
	gDeviceContext->ClearRenderTargetView(tempRenderTargetView, clearColor);
	gDeviceContext->OMSetRenderTargets(1, &tempRenderTargetView, depthStencilView);

	//gDeviceContext->ClearRenderTargetView(renderTargetView, clearColor);
	gDeviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	gDeviceContext->VSSetShader(VS, nullptr, 0);
	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	gDeviceContext->PSSetShader(PS, nullptr, 0);
	gDeviceContext->VSSetConstantBuffers(1, 1, &horizontalConstantBuffer);

	gDeviceContext->PSSetShaderResources(0, 1, &shaderResourceView);
	gDeviceContext->IASetVertexBuffers(0, 1, &planeVertexBuffer, &vertexPosTex, &offset2);
	gDeviceContext->Draw(4, 0); //rita till ännu ett render target
}

void Glow::ApplyBlurOnGlowVertical(ID3D11VertexShader *VS, ID3D11PixelShader *PS){
	UINT32 vertexPosTex = 5 * sizeof(float);
	UINT offset2 = 0;
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP); //ta bort denna sen kanske

	gDeviceContext->RSSetViewports(1, &glowViewPort);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	gDeviceContext->IASetInputLayout(glowInputLayout);
	gDeviceContext->ClearRenderTargetView(renderTargetView, clearColor);
	gDeviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	//gDeviceContext->ClearRenderTargetView(renderTargetView, clearColor);
	gDeviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	gDeviceContext->VSSetShader(VS, nullptr, 0);
	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	gDeviceContext->PSSetShader(PS, nullptr, 0);
	gDeviceContext->VSSetConstantBuffers(1, 1, &verticalConstantBuffer);

	gDeviceContext->PSSetShaderResources(0, 1, &tempShaderResourceView);
	gDeviceContext->IASetVertexBuffers(0, 1, &planeVertexBuffer, &vertexPosTex, &offset2);
	gDeviceContext->Draw(4, 0); //rita till ännu ett render target
}

void Glow::SetPosTexVertexLayout(){
	gDeviceContext->IASetInputLayout(glowInputLayout);
}