#include "Defered.h"

DeferedRenderer::DeferedRenderer(){
	int i;

	for (i = 0; i<2; i++)
	{
		RTVArray[i] = 0;
		SRVArray[i] = 0;
		textures[i] = 0;
	}

	depthBuffer = 0;
	DSV = 0;
};

DeferedRenderer::~DeferedRenderer(){
	for (int i = 0; i < 2; i++){
		textures[i] = NULL;
		SRVArray[i] = NULL;
		RTVArray[i] = NULL;
	}
	
	InputLayout->Release();
	DSV->Release();
	gWorld->Release();
	vertexShader->Release();
	pixelShader->Release();
};

void DeferedRenderer::CreateTextureMap(ID3D11Device* gDevice, int sw, int sh, float sd, float sn, ID3D11ShaderResourceView** RSWArrayImp, vector<int> intArrayTexImp){
	//DEBUGGER
	HRESULT ShaderTest = 0;


	RSWArray = RSWArrayImp;
	intArrayTextures = intArrayTexImp;

	//BUFFER FOR CONSTANT BUFFER
	D3D11_BUFFER_DESC WorldBufferDesc;
	memset(&WorldBufferDesc, 0, sizeof(WorldBufferDesc));
	WorldBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	WorldBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	WorldBufferDesc.ByteWidth = sizeof(WorldMatrix);
	ShaderTest = gDevice->CreateBuffer(&WorldBufferDesc, NULL, &gWorld);


	D3D11_BUFFER_DESC WorldBufferDesc2;
	memset(&WorldBufferDesc2, 0, sizeof(WorldBufferDesc2));
	WorldBufferDesc2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	WorldBufferDesc2.Usage = D3D11_USAGE_DEFAULT;
	WorldBufferDesc2.ByteWidth = sizeof(WorldMatrix2);
	ShaderTest = gDevice->CreateBuffer(&WorldBufferDesc2, NULL, &gWorld2);


	// Viewport description
	DeferedVP.Width = (float)sw;
	DeferedVP.Height = (float)sh;
	DeferedVP.MinDepth = 0.0f;
	DeferedVP.MaxDepth = 1.0f;
	DeferedVP.TopLeftX = 0;
	DeferedVP.TopLeftY = 0;

	// Viewport description
	DeferedVP2.Width = (float)sw;
	DeferedVP2.Height = (float)sh;
	DeferedVP2.MinDepth = 0.0f;
	DeferedVP2.MaxDepth = 1.0f;
	DeferedVP2.TopLeftX = 0;
	DeferedVP2.TopLeftY = 0;


	//Descriptions for Texture,SRV,DSV,RTV
	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_TEXTURE2D_DESC textureDesc2;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc2;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	///////////////////////// Map's Texture
	// Initialize the  texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the texture description.
	// We will need to have this texture bound as a render target AND a shader resource
	textureDesc.Width = sw;
	textureDesc.Height = sh;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;// test later DXGI_FORMAT_R24G8_TYPELESS
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Setup the texture description.
	// We will need to have this texture bound as a render target AND a shader resource
	textureDesc2.Width = sw;
	textureDesc2.Height = sh;
	textureDesc2.MipLevels = 1;
	textureDesc2.ArraySize = 1;
	textureDesc2.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;// test later DXGI_FORMAT_R24G8_TYPELESS
	textureDesc2.SampleDesc.Count = 1;
	textureDesc2.Usage = D3D11_USAGE_DEFAULT;
	textureDesc2.BindFlags = 0;
	textureDesc2.CPUAccessFlags = 0;
	textureDesc2.MiscFlags = 0;
	ShaderTest = gDevice->CreateTexture2D(&textureDesc2, NULL, &rtvTex);
	// Create the textures
	for (int i = 0; i < 3; i++){
		ShaderTest = gDevice->CreateTexture2D(&textureDesc, NULL, &textures[i]);
	}
	/////////////////////// Render to texture's Render Targets
	// Setup the description of the render target view.
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render targets view.
	for (int i = 0; i < 3; i++){
		ShaderTest = gDevice->CreateRenderTargetView(textures[i], &renderTargetViewDesc, &RTVArray[i]);
	}

	renderTargetViewDesc2.Format = textureDesc2.Format;
	renderTargetViewDesc2.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc2.Texture2D.MipSlice = 0;

	ShaderTest = gDevice->CreateRenderTargetView(rtvTex, &renderTargetViewDesc2, &RTVFinal);
	/////////////////////// Map's Shader Resource View
	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	for (int i = 0; i < 3; i++){
		ShaderTest = gDevice->CreateShaderResourceView(textures[i], &shaderResourceViewDesc, &SRVArray[i]);
	}

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = sw;
	depthBufferDesc.Height = sh;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	ShaderTest = gDevice->CreateTexture2D(&depthBufferDesc, NULL, &depthBuffer);

	// Initailze the depth stencil view description.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	ShaderTest = gDevice->CreateDepthStencilView(depthBuffer, &depthStencilViewDesc, &DSV);
	ShaderTest = gDevice->CreateDepthStencilView(depthBuffer, &depthStencilViewDesc, &DSV2);



	//INPUT LAYOUT AND SHADERS
	//create input layout (verified using vertex shader)

	//create   vertexShader
	ID3DBlob* dVS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/DeferedVertex.hlsl", "VS_main", "vs_5_0", &dVS);
	ShaderTest = gDevice->CreateVertexShader(dVS->GetBufferPointer(), dVS->GetBufferSize(), nullptr, &vertexShader);


	//Create DEFERED vertexShader
	ID3DBlob* pDVS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/DeferedFinalVS.hlsl", "VS_main", "vs_5_0", &pDVS);
	ShaderTest = gDevice->CreateVertexShader(pDVS->GetBufferPointer(), pDVS->GetBufferSize(), nullptr, &deferedVertexShader);

	//Create DEFERED pixelShader
	ID3DBlob* pDPS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/DeferedFinalPS.hlsl", "PS_main", "ps_5_0", &pDPS);
	ShaderTest = gDevice->CreatePixelShader(pDPS->GetBufferPointer(), pDPS->GetBufferSize(), nullptr, &deferedPixelShader);


	//create  pixelshader
	ID3DBlob* dPS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/DeferedPixel.hlsl", "PS_main", "ps_5_0", &dPS);
	ShaderTest = gDevice->CreatePixelShader(dPS->GetBufferPointer(), dPS->GetBufferSize(), nullptr, &pixelShader);

	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	ShaderTest = gDevice->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), dVS->GetBufferPointer(), dVS->GetBufferSize(), &InputLayout);
	ShaderTest = gDevice->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), pDVS->GetBufferPointer(), pDVS->GetBufferSize(), &InputLayout2);
	//här är det ett fel
	// #Johan


	//RELEASE BLOBS
	dVS->Release();
	dPS->Release();
	pDPS->Release();
	pDVS->Release();

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	gDevice->CreateSamplerState(&samplerDesc, &sampState);


	D3D11_SAMPLER_DESC samplerDesc3;
	// Create a wrap texture sampler state description.
	samplerDesc3.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc3.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc3.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc3.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc3.MipLODBias = 0.0f;
	samplerDesc3.MaxAnisotropy = 16;
	samplerDesc3.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc3.BorderColor[0] = 0;
	samplerDesc3.BorderColor[1] = 0;
	samplerDesc3.BorderColor[2] = 0;
	samplerDesc3.BorderColor[3] = 0;
	samplerDesc3.MinLOD = 0;
	samplerDesc3.MaxLOD = D3D11_FLOAT32_MAX;
	gDevice->CreateSamplerState(&samplerDesc3, &sampState3);

	CreateWICTextureFromFile(gDevice, L"Textures/normalTest.jpg", nullptr, &NormTex);
	identity = XMMatrixIdentity();
}

void DeferedRenderer::DrawDefered(std::vector<GameObjects*> Mesh, ID3D11DeviceContext* deviceContext, XMMATRIX view, XMMATRIX proj, XMMATRIX world){


	XMStoreFloat4x4(&VertexShaderBuffer.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&VertexShaderBuffer.Projection, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&VertexShaderBuffer.WorldSpace, XMMatrixTranspose(world));
	XMStoreFloat4x4(&VertexShaderBuffer.InvWorld, XMMatrixTranspose(XMMatrixInverse(NULL, world)));
	XMStoreFloat4x4(&VertexShaderBuffer.WVP, XMMatrixTranspose(world*view*proj));

	deviceContext->UpdateSubresource(gWorld, 0, NULL, &VertexShaderBuffer, 0, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &gWorld);

	UINT32 vertexSize = sizeof(float)* 11;
	UINT32 offset = 0;
	
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	deviceContext->IASetInputLayout(InputLayout);
	deviceContext->OMSetRenderTargets(3, RTVArray, DSV);
	deviceContext->RSSetViewports(1, &DeferedVP);
	// Clear the render target buffers.
	for (int i = 0; i < 3; i++)
	{
		deviceContext->ClearRenderTargetView(RTVArray[i], clearColor);
	}

	deviceContext->ClearDepthStencilView(DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->VSSetShader(vertexShader, nullptr, 0);
	deviceContext->GSSetShader(nullptr, nullptr, 0);
	deviceContext->PSSetShader(pixelShader, nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, &sampState);
	deviceContext->PSSetShaderResources(1, 1, &NormTex);
	for (int i = 0; i < Mesh.size(); i++)
	{
		tex = intArrayTextures[Mesh[i]->indexT];
		deviceContext->PSSetShaderResources(0, 1, &RSWArray[tex]);
	
		deviceContext->IASetVertexBuffers(0, 1, &Mesh[i]->vertexBuffer, &vertexSize, &offset);
		deviceContext->Draw(Mesh[i]->nrElements * 3, 0);
	}


};


void DeferedRenderer::DrawDefered2(ID3D11Buffer* buff, ID3D11DeviceContext* deviceContext){

	XMStoreFloat4x4(&VertexShaderBuffer2.WVP, XMMatrixTranspose(XMMatrixScaling(1.0f, 1.0f, 0.0f) * XMMatrixTranslation(0.0f, 0.0f, 0.0f)));
	XMStoreFloat4x4(&VertexShaderBuffer2.World, XMMatrixTranspose(identity));
	deviceContext->UpdateSubresource(gWorld2, 0, NULL, &VertexShaderBuffer2, 0, 0);
	deviceContext->VSSetConstantBuffers(3, 1, &gWorld2);

	UINT32 vertexSize2 = sizeof(float)* 11;
	UINT32 offset2 = 0;
	deviceContext->IASetVertexBuffers(0, 1, &buff, &vertexSize2, &offset2);

	float clearColor2[] = { 0.0f, 1.0f, 1.0f, 1.0f };
//	deviceContext->OMSetRenderTargets(1, &RTVFinal, DSV2);
//	deviceContext->RSSetViewports(1, &DeferedVP2);
//	deviceContext->ClearRenderTargetView(RTVFinal, clearColor2);
//	deviceContext->ClearDepthStencilView(DSV2, D3D11_CLEAR_DEPTH, 1.0f, 0);

	deviceContext->IASetInputLayout(InputLayout2);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	deviceContext->VSSetShader(deferedVertexShader, nullptr, 0);
	deviceContext->GSSetShader(nullptr, nullptr, 0);
	deviceContext->PSSetShader(deferedPixelShader, nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, &sampState3);
	deviceContext->PSSetShaderResources(0, 1, &SRVArray[0]);
	deviceContext->PSSetShaderResources(1, 1, &SRVArray[1]);
	deviceContext->PSSetShaderResources(2, 1, &SRVArray[2]);

	
		deviceContext->Draw(4, 0);


};

HRESULT DeferedRenderer::CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob)
{
	if (!srcFile || !entryPoint || !profile || !blob)
		return E_INVALIDARG;

	*blob = nullptr;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif

	const D3D_SHADER_MACRO defines[] =
	{
		"EXAMPLE_DEFINE", "1",
		NULL, NULL
	};

	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(srcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint, profile,
		flags, 0, &shaderBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}

		if (shaderBlob)
			shaderBlob->Release();

		return hr;
	}

	*blob = shaderBlob;

	return hr;
}
