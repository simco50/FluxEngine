#include "add.hlsl"

cbuffer lol
{
	float4 ele = float4(1,0,0,0);
}
float4 testThing;

cbuffer kaka
{
	float4 elema = float4(1,0,0,0);
	float4x4 mat;
}

void VS(float4 position : POSITION, out float4 positionOut : OUTPOSITION)
{
	float a, b;
	Add(a, b);

	positionOut = position + ele + elema + testThing;
}

void PS(float4 positionIn : POSITION, out float4 colorOut : SV_TARGET)
{
	colorOut = positionIn;
}