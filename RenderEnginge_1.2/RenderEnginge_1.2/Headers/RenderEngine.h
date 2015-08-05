#pragma once
#ifndef RENDERENGINE_H
#define RENDERENGINE_H
#endif


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


/////////////////////
#include "GameTimer.h"
#include "lights.h"
#include "OBJ.h"
#include "Input.h"
#include "HeightMap.h"
#include "Cam.h"
#include "GameObject.h"
#include "RenderToTexture.h"
#include "debugwindowclass.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"
#include "BINimporter.h"
#include "QuadTree.h"
//#include "depthClass.h"

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

	std::unique_ptr<SpriteBatch> spriteBatch;


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
	void subRender(int view);
	virtual void Release();
	void InputHandler();
	void renderToTexture();
	BINimporter theCustomImporter;
	vector<int> intArrayTex;
	std::vector<GameObjects*> renderObjects;
	
	//Shadows
	

public:
	void AddObject(GameObjects gObj);
	//ShadowMap* ShadowShader = new ShadowMap;
	
	//depthClass*	depthTexture = new depthClass;


	//Fps camera
	Vector4 camPosition;
	Vector4 camTarget;
	Vector4 camUp;// = Vector4(0.0f, 1.0f, 0.0f, 0.0f);;
	Vector4 DefaultForward = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
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

	

	//fpsgun
	Vector4 GunPosition;
	Vector4 GunTarget;
	Vector4 GunUp;// = Vector4(0.0f, 1.0f, 0.0f, 0.0f);;
	Vector4 GunDForward = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
	Vector4 GunDRight = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
	Vector4 GunForward = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
	Vector4 GunRight = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
	XMMATRIX GunRotationMatrix;
	XMMATRIX GunCamLook;

	float GunmoveLeftRight = 0.0f;
	float GunmoveBackForward = 0.0f;
	float GunYaw = 0.0f;
	float GunPitch = 0.0f;

	Camera fpsCam;
	POINT mLastMousePos;

	//Fps counter calc func
	void fpscounter();
	GameTimer gTimer;

	//Import Functions
	void ImportObj(char* geometryFileName, char* materialFileName, ID3D11Device* gDev);// , bool isStatic, XMMATRIX startPosMatrix);
	void ImportHeightmap(char* HeightMapFileName, wstring tex1File, wstring tex2File, wstring tex3File, wstring texSplatFile);

	//Window name
	std::wstring mainwname;

	//Geometry functions
	void CreatePlaneDataAndBuffers();
	void Shaders();
	void TextureFunc();

	//Rotation matrix
	XMMATRIX SharkRotation;

	//Picking
	void TestInterSection(float mouseX, float mouseY, Vector4& pickRayInWorldSpacePos, Vector4& pickRayInWorldSpaceDir);
	float pick(Vector4 pickRayInWorldSpacePos, Vector4 pickRayInWorldSpaceDir, std::vector<XMFLOAT3>& vertPosArray, std::vector<int>& indexPosArray, XMMATRIX& worldSpace);
	bool PointInTriangle(Vector4& triV1, Vector4& triV2, Vector4& triV3, Vector4& point);

	//Message handler
	LRESULT MsgProc(HWND hwindow, UINT msg, WPARAM wParam, LPARAM lParam);
	

	//Bool to test Backface culling
	bool Bculling = FALSE;
	bool Bcullingcheck = FALSE;

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
	struct WorldWireFrame{
		XMFLOAT4X4 View;
		XMFLOAT4X4 Projection;
		XMFLOAT4X4 WorldSpace;
	};

	World WorldMatrix1;
	WorldWireFrame WorldMatrixWF;

	struct shadowSettings
	{
		int shadowTesting;
		int pad1;
		int pad2;
		int pad3;
	};
	shadowSettings shadowBufferData;
	//LightBuffers
	struct LightStruct{
		
		DirLight		lDir;
		//float	pad;

	};
	float lightmove = 0;
	LightStruct PrimaryLights;

	struct MatView{
		Material gMaterial;
		XMFLOAT3	gEyePos;
		float	pad;

	};
	MatView MatBuffer1;

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
	//ViewPoint struct
	struct ViewBufferStruct{
		XMFLOAT3 viewPoint;
		float pad;
	};
	ViewBufferStruct ViewP;

	//Struct for HeightMap
	struct HeightMapObject{						 
		ID3D11Buffer* gIndexBuffer;		
		ID3D11Buffer* gVertexBuffer;
		int nmrElement;
		ID3D11ShaderResourceView* tex1shaderResourceView = nullptr;
		ID3D11ShaderResourceView* tex2shaderResourceView = nullptr;
		ID3D11ShaderResourceView* tex3shaderResourceView = nullptr;
		ID3D11ShaderResourceView* splatshaderResourceView = nullptr;
	};											 
	std::vector<HeightMapObject> HeightMapObjects;

	//quadtree!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	struct Float3{
		float x, y, z;
		Float3(float x, float y, float z){
			this->x = x;
			this->y = y;
			this->z = z;
		}
	};


	struct QuadTreeInstance{
		vector<GameObject> gameObjectsToRender;
		//vector<QuadTreeInstance> children;
		BoundingBox box;
		ID3D11Buffer *boxBuffer = nullptr;

		void SetValues(BoundingBox b, ID3D11Device* gDevice){
			box = b;
			std::vector<Float3> boxVertPoints;

			boxVertPoints.push_back(Float3(box.Center.x - box.Extents.x, box.Center.y - box.Extents.y, box.Center.z - box.Extents.z)); //0,0,0
			boxVertPoints.push_back(Float3(box.Center.x + box.Extents.x, box.Center.y - box.Extents.y, box.Center.z - box.Extents.z)); //1,0,0
			boxVertPoints.push_back(Float3(box.Center.x + box.Extents.x, box.Center.y + box.Extents.y, box.Center.z - box.Extents.z)); //1,1,0
			boxVertPoints.push_back(Float3(box.Center.x - box.Extents.x, box.Center.y + box.Extents.y, box.Center.z - box.Extents.z)); //0,1,0
			boxVertPoints.push_back(Float3(box.Center.x - box.Extents.x, box.Center.y - box.Extents.y, box.Center.z - box.Extents.z)); //0,0,0

			boxVertPoints.push_back(Float3(box.Center.x - box.Extents.x, box.Center.y - box.Extents.y, box.Center.z + box.Extents.z)); //0,0,1
			boxVertPoints.push_back(Float3(box.Center.x + box.Extents.x, box.Center.y - box.Extents.y, box.Center.z + box.Extents.z)); //1,0,1
			boxVertPoints.push_back(Float3(box.Center.x + box.Extents.x, box.Center.y + box.Extents.y, box.Center.z + box.Extents.z)); //1,1,1
			boxVertPoints.push_back(Float3(box.Center.x - box.Extents.x, box.Center.y + box.Extents.y, box.Center.z + box.Extents.z)); //0,1,1
			boxVertPoints.push_back(Float3(box.Center.x - box.Extents.x, box.Center.y - box.Extents.y, box.Center.z + box.Extents.z)); //0,0,1

			boxVertPoints.push_back(Float3(box.Center.x + box.Extents.x, box.Center.y - box.Extents.y, box.Center.z + box.Extents.z)); //1,0,1
			boxVertPoints.push_back(Float3(box.Center.x + box.Extents.x, box.Center.y - box.Extents.y, box.Center.z - box.Extents.z)); //1,0,0
			boxVertPoints.push_back(Float3(box.Center.x + box.Extents.x, box.Center.y + box.Extents.y, box.Center.z - box.Extents.z)); //1,1,0
			boxVertPoints.push_back(Float3(box.Center.x + box.Extents.x, box.Center.y + box.Extents.y, box.Center.z + box.Extents.z)); //1,1,1
			boxVertPoints.push_back(Float3(box.Center.x - box.Extents.x, box.Center.y + box.Extents.y, box.Center.z + box.Extents.z)); //0,1,1
			boxVertPoints.push_back(Float3(box.Center.x - box.Extents.x, box.Center.y + box.Extents.y, box.Center.z - box.Extents.z)); //0,1,0


			D3D11_BUFFER_DESC bDesc;
			ZeroMemory(&bDesc, sizeof(D3D11_BUFFER_DESC));
			bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bDesc.Usage = D3D11_USAGE_DEFAULT;
			bDesc.ByteWidth = sizeof(Float3)* (boxVertPoints.size());

			D3D11_SUBRESOURCE_DATA data;
			data.pSysMem = boxVertPoints.data();//<--------
			HRESULT VertexBufferChecker = gDevice->CreateBuffer(&bDesc, &data, &boxBuffer);
		}

		void TestContains(vector<GameObject> gameObjectsPossibleHit){ //skicka in alla gameobjects i världen, (gameObjectsInWorldSpace)
			for each (GameObject ob in gameObjectsPossibleHit)
			{
				if (ob.isStatic == true){ //bara de statiska objekten ska kunna cullas
					ContainmentType test = box.Contains(ob.bbox);
					if (test == 2 || test == 1){ //1 = intersects, 2 = contains, testa ifall nått gameobject ligger i just denna boxen, om den gör det så lägg till den
						gameObjectsToRender.push_back(ob);
					}
				}
			}
		}
	};
	vector<QuadTreeInstance> quadTree;
	void ListQuadTree(int nrSplits, XMFLOAT3 center, XMFLOAT3 extents);
	void CheckFrustumContains(int nrSplits, int);
	int nrSplitsTree = 2; //<-------- ÄNDRA DENNA OM VI BEHÖVER FLER SPLITS I QUADTRÄDET
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


	//Window handlers
	HWND hWindow;
	HINSTANCE hInstance;

	//Start Window functions
	bool InitWindow();
	bool InitDirect3D(HWND);

protected:
	
	//Core varibles for often use
	std::string applicationName;
	UINT screen_Width;
	UINT screen_Height;
	DWORD windowStyle; //se konstruktorn

	BoundingFrustum frustum;

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
	ID3D11Buffer* gWorld;
	ID3D11Buffer* ViewBuffer = nullptr;
	ID3D11Buffer* gLights;
	ID3D11Buffer* PrimaryLightBuffer;
	ID3D11Buffer* MatBuffer;
	ID3D11Buffer* shadowBuffer = nullptr;

	//Shader resources for textures and more
	ID3D11ShaderResourceView* ddsTex1= nullptr;
	ID3D11ShaderResourceView* ddsTex2 = nullptr;
	ID3D11ShaderResourceView* ddsTex3 = nullptr;
	ID3D11ShaderResourceView* AdsResourceView = nullptr;
	ID3D11ShaderResourceView* NpcRV = nullptr;
	ID3D11ShaderResourceView** RSWArray = nullptr;

	//Vertex/geometry layout desc
	ID3D11InputLayout* gVertexLayout = nullptr;
	ID3D11InputLayout* gWireFrameLayout = nullptr;
	

	//Samplers
	ID3D11SamplerState* sampState1 = nullptr;
	ID3D11SamplerState* sampState2 = nullptr;
	//Shaders
	ID3D11VertexShader* gVertexShader = nullptr;
	ID3D11VertexShader* dVertexShader = nullptr;
	ID3D11VertexShader* shadowVertexShader = nullptr;
	ID3D11VertexShader* gWireFrameVertexShader = nullptr;
	ID3D11VertexShader* shader2DVS = nullptr;

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


	//Render States
	ID3D11RasterizerState* NoBcull;


	// SHADOW THINGS


	public:
		const int SHADOWMAP_WIDTH = 1024;
		const int SHADOWMAP_HEIGHT = 1024;
		const float SHADOWMAP_DEPTH = 100.0f;
		const float SHADOWMAP_NEAR = 1.0f;

	UINT32 vertexSize3 = sizeof(float) * 8;
	UINT32 offset3 = 0;

	//World perObjCBData;
	XMMATRIX WVP;
	ID3D11DepthStencilState* gDepthStencilState = nullptr;
	
	
	//Shadow implement
	//The Camera Matrices are now defined in the camera class (mainCamera)
	
	//RENDER TO TEXTURE
	ID3D11Texture2D* renderTargetTextureMap;
	ID3D11RenderTargetView* renderTargetViewMap;
	ID3D11ShaderResourceView* shaderResourceViewMap;
	D3D11_VIEWPORT shadowVP;
	// Our render to textures camera's view and projection matrices
	XMMATRIX mapView;
	XMMATRIX mapProjection;


	XMMATRIX identityM2;
	XMMATRIX WorldInv2;

	void GenerateViewMatrix(XMVECTOR vec1);
	void GenerateProjectionMatrix(float, float);
	void GenerateOrthoMatrix(float, float, float);
	void GetOrthoMatrix(XMMATRIX&);
	void SetDirection(float, float, float);
	void GetViewMatrix(XMMATRIX&);
	void GetProjectionMatrix(XMMATRIX&);
	void SetPosition(float, float, float);
	void SetLookAt(float, float, float);
	void TurnZBufferOn();
	void TurnZBufferOff();
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetDirection();

	Vector3 m_direction;
	Vector3 m_position;
	Vector3 m_lookAt;
	XMMATRIX m_viewMatrix;
	XMMATRIX m_projectionMatrix;
	XMMATRIX m_orthoMatrix;
	ID3D11DepthStencilState* m_depthDisabledStencilState;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11Buffer* d2dIndexBuffer;
	int m_bitmapWidth, m_bitmapHeight = 100;

};