cbuffer PerFrame : register(b1)
{
      float4x4 gViewProj;
}

texture2D gTexture : register(t0);

SamplerState gTextureSampler : register(s0);

/*
BlendState gBlendState 
{     
    BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
	BlendOp = ADD;
	SrcBlendAlpha = ONE;
	DestBlendAlpha = ZERO;
	BlendOpAlpha = ADD;
	RenderTargetWriteMask[0] = 0x0f;
};

DepthStencilState gDepthStencilState
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
	DepthFunc = ALWAYS;
};

RasterizerState gRasterizerState
{
      CullMode = NONE;
      FillMode = SOLID;
      ScissorEnable = TRUE;
      DepthClipEnable = TRUE;
};*/

struct VS_INPUT
{
      float2 pos : POSITION;
      float4 col : COLOR0;
      float2 uv  : TEXCOORD0;
};

struct PS_INPUT
{
      float4 pos : SV_POSITION;
      float4 col : COLOR0;
      float2 uv  : TEXCOORD0;
};

void VS(VS_INPUT input, out PS_INPUT output)
{
      output.pos = mul(gViewProj, float4(input.pos.xy, 0.f, 1.f));
      output.col = input.col;
      output.uv  = input.uv;
}

void PS(PS_INPUT input, out float4 output : SV_TARGET)
{
      output = input.col * gTexture.Sample(gTextureSampler, input.uv);
}