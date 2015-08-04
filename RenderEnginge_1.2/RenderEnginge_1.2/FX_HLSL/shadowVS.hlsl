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
	float4 Pos		: POSITION;
	float2 Tex		: TEXCOORD;
	float3 normals	: NORMAL;
	//LightViewPos for shadow calc
	float4 wPos		: SV_POSITION;
	float4 lightViewPos : TEXCOORD1;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
VS_OUT VS_main(VS_IN input)
{
	VS_OUT output;

	float4 inputpos = float4(input.Pos, 1.0f);
	// Change the position vector to be 4 units for proper matrix calculations.
	

	// Calculate the position of the vertex against the world, view, and projection matrices.
	inputpos = mul(inputpos, WorldSpace);
	inputpos = mul(inputpos, View);
	inputpos = mul(inputpos, Projection);
	output.wPos = inputpos;

	output.Pos = mul(float4(input.Pos, 1.0f), WorldSpace);
	// Calculate the position of the vertice as viewed by the light source.
	output.lightViewPos = mul(inputpos, WorldSpace);
	output.lightViewPos = mul(output.lightViewPos, View);
	output.lightViewPos = mul(output.lightViewPos, Projection);

	// Store the texture coordinates for the pixel shader.
	output.Tex = input.Tex;

	// Calculate the normal vector against the world matrix only.
	output.normals = mul(input.normals, (float3x3)WorldSpace);

	// Normalize the normal vector.
	output.normals = normalize(output.normals);

	return output;
};