


struct VS_OUT{
	float4 wPos		: SV_POSITION;
};

float4 main() : SV_Target
{
	return float4(0.0f, 1.0f, 0.0f, 1.0f);
}