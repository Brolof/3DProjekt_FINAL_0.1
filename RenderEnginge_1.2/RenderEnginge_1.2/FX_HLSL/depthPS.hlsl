
struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 depthPosition : TEXTURE0;
};

float4 PS_main(PixelInputType input) : SV_TARGET
{
	float depthValue;
	float4 color;


	// Get the depth value of the pixel by dividing the Z pixel depth by the homogeneous W coordinate.
	depthValue = input.depthPosition.z / input.depthPosition.w;

	color = float4(depthValue, depthValue, depthValue, 1.0f);
	// First 10% of the depth buffer color red.
	//if (depthValue < 0.9f)
	//{
	//	color = float4(1.0, 0.0f, 0.0f, 1.0f);
	//}

	//// The next 0.025% portion of the depth buffer color green.
	//if (depthValue > 0.9f)
	//{
	//	color = float4(0.0, 1.0f, 0.0f, 1.0f);
	//}

	//// The remainder of the depth buffer color blue.
	//if (depthValue > 0.955f)
	//{
	//	color = float4(0.0, 0.0f, 1.0f, 1.0f);
	//}

	//Return depthvalue as a float4 color
	//Black means shadow,white means lit
	return color;
}
