Texture2D txDiffuse : register(t0);

//konstant buffer med threshhold värde för glowen!!!!!!

SamplerState sampWrap : register(s0);
SamplerState  sampClamp: register(s1);

cbuffer GlowConstantBuffer : register(b3)
{
	float glowThreshHold;
	float glowValue;
	float2 pad3;
};

struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};


float4 main(VS_OUT input) : SV_TARGET
{
	float4 texDiffuse = txDiffuse.Sample(sampWrap, input.Tex);
	texDiffuse.a = 1.0f;

	saturate(texDiffuse);

	if (texDiffuse.r > glowThreshHold){
		texDiffuse.r = glowValue;
		
	}
	else
		texDiffuse.r = 0.0f;

	if (texDiffuse.g > glowThreshHold){
		texDiffuse.g = glowValue;
		
	}
	else
		texDiffuse.g = 0.0f;

	if (texDiffuse.b > glowThreshHold){
		texDiffuse.b = glowValue;
		
	}
	else
		texDiffuse.b = 0.0f;

	//return float4(glowValue, 0, 0, 1.0f);
	return texDiffuse;
}