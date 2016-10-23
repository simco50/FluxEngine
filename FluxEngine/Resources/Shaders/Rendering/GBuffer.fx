texture2D gDiffuse;
texture2D gNormal;
texture2D gDepth;

float4x4 gViewProjInv;
float3 gEyePos;

float4x4 gLightVP;
float3 gLightDirection = float3(-0.577f, -0.577f, 0.577f);
texture2D gShadowMap;

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;// or Mirror or Clamp or Border
    AddressV = Wrap;// or Mirror or Clamp or Border
};

SamplerComparisonState samComparison
{
   // sampler state
   Filter = COMPARISON_MIN_MAG_MIP_LINEAR;
   AddressU = MIRROR;
   AddressV = MIRROR;
 
   // sampler comparison state
   ComparisonFunc = LESS_EQUAL;
};

RasterizerState Solid
{
	FillMode = SOLID;
	CullMode = FRONT;
};

struct VS_INPUT{
	float3 pos : POSITION;
	float2 texCoord : TEXCOORD;
};
struct VS_OUTPUT{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

DepthStencilState EnableDepth
{
	DepthEnable = FALSE;
	DepthWriteMask = ZERO;
};

RasterizerState NoCulling
{
	CullMode = NONE;
};

float2 texOffset( int u, int v )
{
	int width, height;
	gShadowMap.GetDimensions(width, height);
    return float2( u * 1.0f/width, v * 1.0f/height );
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.pos = float4(input.pos, 1);
	output.texCoord = input.texCoord;
	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float2 screenCoord = input.pos.xy;
	float3 loadCoord = float3(screenCoord, 0);
	float depth = gDepth.Load(loadCoord).r;

	//Non-linear Depth to world position
	float2 ndcXY = float2(input.texCoord.x, 1.0f - input.texCoord.y) * 2 - 1;
	float3 ndc = float3(ndcXY, depth);
	float4 worldPos = mul(float4(ndc, 1.0f), gViewProjInv);
	worldPos.xyz /= worldPos.w;

	float4 lightPos = mul(float4(worldPos.xyz, 1), gLightVP);
	lightPos.xyz /= lightPos.w;

	lightPos.x = lightPos.x / 2.0f + 0.5f;
	lightPos.y = lightPos.y / -2.0f + 0.5f;

	lightPos.z -= 0.005f;

	float sum;
	float x, y;
	for (y = -1; y <=1; y += 1.0f)
    {
        for (x = -1; x <= 1; x += 1.0f)
        {
            sum += gShadowMap.SampleCmpLevelZero(samComparison, lightPos.xy + texOffset(x,y), lightPos.z );
        }
    }
    float shadowFactor = sum / 9.0f;
    shadowFactor += 0.4f;
    shadowFactor = saturate(shadowFactor);
	if( lightPos.x < 0.0f || lightPos.x > 1.0f || lightPos.y < 0.0f || lightPos.y > 1.0f || lightPos.z < -1.0f || lightPos.z > 1.0f)
		shadowFactor = 1.0f;

	float3 normal = gNormal.Sample(samLinear, input.texCoord);
	clip((normal.x == 0 && normal.y == 0 && normal.z == 0) ? -1 : 1);

	float4 diffuse = gDiffuse.Sample(samLinear, input.texCoord);

	float strength = saturate(dot(normal, -gLightDirection));
	float3 diffuse_rgb = strength * diffuse.rgb;

	return float4(diffuse_rgb, 1);
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

