#include "Uniforms.hlsl"
#include "Samplers.hlsl"

struct VS_INPUT
{
      float2 position : POSITION;
      float4 color : COLOR0;
      float2 texCoord  : TEXCOORD0;
};

struct PS_INPUT
{
      float4 position : SV_POSITION;
      float4 color : COLOR0;
      float2 texCoord  : TEXCOORD0;
};

void VS(VS_INPUT input, out PS_INPUT output)
{
      output.position = mul(cViewProjVS, float4(input.position.xy, 0.f, 1.f));
      output.color = input.color;
      output.texCoord  = input.texCoord;
}

void PS(PS_INPUT input, out float4 output : SV_TARGET)
{
      output = input.color * tDiffuseTexture.Sample(sDiffuseSampler, input.texCoord);
}