
#include "RenderEngine.h"
#include <string.h>


namespace{
	RenderEngine* pRenderEngine; //pekare till sjävla applikationen
}

// MSG HANDLER FOR WINMAIN

LRESULT CALLBACK MainWindowProc(HWND hwindow, UINT msg, WPARAM wParam, LPARAM lParam){ //denna används endast för att komma åt RenderEnginens MsgProc() och skicka vidare meddelanden till den
	return pRenderEngine->MsgProc(hwindow, msg, wParam, lParam);
}

// CONSTRUCTOR

RenderEngine::RenderEngine(HINSTANCE hInstance, std::string name, UINT scrW, UINT scrH){
	this->hInstance = hInstance;
	applicationName = name;
	screen_Width = scrW; 
	screen_Height = scrH;
	pRenderEngine = this;
	windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX;
}

// DESTRUCTOR

RenderEngine::~RenderEngine(){ //destruktor

}


std::wstring string2wString(const std::string& s){
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}
// INITIALIZE CORE FUNCTIONS

bool RenderEngine::Init(){


	CoInitialize(nullptr);

	HRESULT inputtest;

	if (!InitWindow()){
		return false; //gick inte att skapa window
	}

	if (!InitDirect3D(hWindow)){
		return false; //gick inte att skapa Direct3D
	}
	// Sets and creates viewport
	SetViewport();
	

	CreatePlaneDataAndBuffers();


	//Set Camera values
	//fpsCam.SetPosition(0.0f, 0.4f, -6.0f);
	camPosition = Vector4(-1.10f, 10.70f, -20.2f, 0.0f);
	fpsCam.SetLens(0.25f*3.14f, screen_Width / screen_Height, 1.0f, 100.0f);
	

	//Shadow map init
	

	//Keyboard and mouse handlers
	

	//Initialize Shaders and triangle data
	Shaders();
	InitDirectInput(hInstance);
	theCustomImporter.ImportBIN(gDevice, "Geometry/testFile.bin");
	intArrayTex = theCustomImporter.GetindexArray();
	renderObjects = theCustomImporter.GetObjects();
	TextureFunc();

	for (int i = 0; i < renderObjects.size(); i++) //skapar boundingboxar för objecten
	{
		renderObjects[i]->CreateBBOXVertexBuffer(gDevice);
	}
	
	//ImportHeightmap("Textures/8x8Map.bmp", L"Textures/stone.dds", L"Textures/grass.dds", L"Textures/hippo.dds", L"Textures/splatmap.dds");
	//ImportHeightmap("Textures/JäkligtFinHeightmap.bmp", L"Textures/stone.dds", L"Textures/grass.dds", L"Textures/hippo.dds", L"Textures/splatmap.dds");

	inputtest = m_Input->Initialize(hInstance, hWindow, screen_Width, screen_Height);
	if (inputtest==0) 
	{
		MessageBox(hWindow, "Could not initialize the input object.", "Error", MB_OK);
		return false;
	}

	ListQuadTree(nrSplitsTree, XMFLOAT3(0, 0, 0), XMFLOAT3(10, 10, 10)); //den första boxen som skickas in är den största (världsboxen)

	return true; //om båda funkade så returnera true (y)
	
}

// INITIALIZE WINDOW

bool RenderEngine::InitWindow(){
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.hInstance = hInstance;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW; // hur den ska måla ut allt, fast spelar nog ingen roll i vårt fall
	wcex.lpfnWndProc = MainWindowProc;
	//wcex.hCursor = LoadCursor(NULL, IDC_NO); //skoj
	//wcex.hIcon = LoadIcon(NULL, IDI_ERROR); //skoj * 2
	//wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "RENDERENGINECLASS";
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex)){
		MessageBox(NULL, "Kunde inte registrera window classen", NULL, NULL);
		return false;
	}

	RECT r = { 0, 0, screen_Width, screen_Height };
	AdjustWindowRect(&r, windowStyle, FALSE); //kommer inte kunna resiza skiten
	int width = r.right - r.left;
	int height = r.bottom - r.top;
	mainwname = L"Direct3D Projekt";
	hWindow = CreateWindow("RENDERENGINECLASS",
		"Direct3D Projekt", //INTE SÄKER PÅ DETTA, namnet på applikationen blir typ i kinaspråk så venne, kan vara detta
		WS_OVERLAPPEDWINDOW,//Window handlers
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width, height, nullptr, nullptr, hInstance, nullptr);

	if (!hWindow){
		MessageBox(NULL, "Failed to create window", NULL, NULL);
		return false;
	}


	return true;
}

// CREATE VIEWPORT

void RenderEngine::SetViewport()
{
	vp.Width = screen_Width;
	vp.Height = screen_Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	gDeviceContext->RSSetViewports(1, &vp);
	

	shadowVP.Width = SHADOWMAP_WIDTH;
	shadowVP.Height = SHADOWMAP_HEIGHT;
	shadowVP.MinDepth = SHADOWMAP_NEAR;
	shadowVP.MaxDepth = SHADOWMAP_DEPTH;
	shadowVP.TopLeftX = 0;
	shadowVP.TopLeftY = 0;
}

// FPS COUNTER

void RenderEngine::fpscounter()
{
	// Code computes the avarage frame time and frames per second

	static int framecount = 0;
	static float time = 0.0f;

	//Function is beeing initialiased every time a frame is made
	framecount++;

	// Compute averages over one second period
	if ((gTimer.TotalTime() - time) >= 0.50f)
	{
		float fps = (float)framecount * 2; // fps = framecount / 1
		float mspf = 1000.0f / fps;
		float timer = gTimer.TotalTime();
		// Makes a String for the window handler
		std::wostringstream outs;

		outs.precision(6);
		outs << mainwname << L" "
			<< L"        FPS: " << fps << L" "
			<< L"        Frame Time: " << mspf << L" (ms)"
			<< L"        Time: " << timer << L" sec";

		//Prints the text in the window handler
		SetWindowText(hWindow, "Shadows soon");// outs.str().c_str());

		// Reset for next fps.
		framecount = 0;
		time += 0.50f;

	}
}

// CREATE TEXTURES

void RenderEngine::TextureFunc(){
	//RENDER TO TEXTURE
	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC dpsDesc;
	///////////////////////// Map's Texture
	// Initialize the  texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the texture description.
	// We will have our map be a square
	// We will need to have this texture bound as a render target AND a shader resource
	textureDesc.Width = SHADOWMAP_WIDTH;
	textureDesc.Height = SHADOWMAP_HEIGHT;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Create the texture
	gDevice->CreateTexture2D(&textureDesc, NULL, &depthMap);

	// DONT NEED NOW
	///////////////////////// Render to texture's Render Target
	//// Setup the description of the render target view.
	//renderTargetViewDesc.Format = textureDesc.Format;
	//renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	//renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	//gDevice->CreateRenderTargetView(depthMap, &renderTargetViewDesc, &depthMap);

	/////////////////////// Map's depth stencil view 
	dpsDesc.Flags = 0;
	dpsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dpsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dpsDesc.Texture2D.MipSlice = 0;
	gDevice->CreateDepthStencilView(depthMap, &dpsDesc, &depthStencilcDepthMap);
	/////////////////////// Map's Shader Resource View
	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = textureDesc.MipLevels;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	gDevice->CreateShaderResourceView(depthMap, &shaderResourceViewDesc, &shaderResourceDepthMap);
	//RENDER TO TEXTURE
	//Matrix computation

	//////////////////////// Map's camera information
	// We will have the camera follow the player
	XMVECTOR mapCamPosition = XMVectorSet(0.0f, 25.0f, 1.0f, 0.0f);// XMVectorSetY(camPosition, XMVectorGetY(camPosition) + 100.0f);
	XMVECTOR mapCamTarget = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); //camPosition;
	XMVECTOR mapCamUp = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	//Set the View matrix
	mapView = XMMatrixLookAtLH(mapCamPosition, mapCamTarget, mapCamUp);
	// Build an orthographic projection matrix
	mapProjection = XMMatrixOrthographicLH(512, 512, 1.0f, 1000.0f);


	HRESULT texCheck;
	texCheck = CreateDDSTextureFromFile(gDevice, L"Textures/terrain.dds", nullptr, &ddsTex1);
	texCheck = CreateDDSTextureFromFile(gDevice, L"Textures/shark.dds", nullptr, &ddsTex2);
	texCheck = CreateDDSTextureFromFile(gDevice, L"Textures/stone.dds", nullptr, &AdsResourceView);
	texCheck = CreateDDSTextureFromFile(gDevice, L"Textures/NPC.dds", nullptr, &NpcRV);
	texCheck = CreateWICTextureFromFile(gDevice, L"Textures/hippo.jpg", nullptr, &ddsTex3);

	vector<string> texNames = theCustomImporter.GettexNameArray();
	RSWArray = new ID3D11ShaderResourceView*[texNames.size()];
	std::wstring fest = L"Textures/";
	for (int texIt = 0; texIt < texNames.size(); texIt++){

		std::wstring fest2 = fest + string2wString(texNames[texIt]);;
		const wchar_t* texNameS = fest2.c_str();
		texCheck = CreateDDSTextureFromFile(gDevice, texNameS, nullptr, &RSWArray[texIt]);
		if (texCheck == E_FAIL){
			CreateWICTextureFromFile(gDevice, texNameS, nullptr, &RSWArray[texIt]);
		}
	}
}

// COMPILE SHADERS FROM .HLSL FILES

HRESULT CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob)
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

// CREATE AND COMPILE SHADERS

void RenderEngine::Shaders(){

	HRESULT ShaderTest;
	
	//MAKE SAMPLERS
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	gDevice->CreateSamplerState(&samplerDesc, &sampState1);

	D3D11_SAMPLER_DESC samplerDesc2;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MaxAnisotropy = 16;
	gDevice->CreateSamplerState(&samplerDesc2, &sampState2);
	//DEAFULT SHADERS

	//create vertex shader6
	ID3DBlob* pVS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/deafultVS.hlsl", "VS_main", "vs_5_0", &pVS);
	ShaderTest = gDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &gVertexShader);

	//create vertex depth
	ID3DBlob* dVS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/depthVS.hlsl", "VS_main", "vs_5_0", &dVS);
	ShaderTest = gDevice->CreateVertexShader(dVS->GetBufferPointer(), dVS->GetBufferSize(), nullptr, &dVertexShader);
	
	//create vertex texture
	ID3DBlob* ddVS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/deafultTexVS.hlsl", "VS_main", "vs_5_0", &ddVS);
	ShaderTest = gDevice->CreateVertexShader(ddVS->GetBufferPointer(), ddVS->GetBufferSize(), nullptr, &shader2DVS);

	//Create shadow vertexShader
	ID3DBlob* pSVS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/shadowVS.hlsl", "VS_main", "vs_5_0", &pSVS);
	ShaderTest = gDevice->CreateVertexShader(pSVS->GetBufferPointer(), pSVS->GetBufferSize(), nullptr, &shadowVertexShader);


	//create input layout (verified using vertex shader)
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	ShaderTest = gDevice->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &gVertexLayout);

	D3D11_INPUT_ELEMENT_DESC inputDescPosOnly[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	gDevice->CreateInputLayout(inputDescPosOnly, ARRAYSIZE(inputDescPosOnly), pVS->GetBufferPointer(), pVS->GetBufferSize(), &gWireFrameLayout);
	

	//Create geometry shader
	ID3DBlob* gGS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/BackFaceGShader.hlsl", "gs_main", "gs_5_0", &gGS);
	ShaderTest = gDevice->CreateGeometryShader(gGS->GetBufferPointer(), gGS->GetBufferSize(), nullptr, &gBackFaceShader);

	//create pixel shader
	ID3DBlob* pPS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/deafultPS.hlsl", "PS_main", "ps_5_0", &pPS);
	ShaderTest = gDevice->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &gPixelShader);
	

	//create hitpixel shader
	ID3DBlob* pPSH = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/hitPixelShader.hlsl", "PS_main", "ps_5_0", &pPSH);
	ShaderTest = gDevice->CreatePixelShader(pPSH->GetBufferPointer(), pPSH->GetBufferSize(), nullptr, &hitPixelShader);

	//create depth pixelshader
	ID3DBlob* dPS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/depthPS.hlsl", "PS_main", "ps_5_0", &dPS);
	ShaderTest = gDevice->CreatePixelShader(dPS->GetBufferPointer(), dPS->GetBufferSize(), nullptr, &dPixelShader);

	//create pixel texture
	ID3DBlob* ddPS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/defaultTexPS.hlsl", "PS_main", "ps_5_0", &ddPS);
	ShaderTest = gDevice->CreatePixelShader(ddPS->GetBufferPointer(), ddPS->GetBufferSize(), nullptr, &shader2DPS);


	////create height pixel shader
	ID3DBlob* pPS2 = nullptr;
	//ShaderTest = CompileShader(L"RGBSplatmapPS.hlsl", "main", "ps_5_0", &pPS2);
	//ShaderTest = gDevice->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &splatMapPixelShader);
	//

	////create shadow pixel shader
	ID3DBlob* pSPS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/shadowPS.hlsl", "PS_main", "ps_5_0", &pSPS);
	ShaderTest = gDevice->CreatePixelShader(pSPS->GetBufferPointer(), pSPS->GetBufferSize(), nullptr, &shadowPixelShader);
	

	//WIREFRAME	
	//ShaderTest = gDevice->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &splatMapPixelShader);

	ShaderTest = CompileShader(L"FX_HLSL/WireFrameVS.hlsl", "main", "vs_5_0", &pVS);
	ShaderTest = gDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &gWireFrameVertexShader);

	ShaderTest = CompileShader(L"FX_HLSL/WireFramePS.hlsl", "main", "ps_5_0", &pPS2);
	ShaderTest = gDevice->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &gWireFramePixelShader);

	ShaderTest = CompileShader(L"FX_HLSL/WireFramePSContainsTest.hlsl", "main", "ps_5_0", &pPS2);
	ShaderTest = gDevice->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &gWireFramePixelShaderCONTAINTEST);

	gGS->Release();
	pPSH->Release();
	pPS->Release();
	pPS2->Release();
	pSPS->Release();
	pSVS->Release();
	ddPS->Release();
	ddVS->Release();
}

// CREATE BUFFERS AND TEMP PLANE

void RenderEngine::CreatePlaneDataAndBuffers(){
	HRESULT BufferTest;

	struct PlaneVertex
	{
		float x, y, z;
		float ux, uy;
		float nx, ny, nz;
	}
	PlaneVertices[4] =
	{
		-1.0f, -1.0f, -1.0f,		//v0 
		0.0f, 1.0f,			//t0
		0.0f, 0.0f, -1.0f,		//n0

		-1.0f, 1.0f, -1.0f,		//v1
		0.0f, 0.0f,				//t1
		0.0f, 0.0f, -1.0f,		//n1

		1.0f, -1.0f, -1.0f,		//v2
		1.0f, 1.0f,			//t2
		0.0f, 0.0f, -1.0f,		//n2

		1.0f, 1.0f, -1.0f,		//v3
		1.0f, 0.0f,			//t3
		0.0f, 0.0f, -1.0f,		//n3
	};


	// VertexBuffer description
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(PlaneVertices);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = PlaneVertices;
	gDevice->CreateBuffer(&bufferDesc, &data, &gVertexBuffer);

	// Rotatation And transform World Buffer
	D3D11_BUFFER_DESC transformbuffer;
	memset(&transformbuffer, 0, sizeof(transformbuffer));
	transformbuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	transformbuffer.Usage = D3D11_USAGE_DEFAULT;
	transformbuffer.ByteWidth = sizeof(World);
	BufferTest = gDevice->CreateBuffer(&transformbuffer, NULL, &gWorld);

	// Lightbuffer
	D3D11_BUFFER_DESC lightbufferDesc;
	memset(&lightbufferDesc, 0, sizeof(lightbufferDesc));
	lightbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightbufferDesc.Usage = D3D11_USAGE_DEFAULT;
	lightbufferDesc.CPUAccessFlags = 0;
	lightbufferDesc.MiscFlags = 0;
	lightbufferDesc.ByteWidth = sizeof(LightStruct);
	int bWidht = lightbufferDesc.ByteWidth;
	BufferTest = gDevice->CreateBuffer(&lightbufferDesc, NULL, &PrimaryLightBuffer);

	// ViewPoint Buffer
	D3D11_BUFFER_DESC ViewBufferDesc;
	ViewBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	memset(&ViewBufferDesc, 0, sizeof(ViewBufferDesc));
	ViewBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ViewBufferDesc.CPUAccessFlags = 0;
	ViewBufferDesc.MiscFlags = 0;
	ViewBufferDesc.ByteWidth = sizeof(ViewBufferStruct);
	BufferTest = gDevice->CreateBuffer(&ViewBufferDesc, NULL, &ViewBuffer);

	// Mat Buffer
	D3D11_BUFFER_DESC MatBufferDesc;
	MatBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	memset(&MatBufferDesc, 0, sizeof(MatBufferDesc));
	MatBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	MatBufferDesc.CPUAccessFlags = 0;
	MatBufferDesc.MiscFlags = 0;
	MatBufferDesc.ByteWidth = sizeof(MatView);
	BufferTest = gDevice->CreateBuffer(&MatBufferDesc, NULL, &MatBuffer);

	
	// No backface culling description
	D3D11_RASTERIZER_DESC rastDesc1;
	ZeroMemory(&rastDesc1, sizeof(D3D11_RASTERIZER_DESC));
	rastDesc1.FillMode = D3D11_FILL_SOLID;
	rastDesc1.CullMode = D3D11_CULL_NONE;
	BufferTest = gDevice->CreateRasterizerState(&rastDesc1, &NoBcull);
}

// INITIALIZE DIRECTX OBJECT

bool RenderEngine::InitDirect3D(HWND hWindow){

	DXGI_SWAP_CHAIN_DESC scd;
	//Describe our SwapChain Buffer
	DXGI_MODE_DESC bufferDesc;

	ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

	bufferDesc.Width = screen_Width;
	bufferDesc.Height = screen_Height;
	bufferDesc.RefreshRate.Numerator = 60;
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	scd.BufferDesc = bufferDesc;
	scd.BufferCount = 1;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hWindow;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.SampleDesc.Count = 1; //anti-aliasing
	scd.SampleDesc.Quality = 0;// -kan vi mecka senare men är lite saker som ska göras då
	scd.Windowed = TRUE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags = 0;

	// create a device, device context and swap chain using the information in the scd struct
	HRESULT deviceHr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&gSwapChain,
		&gDevice,
		NULL,
		&gDeviceContext);

	if (SUCCEEDED(deviceHr))
	{
		// get the address of the back buffer
		ID3D11Texture2D* pBackBuffer = nullptr;
		gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		// use the back buffer address to create the render target
		gDevice->CreateRenderTargetView(pBackBuffer, NULL, &gBackRufferRenderTargetView);
		pBackBuffer->Release();

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

		
	

		deviceHr = gDevice->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);

		deviceHr = gDevice->CreateDepthStencilView(depthStencilBuffer, NULL, &gDepthStencilView);

		D3D11_DEPTH_STENCIL_DESC depthStencilDesc2;
		D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;

		// Initialize the description of the stencil state.
		ZeroMemory(&depthStencilDesc2, sizeof(depthStencilDesc2));

		// Set up the description of the stencil state.
		depthStencilDesc2.DepthEnable = true;
		depthStencilDesc2.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc2.DepthFunc = D3D11_COMPARISON_LESS;

		depthStencilDesc2.StencilEnable = true;
		depthStencilDesc2.StencilReadMask = 0xFF;
		depthStencilDesc2.StencilWriteMask = 0xFF;

		// Stencil operations if pixel is front-facing.
		depthStencilDesc2.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc2.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilDesc2.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc2.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing.
		depthStencilDesc2.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc2.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencilDesc2.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc2.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		HRESULT state;
		// Create the depth stencil state.
		state = gDevice->CreateDepthStencilState(&depthStencilDesc2, &m_depthStencilState);
		// Set the depth stencil state.
		gDeviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

		// Clear the second depth stencil state before setting the parameters.
		ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));

		// Now create a second depth stencil state which turns off the Z buffer for 2D rendering.  The only difference is 
		// that DepthEnable is set to false, all other parameters are the same as the other depth stencil state.
		depthDisabledStencilDesc.DepthEnable = false;
		depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
		depthDisabledStencilDesc.StencilEnable = true;
		depthDisabledStencilDesc.StencilReadMask = 0xFF;
		depthDisabledStencilDesc.StencilWriteMask = 0xFF;
		depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Create the state using the device.
		state = gDevice->CreateDepthStencilState(&depthDisabledStencilDesc, &m_depthDisabledStencilState);


		// set the render target as the back buffer
		gDeviceContext->OMSetRenderTargets(1, &gBackRufferRenderTargetView, gDepthStencilView);

		return true; //returnerar att den HAR klarat av att skapa device och swapchain
	}

	return false; //det gick inte att skapa device och swapchain, snyft :'(
}

// MESSAGE HANDLER

LRESULT RenderEngine::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch (msg){
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_CHAR: //en tangentbordsknapp har tryckts i
		switch (wParam){
		case VK_ESCAPE:
			PostQuitMessage(0);
			return 0;
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

// RUN PROGRAM

int RenderEngine::Run(){

	//om allt funkat:
	ShowWindow(hWindow, SW_SHOW);
	MSG msg = { 0 }; //töm alla platser i msg
	gTimer.Reset();

	// BoundingFrustum b(fpsCam.Proj());//frustum mot quadtree
	//frustum = b;
	//frustum.CreateFromMatrix(frustum, fpsCam.Proj());

	while (msg.message != WM_QUIT){
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else{ //applikationen är fortfarande igång
			gTimer.Tick();
			Update(0.0f);
			InputHandler();
			Render();
			fpscounter();
		}
	}
	return static_cast<int>(msg.wParam);
}

// RENDER

void RenderEngine::Render(){

	

	//Static Values
	static bool zoom;
	static  float rot;
	static  int clicktest;
	static int ShootScore;
	static int ltest;
	static  int hitIndex;

	//Increase rotation value per frame
	rot += 0.0003f;

	//Movement and camera rotation speed
	speed = ((gTimer.TotalTime() / gTimer.TotalTime()) / 200.0f + boost);

	//Matrix computation

	camRotationMatrix = XMMatrixRotationRollPitchYaw(camPitch, camYaw, 0);

	camTarget = XMVector3TransformCoord(DefaultForward, camRotationMatrix);
	camTarget = XMVector3Normalize(camTarget);

	camRight = XMVector3TransformCoord(DefaultRight, camRotationMatrix);
	camForward = XMVector3TransformCoord(DefaultForward, camRotationMatrix);
	camUp = XMVector3Cross(camForward, camRight);

	camPosition += moveLeftRight*camRight;
	camPosition += moveBackForward*camForward;

	moveLeftRight = 0.0f;
	moveBackForward = 0.0f;

	camTarget = camPosition + camTarget;

	Vector3 viewPoint = Vector3(camTarget.x, camTarget.y, camTarget.z);
	ViewP.viewPoint = viewPoint;
	ViewP.pad = 0;
	fpsCamLook = XMMatrixLookAtLH(camPosition, camTarget, camUp);

	

	//Update cam
	fpsCam.UpdateViewMatrix();
	//WORLD
	// Sets camera pos and angle
	CamView = fpsCamLook;
	CamProjection = fpsCam.Proj();
	XMMATRIX CamViewProjection = fpsCam.ViewProj();
	XMMATRIX identityM = XMMatrixIdentity();
	XMMATRIX WorldInv = XMMatrixInverse(nullptr, identityM);
	XMMATRIX WorldGun = identityM;


	// KEYBOARD AND MOUSE STUFF
	DIMOUSESTATE mouseCurrState;
	BYTE keyboardState[256];

	//Aquaire devices
	Keyboard->Acquire();
	Mouse->Acquire();

	//Get States
	Mouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);
	Keyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);

	////////////LIGHTS/////////////////////////////////////////////////////

	// Deafult Lights
	//Dir light1
	PrimaryLights.lDir.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.2f);
	PrimaryLights.lDir.Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	PrimaryLights.lDir.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 2.0f);
	PrimaryLights.lDir.Dir = XMFLOAT3(5.0f, -10.0f, 0.0f);
	PrimaryLights.lDir.Pad = 0.0f;

	////Padding
	MatBuffer1.pad = 0.0f;
	//Eyepos
	MatBuffer1.gEyePos = Vector3(camPosition.x, camPosition.y, camPosition.z);
	// Makes a deafualt mat
	MatBuffer1.gMaterial.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	MatBuffer1.gMaterial.Diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	MatBuffer1.gMaterial.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 3.0f);
	MatBuffer1.gMaterial.Reflection = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);

	//// Update lightbuffer

	gDeviceContext->UpdateSubresource(PrimaryLightBuffer, 0, NULL, &PrimaryLights, 0, 0);
	gDeviceContext->PSSetConstantBuffers(0, 1, &PrimaryLightBuffer);

	//// Update matbuffer

	gDeviceContext->UpdateSubresource(MatBuffer, 0, NULL, &MatBuffer1, 0, 0);
	gDeviceContext->PSSetConstantBuffers(1, 1, &MatBuffer);


	//STORE VALUES

	gDeviceContext->UpdateSubresource(ViewBuffer, 0, NULL, &ViewP, 0, 0);
	gDeviceContext->GSSetConstantBuffers(4, 1, &ViewBuffer);

	////////////LIGHTS/////////////////////////////////////////////////////

	 //frustum.Transform(frustum, XMMatrixTranspose(CamView)); //updaterar BoundingFrustum frustum varje frame<-------------------------- inverse eller transpose?
	 //CheckFrustumContains(nrSplitsTree, 0);


//	gDeviceContext->OMSetDepthStencilState(gDepthStencilState, 0);

	//Set no backface culling
	gDeviceContext->RSSetState(NULL);
	gDeviceContext->RSSetState(NoBcull);

	SetPosition(0, 6, 0);
	SetDirection(PrimaryLights.lDir.Dir.x, PrimaryLights.lDir.Dir.y, PrimaryLights.lDir.Dir.z);
	SetLookAt(-2.0f, -1.0f, 0.0f);
	// SHADOW DEPTH TO TEXTURE RENDERING
	XMMATRIX  lightViewMatrix, lightProjectionMatrix, lightOrthoMatrix;

	// SET TARGET AND DEPTHSTENCIL FOR DEPTH RENDER
	ID3D11RenderTargetView* renderTargetViewDepthMap[1] = { 0 };
	gDeviceContext->RSSetViewports(1, &shadowVP);
	gDeviceContext->OMSetRenderTargets(1, renderTargetViewDepthMap, depthStencilcDepthMap);
	gDeviceContext->ClearDepthStencilView(depthStencilcDepthMap, D3D11_CLEAR_DEPTH, 1.0f, 0.0f);

	//RENDER DEPTH BELOW


	/////

	//Set BackGround Color
	gDeviceContext->RSSetViewports(1, &vp);
	float clearColor[] = { 0, 0.3, 0.7f, 1.0f };
	gDeviceContext->ClearRenderTargetView(gBackRufferRenderTargetView, clearColor);
	gDeviceContext->ClearDepthStencilView(gDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	gDeviceContext->OMSetRenderTargets(1, &gBackRufferRenderTargetView, gDepthStencilView);

	int tex = 0;

	//Store matrixes into buffer
	XMStoreFloat4x4(&WorldMatrix1.View, XMMatrixTranspose(CamView));
	XMStoreFloat4x4(&WorldMatrix1.Projection, XMMatrixTranspose(CamProjection));
	XMStoreFloat4x4(&WorldMatrix1.WorldSpace, XMMatrixTranspose(identityM));
	XMStoreFloat4x4(&WorldMatrix1.InvWorld, XMMatrixTranspose(WorldInv));
	XMStoreFloat4x4(&WorldMatrix1.lightView, XMMatrixTranspose(mapView));
	XMStoreFloat4x4(&WorldMatrix1.lightProjection, XMMatrixTranspose(mapProjection));



	gDeviceContext->UpdateSubresource(gWorld, 0, NULL, &WorldMatrix1, 0, 0);
	gDeviceContext->VSSetConstantBuffers(0, 1, &gWorld);



	UINT32 vertexSize2 = sizeof(float) * 8;
	UINT32 offset2 = 0;

	tex = 0;
	gDeviceContext->IASetInputLayout(gVertexLayout);
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gDeviceContext->VSSetShader(dVertexShader, nullptr, 0);
	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	gDeviceContext->PSSetShader(dPixelShader, nullptr, 0);
	// Set the sampler states to the pixel shader.
	gDeviceContext->PSSetSamplers(0, 1, &sampState1);
	gDeviceContext->PSSetSamplers(1, 1, &sampState2);


	//BACKFACE CULLING
	//if (Bculling == TRUE)
	//else if (Bculling == FALSE)
	//	gDeviceContext->GSSetShader(nullptr, nullptr, 0);

	//gDeviceContext->GSSetShader(gBackFaceShader, nullptr, 0);
	gDeviceContext->PSSetShaderResources(1, 1, &shaderResourceDepthMap);

	for (int i = 0; i < renderObjects.size(); i++)
	{
		tex = intArrayTex[renderObjects[i]->indexT];
		gDeviceContext->PSSetShaderResources(0, 1, &RSWArray[tex]);
		gDeviceContext->IASetVertexBuffers(0, 1, &renderObjects[i]->vertexBuffer, &vertexSize2, &offset2);

		gDeviceContext->Draw(renderObjects[i]->nrElements * 3, 0);
	}

	//WIREFRAME!!!
	gDeviceContext->VSSetShader(gWireFrameVertexShader, nullptr, 0);
	gDeviceContext->PSSetShader(gWireFramePixelShader, nullptr, 0);
	gDeviceContext->IASetInputLayout(gWireFrameLayout);
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	UINT32 vertexWireFrameSize = sizeof(float) * 3;

	for (int i = 0; i < renderObjects.size(); i++)
	{
		//tex = intArrayTex[renderObjects[i]->indexT];
		gDeviceContext->IASetVertexBuffers(0, 1, &renderObjects[i]->boundingBoxVertexBuffer, &vertexWireFrameSize, &offset2);

		gDeviceContext->Draw(16, 0);
	}


	//////////////////////////// Draw Terrain Onto Map
	//// Here we will draw our map, which is just the terrain from the mapCam's view

	//// Set our maps Render Target
	//gDeviceContext->OMSetRenderTargets(1, &renderTargetViewMap, gDepthStencilView);

	//// Now clear the render target
	//gDeviceContext->ClearRenderTargetView(renderTargetViewMap, clearColor);
	//gDeviceContext->RSSetViewports(1, &shadowVP);
	//// Update the map's camera
	//XMVECTOR mapCamPosition = XMVectorSet(5.0f, 5.0f, 5.0f, 0.0f);
	//XMVECTOR mapCamTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	//XMVECTOR mapCamUp = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);


	////Set the View matrix
	//mapView = XMMatrixLookAtLH(mapCamPosition, mapCamTarget, mapCamUp);
	//mapProjection = XMMatrixOrthographicLH(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, 1.0f, 100.0f);
	//// Since we just drew the terrain, and all the states are already set the way we want them
	//// (besides the render target) we just need to provide the shaders with the new WVP and draw the terrain again
	//XMStoreFloat4x4(&WorldMatrix1.View, XMMatrixTranspose(CamView));
	//XMStoreFloat4x4(&WorldMatrix1.Projection, XMMatrixTranspose(CamProjection));
	//XMStoreFloat4x4(&WorldMatrix1.WorldSpace, XMMatrixTranspose(identityM));
	//XMStoreFloat4x4(&WorldMatrix1.InvWorld, XMMatrixTranspose(WorldInv));
	//XMStoreFloat4x4(&WorldMatrix1.lightView, XMMatrixTranspose(CamView));
	//XMStoreFloat4x4(&WorldMatrix1.lightProjection, XMMatrixTranspose(CamProjection));


	//XMStoreFloat4x4(&WorldMatrix1.WVP, XMMatrixTranspose(CamView*CamProjection*identityM));

	//gDeviceContext->UpdateSubresource(gWorld, 0, NULL, &WorldMatrix1, 0, 0);
	//gDeviceContext->VSSetConstantBuffers(0, 1, &gWorld);
	//UINT32 vertexSize3 = sizeof(float) * 8;
	//UINT32 offset3 = 0;

	//tex = 0;
	//gDeviceContext->IASetInputLayout(gVertexLayout);
	//gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//gDeviceContext->VSSetShader(dVertexShader, nullptr, 0);
	//gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	//gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	//gDeviceContext->PSSetShader(dPixelShader, nullptr, 0);
	//// Set the sampler states to the pixel shader.
	//gDeviceContext->PSSetSamplers(0, 1, &sampState1);
	//gDeviceContext->PSSetSamplers(1, 1, &sampState2);

	////BACKFACE CULLING
	////if (Bculling == TRUE)
	////else if (Bculling == FALSE)
	////	gDeviceContext->GSSetShader(nullptr, nullptr, 0);

	////gDeviceContext->GSSetShader(gBackFaceShader, nullptr, 0);

	//for (int i = 0; i < renderObjects.size(); i++)
	//{
	//	tex = intArrayTex[renderObjects[i]->indexT];
	//	gDeviceContext->PSSetShaderResources(0, 1, &RSWArray[tex]);
	//	gDeviceContext->IASetVertexBuffers(0, 1, &renderObjects[i]->vertexBuffer, &vertexSize3, &offset3);

	//	gDeviceContext->Draw(renderObjects[i]->nrElements * 3, 0);
	//}

	//// DRAW FROM MAP PERSPECTIVE



	//gDeviceContext->OMSetRenderTargets(1, &gBackRufferRenderTargetView, gDepthStencilView);

	////RENDER TestPlane 2 Tris 
	//UINT32 vertexSize = sizeof(float)* 8;
	//UINT32 offset = 0;
	// WorldGun = XMMatrixScaling(0.5f, 0.5f, 0.0f) * XMMatrixTranslation(0.5f, -0.5f, 0.0f);
	// XMStoreFloat4x4(&WorldMatrix1.WVP, XMMatrixTranspose(WorldGun));
	//gDeviceContext->UpdateSubresource(gWorld, 0, NULL, &WorldMatrix1, 0, 0);
	//gDeviceContext->VSSetConstantBuffers(0, 1, &gWorld);


	//gDeviceContext->IASetInputLayout(gVertexLayout);
	//gDeviceContext->IASetVertexBuffers(0, 1, &gVertexBuffer, &vertexSize, &offset);
	//gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//gDeviceContext->VSSetShader(shader2DVS, nullptr, 0);
	//gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	//gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	//gDeviceContext->PSSetShader(shader2DPS, nullptr, 0);
	//gDeviceContext->PSSetShaderResources(0, 1, &RSWArray[tex]);

	//gDeviceContext->Draw(4, 0);

	//växla back/front buffer
	gSwapChain->Present(0, 0); 
}

//SubRender that renders only, calls twice when shadows are rendered

// UPDATES

void RenderEngine::Update(float dt){
	
	

}

//REALESE AND CLEANUP

void RenderEngine::Release(){

	gDevice->Release();
	depthStencilBuffer->Release();

	m_Input->Shutdown();
	delete m_Input;
	m_Input = 0;

	gVertexBuffer->Release();
	gVertexLayout->Release();
	gVertexShader->Release();
	gPixelShader->Release();
	gDeviceContext->Release();
	NoBcull->Release();
	PrimaryLightBuffer->Release();
	
}

//IMPORT OBJ

void RenderEngine::ImportObj(char* geometryFileName, char* materialFileName, ID3D11Device* gDev){// , bool isStatic, XMMATRIX startPosMatrix){
	static int gameObjectIndex = 0;
	OBJ objectTest(gDev);
	//Load obj
	objectTest.LoadObject(geometryFileName, materialFileName);

	//Test if filename is correct
	OutputDebugStringA(geometryFileName);
	OutputDebugStringA("\n");
	OutputDebugStringA(materialFileName);
	OutputDebugStringA("\n");

	GameObject testObject;
	testObject.ObjName = objectTest.GetName();
	//Fill buffers
	testObject.gameObjectIndex = gameObjectIndex; //används förtillfället vid frustum contains checken
	gameObjectIndex++;
	testObject.gVertexBuffer = *objectTest.GetVertexBuffer();
	testObject.nrElements = objectTest.GetNrElements();

	testObject.verteciesPos = objectTest.GetVertexPositions();
	testObject.verteciesIndex = objectTest.GetIndecies();
	testObject.verteciesPosX = objectTest.GetVerticiesX();
	testObject.verteciesPosY = objectTest.GetVerticiesY();
	testObject.verteciesPosZ = objectTest.GetVerticiesZ();

	//testObject.isStatic = isStatic;
	//testObject.SetBoundingBoxStartPosition(startPosMatrix, gDevice); //gDevice skickas eftersom jag samtidigt skapar boxens geometri för att kunna renderas
	//Pushback vector array
	gameObjects.push_back(testObject);

}

//IMPORT HEIGHTMAPS

//void RenderEngine::ImportHeightmap(char* HeightMapFileName, wstring tex1File, wstring tex2File, wstring tex3File, wstring texSplatFile){
//	HeightMap ImportedHM(gDevice, gDeviceContext);
//
//	ImportedHM.LoadHeightMap(HeightMapFileName);
//	ImportedHM.LoadSplatMap(tex1File, tex2File, tex3File, texSplatFile);
//
//	HeightMapObject cHeightMap;
//
//	cHeightMap.gIndexBuffer = ImportedHM.GetIndexBuffer();
//	cHeightMap.gVertexBuffer = ImportedHM.GetVertexBuffer();
//	cHeightMap.nmrElement = ImportedHM.GetNrElements();
//	cHeightMap.gridSize = ImportedHM.GetGridSize();
//	cHeightMap.vertexHeights = ImportedHM.GetHeights();
//	cHeightMap.tex1shaderResourceView = ImportedHM.GetTex1();
//	cHeightMap.tex2shaderResourceView = ImportedHM.GetTex2();
//	cHeightMap.tex3shaderResourceView = ImportedHM.GetTex3();
//	cHeightMap.splatshaderResourceView = ImportedHM.GetSplatTex();
//
//	HeightMapObjects.push_back(cHeightMap);
//
//
//
//}

//PICKING FUNCTIONS

void RenderEngine::TestInterSection(float mouseX, float mouseY, Vector4& pickRayInWorldSpacePos, Vector4& pickRayInWorldSpaceDir)
{

	
	Vector4 pickRayInViewSpaceDir = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	Vector4 pickRayInViewSpacePos = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

		//Pick ray vecs
		float PRVecX, PRVecY, PRVecZ;

		XMMATRIX CamProj = CamProjection;
		//Transform 2D pick position on screen space to 3D ray in View space to get correct intersections
		PRVecX = (((2.0f * mouseX) / screen_Width) - 1) / XMVectorGetX(CamProj.r[0]);
		PRVecY = -(((2.0f * mouseY) / screen_Height) - 1) / XMVectorGetY(CamProj.r[0]);
		PRVecZ = 1.0f;	//View space's Z direction ranges from 0 to 1, we set 1 to get the ray to go "into the screen"

		pickRayInViewSpaceDir = XMVectorSet(PRVecX, PRVecY, PRVecZ, 0.0f);

		//Uncomment this line if you want to use the center of the screen (client area) , this is for fps games
		pickRayInViewSpaceDir = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

		// Transform 3D Ray from View space to 3D ray in World space
		XMMATRIX pickRayToWorldSpaceMatrix;
	
		//Inverse of View Space matrix is World space matrix
		pickRayToWorldSpaceMatrix = XMMatrixInverse(nullptr, CamView);

		pickRayInWorldSpacePos = XMVector3TransformCoord(pickRayInViewSpacePos, pickRayToWorldSpaceMatrix);
		pickRayInWorldSpaceDir = XMVector3TransformNormal(pickRayInViewSpaceDir, pickRayToWorldSpaceMatrix);
	


}

bool RenderEngine::PointInTriangle(Vector4& triV1, Vector4& triV2, Vector4& triV3, Vector4& point)
{
	//To find out if the point is inside the triangle, we will check to see if the point is on the correct side of each of the triangles edges, with other words , we check if the ray hits inside the triangle or not 

	Vector4 cp1 = XMVector3Cross((triV3 - triV2), (point - triV2));
	Vector4 cp2 = XMVector3Cross((triV3 - triV2), (triV1 - triV2));
	if (XMVectorGetX(XMVector3Dot(cp1, cp2)) >= 0)
	{
		cp1 = XMVector3Cross((triV3 - triV1), (point - triV1));
		cp2 = XMVector3Cross((triV3 - triV1), (triV2 - triV1));
		if (XMVectorGetX(XMVector3Dot(cp1, cp2)) >= 0)
		{
			cp1 = XMVector3Cross((triV2 - triV1), (point - triV1));
			cp2 = XMVector3Cross((triV2 - triV1), (triV3 - triV1));
			if (XMVectorGetX(XMVector3Dot(cp1, cp2)) >= 0)
			{
				return true;
			}
			else
				return false;
		}
		else
			return false;
	}
	return false;
}

float RenderEngine::pick(Vector4 pickRayInWorldSpacePos, Vector4 pickRayInWorldSpaceDir, std::vector<XMFLOAT3>& vertPosArray, std::vector<int>& indexPosArray, XMMATRIX& worldSpace)
{
	//Loop through each triangle in the object
	for (int i = 0; i < indexPosArray.size() / 3; i++)
	{
		
			//Triangle's vertices V1, V2, V3
			Vector4 tri1V1 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			Vector4 tri1V2 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			Vector4 tri1V3 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

			//Temporary 3d floats for each vertex
			XMFLOAT3 tV1, tV2, tV3;

			//Get triangle 
			tV1 = vertPosArray[indexPosArray[(i * 3) + 0]];


			tV2 = vertPosArray[indexPosArray[(i * 3) + 1]];

			tV3 = vertPosArray[indexPosArray[(i * 3) + 2]];


			tri1V1 = XMVectorSet(tV1.x, tV1.y, tV1.z, 0.0f);
			tri1V2 = XMVectorSet(tV2.x, tV2.y, tV2.z, 0.0f);
			tri1V3 = XMVectorSet(tV3.x, tV3.y, tV3.z, 0.0f);

			//Transform the vertices to world space
			tri1V1 = XMVector3TransformCoord(tri1V1, worldSpace);
			tri1V2 = XMVector3TransformCoord(tri1V2, worldSpace);
			tri1V3 = XMVector3TransformCoord(tri1V3, worldSpace);

			//Find the normal using U, V coordinates (two edges)
			Vector4 U = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			Vector4 V = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			Vector4 faceNormal = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

			U = tri1V2 - tri1V1;
			V = tri1V3 - tri1V1;

			//Compute face normal by crossing U, V
			faceNormal = XMVector3Cross(U, V);

			faceNormal = XMVector3Normalize(faceNormal);

			//Calculate a point on the triangle for the plane equation
			Vector4 triPoint = tri1V1;

			//Get plane equation ("Ax + By + Cz + D = 0") Variables
			float tri1A = XMVectorGetX(faceNormal);
			float tri1B = XMVectorGetY(faceNormal);
			float tri1C = XMVectorGetZ(faceNormal);
			float tri1D = (-tri1A*XMVectorGetX(triPoint) - tri1B*XMVectorGetY(triPoint) - tri1C*XMVectorGetZ(triPoint));

			//Now we find where (on the ray) the ray intersects with the triangles plane
			float ep1, ep2, t = 0.0f;
			float planeIntersectX, planeIntersectY, planeIntersectZ = 0.0f;
			Vector4 pointInPlane = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

			ep1 = (XMVectorGetX(pickRayInWorldSpacePos) * tri1A) + (XMVectorGetY(pickRayInWorldSpacePos) * tri1B) + (XMVectorGetZ(pickRayInWorldSpacePos) * tri1C);
			ep2 = (XMVectorGetX(pickRayInWorldSpaceDir) * tri1A) + (XMVectorGetY(pickRayInWorldSpaceDir) * tri1B) + (XMVectorGetZ(pickRayInWorldSpaceDir) * tri1C);

			//Make sure there are no divide by 0 , we dont want crashes do we?
			if (ep2 != 0.0f)
				t = -(ep1 + tri1D) / (ep2);

			if (t > 0.0f)    //Make sure you don't pick objects behind the camera if z is lesser the n 0 then its behind
			{
				//Get the point on the plane
				planeIntersectX = XMVectorGetX(pickRayInWorldSpacePos) + XMVectorGetX(pickRayInWorldSpaceDir) * t;
				planeIntersectY = XMVectorGetY(pickRayInWorldSpacePos) + XMVectorGetY(pickRayInWorldSpaceDir) * t;
				planeIntersectZ = XMVectorGetZ(pickRayInWorldSpacePos) + XMVectorGetZ(pickRayInWorldSpaceDir) * t;

				pointInPlane = XMVectorSet(planeIntersectX, planeIntersectY, planeIntersectZ, 0.0f);

				//Call function to check if point is in the triangle
				if (PointInTriangle(tri1V1, tri1V2, tri1V3, pointInPlane))
				{
					//Return the distance to the hit, so you can check all the other pickable objects in your scene
					//and choose whichever object is closest to the camera
					return t / 2.0f;
				}
			}
		}
		//return the max float value (near infinity) if an object was not picked
		return FLT_MAX;
	
}

//Input handler for Mouse and Keyboard

bool RenderEngine::InitDirectInput(HINSTANCE hInstance)
{
	HRESULT hr;
	hr = DirectInput8Create(hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&DirectInput,
		NULL);

	hr = DirectInput->CreateDevice(GUID_SysKeyboard,
		&Keyboard,
		NULL);

	hr = DirectInput->CreateDevice(GUID_SysMouse,
		&Mouse,
		NULL);

	hr = Keyboard->SetDataFormat(&c_dfDIKeyboard);
	hr = Keyboard->SetCooperativeLevel(hWindow, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	hr = Mouse->SetDataFormat(&c_dfDIMouse);
	hr = Mouse->SetCooperativeLevel(hWindow, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);

	return true;
}

//INPUT HANDLER

void RenderEngine::InputHandler()
{
	//Keyboard and mouse handlers
	DIMOUSESTATE mouseCurrState;
	BYTE keyboardState[256];

	//Aquaire devices
	Keyboard->Acquire();
	Mouse->Acquire();

	//Get States
	Mouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);
	Keyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);

	
	//Boost to move faster
	if (keyboardState[DIK_LSHIFT] & 0x80)
	{
		boost += 0.00002;
	}
	else
		boost = 0;

	//Keyboard and mouse interaction


	if ((keyboardState[DIK_E] & 0x80) && zoom != 1)
	{
		camPosition = camPosition + Vector4(0.1f, 0.2f, 0.45f, 0);
		//GunPitch += 0.15f;
		//GunRotationMatrix = XMMatrixRotationX(40);
		zoom = 1;

	}
	if ((keyboardState[DIK_R] & 0x80) && zoom == 1)
	{
		camPosition = camPosition - Vector4(0.1f, 0.2f, 0.45f, 0);
		//GunPitch -=   0.15f;
		//GunRotationMatrix = XMMatrixRotationY(10);
		zoom = 0;

	}

	if ((keyboardState[DIK_B] & 0x80) && Bcullingcheck == FALSE)
	{
		Bculling = TRUE;
		Bcullingcheck = TRUE;
	}
	if ((keyboardState[DIK_V] & 0x80) && Bcullingcheck == TRUE)
	{
		Bculling = FALSE;
		Bcullingcheck = FALSE;
	}

	if (keyboardState[DIK_A] & 0x80)
	{
		moveLeftRight -= speed;
	}
	if (keyboardState[DIK_D] & 0x80)
	{
		moveLeftRight += speed;
	}
	if (keyboardState[DIK_W] & 0x80)
	{
		moveBackForward += speed;

	}
	if (keyboardState[DIK_S] & 0x80)
	{
		moveBackForward -= speed;
	}
	if ((mouseCurrState.lX != mouseLastState.lX) || (mouseCurrState.lY != mouseLastState.lY))
	{
		camYaw += mouseCurrState.lX * speed / 4;;

		camPitch += mouseCurrState.lY * speed / 4;

		mouseLastState = mouseCurrState;
	}



}

// Quad Tree

void RenderEngine::ListQuadTree(int nrSplits, XMFLOAT3 center, XMFLOAT3 extents){

	BoundingBox b(center, extents);

	QuadTreeInstance lB;
	lB.SetValues(b, gDevice);
	if (nrSplits <= 0) //om det är längst ner i trädet så innebär det att denne ska innehålla gameobjects (förutsatt att det finns några där :- ))
		lB.TestContains(gameObjects); //testa endast mot de längst ner ifall det finns några gameobjects i dem

	quadTree.push_back(lB);
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
			ListQuadTree(newNrSplits, child.Center, child.Extents);
		}
	}
}

//Check Frustum

void RenderEngine::CheckFrustumContains(int nrSplits, int index){
	ContainmentType test = frustum.Contains(quadTree[index].box);
	if (test == 2 || test == 1){ //hit på boxen, contains ELLER intersects
		if (nrSplits > 0){
			int newNrSplits = nrSplits - 1;
			for (int i = 0; i < 4; i++){
				index += 1;
				CheckFrustumContains(newNrSplits, index);
			}
		}
		else{ //botten på trädet
			int objectIndex = 0;
			for each (GameObject var in quadTree[index].gameObjectsToRender)//vad skall göras ifall den ena boxen säger att objektet ska renderas men den andra inte?
			{
				test = frustum.Contains(var.bbox);
				if (test == 2 || test == 1){ //hit på objektets box
					//var.render = true; //RENDERA!
					//var.visibleThisFrame = true; //ÄR "var" BARA EN TEMPORÄR VARIABEL?????????
					//quadTree[index].gameObjectsToRender[objectIndex].render = true;
					//quadTree[index].gameObjectsToRender[objectIndex].visibleThisFrame = true;
					gameObjects[var.gameObjectIndex].visibleThisFrame = true;
					gameObjects[var.gameObjectIndex].render = true;
				}
				else if (gameObjects[var.gameObjectIndex].visibleThisFrame == false){ //om ingen annan box har sagt att denna ska renderas denna framen
					//var.render = false; //LIGGER INTE I FRUSTUMET
					gameObjects[var.gameObjectIndex].render = false;
					//quadTree[index].gameObjectsToRender[objectIndex].render = false;
				}
				objectIndex++;
			}
		}
	}
}







//========================================================================\\
//=============================OLD RENDER STUFF===========================\\
//========================================================================\\
//                                    |
//									  |
//									  v





//////////////////////////////////////////////////////////
///////////////// RENDER FUNCTIONTS //////////////////
//////////////////////////////////////////////////////////

//UINT32 bufferElementSize = sizeof(Float3);
//UINT32 offset1 = 0;

//gDeviceContext->IASetInputLayout(gWireFrameLayout);
//gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

//gDeviceContext->VSSetShader(gWireFrameVertexShader, nullptr, 0);
//gDeviceContext->HSSetShader(nullptr, nullptr, 0);
//gDeviceContext->DSSetShader(nullptr, nullptr, 0);
//gDeviceContext->GSSetShader(nullptr, nullptr, 0);
//gDeviceContext->PSSetShader(gWireFramePixelShader, nullptr, 0);
//for each(QuadTreeInstance box in quadTree){
//	ContainmentType test = frustum.Contains(box.box);
//	if (test == 2 || test == 1)
//		gDeviceContext->PSSetShader(gWireFramePixelShader, nullptr, 0);
//	else
//		gDeviceContext->PSSetShader(gWireFramePixelShaderCONTAINTEST, nullptr, 0);
//	gDeviceContext->IASetVertexBuffers(0, 1, &box.boxBuffer, &bufferElementSize, &offset1);
//	gDeviceContext->Draw(16, 0);
//}

//RENDER OBJ FILES
//
//for each (GameObject var in gameObjects)
//{
//	UINT32 vertexSize = sizeof(float) * 8;
//	UINT32 offset = 0;
//	count += 1;
//
//	if (count == 1){
//		gDeviceContext->PSSetShaderResources(0, 1, &ddsTex1);
//	}
//	else if (count == 2){
//		gDeviceContext->PSSetShaderResources(0, 1, &ddsTex3);
//	}
//	else if (count == 3){
//		gDeviceContext->PSSetShaderResources(0, 1, &ddsTex3);
//	}
//	else if (count == 4){
//		gDeviceContext->PSSetShaderResources(0, 1, &NpcRV);
//	}
//
//	gDeviceContext->IASetInputLayout(gVertexLayout);
//	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	gDeviceContext->IASetVertexBuffers(0, 1, &var.gVertexBuffer, &vertexSize, &offset);
//	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	gDeviceContext->VSSetShader(gVertexShader, nullptr, 0);
//	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
//	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
//	if (Bculling == TRUE)
//		gDeviceContext->GSSetShader(gBackFaceShader, nullptr, 0);
//	else if (Bculling == FALSE)
//		gDeviceContext->GSSetShader(nullptr, nullptr, 0);
//	gDeviceContext->PSSetShader(gPixelShader, nullptr, 0);
//
//	if ((GetKeyState(VK_RBUTTON) & 0x100) != 0 && clicktest == 1)
//	{
//		clicktest = 0;
//		hit = FALSE;
//	}
//
//	if ((GetKeyState(VK_LBUTTON) & 0x100) != 0 && clicktest != 1)
//	{
//		float tempDist;
//		float closestDist = FLT_MAX;
//
//		Vector4 prwsPos, prwsDir;
//		TestInterSection(screen_Width / 2, screen_Height / 2, prwsPos, prwsDir);
//		tempDist = pick(prwsPos, prwsDir, var.verteciesPos, var.verteciesIndex, identityM);
//		ShootScore++;
//		if (tempDist < closestDist)
//		{
//			closestDist = tempDist;
//			hitIndex += 1;
//			_RPT1(0, "Clicks : %d \n", ShootScore);
//			_RPT1(0, "Hits : %d \n", hitIndex);
//			_RPT1(0, "Closest dist: %f \n", closestDist);
//			_RPT1(0, "Obj Name : %s \n", var.ObjName);
//			hit = TRUE;
//		}
//
//		clicktest = 1;
//		if (hit == TRUE){
//			gDeviceContext->PSSetShader(hitPixelShader, nullptr, 0);
//		}
//	}
//
	//if (var.isStatic == true){ //statiska objekt ska endast renderas om de är inom frustumet
	//	if (var.render == true){
	//		var.visibleThisFrame = false;//reseta denna, används vid frustum checken. (om ingen annan box har sagt att denna ska renderas denna framen, nästa box får inte ändra resultatet)
	//		//var.render = false;				
	//		gDeviceContext->Draw(var.nrElements * 3, 0);

	//	}

	//}
	//else
//	gDeviceContext->Draw(var.nrElements * 3, 0);
//
//}

//Render Heightmap´s

//BOXARNA TILL GAMEOBJECTSEN.................................
/*bufferElementSize = sizeof(Float3);
offset1 = 0;

gDeviceContext->IASetInputLayout(gWireFrameLayout);
gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

gDeviceContext->VSSetShader(gWireFrameVertexShader, nullptr, 0);
gDeviceContext->HSSetShader(nullptr, nullptr, 0);
gDeviceContext->DSSetShader(nullptr, nullptr, 0);
gDeviceContext->GSSetShader(nullptr, nullptr, 0);
gDeviceContext->PSSetShader(gWireFramePixelShader, nullptr, 0);
for each (GameObject var in gameObjects)
{
ContainmentType test = frustum.Contains(var.bbox);
if (test == 2 || test == 1)
gDeviceContext->PSSetShader(gWireFramePixelShader, nullptr, 0);
else
gDeviceContext->PSSetShader(gWireFramePixelShaderCONTAINTEST, nullptr, 0);
gDeviceContext->IASetVertexBuffers(0, 1, &var.boxBuffer, &bufferElementSize, &offset1);
gDeviceContext->Draw(16, 0);
}
*/

//Render Heightmap´s

//for each (HeightMapObject var in HeightMapObjects)
//{
//	UINT32 vertexSize = sizeof(float)* 8;
//	UINT32 offset = 0;

//	gDeviceContext->IASetInputLayout(gVertexLayout);
//	gDeviceContext->IASetVertexBuffers(0, 1, &var.gVertexBuffer, &vertexSize, &offset);
//	gDeviceContext->IASetIndexBuffer(var.gIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
//	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

//	gDeviceContext->VSSetShader(gVertexShader, nullptr, 0);
//	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
//	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
//	gDeviceContext->GSSetShader(nullptr, nullptr, 0);
//	//gDeviceContext->PSSetShader(gPixelShader, nullptr, 0);
//	gDeviceContext->PSSetShader(splatMapPixelShader, nullptr, 0);
//	//Bind texture to object
//	//gDeviceContext->PSSetShaderResources(0, 1, &ddsTex1);
//	gDeviceContext->PSSetShaderResources(0, 1, &var.tex1shaderResourceView);
//	gDeviceContext->PSSetShaderResources(1, 1, &var.tex2shaderResourceView);
//	gDeviceContext->PSSetShaderResources(2, 1, &var.tex3shaderResourceView);
//	gDeviceContext->PSSetShaderResources(3, 1, &var.splatshaderResourceView);

//	//gDeviceContext->PSSetShaderResources(0, 1, &var.splatshaderResourceView);

//	gDeviceContext->DrawIndexed((var.nmrElement), 0, 0);
//}

//RENDER TestPlane 2 Tris 
//UINT32 vertexSize = sizeof(float)* 8;
//UINT32 offset = 0;

//gDeviceContext->IASetInputLayout(gVertexLayout);
//gDeviceContext->IASetVertexBuffers(0, 1, &gVertexBuffer, &vertexSize, &offset);
//gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

//gDeviceContext->VSSetShader(gVertexShader, nullptr, 0);
//gDeviceContext->HSSetShader(nullptr, nullptr, 0);
//gDeviceContext->DSSetShader(nullptr, nullptr, 0);
//gDeviceContext->GSSetShader(nullptr, nullptr, 0);
//gDeviceContext->PSSetShader(gPixelShader, nullptr, 0);

//gDeviceContext->Draw(4, 0);



// %%%%%%%%%%%%%%%%%% BEFORE SHADOWS BINARY FORMAT RENDER %%%%%%%%%%%%%%%%%%%%%

//
//UINT32 vertexSize2 = sizeof(float) * 8;
//UINT32 offset2 = 0;
//int tex = 0;
//gDeviceContext->IASetInputLayout(gVertexLayout);
//gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//if (Bculling == TRUE)
//gDeviceContext->GSSetShader(gBackFaceShader, nullptr, 0);
//else if (Bculling == FALSE)
//gDeviceContext->GSSetShader(nullptr, nullptr, 0);
//gDeviceContext->VSSetShader(gVertexShader, nullptr, 0);
//gDeviceContext->PSSetShader(gPixelShader, nullptr, 0);
//gDeviceContext->PSSetSamplers(0, 1, &sampState1);
//for (int i = 0; i < renderObjects.size(); i++)
//{
//
//	tex = intArrayTex[renderObjects[i]->indexT];
//	gDeviceContext->PSSetShaderResources(0, 1, &RSWArray[tex]);
//	gDeviceContext->IASetVertexBuffers(0, 1, &renderObjects[i]->vertexBuffer, &vertexSize2, &offset2);
//
//
//	gDeviceContext->VSSetConstantBuffers(0, 1, &gWorld);
//
//	gDeviceContext->Draw(renderObjects[i]->nrElements * 3, 0);
//}
//
//
////PointLight 1
//PrimaryLights.lPoint.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
//PrimaryLights.lPoint.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
//PrimaryLights.lPoint.Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.2f);
//PrimaryLights.lPoint.Pos = XMFLOAT3(0.0f, 5.0f, 0.0f);
//PrimaryLights.lPoint.Range = 1.2f;
//PrimaryLights.lPoint.Attenuation = XMFLOAT3(0.01f, 0.1f, 0.1f);
////PointLight 2
//PrimaryLights.lPoint1.Ambient = XMFLOAT4(0.2f, 0.9f, 0.2f, 1.0f);
//PrimaryLights.lPoint1.Diffuse = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
//PrimaryLights.lPoint1.Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.2f);
//PrimaryLights.lPoint1.Pos = XMFLOAT3(8.57735f, 1.57735f, 2.07735f);
//PrimaryLights.lPoint1.Range = 0.0f;
//PrimaryLights.lPoint1.Attenuation = XMFLOAT3(0.01f, 0.001f, 0.001f);
////SpotLight 1
//PrimaryLights.lSpot.Ambient = XMFLOAT4(0.2f, 0.2f, 0.9f, 1.0f);
//PrimaryLights.lSpot.Diffuse = XMFLOAT4(0.4f, 0.3f, 0.3f, 1.0f);
//PrimaryLights.lSpot.Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.2f);
//PrimaryLights.lSpot.Dir = XMFLOAT3(0.0, 0.0f, 1.0f);
//PrimaryLights.lSpot.Attenuation = XMFLOAT3(0.1f, 0.1f, 0.1f);
//PrimaryLights.lSpot.Pos = XMFLOAT3(1.0f, 2.0f, 2.0f);
//PrimaryLights.lSpot.Range = 1000.0f;
//PrimaryLights.lSpot.Spot = 1.0f;


//SHADOW CALCS I THANK?

void RenderEngine::SetPosition(float x, float y, float z)
{
	m_position = Vector3(x, y, z);
	return;
}
//The SetLookAt function sets the m_lookAt vector so that we can set where the light is looking at.This vector is used to build the light's view matrix.

void RenderEngine::SetLookAt(float x, float y, float z)
{
	m_lookAt.x = x;
	m_lookAt.y = y;
	m_lookAt.z = z;
	return;
}



XMFLOAT3 RenderEngine::GetPosition()
{
	return m_position;
}
//The view matrix for the light is built using the up vector, the lookAt vector, and the position of the light.

void RenderEngine::GenerateViewMatrix(XMVECTOR view)
{
	Vector3 up;
	Vector3 lookatt;

	// Setup the vector that points upwards.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	// Create the view matrix from the three vectors.
	m_viewMatrix = XMMatrixLookAtLH(m_position, m_lookAt, up);

	return;
}
//The projection matrix for the light is built using the field of view,
//viewing aspect ratio, and the near and far plane of the light range.
//The light we are projecting is more of a square spotlight than a true point light, 
//but this is necessary since we need to align with the sampling from a square shadow map texture.That is why the field of view and aspect ratio are setup for a square projection.

void RenderEngine::GenerateProjectionMatrix(float screenDepth, float screenNear)
{
	float fieldOfView, screenAspect;


	// Setup field of view and screen aspect for a square light source.
	fieldOfView = (float)3.14f / 2.0f;
	screenAspect = 1.0f;

	// Create the projection matrix for the light.
	 m_projectionMatrix = XMMatrixPerspectiveFovLH( fieldOfView, screenAspect, screenNear, screenDepth);

	return;
}
//We also have two new functions to return the view and projection matrices.

void RenderEngine::GetViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
	return;
}


void RenderEngine::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
	return;
}

void RenderEngine::GenerateOrthoMatrix(float width, float depthPlane, float nearPlane)
{
	// Create the orthographic matrix for the light.
	m_orthoMatrix=XMMatrixOrthographicLH( width, width, nearPlane, depthPlane);

	return;
}


void RenderEngine::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
	return;
}
//Helper functions for directional lighting have also been added.

void RenderEngine::SetDirection(float x, float y, float z)
{
	m_direction = Vector3(x, y, z);
	return;
}


XMFLOAT3 RenderEngine::GetDirection()
{
	return m_direction;
}

void RenderEngine::TurnZBufferOn()
{
	gDeviceContext->OMSetDepthStencilState(m_depthStencilState, 1);
	return;
}


void RenderEngine::TurnZBufferOff()
{
	gDeviceContext->OMSetDepthStencilState(m_depthDisabledStencilState, 1);
	return;
}