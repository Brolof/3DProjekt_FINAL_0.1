
#include "ShadowMap.h"

ShadowMap::ShadowMap(){
	SHADOWMAP_WIDTH = 0;
	SHADOWMAP_HEIGHT = 0;
	SHADOWMAP_DEPTH = 0.0f;
	SHADOWMAP_NEAR = 0.0f;
};
ShadowMap::~ShadowMap(){
	depthMapTexture->Release();
	deptMapBuffer->Release();
	depthMapSRV->Release();
	depthMapDSV->Release();
	depthMapRTV->Release();
	depthInputLayout->Release();
	shadowBuffer->Release();
	gWorld->Release();
	depthVertexShader->Release();
	depthPixelShader->Release();
};

void ShadowMap::CreateShadowMap(ID3D11Device* gDevice, DirLight light, XMFLOAT4 lightPos){
	//DEBUGGER
	HRESULT ShaderTest = 0;

	//BUFFER FOR CONSTANT BUFFER
	D3D11_BUFFER_DESC WorldBufferDesc;
	memset(&WorldBufferDesc, 0, sizeof(WorldBufferDesc));
	WorldBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	WorldBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	WorldBufferDesc.ByteWidth = sizeof(DepthWorldMatrix);
	ShaderTest = gDevice->CreateBuffer(&WorldBufferDesc, NULL, &gWorld);


	// Viewport description
	depthVP.Width = (float)SHADOWMAP_WIDTH;
	depthVP.Height = (float)SHADOWMAP_HEIGHT;
	depthVP.MinDepth = SHADOWMAP_NEAR;
	depthVP.MaxDepth = SHADOWMAP_DEPTH;
	depthVP.TopLeftX = 0;
	depthVP.TopLeftY = 0;

	//Descriptions for Texture,SRV,DSV,RTV
	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	///////////////////////// Map's Texture
	// Initialize the  texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the texture description.
	// We will need to have this texture bound as a render target AND a shader resource
	textureDesc.Width = SHADOWMAP_WIDTH;
	textureDesc.Height = SHADOWMAP_HEIGHT;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;// test later DXGI_FORMAT_R24G8_TYPELESS
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Create the texture
	ShaderTest = gDevice->CreateTexture2D(&textureDesc, NULL, &depthMapTexture);


	/////////////////////// Render to texture's Render Target
	// Setup the description of the render target view.
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	ShaderTest = gDevice->CreateRenderTargetView(depthMapTexture, &renderTargetViewDesc, &depthMapRTV);

	/////////////////////// Map's Shader Resource View
	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	ShaderTest = gDevice->CreateShaderResourceView(depthMapTexture, &shaderResourceViewDesc, &depthMapSRV);


	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = SHADOWMAP_WIDTH;
	depthBufferDesc.Height = SHADOWMAP_HEIGHT;
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
	ShaderTest = gDevice->CreateTexture2D(&depthBufferDesc, NULL, &deptMapBuffer);

	// Initailze the depth stencil view description.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	ShaderTest = gDevice->CreateDepthStencilView(deptMapBuffer, &depthStencilViewDesc, &depthMapDSV);


	//INPUT LAYOUT AND SHADERS
	//create input layout (verified using vertex shader)

	//create  depth vertexShader
	ID3DBlob* dVS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/depthVS.hlsl", "VS_main", "vs_5_0", &dVS);
	ShaderTest = gDevice->CreateVertexShader(dVS->GetBufferPointer(), dVS->GetBufferSize(), nullptr, &depthVertexShader);

	//create depth pixelshader
	ID3DBlob* dPS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/depthPS.hlsl", "PS_main", "ps_5_0", &dPS);
	ShaderTest = gDevice->CreatePixelShader(dPS->GetBufferPointer(), dPS->GetBufferSize(), nullptr, &depthPixelShader);

	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	ShaderTest = gDevice->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), dVS->GetBufferPointer(), dVS->GetBufferSize(), &depthInputLayout);

	//RELEASE BLOBS
	dVS->Release();
	dPS->Release();
	//ViewMatrix
	camPosition2 = Vector4(lightPos.x, lightPos.y, lightPos.z, lightPos.w);

	camRotationMatrix2 = XMMatrixRotationRollPitchYaw(0, 0, 0);

	camTarget2 = XMVector3TransformCoord(DefaultForward2, camRotationMatrix2);
	camTarget2 = XMVector3Normalize(camTarget2);

	camRight2 = XMVector3TransformCoord(DefaultRight2, camRotationMatrix2);
	camForward2 = XMVector3TransformCoord(DefaultForward2, camRotationMatrix2);
	camUp2 = XMVector3Cross(camForward2, camRight2);

	camTarget2 = Vector4(0.0f, 0.0f, 1.0f, 0.0f);// camPosition2 + camTarget2;

	fpsCamLook2 = XMMatrixLookAtLH(camPosition2, camTarget2, camUp2);


	lightViewMatrix = fpsCamLook2;
	lightworld = XMMatrixIdentity();
};

void ShadowMap::SetShadowMapConstants(int size, float depth, float Near, int matrixType, float orthosize){
	this->SHADOWMAP_WIDTH = size;
	this->SHADOWMAP_HEIGHT = size;
	this->SHADOWMAP_DEPTH = depth;
	this->SHADOWMAP_NEAR = Near;

	if (matrixType == 1){
		lightCam.SetLens(0.25f*3.14f, size / size, Near, depth);
		depthProjection = lightCam.Proj();

	}
	else if (matrixType == 2){
		depthProjection = XMMatrixOrthographicLH(orthosize, orthosize, Near, depth);
	}
};

void ShadowMap::DrawDepthMap(std::vector<GameObjects*> Mesh, ID3D11DeviceContext* deviceContext){


	XMStoreFloat4x4(&VertexShaderBuffer.View, XMMatrixTranspose(lightViewMatrix));
	XMStoreFloat4x4(&VertexShaderBuffer.Projection, XMMatrixTranspose(depthProjection));
	XMStoreFloat4x4(&VertexShaderBuffer.WorldSpace, XMMatrixTranspose(lightworld));
	XMStoreFloat4x4(&VertexShaderBuffer.InvWorld, XMMatrixTranspose(XMMatrixInverse(NULL, lightworld)));
	XMStoreFloat4x4(&VertexShaderBuffer.lightView, XMMatrixTranspose(lightViewMatrix));
	XMStoreFloat4x4(&VertexShaderBuffer.lightProjection, XMMatrixTranspose(depthProjection));

	deviceContext->UpdateSubresource(gWorld, 0, NULL, &VertexShaderBuffer, 0, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &gWorld);

	UINT32 vertexSize = sizeof(float)* 11;
	UINT32 offset = 0;
	deviceContext->RSSetViewports(1, &depthVP);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	deviceContext->IASetInputLayout(depthInputLayout);
	deviceContext->OMSetRenderTargets(1, &depthMapRTV, depthMapDSV);

	deviceContext->ClearRenderTargetView(depthMapRTV, clearColor);
	deviceContext->ClearDepthStencilView(depthMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->VSSetShader(depthVertexShader, nullptr, 0);
	deviceContext->GSSetShader(nullptr, nullptr, 0);
	deviceContext->PSSetShader(depthPixelShader, nullptr, 0);
	for (int i = 0; i < Mesh.size(); i++)
	{

		deviceContext->IASetVertexBuffers(0, 1, &Mesh[i]->vertexBuffer, &vertexSize, &offset);
		deviceContext->Draw(Mesh[i]->nrElements * 3, 0);
	}

};

HRESULT ShadowMap::CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob)
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
