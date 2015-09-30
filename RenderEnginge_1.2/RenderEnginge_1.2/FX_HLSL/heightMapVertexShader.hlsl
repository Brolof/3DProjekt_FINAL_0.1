

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
	float3 tunormal : NORMAL;
};

struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
	float4 tunormal : NORMAL;
	float3 height: TEXCOORD1;
};

//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------
VS_OUT VS_main(VS_IN input)
{
	VS_OUT output = (VS_OUT)0;

	float4 outpos = float4(input.Pos, 1.0f);
		outpos = mul(outpos, WorldSpace);
		outpos = mul(outpos, View);
		outpos = mul(outpos, Projection);

	output.Pos = outpos;
	output.Tex = input.Tex;
	output.tunormal = float4(input.tunormal, 1.0f);
	output.height = float3(input.Pos);
	return output;
}