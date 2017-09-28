Texture2D tex : register(t0);

SamplerState state : register(s0);

struct VS_INPUT
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
};

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

void VS(VS_INPUT input, out PS_INPUT output)
{
	output.position = float4(input.position.x, input.position.y, 0, 1);
	output.texCoord = input.texCoord;
}

void PS(PS_INPUT input, out float4 output : SV_TARGET)
{
	output = tex.Sample(state, input.texCoord);
}
