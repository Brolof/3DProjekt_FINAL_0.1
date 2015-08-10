
//VERTEX SHADER
cbuffer World : register (b0)
{
	matrix View;
	matrix Projection;
	matrix WorldSpace;
	matrix WorldSpaceInv;

	matrix lightView;
	matrix lightProjection;
};


struct VertexInputType
{
	float3 position : POSITION;
	float2 Tex : TEXCOORD;
	float3 normals : NORMAL;
};

struct PixelInputType
{
	float4 position :  SV_POSITION;
	float4 depthPosition : TEXTURE0;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType VS_main(VertexInputType input)
{
	PixelInputType output;


	// Change the position vector to be 4 units for proper matrix calculations.
	float4 position = float4(input.position, 1.0f);

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(position, WorldSpace);
	output.position = mul(output.position, View);
	output.position = mul(output.position, Projection);

	// Store the position value in a second input value for depth value calculations.
	output.depthPosition = output.position;

	return output;
}


