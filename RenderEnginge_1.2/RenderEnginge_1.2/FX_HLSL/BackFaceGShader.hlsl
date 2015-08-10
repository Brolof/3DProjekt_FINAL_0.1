
cbuffer ViewBuffer : register(b4)
{
	float3 ViewPoint;
	float pad;

};


struct GS_IN
{
	float3 pos		: POSITION;
	float2 Tex		: TEXCOORD;
	float4 normals  : NORMAL;
	float4 wpos		: SV_POSITION;
};

struct GS_OUT
{
	float3 pos		: POSITION;
	float2 Tex		: TEXCOORD;
	float4 normals  : NORMAL;
	float4 wpos		: SV_POSITION;
};


[maxvertexcount(3)]
void gs_main(triangle GS_IN vertexInput[3], inout TriangleStream<GS_OUT> TriangleOutput)
{

	GS_OUT bthquads[3];

		float3 Vec1 = float3(vertexInput[1].wpos.xyz) - float3(vertexInput[0].wpos.xyz);
		float3 Vec2 = float3(vertexInput[2].wpos.xyz) - float3(vertexInput[0].wpos.xyz);

		float3 NormalDir = normalize(cross(Vec1.xyz, Vec2.xyz));
		float3 ViewDir = normalize(float3(vertexInput[0].wpos.xyz) - float3(ViewPoint));


		float NormalPointer = dot(ViewDir, NormalDir);
	if(NormalPointer < 0){


	for (int i = 0; i<3; i++)
	{
		
		bthquads[i].pos = vertexInput[i].pos;
		bthquads[i].Tex = vertexInput[i].Tex;
		bthquads[i].normals = (vertexInput[i].normals);
		bthquads[i].wpos = vertexInput[i].wpos;
	
	

		TriangleOutput.Append(bthquads[i]);
	}

	TriangleOutput.RestartStrip();
	}
}