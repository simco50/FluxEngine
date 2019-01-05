#include "Uniforms.hlsl"
#include "Samplers.hlsl"

struct VS_INPUT
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

void VSMain(VS_INPUT input, out VS_OUTPUT output)
{
	output.position = float4(input.position, 1);
	output.texCoord = input.texCoord;
}

void PSMain(VS_OUTPUT input, out float4 output : SV_TARGET)
{
	output = Sample2D(Diffuse, input.texCoord);
}
