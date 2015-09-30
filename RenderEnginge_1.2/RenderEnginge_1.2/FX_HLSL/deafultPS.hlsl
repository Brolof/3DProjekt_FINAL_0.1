Texture2D txDiffuse : register(t0);
SamplerState sampAni: register(s0);


<<<<<<< HEAD
struct GeoOutPut
=======
SamplerState sampAni : register(s0);


cbuffer LightStruct : register(b0)
{
	DirLight		lDir;
	PointLight		lPoint;
	PointLight		lPoint1;
	SpotLight		lSpot;
};
cbuffer MatView : register(b1)
>>>>>>> Merged
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
	float4 tunormal : NORMAL;

<<<<<<< HEAD
=======
struct VS_OUT
{
	float4 Pos		: POSITION;
	float2 Tex		: TEXCOORD;

>>>>>>> Merged
};

float4 PS_main(GeoOutPut input) : SV_Target
{
<<<<<<< HEAD
	float4 diffuse = txDiffuse.Sample(sampAni, input.Tex);
	return float4(input.tunormal);
}
=======
	float4 Texdiffuse = txDiffuse.Sample(sampAni, input.Tex);

	return Texdiffuse;

};
>>>>>>> Merged
