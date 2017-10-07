#if defined(COMPILE_VS)
cbuffer cPerFrameVS : register(b0)
{
	float4x4 cWorld;
	float4x4 cWorldViewProj;
}
#endif

#if defined(COMPILE_PS)

cbuffer cPerFramePS : register(b0)
{
	float3 cLightDirection;
}

cbuffer cPerMeshPS : register(b1)
{
	float4 cColor;
}
#endif

Texture2D cTexture : register(t0);
SamplerState cSampler : register(s0);

struct VS_INPUT
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
};

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
};

void VS(VS_INPUT input, out PS_INPUT output)
{
	output.position = mul(cWorldViewProj, float4(input.position, 1.0f));
	output.normal = normalize(mul((float3x3)cWorld, input.normal));
	output.texCoord = input.texCoord;
}

void PS(PS_INPUT input, out float4 output : SV_TARGET)
{
	float diffuseStrength = saturate(dot(input.normal, -cLightDirection));
	float4 sample = cTexture.Sample(cSampler, input.texCoord);
	output = float4((sample.rgb * cColor.rgb) * diffuseStrength, 1.0f);
}
