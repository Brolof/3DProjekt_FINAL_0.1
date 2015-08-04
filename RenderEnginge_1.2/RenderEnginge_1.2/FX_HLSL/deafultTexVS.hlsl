//VERTEX SHADER
cbuffer World : register (c0)
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
	float3 tunormal : NORMAL;
};

//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------
VS_OUT VS_main(VS_IN input)
{
	VS_OUT output = (VS_OUT)0;

	float4 outpos = float4(input.Pos, 1.0f);
		outpos = mul(outpos, WVP);
	//	outpos = mul(outpos, View);
		//outpos = mul(outpos, Projection);

		output.Pos = outpos;
	output.Tex = input.Tex;
	output.tunormal = input.tunormal;

	return output;
}