//////////////
// TEXTURES //
//////////////
Texture2D txDiffuse : register(t0);
Texture2D depthMapTexture : register(t1);
Texture2D txNormalMap : register(t2);

//////////////
// INCLUDES //
//////////////
#include "LightHelper.fx"

///////////////////
// SAMPLE STATES //
///////////////////
SamplerState sampWrap : register(s0);
SamplerState  sampClamp: register(s1);
//////////////////////
// CONSTANT BUFFERS //
//////////////////////

cbuffer LightStruct : register(b0)
{
	DirLight	lDir;
	PointLight pLights[200];
	SpotLight spot;
};
cbuffer MatView : register(b1)
{
	Material gMaterial;
	float3	gEyePos;
	float	pad;
};

cbuffer Options : register(b3)
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
//////////////
// TYPEDEFS //
//////////////
struct VS_OUT
{

	float4 Pos		: POSITION;
	float2 tex		: TEXCOORD;
	float3 normal	: NORMAL;
	float3 tangent : TANGENT;

	float4 wPos		: SV_POSITION;
	float4 lightViewPos : TEXCOORD1;
	float3 lightPos : TEXCOORD2;
	float3 viewDir : TEXCOORD3;
};



////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 PS_main(VS_OUT input) : SV_TARGET
{
	float4 color, color2;
	float4 textureColor;
	float3 lightDir;
	//NORMAL MAPPING
	float4 normalMap;
	float3x3 texSpace;
	float3 biTangent;
	float3 bumpNormal;
	float3 toEye;

	//// Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
	//input.viewDir = input.viewDir.xyz - input.Pos.xyz;

	//// Normalize the viewing direction vector.
	//input.viewDir = normalize(input.viewDir);
	//float dTeye = length(input.viewDir);
	//input.viewDir /= dTeye;

	//
	if (option1 == 1){
		//NORMAL MAPPING
		// The toEye vector is used in lighting.
		toEye = input.viewDir - input.Pos.xyz;

		// Cache the distance to the eye from this surface point.
		float distToEye = length(toEye);

		// Normalize.
		toEye /= distToEye;

		//Load normals from normal map
		float4 normalSample = txNormalMap.Sample(sampWrap, input.tex);
			float3 normalT = 2.0f*normalSample - 1.0f;
			float3 N = input.normal.xyz;
			float3 T = normalize(input.tangent - dot(input.tangent, N)*N);
			float3 B = cross(N, T);
			float3x3 TBN = float3x3(T, B, N);
			// Transform from tangent space to world space.

			input.normal.xyz = mul(normalT, TBN);
	}
	////NORMAL MAPPING

	//return Specular;
	if (option2 == 1){
		return float4(input.normal.xyz, 1.0f);
	}


	ComputeDirrLight(lDir, input.lightViewPos, input.Pos, input.normal, toEye, sampClamp, depthMapTexture, color, option7);
	textureColor = txDiffuse.Sample(sampWrap, input.tex);
	color = color*textureColor;
	color = saturate(color);

	ComputeSpotLight(gMaterial, spot, input.Pos, input.normal, toEye, color2);
	color2 = color2*textureColor;
	color2 = saturate(color2);

	//float attenuation = 0;
	//float3 lightvec = float3(0, 0, 0);

	//	//set deafult values
	//	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	//	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	//	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	//	float4 A, D, S;

	//for (int i = 0; i < 10; i++){
	//	ComputePointLight(gMaterial, pLights[i], input.Pos, float4(input.normal, 1.0f), input.viewDir, A, D, S);

	//	lightvec = float3(pLights[i].Pos) - float3(input.Pos.xyz);
	//	attenuation = saturate(1.0f - length(lightvec) / 21.1f);s
	//	ambient += A*attenuation;
	//	diffuse += D*attenuation;
	//	spec += S *attenuation;
	//}
	//float4 litColor = ambient + diffuse + spec;
	//	return float4(litColor)*textureColor;


	return color + color2;

}
