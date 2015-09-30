#pragma once
#ifndef RENDERENGINE_H
#define RENDERENGINE_H
#endif

//CRUCIAL
#include <windows.h>
#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <DirectXMathVector.inl>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>


//INCLUDES
#include "GameTimer.h"
#include "OBJ.h"
#include "Input.h"
#include "HeightMap.h"
#include "HeightMap2.h"
#include "GameObject.h"
#include "debugwindowclass.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"
#include "BINimporter.h"

//VISUAL
#include "ShadowMap.h"
#include "QuadTree.h"
#include "Glow.h"
#include "Defered.h"
//SIMPLIFIERS
#include "SimpleMath.h"
#include "SpriteFont.h"
#include "SpriteBatch.h"



using namespace DirectX;
using namespace DirectX::SimpleMath;



#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")


class RenderEngine{ //DENNA KLASSEN ?R ABSTRAKT - g?r inte instantiera


public:
	//Input handler for keyboard and mouse interactions
	Input* m_Input = new Input();
	IDirectInputDevice8* Keyboard;
	IDirectInputDevice8* Mouse;
	DIMOUSESTATE mouseLastState;
	LPDIRECTINPUT8 DirectInput;

	bool hit;
	int renderBool = 0;
	bool highttest = 0;
	std::wostringstream outs;
	float hTest = 0;
	//FONTS AND BATCHES
	std::unique_ptr<DirectX::SpriteFont> spritefont;
	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;

public:
	//Window Constructor
	RenderEngine(HINSTANCE hInstance, std::string name, UINT scrW, UINT scrH);
	virtual ~RenderEngine();

	//Core functions
	bool InitDirectInput(HINSTANCE hInstance);
	virtual bool Init();
	void SetViewport();
	int Run();
	void Update(float dt);
	void Render();
	virtual void Release();
	void InputHandler();
	void makelights();
	// MESH IMPORTER
	BINimporter theCustomImporter;
	vector<int> intArrayTex;

	//MESH ARRAY
	std::vector<GameObjects*> renderObjects;

	///MESH ARRAY WITH TRANSPARENT OBJECTS
	std::vector<GameObjects*> transparentObjects; //dessa är de ända som kommer sorteras från avstånd av kameran
	
	// QUADTREE CULLING
	QuadTree *quadTree = nullptr;
	
	//GLOW
	Glow *glow = nullptr;;
	
	//Shadows
	ShadowMap* shadowMap;
	ID3D11ShaderResourceView* shadowTexture = nullptr;
	
	//DEFERED
	DeferedRenderer* DeferedRender;

	//NEW HEIGHTMAP
	heightMap2* HeightMap2;

public:
	void AddObject(GameObjects gObj);

	//Fps camera
	Vector4 camPosition;
	Vector4 camTarget;
	Vector4 camUp;// = Vector4(0.0f, 1.0f, 0.0f, 0.0f);;
	Vector4 DefaultForward = Vector4(0.0f, -1.0f, 0.0f, 0.0f);
	Vector4 DefaultRight = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
	Vector4 camForward = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
	Vector4 camRight = Vector4(1.0f, 0.0f, 0.0f, 0.0f);

	XMMATRIX camRotationMatrix;
	XMMATRIX fpsCamLook;
	XMMATRIX CamProjection;
	XMMATRIX CamView;

	float moveLeftRight = 0.0f;
	float moveBackForward = 0.0f;
	float camYaw = 0.0f;
	float camPitch = 0.0f;
	float speed = 0.0f;
	float boost = 0.0f;
	float zoom = 0.0f;

	Camera fpsCam;
	POINT mLastMousePos;

	//Fps counter calc func
	void fpscounter();
	GameTimer gTimer;

	//Import Functions
	void ImportObj(char* geometryFileName, char* materialFileName, ID3D11Device* gDev);// , bool isStatic, XMMATRIX startPosMatrix);

	//Window name
	std::wstring mainwname;

	//Geometry functions
	void CreatePlaneDataAndBuffers();
	void Shaders();
	void TextureFunc();


	HRESULT CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob);

	//Message handler
	LRESULT MsgProc(HWND hwindow, UINT msg, WPARAM wParam, LPARAM lParam);
	HRESULT KUK = 0;


	//Bool to test Backface culling
	bool Bculling = FALSE;
	bool Bcullingcheck = FALSE;

	////STRUCT DESCRIPTIONS FOR CBUFFERS
	struct World2
	{
		XMFLOAT4X4 WVP;
		XMFLOAT4X4 World;
	};
	//Structs for cBuffers
	struct World
	{
		XMFLOAT4X4 View;
		XMFLOAT4X4 Projection;
		XMFLOAT4X4 WorldSpace;
		XMFLOAT4X4 InvWorld;
		XMFLOAT4X4 WVP;
	
		//Shadow matrixs
		XMFLOAT4X4 lightView;
		XMFLOAT4X4 lightProjection;
	};

	struct LightMatrix{
	//Shadow matrixs
	XMFLOAT4X4 lightView;
	XMFLOAT4X4 lightProjection;
	};
	//LightBuffers
	struct LightStruct{
		
		DirLight		lDir;
		PointLight    pLights[200];
		SpotLight spot;
		//float	pad;

	};
	float lightmove = 0;
	
	struct MatView{
		Material gMaterial;
		XMFLOAT3	gEyePos;
		float	pad;

	};
	
	struct WorldWireFrame{
		XMFLOAT4X4 View;
		XMFLOAT4X4 Projection;
		XMFLOAT4X4 WorldSpace;
	};

	struct HeightmapInfo{
		float heightElements;
		XMFLOAT3 pad1;

		HeightmapInfo hmInfoConstant(){
			pad1.x = 1;
			pad1.y = 1;
			pad1.z = 1;
		}
	};
	HeightmapInfo hmapTest;
	//ViewPoint struct
	struct ViewBufferStruct{
		XMFLOAT3 viewPoint;
		float pad;
	};

	//STRUCTS FOR CBUFFERS
	WorldWireFrame WorldMatrixWF;
	HeightmapInfo heightmapInfo;
	MatView MatBuffer1;
	World2 WorldMatrix2;
	LightMatrix LightMatrix1;
	LightStruct PrimaryLights;
	World WorldMatrix1;
	ViewBufferStruct ViewP;


	//VertexBuffer for picking, saves vertexpositions
	struct VertexPos{
		float vx, vy, vz;
	};

	//Struct for objLoader
	struct GameObject{
		int gameObjectIndex;
		ID3D11Buffer* gVertexBuffer;
		//ID3D11Buffer* gIndexBuffer;
		UINT nrElements = 0;
		BoundingBox bbox;

		char* ObjName;
		std::vector<XMFLOAT3> verteciesPos;
		std::vector<float> verteciesPosX;
		std::vector<float> verteciesPosY;
		std::vector<float> verteciesPosZ;
		std::vector<int> verteciesIndex;

		/*void Scaling(XMMATRIX Scale);
		void Rotate(XMMATRIX Rot);
		void Transform(XMMATRIX Trans);*/
		bool isStatic = false; //statiska object kommer påverkas av frustum culling (y)
		bool render = false; //används bara på statiska object
		bool visibleThisFrame = false; //ska användas för ifall flera boxes säger olika om hurvida denna ska renderas eller inte

		ID3D11Buffer *boxBuffer = nullptr;
		void SetBoundingBoxStartPosition(XMMATRIX worldPos, ID3D11Device* gDevice){
			bbox.Transform(bbox, worldPos); //<------------------------	varför transformerar jag den egentligen? då måste jag ju anta att alla vertiser oxå är det!

			//används för att rendera boxen (wireframe)
			std::vector<Float3> boxVertPoints;
			boxVertPoints.push_back(Float3(bbox.Center.x - bbox.Extents.x, bbox.Center.y - bbox.Extents.y, bbox.Center.z - bbox.Extents.z)); //0,0,0
			boxVertPoints.push_back(Float3(bbox.Center.x + bbox.Extents.x, bbox.Center.y - bbox.Extents.y, bbox.Center.z - bbox.Extents.z)); //1,0,0
			boxVertPoints.push_back(Float3(bbox.Center.x + bbox.Extents.x, bbox.Center.y + bbox.Extents.y, bbox.Center.z - bbox.Extents.z)); //1,1,0
			boxVertPoints.push_back(Float3(bbox.Center.x - bbox.Extents.x, bbox.Center.y + bbox.Extents.y, bbox.Center.z - bbox.Extents.z)); //0,1,0
			boxVertPoints.push_back(Float3(bbox.Center.x - bbox.Extents.x, bbox.Center.y - bbox.Extents.y, bbox.Center.z - bbox.Extents.z)); //0,0,0

			boxVertPoints.push_back(Float3(bbox.Center.x - bbox.Extents.x, bbox.Center.y - bbox.Extents.y, bbox.Center.z + bbox.Extents.z)); //0,0,1
			boxVertPoints.push_back(Float3(bbox.Center.x + bbox.Extents.x, bbox.Center.y - bbox.Extents.y, bbox.Center.z + bbox.Extents.z)); //1,0,1
			boxVertPoints.push_back(Float3(bbox.Center.x + bbox.Extents.x, bbox.Center.y + bbox.Extents.y, bbox.Center.z + bbox.Extents.z)); //1,1,1
			boxVertPoints.push_back(Float3(bbox.Center.x - bbox.Extents.x, bbox.Center.y + bbox.Extents.y, bbox.Center.z + bbox.Extents.z)); //0,1,1
			boxVertPoints.push_back(Float3(bbox.Center.x - bbox.Extents.x, bbox.Center.y - bbox.Extents.y, bbox.Center.z + bbox.Extents.z)); //0,0,1

			boxVertPoints.push_back(Float3(bbox.Center.x + bbox.Extents.x, bbox.Center.y - bbox.Extents.y, bbox.Center.z + bbox.Extents.z)); //1,0,1
			boxVertPoints.push_back(Float3(bbox.Center.x + bbox.Extents.x, bbox.Center.y - bbox.Extents.y, bbox.Center.z - bbox.Extents.z)); //1,0,0
			boxVertPoints.push_back(Float3(bbox.Center.x + bbox.Extents.x, bbox.Center.y + bbox.Extents.y, bbox.Center.z - bbox.Extents.z)); //1,1,0
			boxVertPoints.push_back(Float3(bbox.Center.x + bbox.Extents.x, bbox.Center.y + bbox.Extents.y, bbox.Center.z + bbox.Extents.z)); //1,1,1
			boxVertPoints.push_back(Float3(bbox.Center.x - bbox.Extents.x, bbox.Center.y + bbox.Extents.y, bbox.Center.z + bbox.Extents.z)); //0,1,1
			boxVertPoints.push_back(Float3(bbox.Center.x - bbox.Extents.x, bbox.Center.y + bbox.Extents.y, bbox.Center.z - bbox.Extents.z)); //0,1,0

			D3D11_BUFFER_DESC bDesc;
			ZeroMemory(&bDesc, sizeof(D3D11_BUFFER_DESC));
			bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bDesc.Usage = D3D11_USAGE_DEFAULT;
			bDesc.ByteWidth = sizeof(Float3)* (boxVertPoints.size());

			D3D11_SUBRESOURCE_DATA data;
			data.pSysMem = boxVertPoints.data();//<--------
			HRESULT VertexBufferChecker = gDevice->CreateBuffer(&bDesc, &data, &boxBuffer);
		}
	};
	std::vector<GameObject> gameObjects;

	void ImportHeightmap(char* HeightMapFileName, wstring tex1File, wstring tex2File, wstring tex3File, wstring texSplatFile);
	//Struct for HeightMap
	struct HeightMapObject{
		ID3D11Buffer* gIndexBuffer;
		ID3D11Buffer* gVertexBuffer;
		int nmrElement;
		ID3D11ShaderResourceView* tex1shaderResourceView = nullptr;
		ID3D11ShaderResourceView* tex2shaderResourceView = nullptr;
		ID3D11ShaderResourceView* tex3shaderResourceView = nullptr;
		ID3D11ShaderResourceView* splatshaderResourceView = nullptr;

		HeightmapInfo HMInfoConstant;
	};
	std::vector<HeightMapObject*> heightMapObjects;

	//quadtree!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	struct Float3{
		float x, y, z;
		Float3(float x, float y, float z){
			this->x = x;
			this->y = y;
			this->z = z;
		}
	};


	void RenderHeightMap();
	//Window handlers
	HWND hWindow;
	HINSTANCE hInstance;

	//Start Window functions
	bool InitWindow();
	bool InitDirect3D(HWND);

	public:
	
	//Core varibles for often use
	std::string applicationName;
	UINT screen_Width;
	UINT screen_Height;
	DWORD windowStyle; //se konstruktorn

	BoundingFrustum frustum;

	//VIEWPORTS
	D3D11_VIEWPORT vp;

	//MATRIX
	XMMATRIX identityM = XMMatrixIdentity();
	XMMATRIX quadMatrix;
	//Core functions for windown and program setup
	ID3D11Device* gDevice = nullptr;
	ID3D11DeviceContext* gDeviceContext = nullptr;
	ID3D11RenderTargetView* gBackRufferRenderTargetView = nullptr;
	IDXGISwapChain* gSwapChain = nullptr;
	ID3D11DepthStencilView* gDepthStencilView = nullptr;
	//2D Textures
	ID3D11Texture2D* depthStencilBuffer = nullptr;

	//Buffers
	ID3D11Buffer* gVertexBuffer = nullptr;
	ID3D11Buffer* gVertexBuffer2 = nullptr;
	ID3D11Buffer* gWorld = nullptr;
	ID3D11Buffer* gWorld2 = nullptr;
	ID3D11Buffer* heightmapInfoConstant;
	ID3D11Buffer* ViewBuffer = nullptr;
	ID3D11Buffer* gLights = nullptr;
	ID3D11Buffer* PrimaryLightBuffer = nullptr;
	ID3D11Buffer* MatBuffer = nullptr;
	ID3D11Buffer* shadowBuffer = nullptr;
	ID3D11Buffer* gLMat = nullptr;
	
	//Shader resources for textures and more
	ID3D11ShaderResourceView* ddsTex1= nullptr;
	ID3D11ShaderResourceView* ddsTex2 = nullptr;
	ID3D11ShaderResourceView* ddsTex3 = nullptr;
	ID3D11ShaderResourceView*   normalMap = nullptr;
	ID3D11ShaderResourceView* AdsResourceView = nullptr;
	ID3D11ShaderResourceView* NpcRV = nullptr;
	ID3D11ShaderResourceView** RSWArray = nullptr;
	
	//Vertex/geometry layout desc
	ID3D11InputLayout* gVertexLayout = nullptr;
	ID3D11InputLayout* gVertexLayout2 = nullptr;
	ID3D11InputLayout* gWireFrameLayout = nullptr;
	ID3D11InputLayout* gSplatmapLayout = nullptr;

	//GLOW BLOB THAT TAKES A TRIP TO GLOW.H
	ID3DBlob* glowBlob = nullptr;

	//Samplers
	ID3D11SamplerState* sampState1 = nullptr;
	ID3D11SamplerState* sampState2 = nullptr;
	ID3D11BlendState* gBlendStateTransparency = nullptr;
	ID3D11BlendState* gBlendStateDefault = nullptr;

	//Shaders
	ID3D11VertexShader* splatMapVertexShader = nullptr;
	ID3D11VertexShader* gVertexShader = nullptr;
	ID3D11VertexShader* dVertexShader = nullptr;
	ID3D11VertexShader* shadowVertexShader = nullptr;
	ID3D11VertexShader* gWireFrameVertexShader = nullptr;
	ID3D11VertexShader* shader2DVS = nullptr;
	ID3D11VertexShader* glowVertexShader = nullptr;
	ID3D11VertexShader* horizontalBlurVertexShader = nullptr;
	ID3D11VertexShader* verticalBlurVertexShader = nullptr;
	ID3D11VertexShader* heightVertexShader = nullptr;


	ID3D11GeometryShader* gGeometryShader = nullptr;
	ID3D11GeometryShader* gBackFaceShader = nullptr;

	ID3D11PixelShader* gPixelShader = nullptr;
	ID3D11PixelShader* dPixelShader = nullptr;
	ID3D11PixelShader* splatMapPixelShader = nullptr;
	ID3D11PixelShader* hitPixelShader = nullptr;
	ID3D11PixelShader* shadowPixelShader = nullptr;
	ID3D11PixelShader* gWireFramePixelShader = nullptr;
	ID3D11PixelShader* gWireFramePixelShaderCONTAINTEST = nullptr;
	ID3D11PixelShader* shader2DPS = nullptr;
	ID3D11PixelShader* glowPixelShader = nullptr;
	ID3D11PixelShader* horizontalBlurPixelShader = nullptr;
	ID3D11PixelShader* verticalBlurPixelShader = nullptr;
	ID3D11PixelShader* heightPixelShader = nullptr;

	//Render States
	ID3D11RasterizerState* NoBcull;
	ID3D11Buffer* d2dIndexBuffer;
	int m_bitmapWidth, m_bitmapHeight = 100;

	//blendstates and transparency
	void BlendStates();
	ID3D11BlendState *transparency;
	ID3D11RasterizerState *counterCWCullmode; //dessa används vid transpareny mojs
	ID3D11RasterizerState *CWCullmode;


	//ENABLE/DISABLE DEPTH
	ID3D11DepthStencilState* m_depthDisabledStencilState;
	ID3D11DepthStencilState* m_depthStencilState;
	void TurnZBufferOn();
	void TurnZBufferOff();
	void AlphaBlendOn();
	void AlphaBlendOff();
	// SHADER TESTER
	ID3D11Buffer* shaderTest = nullptr;
	//Structs for cBuffers
	struct Options
	{
		int option1;
		int option2;
		int option3;
		int option4;
		int option5;
		int option6;
		int option7;
		int option8;
	};
	Options optionStruct;

public:

	int speedMultiplier=1;
	void SetFonts();
	float fps2;
	//PICKING
	BoundingBox testBox;
	Ray testRay;
	bool check ;
	bool check2;
	float tempDist;
	bool picking;
	//Picking
	void TestInterSection(float mouseX, float mouseY, Vector4& pickRayInWorldSpacePos, Vector4& pickRayInWorldSpaceDir);
	float pick(Vector4 pickRayInWorldSpacePos, Vector4 pickRayInWorldSpaceDir, std::vector<XMFLOAT3>& vertPosArray, std::vector<int>& indexPosArray, XMMATRIX& worldSpace);
	bool PointInTriangle(Vector4& triV1, Vector4& triV2, Vector4& triV3, Vector4& point);

};