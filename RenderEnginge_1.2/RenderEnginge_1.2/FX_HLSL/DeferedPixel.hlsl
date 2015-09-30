Texture2D txDiffuse : register(t0);
Texture2D txNorm : register(t1);
SamplerState sampAniWrap: register(s0);


struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
	float3 tunormal : NORMAL;
	float3 tangent : TANGENT;
	float4 depthPosition : TEXTURE0;
};

struct PS_OUT
{
	float4 color : SV_Target0;
	float4 normal : SV_Target1;
	float4 depth : SV_Target2;
};

PS_OUT PS_main(VS_OUT input) : SV_Target
{
	PS_OUT output;
	float depthValue;
	float4 depthValueFinal;

	//Load normals from normal map
	float4 normalSample = txNorm.Sample(sampAniWrap, input.Tex);
		float3 normalT = 2.0f*normalSample - 1.0f;
		float3 N = input.tunormal.xyz;
		float3 T = normalize(input.tangent - dot(input.tangent, N)*N);
		float3 B = cross(N, T);
		float3x3 TBN = float3x3(T, B, N);
		input.tunormal.xyz = mul(normalT, TBN);
	// Get the depth value of the pixel by dividing the  pixel depth by the W coordinate.
	depthValue = input.depthPosition.z / input.depthPosition.w;

	depthValueFinal = float4(depthValue, depthValue, depthValue, 1.0f);


		output.color = txDiffuse.Sample(sampAniWrap, input.Tex);
		output.normal = float4(input.tunormal.xyz, 1.0f);
		output.depth = input.depthPosition;
		return output;
}