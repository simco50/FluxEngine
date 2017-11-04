Texture2D gTexture : register(t0);

SamplerState gTextureSampler
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

DepthStencilState gDepthStencilState
{
	DepthEnable = FALSE;
	DepthWriteMask = ZERO;
};

RasterizerState gRasterizerState
{
	CullMode = BACK;
};

struct VS_INPUT{
	float3 Pos : POSITION;
	float2 Tex : TEXCOORD;
};

struct VS_OUTPUT{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};

VS_OUTPUT VS(VS_INPUT input){

	VS_OUTPUT output = (VS_OUTPUT)0;

	output.Pos = float4(input.Pos , 1);
	output.Tex = input.Tex;

	return output;
}

float4 PS(VS_OUTPUT input):SV_TARGET
{
	float3 rgb = gTexture.Sample(gTextureSampler, input.Tex).rgb;
	return float4(rgb, 1.0f);
}

technique11 Default
{
	pass one
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);

		SetVertexShader( CompileShader ( vs_4_0, VS() ));
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader ( ps_4_0, PS() ));
	}
}

