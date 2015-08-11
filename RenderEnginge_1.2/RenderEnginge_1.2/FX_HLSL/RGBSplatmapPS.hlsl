//PIXEL SHADER
#include "LightHelper.fx"

Texture2D RTexture : register(t0);
Texture2D GTexture : register(t1);
Texture2D BTexture : register(t2);
Texture2D SplatTexture : register(t3);
SamplerState normalSamState : register(s8);
SamplerState wrapSamState : register(s9);


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

cbuffer heightmapInfo : register(b3){
	float heightElements;
	float3 pad1;
}

struct VS_OUT
{
	float4 Pos		: POSITION;
	float2 Tex		: TEXCOORD;
	float2 AlphaTex : TEXCOORDALPHA;
	float3 normals	: NORMAL;
	//LightViewPos for shadow calc
	float4 wPos		: SV_POSITION;
	float4 lightViewPos : TEXCOORD1;
};


float4 main(VS_OUT input) : SV_Target
{
	input.normals = normalize(input.normals);
	//input.tunormal = float4(abs(input.tunormal.x), abs(input.tunormal.y), abs(input.tunormal.z), 1.0f);
	float3 lpos = lPoint.Pos;
	float3 toEyeWorld = normalize(gEyePos - input.Pos);

		//set deafult values
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 A, D, S;

	

	float4 rTex = RTexture.Sample(wrapSamState, input.Tex);
		float4 gTex = GTexture.Sample(wrapSamState, input.Tex);
		float4 bTex = BTexture.Sample(wrapSamState, input.Tex);
		//float4 splatTex = SplatTexture.Sample(normalSamState, input.AlphaTex);
		float4 splatTex = SplatTexture.Sample(normalSamState, input.Tex / heightElements);

		float4 finalTexture = (rTex * splatTex.r + gTex * splatTex.g + bTex * splatTex.b);

		float4 finalTex = float4(rTex.x * splatTex.r + gTex.x * splatTex.g + bTex.x * splatTex.b, rTex.y * splatTex.r + gTex.y * splatTex.g + bTex.y * splatTex.b, rTex.z * splatTex.r + gTex.z * splatTex.g + bTex.z * splatTex.b, 1);
		/*ComputePointLight(gMaterial, lPoint, input.Pos.xyz, input.tunormal, toEyeWorld, A, D, S);
		ambient += A;
		diffuse += D;
		spec	+= S;*/

	ComputeDirLight(gMaterial, lDir, input.normals, toEyeWorld, A, D, S);
	ambient += A;
	diffuse += D;
	spec += S;

	float4 finalcolor = finalTexture * (ambient + diffuse) + spec;// +float4(input.color, 1.0f);

		//See only tex
		//return Texdiffuse;

		//See with light
		return finalTex;
	//return finalTexture;
	//return float4(1,0,0,1);
};