float4x4 gViewProj : VIEWPROJ;
texture2D gTexture;

SamplerState gTextureSampler
{
      Filter = MIN_MAG_MIP_Linear;
      AddressU = WRAP;
      AddressV = WRAP;
      ComparisonFunc = ALWAYS;
};

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
};

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

PS_INPUT Main_VS(VS_INPUT input)
{
      PS_INPUT output;
      output.pos = mul( float4(input.pos.xy, 0.f, 1.f), gViewProj);
      output.col = input.col;
      output.uv  = input.uv;
      return output;
}

float4 Main_PS(PS_INPUT input) : SV_TARGET
{
      float4 color = input.col * gTexture.Sample(gTextureSampler, input.uv);
      return color;
}

technique11 Default
{
      pass p0
      {
            SetRasterizerState(gRasterizerState);
            SetBlendState(gBlendState, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
            SetDepthStencilState(gDepthStencilState, 0);
            
            SetVertexShader( CompileShader ( vs_4_0, Main_VS() ));
            SetGeometryShader(NULL);

            SetPixelShader( CompileShader ( ps_4_0, Main_PS() ));
      }
}