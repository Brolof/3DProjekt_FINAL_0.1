// Edited by Oskar Hellstr ? m
//Edited 2015-01-21  11:30

#include <windows.h>
#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <string>

using namespace DirectX;

// Define Material
struct Material
{
	
	Material() { ZeroMemory(this, sizeof(this)); }
	
	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;
	XMFLOAT4 Reflection;
};


//DEFINE DIFFERENT TYPES OF LIGHTS

struct DirLight
{
	DirLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;
	XMFLOAT3 Dir;
	float Pad; // To make more then 1 light

};

struct PointLight
{
	PointLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;
	
	XMFLOAT3 Pos;
	float Range;

	XMFLOAT3 Attenuation;
	float Pad;  // To make more then 1 light

};

struct SpotLight
{
	SpotLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	XMFLOAT3 Pos;
	float Range;

	XMFLOAT3 Dir;
	float Spot;

	XMFLOAT3 Attenuation;
	float Pad;  // To make more then 1 light

};