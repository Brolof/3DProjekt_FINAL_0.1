Texture2D TextureToBeBlured : register(t0);

SamplerState sampWrap : register(s0);
SamplerState  sampClamp: register(s1);

struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float2 texCoord1 : TEXCOORD1;
	float2 texCoord2 : TEXCOORD2;
	float2 texCoord3 : TEXCOORD3;
	float2 texCoord4 : TEXCOORD4;
	float2 texCoord5 : TEXCOORD5;
	float2 texCoord6 : TEXCOORD6;
	float2 texCoord7 : TEXCOORD7;
	float2 texCoord8 : TEXCOORD8;
	float2 texCoord9 : TEXCOORD9;
};


float4 main(VS_OUT input) : SV_TARGET
{
	float weight0, weight1, weight2, weight3, weight4;
	float normalization;
	float4 color;

	// Create the weights that each neighbor pixel will contribute to the blur.
	weight0 = 1.0f;
	weight1 = 0.6f;
	weight2 = 0.55f;
	weight3 = 0.5f;
	weight4 = 0.4f;

	// Create a normalized value to average the weights out a bit.
	normalization = (weight0 + 2.0f * (weight1 + weight2 + weight3 + weight4));

	// Normalize the weights.
	weight0 = weight0 / normalization;
	weight1 = weight1 / normalization;
	weight2 = weight2 / normalization;
	weight3 = weight3 / normalization;
	weight4 = weight4 / normalization;

	color = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 test = TextureToBeBlured.Sample(sampClamp, input.texCoord1);
	// Add the nine horizontal pixels to the color by the specific weight of each.
	color += TextureToBeBlured.Sample(sampClamp, input.texCoord1) * weight4;
	color += TextureToBeBlured.Sample(sampClamp, input.texCoord2) * weight3;
	color += TextureToBeBlured.Sample(sampClamp, input.texCoord3) * weight2;
	color += TextureToBeBlured.Sample(sampClamp, input.texCoord4) * weight1;
	color += TextureToBeBlured.Sample(sampClamp, input.texCoord5) * weight0;
	color += TextureToBeBlured.Sample(sampClamp, input.texCoord6) * weight1;
	color += TextureToBeBlured.Sample(sampClamp, input.texCoord7) * weight2;
	color += TextureToBeBlured.Sample(sampClamp, input.texCoord8) * weight3;
	color += TextureToBeBlured.Sample(sampClamp, input.texCoord9) * weight4;

	//float4 texDiffuse = TextureToBeBlured.Sample(sampWrap, input.tex);
	color.a = 1.0f;
	//return float4(1, 0, 0, 1);
	return color;
}