//////////////
// TEXTURES //
//////////////
Texture2D txDiffuse : register(t0);
Texture2D depthMapTexture : register(t1);

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


//////////////
// TYPEDEFS //
//////////////
struct VS_OUT
{
	float4 position : POSITION;
	float2 tex : TEXCOORD;
	float3 normal : NORMAL;
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
	float2 projectTexCoord;
	float depthValue;
	float lightDepthValue;
	float lightIntensity;
	float4 textureColor;
	float3 lightDir;

	//NEW
	float3 reflection;
	float4 specular;
	//

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
	
	//GÅR ALDRIG INN I LOOPEN
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
		if (lightDepthValue < depthValue)
		{
			//We calculate directional lighting now instead of positional.

				// Calculate the amount of light on this pixel.
			lightIntensity = saturate(dot(input.normal, lightDir));

			if (lightIntensity > 0.0f)
			{
				// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
				color += (lDir.Diffuse * lightIntensity);

				// Saturate the final light color.
				color = saturate(color);

				// Calculate the reflection vector based on the light intensity, normal vector, and light direction.
				reflection = normalize(2 * lightIntensity * input.normal - lDir.Dir);
					// Determine the amount of specular light based on the reflection vector, viewing direction, and specular power.
				specular = pow(saturate(dot(reflection, input.viewDir)), 10.0f);
			

			}
		}
	}
//	I have added an additional else clause to handle the case where objects are not inside the shadow map area.If we find that a pixel is outside this region then we do just regular directional lighting.Note that you can comment out this else clause to see exactly where your shadow map range begins and ends.

	else
	{
		// If this is outside the area of shadow map range then draw things normally with regular lighting.
		lightIntensity = saturate(dot(input.normal, lightDir));
		if (lightIntensity > 0.0f)
		{ 
			color += (lDir.Diffuse * lightIntensity);
			color = saturate(color);
		}
	}

	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	textureColor = txDiffuse.Sample(sampWrap, input.tex);

	// Combine the light and texture color.
	color = color* textureColor;
	color = saturate(color + specular);
	return color;



}