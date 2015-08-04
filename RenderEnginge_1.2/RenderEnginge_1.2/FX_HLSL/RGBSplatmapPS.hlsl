//PIXEL SHADER
#include "LightHelper.fx"

Texture2D RTexture : register(t0);
Texture2D GTexture : register(t1);
Texture2D BTexture : register(t2);
Texture2D SplatTexture : register(t3);
SamplerState normalSamState
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;
	AdressU = WRAP;
	AdressV = WRAP;
};


cbuffer Lights1: register (b1)
{
	PointLight lPoint;

	Material gMaterial;
	float3 gEyePos;

	float pad;

};

cbuffer cbPerFrame2: register (b2)
{
	DirLight lDir;


};

struct VS_OUT
{
	float3 Pos		: POSITION;
	float2 Tex		: TEXCOORD;
	float4 tunormal : NORMAL;
	float4 wPos		: SV_POSITION;
};

float4 main(VS_OUT input) : SV_Target
{
	input.tunormal = normalize(input.tunormal);
	//input.tunormal = float4(abs(input.tunormal.x), abs(input.tunormal.y), abs(input.tunormal.z), 1.0f);
	float3 lpos = lPoint.Pos;
	float3 toEyeWorld = normalize(gEyePos - input.Pos);

		//set deafult values
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 A, D, S;

	

	float4 rTex = RTexture.Sample(normalSamState, input.Tex);
	float4 gTex = GTexture.Sample(normalSamState, input.Tex);
	float4 bTex = BTexture.Sample(normalSamState, input.Tex);
	float4 splatTex = SplatTexture.Sample(normalSamState, input.Tex);
	
	float4 finalTexture = (rTex * splatTex.r + gTex * splatTex.g + bTex * splatTex.b);
		/*ComputePointLight(gMaterial, lPoint, input.Pos.xyz, input.tunormal, toEyeWorld, A, D, S);
		ambient += A;
		diffuse += D;
		spec	+= S;*/

	ComputeDirLight(gMaterial, lDir, input.tunormal, toEyeWorld, A, D, S);
	ambient += A;
	diffuse += D;
	spec += S;

	float4 finalcolor = finalTexture * (ambient + diffuse) + spec;// +float4(input.color, 1.0f);

		//See only tex
		//return Texdiffuse;

		//See with light
		return rTex;
	//return finalTexture;
	//return float4(1,0,0,1);
};