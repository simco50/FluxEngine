texture2D gDepth;
float4x4 gViewProjInv : VIEWPROJINV;
float3 gLightDirection = float3(-0.577f, -0.577f, 0.577f);

struct VS_INPUT
{
	float3 pos : POSITION;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

DepthStencilState DSS
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

RasterizerState RS
{
	CullMode = BACK;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = float4(input.pos, 1);
	output.texCoord.x = (input.pos.x + 1) / 2.0f;
	output.texCoord.y = (1 - input.pos.y) / 2.0f;
	return output;
}

float2 texOffset(int u, int v)
{
	int width, height;
	gDepth.GetDimensions(width, height);
	return float2(u * 1.0f / width, v * 1.0f / height);
}

float3 toWorld(float2 texCoord)
{
	float z = gDepth.Sample(samLinear, texCoord).r;
	float x = texCoord.x * 2 - 1;
	float y = (1 - texCoord.y) * 2 - 1;
	float4 projectedPos = float4(x, y, z, 1);
	float4 pos = mul(projectedPos, gViewProjInv);
	projectedPos.xyz /= projectedPos.w;
	return pos.xyz;
}


float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float4 depth = gDepth.Sample(samLinear, input.texCoord);
	clip(depth.g == 0 ? 1 : -1);

	return float4(depth.x, depth.x, depth.x , 1);
}

technique11 Default
{
	pass P0
	{
		SetRasterizerState(RS);
		SetDepthStencilState(DSS, 0);

		SetVertexShader(CompileShader(vs_4_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PS()));
	}
}