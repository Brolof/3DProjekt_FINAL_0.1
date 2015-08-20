
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
	RenderEngine::Release();
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

	HRESULT inputtest;


	CoInitialize(nullptr);

	if (!InitWindow()){
		return false; //gick inte att skapa window
	}

	if (!InitDirect3D(hWindow)){
		return false; //gick inte att skapa Direct3D
	}
	// Sets and creates viewport
	SetViewport();
	
	//LIGHT
	//Deafult Lights
	//Dir light1                    PLACE IN OWN FUNCTION LATER
	PrimaryLights.lDir.Ambient = XMFLOAT4(0.12f, 0.12f, 0.12f, 0.2f);
	PrimaryLights.lDir.Diffuse = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);
	PrimaryLights.lDir.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 2.0f);
	PrimaryLights.lDir.Dir = XMFLOAT3(0.0f, -10.0f, 0.8f);
	PrimaryLights.lDir.Pad = 0.0f;
	////////////LIGHTS/////////////////////////////////////////////////////

	// Deafult Lights
	////Padding
	MatBuffer1.pad = 0.0f;
	//Eyepos
	MatBuffer1.gEyePos = Vector3(camPosition.x, camPosition.y, camPosition.z);
	// Makes a deafualt mat
	MatBuffer1.gMaterial.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	MatBuffer1.gMaterial.Diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	MatBuffer1.gMaterial.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 3.0f);
	MatBuffer1.gMaterial.Reflection = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	//

	CreatePlaneDataAndBuffers();

	//Set Camera values
	//fpsCam.SetPosition(0.0f, 0.4f, -6.0f);
	camPosition = Vector4(0.0f, 10.70f, 0.0f, 0.0f);
	fpsCam.SetLens(0.25f*3.14f, screen_Width / screen_Height, 1.0f, 100.0f);

	//Shadow map init
	shadowMap = new ShadowMap;
	shadowMap->SetShadowMapConstants(1024, 50.0f, 1.0f, 2, 50.0f);
	shadowMap->CreateShadowMap(gDevice, PrimaryLights.lDir, XMFLOAT4(0.0f, 20.0f, -10.0f, 0.0f));

	//Initialize Shaders and triangle data
	Shaders();
	BlendStates();
	InitDirectInput(hInstance);
	theCustomImporter.ImportBIN(gDevice, "Geometry/testFile.bin");
	intArrayTex = theCustomImporter.GetindexArray();
	//GET OBJECTS FROM IMPORTER
	renderObjects = theCustomImporter.GetObjects();
	transparentObjects = theCustomImporter.GetTransparentObjects();
	TextureFunc();
	
	for (int i = 0; i < renderObjects.size(); i++) //skapar boundingboxar för objecten
	{
		renderObjects[i]->CreateBBOXVertexBuffer(gDevice);
	}

	quadTree = new QuadTree(renderObjects, 2, gDevice, XMFLOAT3(10, 10, 10));
	glow = new Glow(gDevice, gDeviceContext, screen_Width, screen_Height, glowVertexShader, glowPixelShader, glowBlob);

	
	ImportHeightmap("Textures/JäkligtFinHeightmap.bmp", L"Textures/stone_texture1.dds", L"Textures/happy-smug-sloth.dds", L"Textures/sky_textureball.dds", L"Textures/splatmap_texture.png");

	// CREATE INPUT OBJECT
	inputtest = m_Input->Initialize(hInstance, hWindow, screen_Width, screen_Height);

	if (inputtest==0) 
	{
		MessageBox(hWindow, "Could not initialize the input object.", "Error", MB_OK);
		return false;
	}

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
	vp.Width = (float)screen_Width;
	vp.Height = (float)screen_Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

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


		outs.precision(6);
		outs << mainwname << L" "
			<< L"        FPS: " << fps << L" "
			<< L"        Frame Time: " << mspf << L" (ms)"
			<< L"        Time: " << timer << L" sec";
	
		//Prints the text in the window handler
		SetWindowText(hWindow, "Shadows, BF-Culling, Normal Mapping, BinImport, ");// outs.str().c_str());

		// Reset for next fps.
		framecount = 0;
		time += 0.50f;

	}
}

// CREATE TEXTURES

void RenderEngine::TextureFunc(){

	HRESULT texCheck;
	texCheck = CreateWICTextureFromFile(gDevice, L"Textures//maleNormalmap2.jpg", nullptr, &normalMap);
	texCheck = CreateDDSTextureFromFile(gDevice, L"Textures/normalTest.jpg", nullptr, &ddsTex1);
	texCheck = CreateDDSTextureFromFile(gDevice, L"Textures/shark.dds", nullptr, &ddsTex2);
	texCheck = CreateDDSTextureFromFile(gDevice, L"Textures/stone.dds", nullptr, &AdsResourceView);
	texCheck = CreateDDSTextureFromFile(gDevice, L"Textures/NPC.dds", nullptr, &NpcRV);
	texCheck = CreateWICTextureFromFile(gDevice, L"Textures/normalTest.jpg", nullptr, &ddsTex3);

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

HRESULT RenderEngine::CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob)
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

	HRESULT ShaderTest = 0;
	
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
	ID3DBlob* layoutblobl = nullptr;

	//create vertex shaders
	ID3DBlob* pVS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/deafultVS.hlsl", "VS_main", "vs_5_0", &pVS);
	KUK = gDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &gVertexShader);

	//create vertex depth
	ID3DBlob* dVS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/depthVS.hlsl", "VS_main", "vs_5_0", &dVS);
	ShaderTest = gDevice->CreateVertexShader(dVS->GetBufferPointer(), dVS->GetBufferSize(), nullptr, &dVertexShader);
	
	//create vertex texture
	ID3DBlob* ddVS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/deafultTexVS.hlsl", "VS_main", "vs_5_0", &ddVS);
	ShaderTest = gDevice->CreateVertexShader(ddVS->GetBufferPointer(), ddVS->GetBufferSize(), nullptr, &shader2DVS);

	ID3DBlob* dVS2 = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/RGBSplatmapVS.hlsl", "main", "vs_5_0", &dVS2);
	ShaderTest = gDevice->CreateVertexShader(dVS2->GetBufferPointer(), dVS2->GetBufferSize(), nullptr, &splatMapVertexShader);

	//GLOW SHADERS
	ShaderTest = CompileShader(L"FX_HLSL/GlowVS.hlsl", "main", "vs_5_0", &glowBlob);
	ShaderTest = gDevice->CreateVertexShader(glowBlob->GetBufferPointer(), glowBlob->GetBufferSize(), nullptr, &glowVertexShader);

	ID3DBlob* pSVS3 = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/HorizontalBlurVS.hlsl", "main", "vs_5_0", &pSVS3);
	ShaderTest = gDevice->CreateVertexShader(pSVS3->GetBufferPointer(), pSVS3->GetBufferSize(), nullptr, &horizontalBlurVertexShader);

	ID3DBlob* pSVS4 = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/VerticalBlurVS.hlsl", "main", "vs_5_0", &pSVS4);
	ShaderTest = gDevice->CreateVertexShader(pSVS4->GetBufferPointer(), pSVS4->GetBufferSize(), nullptr, &verticalBlurVertexShader);

	//Create shadow vertexShader
	ID3DBlob* pSVS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/shadowVS.hlsl", "VS_main", "vs_5_0", &pSVS);
 	ShaderTest = gDevice->CreateVertexShader(pSVS->GetBufferPointer(), pSVS->GetBufferSize(), nullptr, &shadowVertexShader);

	// INPUT LAYOUT MÅSTE VARA ANPASSAD TILL VERTEX SHADER
	// INPUT LAYOUT MÅSTE VARA ANPASSAD TILL VERTEX SHADER

	//create input layout (verified using vertex shader)
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	
	};
	//create input layout (verified using vertex shader)
	D3D11_INPUT_ELEMENT_DESC inputDesc2[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }

	};

	D3D11_INPUT_ELEMENT_DESC inputDescSplatmap[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORDALPHA", 0, DXGI_FORMAT_R32G32_FLOAT, 20, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3D11_INPUT_ELEMENT_DESC inputDescPosOnly[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	//CREATE  THE LAYOUTS
	ShaderTest = gDevice->CreateInputLayout(inputDescPosOnly, ARRAYSIZE(inputDescPosOnly), pVS->GetBufferPointer(), pVS->GetBufferSize(), &gWireFrameLayout);
	ShaderTest = gDevice->CreateInputLayout(inputDescSplatmap, ARRAYSIZE(inputDescSplatmap), pVS->GetBufferPointer(), pVS->GetBufferSize(), &gSplatmapLayout);
	ShaderTest = gDevice->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), pSVS->GetBufferPointer(), pSVS->GetBufferSize(), &gVertexLayout);
	ShaderTest = gDevice->CreateInputLayout(inputDesc2, ARRAYSIZE(inputDesc2), ddVS->GetBufferPointer(), ddVS->GetBufferSize(), &gVertexLayout2);

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
	ShaderTest = CompileShader(L"FX_HLSL/RGBSplatmapPS.hlsl", "main", "ps_5_0", &pPS2);
	ShaderTest = gDevice->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &splatMapPixelShader);
	

	////create shadow pixel shader
	ID3DBlob* pSPS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/shadowPS.hlsl", "PS_main", "ps_5_0", &pSPS);

   	ShaderTest = gDevice->CreatePixelShader(pSPS->GetBufferPointer(), pSPS->GetBufferSize(), nullptr, &shadowPixelShader);
	


	//WIREFRAME	
	ShaderTest = CompileShader(L"FX_HLSL/WireFrameVS.hlsl", "main", "vs_5_0", &pVS);
	ShaderTest = gDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &gWireFrameVertexShader);

	ShaderTest = CompileShader(L"FX_HLSL/WireFramePS.hlsl", "main", "ps_5_0", &pPS2);
	ShaderTest = gDevice->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &gWireFramePixelShader);

	ShaderTest = CompileShader(L"FX_HLSL/WireFramePSContainsTest.hlsl", "main", "ps_5_0", &pPS2);
	ShaderTest = gDevice->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &gWireFramePixelShaderCONTAINTEST);

	ShaderTest = CompileShader(L"FX_HLSL/GlowPS.hlsl", "main", "ps_5_0", &pPS2);
	ShaderTest = gDevice->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &glowPixelShader);

	ShaderTest = CompileShader(L"FX_HLSL/HorizontalBlurPS.hlsl", "main", "ps_5_0", &pPS2);
	ShaderTest = gDevice->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &horizontalBlurPixelShader);

	ShaderTest = CompileShader(L"FX_HLSL/VerticalBlurPS.hlsl", "main", "ps_5_0", &pPS2);
	ShaderTest = gDevice->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &verticalBlurPixelShader);

	gGS->Release();
	pPSH->Release();
	pPS->Release();
	pPS2->Release();
	pSPS->Release();
	pSVS->Release();
	ddPS->Release();
	ddVS->Release();
	pSVS3->Release();

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
		0.0f, 1.0f, 1.0f,		//		//n3

		-1.0f, 1.0f, -1.0f,		//v1
		0.0f, 0.0f,				//t1
		0.0f, 1.0f, 1.0f,			//n3

		1.0f, -1.0f, -1.0f,		//v2
		1.0f, 1.0f,			//t2
		0.0f, 1.0f, 1.0f,			//n3

		1.0f, 1.0f, -1.0f,		//v3
		1.0f, 0.0f,			//t3
		0.0f, 1.0f, 1.0f	//n3
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

	D3D11_BUFFER_DESC transformbuffer2;
	memset(&transformbuffer2, 0, sizeof(transformbuffer2));
	transformbuffer2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	transformbuffer2.Usage = D3D11_USAGE_DEFAULT;
	transformbuffer2.ByteWidth = sizeof(World2);
	BufferTest = gDevice->CreateBuffer(&transformbuffer2, NULL, &gWorld2);

	//heightmap stuff, till splatmap
	D3D11_BUFFER_DESC heightmapBuffer;
	memset(&heightmapBuffer, 0, sizeof(heightmapBuffer));
	heightmapBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	heightmapBuffer.Usage = D3D11_USAGE_DEFAULT;
	heightmapBuffer.ByteWidth = sizeof(heightmapInfo);
	BufferTest = gDevice->CreateBuffer(&heightmapBuffer, NULL, &heightmapInfoConstant);

	
	// Rotatation And transform World Buffer
	D3D11_BUFFER_DESC lightMatrixBDesc;
	memset(&lightMatrixBDesc, 0, sizeof(lightMatrixBDesc));
	lightMatrixBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightMatrixBDesc.Usage = D3D11_USAGE_DEFAULT;
	lightMatrixBDesc.ByteWidth = sizeof(LightMatrix);
	BufferTest = gDevice->CreateBuffer(&lightMatrixBDesc, NULL, &gLMat);


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

	//Options buffer
	D3D11_BUFFER_DESC OBufferDesc;
	OBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	memset(&OBufferDesc, 0, sizeof(OBufferDesc));
	OBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	OBufferDesc.CPUAccessFlags = 0;
	OBufferDesc.MiscFlags = 0;
	OBufferDesc.ByteWidth = sizeof(Options);
	BufferTest = gDevice->CreateBuffer(&OBufferDesc, NULL, &shaderTest);

	
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

	Vector3 viewPoint = Vector3(camPosition.x, camPosition.y, camPosition.z);
	ViewP.viewPoint = viewPoint;
	ViewP.pad = 0;
	fpsCamLook = XMMatrixLookAtLH(camPosition, camTarget, camUp);

	//Update cam
	fpsCam.UpdateViewMatrix();

	// KEYBOARD AND MOUSE STUFF
	DIMOUSESTATE mouseCurrState;
	BYTE keyboardState[256];

	//Aquaire devices
	Keyboard->Acquire();
	Mouse->Acquire();

	//Get States
	Mouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);
	Keyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);


	//// Update lightbuffer

	gDeviceContext->UpdateSubresource(PrimaryLightBuffer, 0, NULL, &PrimaryLights, 0, 0);
	gDeviceContext->PSSetConstantBuffers(0, 1, &PrimaryLightBuffer);

	//// Update matbuffer

	gDeviceContext->UpdateSubresource(MatBuffer, 0, NULL, &MatBuffer1, 0, 0);
	gDeviceContext->PSSetConstantBuffers(1, 1, &MatBuffer);

	//STORE VALUE
	gDeviceContext->UpdateSubresource(ViewBuffer, 0, NULL, &ViewP, 0, 0);
	gDeviceContext->VSSetConstantBuffers(4, 1, &ViewBuffer);


	gDeviceContext->UpdateSubresource(ViewBuffer, 0, NULL, &ViewP, 0, 0);
	gDeviceContext->GSSetConstantBuffers(4, 1, &ViewBuffer);

	////////////LIGHTS/////////////////////////////////////////////////////

	//Set no backface culling
	gDeviceContext->RSSetState(NULL);
	gDeviceContext->RSSetState(NoBcull);

	// SHADOW DEPTH RENDER PASS//
	shadowMap->DrawDepthMap(renderObjects, gDeviceContext);
	shadowTexture = shadowMap->GetSRV();


	//SET MATRIXES FOR NORMAL RENDER
	CamView = fpsCamLook;
	CamProjection = fpsCam.Proj();


	quadTree->StartFrustumTest(CamProjection, CamView);

	//NORMAL RENDER PASS FROM EYE POS 
	gDeviceContext->OMSetRenderTargets(1, &gBackRufferRenderTargetView, gDepthStencilView);
	gDeviceContext->RSSetViewports(1, &vp);
	float clearColor[] = { 0, 0.3, 0.7f, 1.0f };
	gDeviceContext->ClearRenderTargetView(gBackRufferRenderTargetView, clearColor);
	gDeviceContext->ClearDepthStencilView(gDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	int tex = 0;

	//WVP AND OTHERS
	XMStoreFloat4x4(&WorldMatrix1.View, XMMatrixTranspose(CamView));
	XMStoreFloat4x4(&WorldMatrix1.Projection, XMMatrixTranspose(CamProjection));
	XMStoreFloat4x4(&WorldMatrix1.WorldSpace, XMMatrixTranspose(identityM));
	XMStoreFloat4x4(&WorldMatrix1.InvWorld, XMMatrixTranspose(XMMatrixInverse(NULL, identityM)));
	XMStoreFloat4x4(&WorldMatrix1.lightView, XMMatrixTranspose(shadowMap->GetLightView()));
	XMStoreFloat4x4(&WorldMatrix1.lightProjection, XMMatrixTranspose(shadowMap->GetLightProj()));

	gDeviceContext->UpdateSubresource(gWorld, 0, NULL, &WorldMatrix1, 0, 0);
	gDeviceContext->VSSetConstantBuffers(0, 1, &gWorld);

	//wireframe constantbuffer or ?????????????????
	XMStoreFloat4x4(&WorldMatrixWF.View, XMMatrixTranspose(CamView));
	XMStoreFloat4x4(&WorldMatrixWF.Projection, XMMatrixTranspose(CamProjection));
	XMStoreFloat4x4(&WorldMatrixWF.WorldSpace, XMMatrixTranspose(identityM));


	//LIGHT MATRIXS
	XMStoreFloat4x4(&LightMatrix1.lightView, XMMatrixTranspose(shadowMap->GetLightView()));
	XMStoreFloat4x4(&LightMatrix1.lightProjection, XMMatrixTranspose(shadowMap->GetLightProj()));

	//LIGHT INFO BUFFER
	gDeviceContext->UpdateSubresource(gLMat, 0, NULL, &LightMatrix1, 0, 0);
	gDeviceContext->VSSetConstantBuffers(1, 1, &gLMat);

	//SHADER OPTIONS
	gDeviceContext->UpdateSubresource(shaderTest, 0, NULL, &optionStruct, 0, 0);
	gDeviceContext->PSSetConstantBuffers(3, 1, &shaderTest);

	UINT32 vertexSize2 = sizeof(float) * 11;
	UINT32 offset2 = 0;
	tex = 0;
	gDeviceContext->IASetInputLayout(gVertexLayout);
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gDeviceContext->VSSetShader(shadowVertexShader, nullptr, 0);
	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	////BACKFACE CULLING
	if (Bculling == TRUE)
	gDeviceContext->GSSetShader(gBackFaceShader, nullptr, 0);
	else if (Bculling == FALSE)
	gDeviceContext->GSSetShader(nullptr, nullptr, 0);
	gDeviceContext->PSSetShader(shadowPixelShader, nullptr, 0);

	// Set the sampler states to the pixel shader.
	gDeviceContext->PSSetSamplers(0, 1, &sampState1);
	gDeviceContext->PSSetSamplers(1, 1, &sampState2);

	gDeviceContext->PSSetShaderResources(1, 1, &shadowTexture);
	gDeviceContext->PSSetShaderResources(2, 1, &ddsTex3);
	//for (int i = 0; i < renderObjects.size(); i++)
	//{
	//	/*if (renderObjects[i]->GetActive() == true && renderObjects[i]->isTransparent == false){
	//		renderObjects[i]->CalculateWorld();*/

	//		if (i == 4){
	//			gDeviceContext->PSSetShaderResources(2, 1, &normalMap);
	//		}
	//		tex = intArrayTex[renderObjects[i]->indexT];
	//		gDeviceContext->PSSetShaderResources(0, 1, &RSWArray[tex]);

	//		gDeviceContext->IASetVertexBuffers(0, 1, &renderObjects[i]->vertexBuffer, &vertexSize2, &offset2);
	//		gDeviceContext->Draw(renderObjects[i]->nrElements * 3, 0);
	//	
	//}
	//
	//float clearColor[] = { 0.0f, 0.3f, 0.7f, 1.0f };
	UINT32 vertexPosTex = sizeof(float)* 5;
	//GLOWTEST!!!!!!!!!
	tex = 0;

	gDeviceContext->PSSetSamplers(0, 1, &sampState1);
	gDeviceContext->PSSetSamplers(1, 1, &sampState2);

	//rendera alla objecten på glowmapen, endast ljusa färger kommer med på denna map!
	glow->DrawToGlowMap();
	for (int i = 0; i < renderObjects.size(); i++) //objekten i scenen
	{
		renderObjects[i]->CalculateWorld();
		XMStoreFloat4x4(&WorldMatrix1.WorldSpace, XMMatrixTranspose(renderObjects[i]->world));
		gDeviceContext->UpdateSubresource(gWorld, 0, NULL, &WorldMatrix1, 0, 0);
		gDeviceContext->VSSetConstantBuffers(0, 1, &gWorld);

		tex = intArrayTex[renderObjects[i]->indexT];
		gDeviceContext->PSSetShaderResources(0, 1, &RSWArray[tex]);
		gDeviceContext->IASetVertexBuffers(0, 1, &renderObjects[i]->vertexBuffer, &vertexSize2, &offset2);

		gDeviceContext->Draw(renderObjects[i]->nrElements * 3, 0);

	}
	//kör om med blur här!!!!!!!!!
	gDeviceContext->UpdateSubresource(gWorld, 0, NULL, &WorldMatrix1, 0, 0);
	//BÅDA DESSA MÅSTE VARA MED!!!!
	glow->ApplyBlurOnGlowHorizontal(horizontalBlurVertexShader, horizontalBlurPixelShader);
	glow->ApplyBlurOnGlowVertical(verticalBlurVertexShader, verticalBlurPixelShader);
	


	gDeviceContext->RSSetViewports(1, &vp);
	//Set BackGround Color
	//float clearColor[] = { 0, 0.3, 0.7f, 1.0f };
	gDeviceContext->ClearRenderTargetView(gBackRufferRenderTargetView, clearColor);
	gDeviceContext->ClearDepthStencilView(gDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	gDeviceContext->OMSetRenderTargets(1, &gBackRufferRenderTargetView, gDepthStencilView);

	tex = 0;

	TurnZBufferOn();
	//transparent objects
	float blendFactor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	gDeviceContext->OMSetBlendState(transparency, blendFactor, 0xffffffff);

	////rita ut glowen**********************************************************************************************
	//gDeviceContext->IASetInputLayout(gVertexLayout);
	glow->SetPosTexVertexLayout();
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//TurnZBufferOff();
	gDeviceContext->VSSetShader(gVertexShader, nullptr, 0);
	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	gDeviceContext->PSSetShader(gPixelShader, nullptr, 0);

	gDeviceContext->UpdateSubresource(gWorld, 0, NULL, &WorldMatrix1, 0, 0);
	gDeviceContext->PSSetShaderResources(0, 1, &glow->shaderResourceView); //inte tempShader sen
	gDeviceContext->IASetVertexBuffers(0, 1, &glow->planeVertexBuffer, &vertexPosTex, &offset2);
	gDeviceContext->Draw(4, 0);
	//*************************************************************************************************************


	//for (int i = 0; i < transparentObjects.size(); i++){
	//	if (transparentObjects[i]->GetActive() == true){

	//		XMStoreFloat4x4(&WorldMatrix1.WorldSpace, XMMatrixTranspose(transparentObjects[i]->world));
	//		gDeviceContext->UpdateSubresource(gWorld, 0, NULL, &WorldMatrix1, 0, 0);
	//		gDeviceContext->VSSetConstantBuffers(0, 1, &gWorld);

	//		tex = intArrayTex[transparentObjects[i]->indexT];
	//		gDeviceContext->PSSetShaderResources(0, 1, &RSWArray[tex]);
	//		gDeviceContext->IASetVertexBuffers(0, 1, &transparentObjects[i]->vertexBuffer, &vertexSize2, &offset2);

	//		gDeviceContext->Draw(transparentObjects[i]->nrElements * 3, 0);
	//	}
	//}

	gDeviceContext->OMSetBlendState(0, 0, 0xffffffff); //ingen blending, denna ändras sen i slutet till transparenta objekt (y)

	//Render Heightmap´s
	//for (int i = 0; i < heightMapObjects.size(); i++)
	//{
	//	UINT32 vertexSize = sizeof(float)* 10;
	//	UINT32 offset = 0;

	//	XMStoreFloat4x4(&WorldMatrixWF.WorldSpace, XMMatrixTranspose(identityM));
	//	gDeviceContext->UpdateSubresource(gWorld, 0, NULL, &WorldMatrix1, 0, 0);
	//	gDeviceContext->VSSetConstantBuffers(0, 1, &gWorld);

	//	gDeviceContext->UpdateSubresource(heightmapInfoConstant, 0, NULL, &heightMapObjects[i]->HMInfoConstant, 0, 0);
	//	gDeviceContext->PSSetConstantBuffers(3, 1, &heightmapInfoConstant);


	//	gDeviceContext->IASetInputLayout(gSplatmapLayout);
	//	gDeviceContext->IASetVertexBuffers(0, 1, &heightMapObjects[i]->gVertexBuffer, &vertexSize, &offset);
	//	gDeviceContext->IASetIndexBuffer(heightMapObjects[i]->gIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//	gDeviceContext->VSSetShader(splatMapVertexShader, nullptr, 0);
	//	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	//	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	//	gDeviceContext->GSSetShader(nullptr, nullptr, 0);
	//	//gDeviceContext->PSSetShader(gPixelShader, nullptr, 0);
	//	gDeviceContext->PSSetShader(splatMapPixelShader, nullptr, 0);
	//	gDeviceContext->PSSetSamplers(8, 1, &sampState2); //clamp 
	//	gDeviceContext->PSSetSamplers(9, 1, &sampState1); //wrap samp
	//	//Bind texture to object
	//	//gDeviceContext->PSSetShaderResources(0, 1, &ddsTex1);
	//	gDeviceContext->PSSetShaderResources(0, 1, &heightMapObjects[i]->tex1shaderResourceView);
	//	gDeviceContext->PSSetShaderResources(1, 1, &heightMapObjects[i]->tex2shaderResourceView);
	//	gDeviceContext->PSSetShaderResources(2, 1, &heightMapObjects[i]->tex3shaderResourceView);
	//	gDeviceContext->PSSetShaderResources(3, 1, &heightMapObjects[i]->splatshaderResourceView);

	//	//gDeviceContext->PSSetShaderResources(0, 1, &var.splatshaderResourceView);

	//	gDeviceContext->DrawIndexed((heightMapObjects[i]->nmrElement), 0, 0);
	//}

	////WIREFRAME!!!
	//gDeviceContext->PSSetSamplers(8, 1, &sampState2);
	//gDeviceContext->VSSetShader(gWireFrameVertexShader, nullptr, 0);
	//gDeviceContext->PSSetShader(gWireFramePixelShader, nullptr, 0);
	//gDeviceContext->IASetInputLayout(gWireFrameLayout);
	//gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	//UINT32 vertexWireFrameSize = sizeof(float)* 3;

	//for (int i = 0; i < renderObjects.size(); i++)
	//{
	//	XMStoreFloat4x4(&WorldMatrixWF.WorldSpace, XMMatrixTranspose(renderObjects[i]->world)); //använder wireframe matrisen istället här
	//	gDeviceContext->UpdateSubresource(gWorld, 0, NULL, &WorldMatrixWF, 0, 0);
	//	gDeviceContext->VSSetConstantBuffers(0, 1, &gWorld);

	//	gDeviceContext->IASetVertexBuffers(0, 1, &renderObjects[i]->boundingBoxVertexBuffer, &vertexWireFrameSize, &offset2);

	//	gDeviceContext->Draw(16, 0);
	//}


	////quadträdet
	//XMStoreFloat4x4(&WorldMatrixWF.WorldSpace, XMMatrixTranspose(identityM)); //använder wireframe matrisen istället här
	//gDeviceContext->UpdateSubresource(gWorld, 0, NULL, &WorldMatrixWF, 0, 0);
	//gDeviceContext->VSSetConstantBuffers(0, 1, &gWorld);

	//for (int i = 0; i < quadTree->quadTreeBranches.size(); i++)
	//{
	//	if (quadTree->quadTreeBranches[i]->isInFrustum == true){
	//		gDeviceContext->IASetVertexBuffers(0, 1, &quadTree->quadTreeBranches[i]->boxBuffer, &vertexWireFrameSize, &offset2);
	//		gDeviceContext->Draw(16, 0);
	//	}
	//}




	//******************************************************************************************************



	
	//RENDER TEST PLANES
	/*TurnZBufferOff();
	UINT32 vertexSize = sizeof(float)* 8;
	UINT32 offset = 0;
	quadMatrix = XMMatrixScaling(0.25f, 0.25f, 0.0f) * XMMatrixTranslation(0.75f, -0.75f, 0.0f);
	XMStoreFloat4x4(&WorldMatrix2.WVP, XMMatrixTranspose(quadMatrix));
	gDeviceContext->UpdateSubresource(gWorld2, 0, NULL, &WorldMatrix2, 0, 0);
	gDeviceContext->VSSetConstantBuffers(3, 1, &gWorld2);


	gDeviceContext->IASetInputLayout(gVertexLayout2);
	gDeviceContext->IASetVertexBuffers(0, 1, &gVertexBuffer, &vertexSize, &offset);
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	gDeviceContext->VSSetShader(shader2DVS, nullptr, 0);
	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	gDeviceContext->GSSetShader(nullptr, nullptr, 0);
	gDeviceContext->PSSetShader(shader2DPS, nullptr, 0);
	gDeviceContext->PSSetShaderResources(0, 1, &ddsTex3);


	gDeviceContext->Draw(4, 0);

	quadMatrix = XMMatrixScaling(0.25f, 0.25f, 0.0f) * XMMatrixTranslation(0.75f, -0.25f, 0.0f);
	XMStoreFloat4x4(&WorldMatrix2.WVP, XMMatrixTranspose(quadMatrix));
	gDeviceContext->UpdateSubresource(gWorld2, 0, NULL, &WorldMatrix2, 0, 0);
	gDeviceContext->VSSetConstantBuffers(3, 1, &gWorld2);

	gDeviceContext->IASetInputLayout(gVertexLayout2);
	gDeviceContext->IASetVertexBuffers(0, 1, &gVertexBuffer, &vertexSize, &offset);
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	gDeviceContext->VSSetShader(shader2DVS, nullptr, 0);
	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	gDeviceContext->GSSetShader(nullptr, nullptr, 0);
	gDeviceContext->PSSetShader(shader2DPS, nullptr, 0);
	gDeviceContext->PSSetShaderResources(0, 1, &shadowTexture);


	gDeviceContext->Draw(4, 0);
	
	TurnZBufferOn();*/

	//växla back/front buffer
	gSwapChain->Present(0, 0); 
}


// UPDATES

void RenderEngine::Update(float dt){
	
	

}

//REALESE AND CLEANUP

void RenderEngine::Release(){

	shadowMap->~ShadowMap();
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
	delete quadTree;
	quadTree = NULL;

	delete glow;
	glow = NULL;
	counterCWCullmode->Release();
	transparency->Release();
	gDevice->Release();
	
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

void RenderEngine::ImportHeightmap(char* HeightMapFileName, wstring tex1File, wstring tex2File, wstring tex3File, wstring texSplatFile){
	HeightMap ImportedHM(gDevice, gDeviceContext);

	ImportedHM.LoadHeightMap(HeightMapFileName);
	ImportedHM.LoadSplatMap(tex1File, tex2File, tex3File, texSplatFile);

	HeightMapObject *cHeightMap = new HeightMapObject;

	cHeightMap->gIndexBuffer = ImportedHM.GetIndexBuffer();
	cHeightMap->gVertexBuffer = ImportedHM.GetVertexBuffer();
	cHeightMap->nmrElement = ImportedHM.GetNrElements();
	//cHeightMap->gridSize = ImportedHM.GetGridSize();
	//cHeightMap->vertexHeights = ImportedHM.GetHeights();
	cHeightMap->HMInfoConstant.heightElements = ImportedHM.heightElements;
	cHeightMap->tex1shaderResourceView = ImportedHM.GetTex1();
	cHeightMap->tex2shaderResourceView = ImportedHM.GetTex2();
	cHeightMap->tex3shaderResourceView = ImportedHM.GetTex3();
	cHeightMap->splatshaderResourceView = ImportedHM.GetSplatTex();

	heightMapObjects.push_back(cHeightMap);



}

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
		fpsCam.SetLens(0.25f*3.14f, screen_Width / screen_Height, 1.0f, 100.0f);
		zoom = 1;

	}
	if ((keyboardState[DIK_R] & 0x80) && zoom == 1)
	{
		fpsCam.SetLens(0.6f*3.14f, screen_Width / screen_Height, 1.0f, 100.0f);
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
	if (keyboardState[DIK_F1] & 0x80)
	{
		optionStruct.option1 = 1;
	};
	
	if (keyboardState[DIK_F2] & 0x80)
	{
		optionStruct.option1 = 0;
	};
	if (keyboardState[DIK_F3] & 0x80)
	{
		optionStruct.option2 = 1;
	};
	if (keyboardState[DIK_F4] & 0x80)
	{
		optionStruct.option2 = 0;
	}
	if (keyboardState[DIK_F5] & 0x80)
	{
		optionStruct.option5 = 1;
	}
	if (keyboardState[DIK_F6] & 0x80)
	{
		optionStruct.option6 = 1;
	}
	if (keyboardState[DIK_F7] & 0x80)		
	{
		optionStruct.option7 = 1;
	}
	if (keyboardState[DIK_F8] & 0x80)
	{
		optionStruct.option8 = 1;
	}



}

//ENABLE DEPTH
void RenderEngine::TurnZBufferOn()
{
	gDeviceContext->OMSetDepthStencilState(m_depthStencilState, 1);
	return;
}


//DISABLE DEPTH
void RenderEngine::TurnZBufferOff()
{
	gDeviceContext->OMSetDepthStencilState(m_depthDisabledStencilState, 1);
	return;
}

//BLENDSTATES FOR TRANSPARENCY
void RenderEngine::BlendStates(){
	HRESULT hr;

	D3D11_BLEND_DESC bDesc;
	ZeroMemory(&bDesc, sizeof(bDesc));

	D3D11_RENDER_TARGET_BLEND_DESC rtbDesc;
	ZeroMemory(&rtbDesc, sizeof(rtbDesc));

	rtbDesc.BlendEnable = true;
	rtbDesc.SrcBlend = D3D11_BLEND_SRC_COLOR;
	rtbDesc.DestBlend = D3D11_BLEND_BLEND_FACTOR;
	rtbDesc.BlendOp = D3D11_BLEND_OP_ADD;
	rtbDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtbDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbDesc.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

	bDesc.AlphaToCoverageEnable = false;
	bDesc.RenderTarget[0] = rtbDesc;

	gDevice->CreateBlendState(&bDesc, &transparency);

	//cull counter/clockwise
	D3D11_RASTERIZER_DESC cmdesc;
	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));

	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_BACK;

	cmdesc.FrontCounterClockwise = true;
	hr = gDevice->CreateRasterizerState(&cmdesc, &counterCWCullmode);

	cmdesc.FrontCounterClockwise = false;
	hr = gDevice->CreateRasterizerState(&cmdesc, &CWCullmode);
}