#include "Uniforms.hlsl"
#include "Samplers.hlsl"
#include "Constants.hlsl"
#include "Lighting.hlsl"

struct VS_INPUT
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD0;
};

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float3 texCoord : TEXCOORD0;
};

#ifdef COMPILE_VS
PS_INPUT VSMain(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.position = mul(float4(input.position, 0.0f), cWorldViewProj).xyww;
	output.texCoord = input.position;
	return output;
}
#endif

#ifdef COMPILE_PS
float4 PSMain(PS_INPUT input) : SV_TARGET
{
	return tCubeTexture.Sample(sCubeSampler, input.texCoord);
}
#endif