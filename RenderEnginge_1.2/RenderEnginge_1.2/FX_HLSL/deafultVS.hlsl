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
	float3 normals : NORMAL;
	
};

struct VS_OUT
{
	float4 pos		: POSITION;
	float2 Tex		: TEXCOORD;
	float3 normals	: NORMAL;
	float4 wPos		: SV_POSITION;
	//LightViewPos for shadow calc
	float4 lightViewPos : TEXCOORD1;
};
//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------
VS_OUT VS_main(VS_IN input)
{
	VS_OUT output;

	float4 outpos = float4(input.Pos, 1.0f);
	outpos = mul(outpos,WorldSpace);
	outpos = mul(outpos, View);
	outpos = mul(outpos, Projection);
	output.wPos = mul(float4(input.Pos, 1.0f), WVP);

	output.pos = mul(float4(input.Pos,1.0f), WVP);
	output.Tex = input.Tex;

	output.normals = mul((input.normals), (float3x3)WorldSpace);
	normalize(output.normals);

	// Shadow Calculations
	//Calculate the vertex pos from the lights view 
	//(The distance explained with a new matrix)
	output.lightViewPos = mul(float4(input.Pos, 1.0), WorldSpace);
	output.lightViewPos = mul(output.lightViewPos, lightView);
	output.lightViewPos = mul(output.lightViewPos, lightProjection);

	return output;
};
