//PIXEL SHADER
#include "LightHelper.fx"
Texture2D txDiffuse : register(t0);

//The Shadow Map, or the-
//-depth buffer map rendered from the light's perspective.
//Texture2D depthMapTexture : register(t1);

SamplerState sampAni : register(s0);


cbuffer LightStruct : register(b0)
{
	DirLight		lDir;
	PointLight		lPoint;
	PointLight		lPoint1;
	SpotLight		lSpot;	
};
cbuffer MatView : register(b1)
{
	Material gMaterial;
	float3	gEyePos;
	float	pad;
};

struct VS_OUT
{
	float4 Pos		: POSITION;
	float2 Tex		: TEXCOORD;
	float3 tunormal : NORMAL;
	float4 wPos		: SV_POSITION;
	//LightViewPos for shadow calc
	float4 lightViewPos : TEXCOORD1;
};

float4 PS_main(VS_OUT input) : SV_Target

{

	float3 toEyeWorld = gEyePos.xyz;
	float4 fincolor1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
	//set deafult values
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 A, D, S = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 normalen = float4(input.tunormal, 1.0f);
	float4 Texdiffuse = txDiffuse.Sample(sampAni, input.Tex);

	// ComputeDirLight(Material mat1, DirLight L1, float4 lightViewPos, float4 pos,float4 normal1, float3 toEye, SamplerState samplerType, Texture2D depthMap,out float4 ambient, out float4 diffuse, out float4 spec)
		//ComputeDirrLight(lDir, input.lightViewPos, input.Pos, float4(input.tunormal, 1.0f), gEyePos, sampAni, depthMapTexture,fincolor1);


		//ComputeDirLight(gMaterial, lDir, input.tunormal, toEyeWorld, A, D, S);
		//ambient += A;
		//diffuse += D;
		//spec	+= S;
		//fincolor += outColor;
		// Invert the light direction for calculations.




	float4 fincolor = (Texdiffuse*(fincolor1));// +float4(input.color, 1.0f);

		 //See only tex
		 //return Texdiffuse;

		 //See with light
		 //return float4(lPoint.Pos, 1.0f);
	
		 return Texdiffuse;
		//Test Light Pos
		//return float4(gMaterial.Specular);

		//See only color
		//return float4(gMaterial.Diffuse);
	

	//Debug normals
	//return float4(tempnormal.x, tempnormal.y, tempnormal.z, 1.0f);
	//return mul(float4(input.tunormal), float4(-1.0f, -1.0f, -1.0f, -1.0f));
	//return float4(abs(input.tunormal.x), abs(input.tunormal.y), abs(input.tunormal.z), 1.0f);
	//return float4(input.tunormal);
	//float3 tempnormal = mul(float3(0.0f, 0.0f, -1.0f), (input.tunormal.xyz));
};


//MAKE CONE LIGHT
//if (lightIntensity1 > 0.0f)
//{
////Calculate falloff from center to edge of pointlight cone
//color1 *= pow(max(dot(-lightPos1, lDir.Dir), 0.0f), lSpot.Spot);
//}





//MAKE POINT LIGHTS
//
//// Determine the light positions based on the position of the lights and the position of the vertex in the world.
//float3 lightPos1 = lPoint.Pos.xyz - input.Pos.xyz;
//float3 lightPos2 = lPoint1.Pos.xyz - input.Pos.xyz;
//
//// Normalize the light position vectors.
//lightPos1 = normalize(lightPos1);
//lightPos2 = normalize(lightPos2);
//
//
//// Initialize the specular color.
//float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
//
//float3 reflection;
//float3 lightDir;
//float lightIntensity, lightIntensity1, lightIntensity2;
//
//float4 color, color1, color2;
//float4 color;
//float3 lightDir;
//float lightIntensity
//lightDir = -lDir.Dir;
//// Set the default output color to the ambient light value for all pixels.
//color = lDir.Ambient;
//// Calculate the amount of light on this pixel.
//lightIntensity = saturate(dot(input.tunormal, lightDir));
//
//// Calculate the different amounts of light on this pixel based on the positions of the lights.
//lightIntensity1 = saturate(dot(input.tunormal, lightPos1))*lPoint.Range;
//lightIntensity2 = saturate(dot(input.tunormal, lightPos2))*lPoint1.Range;
////	The amount of color contributed by each point light is calculated from the intensity of the point light and the light color.
//
//// Determine the diffuse color amount of each of the four lights.
//color1 = lPoint.Diffuse * lightIntensity1;
//color2 = lPoint1.Diffuse* lightIntensity2;
//
////And finally the diffuse value of the light is combined with the texture pixel value to produce the color result.
//
//if (lightIntensity > 0.0f)
//{
//	// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
//	color += (lDir.Diffuse * lightIntensity);
//
//	// Saturate the ambient and diffuse color.
//	color = saturate(color);
//	//	The reflection vector for specular lighting is calculated here in the pixel shader provided the light intensity is greater than zero.This is the same equation as listed at the beginning of the tutorial.
//	//float3 v = reflect(-lightDir, input.tunormal);
//	// Calculate the reflection vector based on the light intensity, normal vector, and light direction.
//	reflection = normalize(2 * lightIntensity * input.tunormal + lightDir);
//	//	The amount of specular light is then calculated using the reflection vector and the viewing direction.The smaller the angle between the viewer and the light source the greater the specular light reflection will be.The result is taken to the power of the specularPower value.The lower the specularPower value the greater the final effect is.
//
//	// Determine the amount of specular light based on the reflection vector, viewing direction, and specular power.
//	specular = pow(saturate(dot(reflection, toEyeWorld)), 20.0f);
//}
//
//
//// Multiply the texture pixel and the input color to get the textured result.
//color = color * Texdiffuse;
////We don't add the specular effect until the end. It is a highlight and needs to be added to the final value or it will not show up properly.
//// Add the specular component last to the output color.
//color = saturate(color + specular);
//color = saturate(color1 + color2) * Texdiffuse;