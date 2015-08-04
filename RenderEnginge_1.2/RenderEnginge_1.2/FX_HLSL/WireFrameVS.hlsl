//VERTEX SHADER
cbuffer World : register (c3)
{
	matrix View;
	matrix Projection;
	matrix WorldSpace;

};


struct VS_IN
{
	float3 Pos : POSITION;
};

struct VS_OUT
{
	float4 wPos		: SV_POSITION;
};
//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------
VS_OUT main(VS_IN input)
{
	VS_OUT output;

	float4 outpos = float4(input.Pos, 1.0f);

	outpos = mul(outpos, WorldSpace);

	outpos = mul(outpos, View);
	outpos = mul(outpos, Projection);

	output.wPos = outpos;// float4(input.Pos.x, input.Pos.y, input.Pos.z, 1.0f);

	return output;
};
