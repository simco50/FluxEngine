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

#ifdef COMPILE_VS
PS_INPUT VSMain(VS_INPUT input)
{
      PS_INPUT output = (PS_INPUT)0;

      output.position = mul(float4(input.position.xy, 0.f, 1.f), cViewProj);
      output.color = input.color;
      output.texCoord  = input.texCoord;

      return output;
}
#endif

#ifdef COMPILE_PS
float4 PSMain(PS_INPUT input) : SV_TARGET
{
      return input.color * tDiffuseTexture.Sample(sDiffuseSampler, input.texCoord);
}
#endif