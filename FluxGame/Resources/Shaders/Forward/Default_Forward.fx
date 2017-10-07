float4x4 gWorld : WORLD;
float4x4 gWorldViewProj : WORLDVIEWPROJECTION; 
float3 gLightDirection = float3(-0.577f, -0.577f, 0.577f);
float4 gColor = float4(0.5f, 0.8f, 0.4f, 1.0f);
float gIntensity = 1.5f;

bool gUseDiffuseTexture = false;
texture2D gDiffuseTexture;

struct VS_INPUT{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
};

struct VS_OUTPUT{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
};

BlendState AlphaBlending
{
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
	BlendOp = ADD;
};

DepthStencilState EnableDepth
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;// or Mirror or Clamp or Border
    AddressV = Wrap;// or Mirror or Clamp or Border
};

RasterizerState BackfaceCull
{
	CullMode = BACK;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input){
	VS_OUTPUT output;
	output.pos = mul ( float4(input.pos,1.0f), gWorldViewProj );
	output.normal = normalize(mul(input.normal, (float3x3)gWorld));
	output.texCoord = input.texCoord;
	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float4 color = float4(gColor.rgb, 1);
	if(gUseDiffuseTexture)
		color = gDiffuseTexture.Sample(samLinear, input.texCoord);
	clip(color.a < 0.05f ? -1 : 1);

	float diffuseStrength = dot(input.normal, -gLightDirection);
	diffuseStrength = diffuseStrength * 0.5 + 0.5;
	diffuseStrength = saturate(diffuseStrength);
	color = float4(color.rgb * diffuseStrength, color.a);
	
	return color;
}


//--------------------------------------------------------------------------------------
// Technique
//--------------------------------------------------------------------------------------
technique11 Default
{
    pass P0
    {
		SetRasterizerState(BackfaceCull);
		SetDepthStencilState(EnableDepth, 0);
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
        SetBlendState(AlphaBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF);
    }
}