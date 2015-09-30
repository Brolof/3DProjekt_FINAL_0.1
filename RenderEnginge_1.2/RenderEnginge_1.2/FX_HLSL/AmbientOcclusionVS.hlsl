//VERTEX SHADER
cbuffer World : register (b0)
{
	matrix View;
	matrix Projection;
	matrix WorldSpace;
	matrix WorldSpaceInv;
	matrix WVP;
	matrix lightView;
	matrix lightProjection;
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
	float4 posH		: POSITION;
	float2 Tex		: TEXCOORD;
	float3 normals	: NORMAL;
	float4 posV		: SV_POSITION;

};
//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------
VS_OUT VS_main(VS_IN input)
{
	VS_OUT output;

	float4 outpos = float4(input.Pos, 1.0f);
		outpos = mul(outpos, WorldSpace);
	outpos = mul(outpos, View);
	outpos = mul(outpos, Projection);
	output.wPos = mul(float4(input.Pos, 1.0f), WVP);

	output.pos = mul(float4(input.Pos, 1.0f), WVP);
	output.Tex = input.Tex;

	output.normals = mul((input.normals), (float3x3)WorldSpace);
	normalize(output.normals);

	// Input pos 
	output.posV = mul(float4(input.Pos, 1.0), lightView);

	return output;
};
