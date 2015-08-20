
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
	float Spot;

	float3 Att;
	float Pad;  // To make more then 1 light

};

//Light computations
//void ComputeDirLight(Material mat1, DirLight L1, float4 lightViewPos, float4 pos,
//	float4 normal1, float3 toEye, SamplerState samplerType, Texture2D depthMap,
//	out float4 ambient, out float4 diffuse, out float4 spec)
		 //ComputeDirrLight


	void ComputeDirrLight(DirLight L1, float4 lightViewPos,float4 pos,
	float4 normal1, float3 toEye, SamplerState samplerType, Texture2D depthMap,out float4 uColor)
{

	uColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
	//Output Colors
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
		
			//SHADOW MAPPING-----------////-----------////-----------////-----------////
			//Shadow mapping requires a bias adjustment when comparing the depth of the light and the depth of the object due to the low floating point precision of the depth map.
			float bias = 0.001f;	//Set the bias value for fixing the floating point precision issues.
			float2 projectTexCoord;
			float depthValue;
			float lightDepthValue;

			lightViewPos.xyz /= lightViewPos.w;

			//Calculate the projected texture coordinates for sampling the shadow map (depth buffer texture) based on the light's viewing position
			//lightViewPos.xy is in [-1, 1], but to sample the shadow map we need [0, 1], so scale by ½ and add ½
			projectTexCoord.x = lightViewPos.x / 2.0f + 0.5f;
			projectTexCoord.y = -lightViewPos.y / 2.0f + 0.5f;
			//SHADOW MAPPING-----------////-----------////-----------////-----------////

		
			float3 V = normalize(toEye - pos.xyz).xyz;

						//SHADOW MAPPING-----------////-----------////-----------////-----------////
						//Check if the projected coordinates are in the view of the light, if not then the pixel gets just an ambient value.
						//Determine if the projected coordinates are in the 0 to 1 range.  If so then this pixel is in the view of the light.
						if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
						{
							//Now that we are in the view of the light we will retrieve the depth value from the shadow map (depthMap). 
							//The depth value we get from the texture translates into the distance to the nearest object.
							//Sample the shadow map depth value from the depth texture using the sampler at the projected texture coordinate location.
							depthValue = depthMap.Sample(samplerType, projectTexCoord).r;	//We only sample the RED channel couse its a greyscale map with xy values

							//Now that we have the depth of the object for this pixel we need the depth of the light to determine if it is in front or behind the object.
							//We get this from the lightViewPosition. Note that we need to subtract the bias from this or we will get the floating point precision issue.
							lightDepthValue = lightViewPos.z / lightViewPos.w;		//Calculate the depth of the light.
							lightDepthValue = lightDepthValue - bias;		//Subtract the bias from the lightDepthValue.

							//Now we perform the comparison between the light depth and the object depth. If the light is closer to us then no shadow.
							//But if the light is behind an object in the shadow map then it gets shadowed. Note that a shadow just means we only apply ambient light. 
							//Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
							if (lightDepthValue < depthValue)	//If the light is in front of the object then light the pixel, if not then shadow this pixel since an object (occluder) is casting a shadow on it.
							{
								
								float3 lightDir;
								float lightIntensity;
								lightDir = -L1.Dir;
								// Set the default output color to the ambient light value for all pixels.
								color = float4(0.0f,1.0f,0.0f,1.0f);
								// Calculate the amount of light on this pixel.
								lightIntensity = saturate(dot(normal1, lightDir));

	
								//And finally the diffuse value of the light is combined with the texture pixel value to produce the color result.

								if (lightIntensity > 0.0f)
								{
									// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
									color += (L1.Diffuse * lightIntensity);
									// Saturate the ambient and diffuse color.
									color = saturate(color);	
									diffuse = L1.Diffuse;
									ambient = L1.Ambient;
									spec = L1.Specular;
								}
	
								color = saturate(color);
							}
						}
						//SHADOW MAPPING-----------////-----------////-----------////-----------////
					
			//outputcolor = color;



			//return color;
			uColor = L1.Diffuse;
}	

void ComputePointLight(Material mat1, PointLight L1,float3 pos, float4 normal1, float3 toEye, out float4 ambient, out float4 diffuse, out float4 spec)
{
	//Output Colors
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightvec =float3(L1.Pos)-float3(pos);

		//Distance from surface to pLight

		float d = length(lightvec);

		//Range test if the light hits the surface
	if (d < L1.Range)
		return;

		//Normalize lightvec
		
		lightvec /= d;

		//Ambient term made with light and mat
		ambient =  L1.Ambient;

		//Diffuse factor definition, if no light, no color

		float diffuseFactor = dot(lightvec, normal1);

		//Compute diffuse and spec
	
		if (diffuseFactor > 0.0f)
		{

			float3 v = reflect(-lightvec, normal1);
				float specFactor = mul(max(dot(v, toEye), 0.0f), mat1.Specular.w);

			diffuse = diffuseFactor*L1.Diffuse;
			spec = specFactor		*L1.Specular;
		}
		//Attenuate , also called dropof xD
		float att = 1.0f / dot(L1.Att, float3(1.0f, d, d*d));

		diffuse *= att;
		spec *= att;

}

void ComputeSpotLight(Material mat1, SpotLight L1, float3 pos, float3 normal1, float3 toEye, out float4 ambient, out float4 diffuse, out float4 spec)
{
	//Output Colors
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightvec = L1.Pos - pos;

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

	//Compute diffuse and spec
	if (diffuseFactor > 0.0f)
	{

		float3 v = reflect(-lightvec, normal1);
			float specFactor = pow(max(dot(v, toEye), 0.0f), mat1.Specular.w);

		diffuse = diffuseFactor * mat1.Diffuse*L1.Diffuse;
		spec = specFactor		* mat1.Specular*L1.Specular;
	}

	//Scale ligth by factor and attenuation
	float spot = pow(max(dot(-lightvec, L1.Dir), 0.0f), L1.Spot);

	float att = spot / dot(L1.Att, float3(1.0f, d, d*d));

		ambient *= att;
		diffuse *= att;
		spec *= att;

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
