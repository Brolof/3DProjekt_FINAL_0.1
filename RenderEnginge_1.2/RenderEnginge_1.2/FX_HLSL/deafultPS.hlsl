//PIXEL SHADER
#include "LightHelper.fx"
Texture2D txDiffuse : register(t0);

//The Shadow Map, or the-
//-depth buffer map rendered from the light's perspective.
//Texture2D depthMapTexture : register(t1);

SamplerState sampAni : register(s0);


cbuffer LightStruct : register(b0)
{
	DirLight		lDir;
	PointLight		lPoint;
	PointLight		lPoint1;
	SpotLight		lSpot;	
};
cbuffer MatView : register(b1)
{
	Material gMaterial;
	float3	gEyePos;
	float	pad;
};

struct VS_OUT
{
	float4 Pos		: POSITION;
	float2 Tex		: TEXCOORD;
	
};

float4 PS_main(VS_OUT input) : SV_Target

{
	float4 Texdiffuse = txDiffuse.Sample(sampAni, input.Tex);
		
	return Texdiffuse;
	
};

