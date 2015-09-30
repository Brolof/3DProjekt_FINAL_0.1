
 //Material definitions
struct Material
{

	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Reflection;
};

//DEFINE DIFFERENT TYPES OF LIGHTS

struct DirLight
{

	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Dir;
	float Pad; // To make more then 1 light

};

struct PointLight
{


	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Pos;

	float Range;

	float3 Att;
	
	float Pad;// To make more then 1 light
};

struct SpotLight
{
	

	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Pos;
	float Range;

	float3 Dir;
	float Spot; // The cone

	float3 Att;
	float Pad;  // To make more then 1 light

};

//Light computations

	void ComputeDirrLight(DirLight L1, float4 lightViewPos,float4 pos,
	float3 normal1, float3 toEye, SamplerState samplerType, Texture2D depthMap,out float4 uColor,int op8)
{

		// The vector from the surface to the light.
		float3 lightvec = float3(L1.Dir) - pos.xyz;

		float bias;
		float2 projectTexCoord;
		float depthValue;
		float lightDepthValue;
		float lightIntensity;
		float3 lightDir;
	
		float3 reflection;
		float4 specular;
		//
		// Invert the light direction.
		lightDir = -L1.Dir;

		// Set the default output color to the ambient light value for all pixels.
		uColor = L1.Ambient;


	
		// Set the bias value for fixing the floating point precision issues.
		bias = 0.001f;

		// Set the default output color to the ambient light value for all pixels.
		uColor = L1.Ambient;

		// Calculate the projected texture coordinates.
		projectTexCoord.x = lightViewPos.x / lightViewPos.w / 2.0f + 0.5f;
		projectTexCoord.y = -lightViewPos.y / lightViewPos.w / 2.0f + 0.5f;

		// Determine if the projected coordinates are in the 0 to 1 range.  If so then this pixel is in the view of the light.
		//The pixel shader will need to invert the light direction.
		if (op8 == false){
			lightIntensity = saturate(dot(normal1.xyz, lightDir));

			if (lightIntensity > 0.0f)
			{


				// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
				uColor += (L1.Diffuse * lightIntensity);
				uColor = saturate(uColor);
			}

		}
		// SHADOWS
		else if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
		{
			// Sample the shadow map depth value from the depth texture using the sampler at the projected texture coordinate location.
			depthValue = depthMap.Sample(samplerType, projectTexCoord).r;

			// Calculate the depth of the light.
			lightDepthValue = lightViewPos.z / lightViewPos.w;
			// Subtract the bias from the lightDepthValue.
			lightDepthValue = lightDepthValue - bias;

			// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
			// If the light is in front of the object then light the pixel, if not then shadow this pixel since an object (occluder) is casting a shadow on it.
			//We calculate directional lighting now instead of positional.

			if (lightDepthValue < depthValue){

				// Calculate the amount of light on this pixel.
				//	lightIntensity = saturate(dot(input.normal, lightDir));

				lightIntensity = saturate(dot(normal1.xyz, lightDir));

				if (lightIntensity > 0.0f)
				{


					// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
					uColor += (L1.Diffuse * lightIntensity);



					float3 v = reflect(-lightvec, normal1);
						float specFactor = pow(max(dot(v, toEye), 0.0f),16.0f);

					specular = specFactor*L1.Specular;
					// Saturate the final light color.
					uColor = saturate(uColor);


				}
			}

		}
		//	IF TEST FOR  OBJECTS OUTSIDE SHADOW MAP
		else
		{
			lightIntensity = saturate(dot(normal1.xyz, lightDir));
			if (lightIntensity > 0.0f)
			{
				uColor += (L1.Diffuse * lightIntensity);
				uColor = saturate(uColor);
			}
		}

}	

	
	void ComputePointLight(Material mat, PointLight L, float3 pos, float4 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 spec)
	{
		// Initialize outputs.
		ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
		diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

		// The vector from the surface to the light.
		float3 lightVec = float3(L.Pos) - pos;

			// The distance from surface to light.
			float d = length(lightVec);

		// Range test.
		if (d > 8)
			return;
		
		// Normalize the light vector.
		lightVec /= d;

		// Ambient term.
		ambient =  L.Ambient;

		// Add diffuse and specular term, provided the surface is in 
		// the line of site of the light.

		float diffuseFactor = dot(lightVec, normal);

		// Flatten to avoid dynamic branching.
		[flatten]
		if (diffuseFactor > 0.0f)
		{
			float3 v = reflect(-lightVec, normal);
				float specFactor = pow(max(dot(v, toEye), 0.0f), 32);

			diffuse = diffuseFactor  * L.Diffuse;
			spec = specFactor  * L.Specular;
		}
	//

		// Attenuate
		float att =  1.0f / dot(L.Att, float3(1.0f, d, d*d));

		//diffuse *= att;
		//spec *= att;
	}

void ComputeSpotLight(Material mat1, SpotLight L1, float3 pos, float3 normal1, float3 toEye, out float4 color)
{
	float4 ambient;
	float4 diffuse;
	float4 spec;
	//Output Colors
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightvec = float3(L1.Pos) - pos;

		//Distance from surface to pLight

		float d = length(lightvec);

	//Range test if the light hits the surface
	if (d > L1.Range)
		return;

	//Normalize lightvec

	lightvec /= d;

	//Ambient term made with light and mat
	ambient = mat1.Ambient * L1.Ambient;

	//Diffuse factor definition, if no light, no color

	float diffuseFactor = dot(lightvec, normal1);

	// Flatten to avoid dynamic branching.
	[flatten]
	//Compute diffuse and spec
	if (diffuseFactor > 0.0f)
	{

		float3 v = reflect(-lightvec, normal1);
			float specFactor = pow(max(dot(v, toEye), 0.0f), mat1.Specular.w);

		diffuse = diffuseFactor *L1.Diffuse;
		spec = specFactor		* mat1.Specular*L1.Specular;
	}

	//Scale ligth by factor and attenuation
	float spot = pow(max(dot(-lightvec, L1.Dir), 0.0f), L1.Spot);

	float att = spot / dot(L1.Att, float3(1.0f, d, d*d));

		ambient *= att;
		diffuse *= att;
		spec *= att;
		

		color = diffuse;

}


void ComputeDirLight(Material mat1, DirLight L1, float3 normal1, float3 toEye, out float4 ambient, out float4 diffuse, out float4 spec)
{
	//Output Colors
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightvec = -L1.Dir;

		//Ambient term made with light and mat
		ambient = mat1.Ambient * L1.Ambient;

	//Diffuse factor definition, if no light, no color

	float diffuseFactor = dot(lightvec, normal1);

	//Compute diffuse and spec
	if (diffuseFactor > 0.0f)
	{

		float3 v = reflect(-lightvec, normal1);
			float specFactor = pow(max(dot(v, toEye), 0.0f), mat1.Specular.w);

		diffuse = diffuseFactor * mat1.Diffuse*L1.Diffuse;
		spec = specFactor		* mat1.Specular*L1.Specular;
	}

}

// OLD DIR LIGHT
//
////Output Colors
//ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
//diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
//spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
//
//float3 lightvec = -L1.Dir;
//
////Ambient term made with light and mat
//ambient = mat1.Ambient * L1.Ambient;
//
////Diffuse factor definition, if no light, no color
//
//float diffuseFactor = dot(lightvec, normal1);
//
////Compute diffuse and spec
//if (diffuseFactor > 0.0f)
//{
//
//	float3 v = reflect(-lightvec, normal1);
//		float specFactor = pow(max(dot(v, toEye), 0.0f), mat1.Specular.w);
//
//	diffuse = diffuseFactor * mat1.Diffuse*L1.Diffuse;
//	spec = specFactor		* mat1.Specular*L1.Specular;
//}
//
