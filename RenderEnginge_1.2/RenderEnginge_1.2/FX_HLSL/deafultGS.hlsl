// GEOMETRY SHADER


struct GS_IN
{
	float3 pos : POSITION;
	float2 Tex	: TEXCOORD;
	float4 normals : NORMAL;
	float4 wpos : SV_POSITION;
};

struct GS_OUT
{
	float3 pos				: POSITION;
	
	float2 Tex	: TEXCOORD;
	float4 normal		    : NORMAL;
	float4 wpos             : SV_POSITION;
};


[maxvertexcount(6)]
void gs_main(triangle GS_IN vertexInput[3], inout TriangleStream<GS_OUT> TriangleOutput)
{

	GS_OUT bthquads[3];

	float4 cross1 = vertexInput[1].wpos - vertexInput[0].wpos;
		float4 cross2 = vertexInput[2].wpos - vertexInput[0].wpos;
		float3 crossprod = cross(cross1.xyz, cross2.xyz);
		float4 fincross = float4(crossprod, 0);

	for (int i = 0; i < 3; i += 1)
	{
		for (int u = 0; u < 3; u += 1)
		{
			bthquads[u].wpos = vertexInput[u].wpos;// +float4(i*fincross);
			bthquads[u].normal = (vertexInput[u].normals);// , WorldSpaceInv);// +outpos;
			//bthquads[i].normal = mul(bthquads[i].normal, View);
			//bthquads[i].normal = mul(bthquads[i].normal, Projection);

			bthquads[u].pos = vertexInput[u].pos + float4(i*fincross);
			bthquads[u].Tex = vertexInput[u].Tex;

			TriangleOutput.Append(bthquads[u]);
		}
		TriangleOutput.RestartStrip();
	}

}