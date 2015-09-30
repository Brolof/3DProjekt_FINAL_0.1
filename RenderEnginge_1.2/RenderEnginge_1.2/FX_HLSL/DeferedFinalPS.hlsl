Texture2D colorTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D txDepthTexture : register(t2);

SamplerState sampPoint : register(s0);


#include "LightHelper.fx"

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




struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
	float3 wPOS : TEXCOORD1;
	float3 ViewPoint : TEXCOORD2;
	float3 lightPos1 : TEXCOORD3;
};


float4 PS_main(VS_OUT input) : SV_TARGET
{
	

	float4 txDiff;
	float4 txNorm;
	float4 txDepth;

	txDiff = colorTexture.Sample(sampPoint, input.Tex);
	txNorm = normalTexture.Sample(sampPoint, input.Tex);
	txDepth = txDepthTexture.Sample(sampPoint, input.Tex);


			// Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
			input.ViewPoint = input.ViewPoint.xyz - txDepth.xyz;

		// Normalize the viewing direction vector.
		input.ViewPoint = normalize(input.ViewPoint);
		float dTeye = length(input.ViewPoint);
		input.ViewPoint /= dTeye;

		float attenuation = 0;
		float3 lightvec = float3(0, 0, 0);

			//set deafult values
			float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
			float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
			float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

			float4 A, D, S;
	
		for (int i = 0; i < 10; i++){
			ComputePointLight(gMaterial, pLights[i], txDepth, txNorm, input.ViewPoint, A, D, S);

			lightvec = float3(pLights[i].Pos) - float3(txDepth.xyz);
			 attenuation = saturate(1.0f - length(lightvec) / 6.7f);
			ambient += A*attenuation;
			diffuse += D*attenuation;
			spec += S *attenuation;
		

		}

		float4 litColor = ambient + diffuse + spec;
			return float4(litColor)*txDiff;
};
