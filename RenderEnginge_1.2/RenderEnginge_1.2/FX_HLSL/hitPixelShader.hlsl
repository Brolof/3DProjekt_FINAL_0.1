//PIXEL SHADER
#include "LightHelper.fx"


struct VS_OUT
{
	float3 Pos		: POSITION;
	float2 Tex		: TEXCOORD;
	float4 tunormal : NORMAL;
	float4 wPos		: SV_POSITION;
};

float4 PS_main(VS_OUT input) : SV_Target

{
		return float4(1.0f,0.0f,0.0f,1.0f);

};