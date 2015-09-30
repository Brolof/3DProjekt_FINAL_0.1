
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
	float3 tangent : TANGENT;

	float4 wpos		: SV_POSITION;
	float4 lightViewPos : TEXCOORD1;
	float3 lightPos : TEXCOORD2;
	float3 viewDir : TEXCOORD3;
};

struct GS_OUT
{
	float3 pos		: POSITION;
	float2 Tex		: TEXCOORD;
	float4 normals  : NORMAL;
	float3 tangent : TANGENT;

	float4 wpos		: SV_POSITION;
	float4 lightViewPos : TEXCOORD1;
	float3 lightPos : TEXCOORD2;
	float3 viewDir : TEXCOORD3;
};


[maxvertexcount(3)]
void gs_main(triangle GS_IN vertexInput[3], inout TriangleStream<GS_OUT> TriangleOutput)
{


	GS_OUT bthquads[3];

	//NORMAL MAPPING
	/*	float3 testTangent = float3(0, 0, 0);
	float tangentTex1 = 0;
	float tangentTex2 = 0;
	tangentTex1 = (vertexInput[1].Tex.y) - (vertexInput[0].Tex.y);
	tangentTex2 = (vertexInput[2].Tex.y) - (vertexInput[0].Tex.y);
	testTangent = normalize(tangentTex2*Vec1 - tangentTex1*Vec2);
	*/
	//	
	float3 Vec1 = float3(vertexInput[1].pos.xyz) - float3(vertexInput[0].pos.xyz);
		float3 Vec2 = float3(vertexInput[2].pos.xyz) - float3(vertexInput[0].pos.xyz);


		float3 NormalDir = normalize(cross(Vec1.xyz, Vec2.xyz));

		float3 ViewDir = normalize(float3(vertexInput[0].pos.xyz) - float3(ViewPoint));


		float NormalPointer = dot(ViewDir, NormalDir);
	if (NormalPointer < 0){

		for (int i = 0; i<3; i++)
		{

			bthquads[i].pos = vertexInput[i].pos;
			bthquads[i].Tex = vertexInput[i].Tex;
			bthquads[i].normals = float4(NormalDir, 1.0f);// vertexInput[i].normals;// (vertexInput[i].normals);
			bthquads[i].tangent = (vertexInput[i].tangent);
			bthquads[i].wpos = vertexInput[i].wpos;
			bthquads[i].lightViewPos = vertexInput[i].lightViewPos;
			bthquads[i].lightPos = vertexInput[i].lightPos;
			bthquads[i].viewDir = vertexInput[i].viewDir;


			TriangleOutput.Append(bthquads[i]);
		}

		TriangleOutput.RestartStrip();
	}
}