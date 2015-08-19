//////////////
// TEXTURES //
//////////////
Texture2D txDiffuse : register(t0);
Texture2D depthMapTexture : register(t1);
Texture2D txNormalMap : register(t2);

//////////////
// INCLUDES //
//////////////
#include "LightHelper.fx"

///////////////////
// SAMPLE STATES //
///////////////////
SamplerState sampWrap : register(s0);
SamplerState  sampClamp: register(s1);
//////////////////////
// CONSTANT BUFFERS //
//////////////////////

cbuffer LightStruct : register(b0)
{
	DirLight	lDir;	
};
cbuffer MatView : register(b1)
{
	Material gMaterial;
	float3	gEyePos;
	float	pad;
};

cbuffer Options : register(b3)
{
	int option1;
	int option2;
	int option3;
	int option4;
	int option5;
	int option6;
	int option7;
	int option8;
};
//////////////
// TYPEDEFS //
//////////////
struct VS_OUT
{
	
	float4 Pos		: POSITION;
	float2 tex		: TEXCOORD;
	float4 normal	: NORMAL;
	float3 tangent : TANGENT;

	float4 wPos		: SV_POSITION;
	float4 lightViewPos : TEXCOORD1;
	float3 lightPos : TEXCOORD2;
	float3 viewDir : TEXCOORD3;
};



////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 PS_main(VS_OUT input) : SV_TARGET
{
	float bias;
	float4 color;
	float4 color2;
	float2 projectTexCoord;
	float depthValue;
	float lightDepthValue;
	float lightIntensity;
	float4 textureColor;
	float3 lightDir;
	//NORMAL MAPPING
	float4 normalMap;
	float3x3 texSpace;
	float3 biTangent;
	float3 bumpNormal;
	//NEW
	float3 reflection;
	float4 specular;
	//
	input.normal = normalize(input.normal);
	float4 texDiffuse = txDiffuse.Sample(sampWrap, input.tex);
	
		float4 dTexture = depthMapTexture.Sample(sampWrap, input.tex);
	//The pixel shader will need to invert the light direction.

		// Invert the light direction.
		lightDir = -lDir.Dir;

	// Set the bias value for fixing the floating point precision issues.
	bias = 0.001f;

	// Set the default output color to the ambient light value for all pixels.
	color = lDir.Ambient;

	// Calculate the projected texture coordinates.
	projectTexCoord.x = input.lightViewPos.x / input.lightViewPos.w / 2.0f + 0.5f;
	projectTexCoord.y = -input.lightViewPos.y / input.lightViewPos.w / 2.0f + 0.5f;

	// Determine if the projected coordinates are in the 0 to 1 range.  If so then this pixel is in the view of the light.

	//NORMAL MAPPING
	// The toEye vector is used in lighting.
	float3 toEye = input.viewDir - input.Pos.xyz;

		// Cache the distance to the eye from this surface point.
		float distToEye = length(toEye);

	// Normalize.
	toEye /= distToEye;

		//Load normals from normal map
		float4 normalSample = txNormalMap.Sample(sampWrap, input.tex);
		float3 normalT = 2.0f*normalSample - 1.0f;
		float3 N = input.normal.xyz;
		float3 T = normalize(input.tangent - dot(input.tangent, N)*N);
		float3 B = cross(N, T);
		float3x3 TBN = float3x3(T, B, N);

		// Transform from tangent space to world space.
		if (option1==1){ 
	input.normal.xyz = mul(normalT, TBN);
		}
	////NORMAL MAPPING

	// SHADOWS
	if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
	{
		// Sample the shadow map depth value from the depth texture using the sampler at the projected texture coordinate location.
		depthValue = depthMapTexture.Sample(sampClamp, projectTexCoord).r;

		// Calculate the depth of the light.
		lightDepthValue = input.lightViewPos.z / input.lightViewPos.w;
		// Subtract the bias from the lightDepthValue.
		lightDepthValue = lightDepthValue - bias;

		// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
		// If the light is in front of the object then light the pixel, if not then shadow this pixel since an object (occluder) is casting a shadow on it.
	//We calculate directional lighting now instead of positional.

		if (lightDepthValue < depthValue){

				// Calculate the amount of light on this pixel.
		//	lightIntensity = saturate(dot(input.normal, lightDir));

			lightIntensity = saturate(dot(input.normal.xyz, lightDir));
		
			if (lightIntensity > 0.0f)
			{


				// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
				color += (lDir.Diffuse * lightIntensity);

				// Saturate the final light color.
				color = saturate(color);

				// Calculate the reflection vector based on the light intensity, normal vector, and light direction.
				//	reflection = normalize(2 * lightIntensity * input.normal - float3(0, -10, 0));
				reflection = normalize(2 * lightIntensity * input.normal.xyz - float3(0, -10, 0));
				// Determine the amount of specular light based on the reflection vector, viewing direction, and specular power.
				specular = pow(saturate(dot(reflection, toEye)), 10.0f);

			}
			}
		
	}
	//	IF TEST FOR  OBJECTS OUTSIDE SHADOW MAP
	else
	{
		lightIntensity = saturate(dot(input.normal.xyz, lightDir));
		if (lightIntensity > 0.0f)
		{ 
			color += (lDir.Diffuse * lightIntensity);
			color = saturate(color);
		}
	}

	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	textureColor = txDiffuse.Sample(sampWrap, input.tex);

	// Combine the light and texture color.
	color = color;// *textureColor;
	color = saturate(color);

//NEW//




	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Reflect;
	// Initialize outputs.
	Ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	Diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	Specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The light vector aims opposite the direction the light rays travel.
	float3 lightVec = -lDir.Dir;

		// Add ambient term.
		Ambient = lDir.Ambient;

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.

	float diffuseFactor = dot(lightVec, input.normal);

	// Flatten to avoid dynamic branching.
	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, input.normal);
			float specFactor = pow(max(dot(v, toEye), 0.0f), 0.2f);

		Diffuse = diffuseFactor * lDir.Diffuse;
		Specular = specFactor * lDir.Specular;
	}


	color2 = textureColor*(Diffuse);

//NEW//

	//return Specular;
	if (option2 == 1){
	return float4(input.normal.xyz, 1.0f);
	}
	return color;


	// OK BELOW
	//	projected
	//	lightviewpos
}