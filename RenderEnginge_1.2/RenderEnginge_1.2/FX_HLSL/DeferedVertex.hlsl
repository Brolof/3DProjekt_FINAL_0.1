//DEFERED VERTEX SHADER
cbuffer World : register (b0)
{
	matrix View;
	matrix Projection;
	matrix WorldSpace;
	matrix WorldSpaceInv;
	matrix WVP;

};


struct VS_IN
{
	float3 Pos : POSITION;
	float2 Tex : TEXCOORD;
	float3 normals : NORMAL;
	float3 tangent : TANGENT;
};

struct VS_OUT
{
	float4 pos		: SV_POSITION;
	float2 Tex		: TEXCOORD;
	float3 normals	: NORMAL;
	float3 tangent : TANGENT;
	float4 depthPosition : TEXTURE0;
};
//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------
VS_OUT VS_main(VS_IN input)
{
	VS_OUT output;

	float4 outpos = float4(input.Pos, 1.0f);
	outpos = mul(outpos, WorldSpace);
	output.depthPosition = outpos;
	outpos = mul(outpos, View);
	outpos = mul(outpos, Projection);
	output.pos = outpos;
	output.Tex = input.Tex;
	output.tangent = mul((input.tangent), (float3x3)WorldSpace);

	output.normals = mul((input.normals), (float3x3)WorldSpace);
	normalize(output.normals);

	return output;
};
