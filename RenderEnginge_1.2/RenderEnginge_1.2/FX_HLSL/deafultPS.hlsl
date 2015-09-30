Texture2D txDiffuse : register(t0);
SamplerState sampAni: register(s0);



struct GeoOutPut
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
	float4 tunormal : NORMAL;

};

float4 PS_main(GeoOutPut input) : SV_Target
{
	float4 Texdiffuse = txDiffuse.Sample(sampAni, input.Tex);

	return Texdiffuse;

};

