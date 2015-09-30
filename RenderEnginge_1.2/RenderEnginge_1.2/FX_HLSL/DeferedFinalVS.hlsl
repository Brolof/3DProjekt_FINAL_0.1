//VERTEX SHADER
cbuffer World2 : register (b3)
{
	matrix WVP;
	matrix worldMatrix;

};

cbuffer CamBuffer : register (b4)
{

	float3 viewPoint;
	float pad;
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
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
	float3 wPOS : TEXCOORD1;
	float3 ViewPoint : TEXCOORD2;
	float3 lightPos1 : TEXCOORD3;

};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
VS_OUT VS_main(VS_IN input)
{
	VS_OUT output;

	float3 worldPosition;
	float4 outpos = float4(input.Pos, 1.0f);
		outpos = mul(outpos, WVP);
	//	outpos = mul(outpos, View);
		//outpos = mul(outpos, Projection);

		output.Pos = outpos;
	output.Tex = input.Tex;


	//// Change the position vector to be 4 units for proper matrix calculations.
	//output.Pos.w = 1.0f;

	//// Calculate the position of the vertex against the world, view, and projection matrices.
	//output.Pos = mul(input.Pos, WorldSpace);
	//output.Pos = mul(output.Pos, View);
	//output.Pos = mul(output.Pos, Projection);
	worldPosition = float3(0, 0, 0);// mul(input.Pos, (float3x3)worldMatrix);
	output.wPOS = worldPosition;
	// Store the texture coordinates for the pixel shader.
	output.Tex = input.Tex;

	output.lightPos1.xyz = float3(0, 2, 2);// -output.Pos;
	//output.lightPos1 = normalize(output.lightPos1);

	output.ViewPoint = (viewPoint);// (float3x3)worldMatrix);


	return output;
}