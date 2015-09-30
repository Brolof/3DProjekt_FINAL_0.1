
//BLUR TAR EMOT EN PLAN TEXTUR, KAN ALLTSÅ INTE RENDERA OBJEKT RAKT AV, BARA EN IMAGE
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

cbuffer Horizontal : register (b1){
	float screenSize; //på ett håll i taget!
	float blurAmount;
	float2 pad3;
}


struct VS_IN
{
	float3 Pos : POSITION;
	float2 Tex : TEXCOORD;
	
};

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


<<<<<<< HEAD
VS_OUT VS_main(VS_IN input)
=======
VS_OUT main(VS_IN input)
>>>>>>> Merged
{
	VS_OUT output;

	float4 inputpos = float4(input.Pos, 1.0f);
	
	inputpos = mul(inputpos, WorldSpace);
	//inputpos = mul(inputpos, View);
	//inputpos = mul(inputpos, Projection);
	output.Pos = inputpos;

	//output.Pos = mul(float4(input.Pos, 1.0f), WorldSpace);
	
	output.Tex = input.Tex;
	float texelSize = 1.0f / screenSize * blurAmount; //multiplicera denna för mer blur

	output.texCoord1 = input.Tex + float2(texelSize * -4.0f, 0.0f); //adda i u
	output.texCoord2 = input.Tex + float2(texelSize * -3.0f, 0.0f);
	output.texCoord3 = input.Tex + float2(texelSize * -2.0f, 0.0f);
	output.texCoord4 = input.Tex + float2(texelSize * -1.0f, 0.0f);
	output.texCoord5 = input.Tex + float2(texelSize *  0.0f, 0.0f);
	output.texCoord6 = input.Tex + float2(texelSize *  1.0f, 0.0f);
	output.texCoord7 = input.Tex + float2(texelSize *  2.0f, 0.0f);
	output.texCoord8 = input.Tex + float2(texelSize *  3.0f, 0.0f);
	output.texCoord9 = input.Tex + float2(texelSize *  4.0f, 0.0f);

	return output;
}
