//VERTEX SHADER
cbuffer World : register (c0)
{
	matrix View;
	matrix Projection;
	matrix WorldSpace;
	matrix WorldSpaceInv;

	matrix lightView;
	matrix lightProjection;
};


struct VS_IN
{
	float3 Pos : POSITION;
	float2 Tex : TEXCOORD;
	float3 normals : NORMAL;
};

struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	
};


VS_OUT VS_main(VS_IN input)
{
	VS_OUT output;

	float4 outputPos;
	outputPos = float4(input.Pos, 1.0f);

	outputPos = mul(outputPos, WorldSpace);
	outputPos = mul(outputPos, View);
	outputPos = mul(outputPos, Projection);

	output.Pos = outputPos;

	//output.Pos = mul(float4(input.Pos, 1.0f), WorldSpace);

	output.Tex = input.Tex;

	return output;
}
