#include "Uniforms.hlsl"

struct VS_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
};

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

void VS(VS_INPUT input, out PS_INPUT output)
{
	output.position = float4(input.position.x, input.position.y, 0, 1);
	float ping = (sin(cElapsedTimeVS) + 1) / 2.0f;
	output.color = float4(ping,1-ping,ping,1);
}

void PS(PS_INPUT input, out float4 output : SV_TARGET)
{
	output = input.color;
}
