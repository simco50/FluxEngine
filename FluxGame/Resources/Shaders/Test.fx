Texture2D gDiffuseTexture : register(t0);
bool gUseDiffuseTexture = true;
float4 gColor;

SamplerState gTextureSampler
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

struct VS_OUTPUT{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD;
};

VS_OUTPUT VS(uint id : SV_VertexID)
{
	VS_OUTPUT output;
	// generate clip space position
	output.pos.x = (float)(id / 2) * 4.0 - 1.0;
	output.pos.y = (float)(id % 2) * 4.0 - 1.0;
	output.pos.z = 0.0;
	output.pos.w = 1.0;
	// texture coordinates
	output.tex.x = (float)(id / 2) * 2.0;
	output.tex.y = 1.0 - (float)(id % 2) * 2.0;

	return output;
}

float4 PS(VS_OUTPUT input):SV_TARGET
{
	float3 rgb = gDiffuseTexture.Sample(gTextureSampler, input.tex).rgb;
	return float4(rgb, 1.0f);
}

technique11 Default
{
	pass one
	{
		SetVertexShader( CompileShader ( vs_4_0, VS() ));
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader ( ps_4_0, PS() ));
	}
}

