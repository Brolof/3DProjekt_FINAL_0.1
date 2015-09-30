//PIXEL SHADER
#include "LightHelper.fx"


struct VS_OUT
{
	float4 Pos		: POSITION;
	float2 Tex		: TEXCOORD;
	float3 normals	: NORMAL;
	float3 tangent : TANGENT;


	//LightViewPos for shadow calc
	float4 wPos		: SV_POSITION;
	float4 lightViewPos : TEXCOORD1;
	float3 lightPos : TEXCOORD2;
	float3 viewDir : TEXCOORD3;
};

float4 PS_main(VS_OUT input) : SV_Target

{
		return float4(1.0f,0.0f,0.0f,1.0f);

};