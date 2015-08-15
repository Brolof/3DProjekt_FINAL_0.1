Texture2D txDiffuse : register(t0);

//konstant buffer med threshhold värde för glowen!!!!!!

SamplerState sampWrap : register(s0);
SamplerState  sampClamp: register(s1);

struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};


float4 main(VS_OUT input) : SV_TARGET
{
	float4 texDiffuse = txDiffuse.Sample(sampWrap, input.Tex);

	if (texDiffuse.r > 0.1f)
		texDiffuse.r = 1.0f;
	else
		texDiffuse.r = 0.0f;

	if (texDiffuse.g > 0.1f)
		texDiffuse.g = 1.0f;
	else
		texDiffuse.g = 0.0f;

	if (texDiffuse.b > 0.1f)
		texDiffuse.b = 1.0f;
	else
		texDiffuse.b = 0.0f;

	texDiffuse.a = 1.0f;
	return texDiffuse;
}