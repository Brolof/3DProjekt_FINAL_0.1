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

cbuffer LightMatrix : register (b1)
{

	matrix lightView2;
	matrix lightProjection2;
};

cbuffer CamBuffer : register (b4)
{

	float3 viewPoint;
	float pad;
};



struct VS_IN
{

	float3 Pos		: POSITION;
	float2 Tex		: TEXCOORD;
	float3 normals	: NORMAL;
	float3 tangent : TANGENT;


};

struct VS_OUT
{
	float4 Pos		: POSITION;
	float2 Tex		: TEXCOORD;
	float3 normals	: NORMAL;
	float3 tangent : TANGENT;

	
	//LightViewPos for shadow calc
	float4 wPos		: SV_POSITION;
	float4 lightViewPos : TEXCOORD1;
	float3 lightPos : TEXCOORD2;
	float3 viewDir : TEXCOORD3;
	
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
VS_OUT VS_main(VS_IN input)
{


	VS_OUT output;
	float4 worldPosition;

	float4 inputpos = float4(input.Pos, 1.0f);

		float4 inputpos2 = float4(input.Pos, 1.0f);
	// Change the position vector to be 4 units for proper matrix calculations.
	

	// Calculate the position of the vertex against the world, view, and projection matrices.
	inputpos = mul(inputpos, WorldSpace);
	inputpos = mul(inputpos, View);
	inputpos = mul(inputpos, Projection); 
	output.wPos = inputpos;

	output.Pos = mul(float4(input.Pos, 1.0f), WorldSpace);
	// Calculate the position of the vertice as viewed by the light source.
	output.lightViewPos = mul(inputpos2, WorldSpace);
	output.lightViewPos = mul(output.lightViewPos, lightView2);
	output.lightViewPos = mul(output.lightViewPos, lightProjection2);

	// Store the texture coordinates for the pixel shader.
	output.Tex = input.Tex;

	// Calculate the normal vector against the world matrix only.
	output.normals = mul(input.normals, (float3x3)WorldSpace);

	//NORMAL MAPPING
	output.tangent = mul(input.tangent, (float3x3)WorldSpace);

	// Normalize the normal vector.
	output.normals = normalize(output.normals);

	// Calculate the position of the vertex in the world.
	worldPosition = mul(input.Pos, WorldSpace);

	// Determine the light position based on the position of the light and the position of the vertex in the world.
<<<<<<< HEAD
	output.lightPos = float3(0.0f, 20.0f, -10.0f);// -worldPosition.xyz;
=======
	output.lightPos = float3(0.0f, 8.0f, 0.0f);// -worldPosition.xyz;
>>>>>>> Merged
	output.lightPos = normalize(output.lightPos);

	// Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
	output.viewDir = viewPoint.xyz;// -worldPosition.xyz;

	// Normalize the viewing direction vector.
	//output.viewDir = normalize(output.viewDir);
	return output;
};