#include "LightHelper.fx"

Texture2D RTexture : register(t0);
Texture2D GTexture : register(t1);
Texture2D BTexture : register(t2);
Texture2D SplatTexture : register(t3);
SamplerState normalSamState : register(s8);
SamplerState wrapSamState : register(s9);

cbuffer heightmapInfo : register(b5){
	float heightElements;
	float3 pad1;
}


struct GeoOutPut
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
	float4 tunormal : NORMAL;
	float3 height: TEXCOORD1;

};

float4 PS_main(GeoOutPut input) : SV_Target{

	float4 diffuse = RTexture.Sample(normalSamState, input.Tex);
	diffuse.a = 1.0f;

	float4 rTex = RTexture.Sample(wrapSamState, input.Tex / heightElements * 2);
		float4 gTex = GTexture.Sample(wrapSamState, input.Tex / heightElements * 2);
		float4 bTex = BTexture.Sample(wrapSamState, input.Tex / heightElements * 2);
		//float4 splatTex = SplatTexture.Sample(normalSamState, input.AlphaTex);
		float4 splatTex = SplatTexture.Sample(normalSamState, input.Tex / heightElements); //heightelements är hur många vertiser det finns i ett led, uv delas alltså up mellan 0 och 1

		float4 finalTexture = (rTex * splatTex.r + gTex * splatTex.g + bTex * splatTex.b);

		float4 finalTex = float4(rTex.x * splatTex.r + gTex.x * splatTex.g + bTex.x * splatTex.b, rTex.y * splatTex.r + gTex.y * splatTex.g + bTex.y * splatTex.b, rTex.z * splatTex.r + gTex.z * splatTex.g + bTex.z * splatTex.b, 1);
		//return finalTex;
	if (input.height.y > -10.0f && input.height.y < 2.0f){
		return float4(rTex);
	}
	else if (input.height.y < 30.0f && input.height.y > 2.0f){
		return float4(gTex);
	}
	else if (input.height.y > 30.0){
		return float4(bTex);
	}
	else
		return float4(0, 0, 0, 1);
}