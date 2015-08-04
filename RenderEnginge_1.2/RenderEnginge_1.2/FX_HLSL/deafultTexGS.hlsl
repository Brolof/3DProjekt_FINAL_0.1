// GEOMETRY SHADER

cbuffer World : register(c0)
{
	matrix View;
	matrix Projection;
	matrix WorldSpace;

};


struct GS_IN
{
	float4 pos : SV_POSITION;
	float3 color  : COLOR;
	float3 normals : NORMAL;
	float3 wpos : POSITION;
};

struct GS_OUT
{
	float4 pos              : SV_POSITION;
	float3 normal		    : NORMAL;
	float3 color			: COLOR;
	float3 wpos				: POSITION;
};


[maxvertexcount(3)]
void gs_main(triangle GS_IN vertexInput[3], inout TriangleStream<GS_OUT> TriangleOutput)
{

	GS_OUT bthquads[3];

	float4 cross1 = vertexInput[1].wpos - vertexInput[0].wpos;
		float4 cross2 = vertexInput[2].wpos - vertexInput[0].wpos;
		float3 crossprod = cross(cross1.xyz, cross2.xyz);
		float4 fincross = float4(crossprod, 0);

	for (int i = 0; i<3; i += 1)
	{
		float4 outpos = (vertexInput[i].pos) + float4(fincross);

			outpos = mul((outpos), (WorldSpace));
		outpos = mul((outpos), (View));
		outpos = mul((outpos), (Projection));

		bthquads[i].wpos = outpos;
		bthquads[i].normal = mul(vertexInput[i].normals, WorldSpace);
		bthquads[i].pos = vertexInput[i].pos;
		bthquads[i].color = vertexInput[i].color;


		TriangleOutput.Append(bthquads[i]);
	}
	TriangleOutput.RestartStrip();
}