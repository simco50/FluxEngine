cbuffer cPerModel : register(b0)
{
	float4 cColor;
	float3 cLightDirection;
	float4x4 cWorld;
	float4x4 cWorldViewProj;
}


struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
};

void VS(VS_INPUT input, out PS_INPUT output)
{
	output.position = mul(cWorldViewProj, float4(input.position, 1.0f));
	output.normal = normalize(mul((float3x3)cWorld, input.normal));
}

void PS(PS_INPUT input, out float4 output : SV_TARGET)
{
	float diffuseStrength = saturate(dot(input.normal, -cLightDirection));
	output = float4(cColor.rgb * diffuseStrength, 1.0f);
}
