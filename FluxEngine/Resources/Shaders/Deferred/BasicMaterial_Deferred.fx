float4x4 gWorld : WORLD;
float4x4 gWorldViewProj : WORLDVIEWPROJECTION; 
float3 gLightDirection = float3(-0.577f, -0.577f, 0.577f);

float4 gColor = float4(1,0,0,1);

//User set values
bool gUseDiffuseMap = false;
texture2D gDiffuseMap;
bool gUseNormalMap = false;
texture2D gNormalMap;

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;// or Mirror or Clamp or Border
    AddressV = Wrap;// or Mirror or Clamp or Border
};

RasterizerState Solid
{
	FillMode = SOLID;
	CullMode = FRONT;
};

struct VS_INPUT{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float3 tangent : TANGENT;
};
struct VS_OUTPUT{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float3 tangent : TANGENT;
};
struct PS_OUTPUT
{
	float4 diffuse : SV_TARGET0;
	float2 normal : SV_TARGET1;
};

DepthStencilState EnableDepth
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

RasterizerState NoCulling
{
	CullMode = BACK;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = mul ( float4(input.pos,1.0f), gWorldViewProj );
	output.normal = normalize(mul(input.normal, (float3x3)gWorld));
	output.texCoord = input.texCoord;
	output.tangent = normalize(input.tangent);
	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
PS_OUTPUT PS(VS_OUTPUT input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;
	output.diffuse = gColor;
	if(gUseDiffuseMap)
		output.diffuse = gDiffuseMap.Sample(samLinear, input.texCoord);
	float3 normal = input.normal;
	if(gUseNormalMap)
	{
		float3 binormal = normalize(cross(input.tangent, normal));
		float3x3 world = float3x3(input.tangent, binormal, normal);
		float3 sampledNormal = gNormalMap.Sample(samLinear, input.texCoord) * 2.0f - 1.0f;
		sampledNormal.y *= -1;
		sampledNormal = saturate(sampledNormal);
		normal = mul(sampledNormal, world);
	}

	output.normal = normal;
	return output;
}

//--------------------------------------------------------------------------------------
// Technique
//--------------------------------------------------------------------------------------
technique11 Default
{
    pass P0
    {
		SetRasterizerState(NoCulling);
		SetDepthStencilState(EnableDepth, 0);

		SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}

