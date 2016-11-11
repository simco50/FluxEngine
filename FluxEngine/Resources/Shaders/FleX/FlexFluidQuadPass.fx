texture2D gDepth;
float3 gLightDirection = float3(-0.577f, -0.577f, 0.577f);
float4x4 gViewInv : VIEWINV;
float4x4 gProjInv : PROJINV;

float gSpecularStrength = 1.0f;
float gShininess = 8.0f;

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

float2 texOffset( int u, int v )
{
	int width, height;
	gDepth.GetDimensions(width, height);
    return float2( u * 1.0f/width, v * 1.0f/height );
}

float3 WorldPosFromDepth(float depth, float2 texCoord) 
{
    float z = depth * 2.0 - 1.0;
    float4 clipSpacePosition = float4(texCoord * 2.0 - 1.0, z, 1.0);
    float4 viewSpacePosition = mul(gProjInv, clipSpacePosition);
    viewSpacePosition /= viewSpacePosition.w;
    float4 worldSpacePosition = mul(gViewInv, viewSpacePosition);
    return worldSpacePosition.xyz;
}

float blur(float2 texCoord)
{
	float sum;
	float x, y;
	for (y = -1; y <=1; y += 1.0f)
    {
        for (x = -1; x <= 1; x += 1.0f)
        {
            sum += gDepth.Sample(samLinear, texCoord + texOffset(x,y) ).r;
        }
    }
    sum /= 9.0f;
    return sum;
}

float3 normal_from_depth(float depth, float2 texcoords) {
  
  const float2 offset1 = float2(0.0,0.001);
  const float2 offset2 = float2(0.001,0.0);
  
  float depth1 = blur(texcoords + offset1);
  float depth2 = blur(texcoords + offset2);
  
  float3 p1 = float3(offset1, depth1 - depth);
  float3 p2 = float3(offset2, depth2 - depth);
  
  float3 normal = cross(p1, p2);
  normal.z = -normal.z;
  
  return normalize(normal);
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float2 texCoord = (input.pos.xy + 1) / 2.0f;
	float depth = gDepth.Sample(samLinear, input.texCoord).r;
	clip(depth == 1 ? -1 : 1);

  float3 normal = normal_from_depth(depth, input.texCoord);

  float3 viewDirection = normalize(WorldPosFromDepth(depth, texCoord) - gViewInv[3].xyz);

  float3 hv = -normalize(gLightDirection + viewDirection);
  float specularStrength = dot(normal, hv);
  specularStrength = gSpecularStrength * pow(saturate(specularStrength), gShininess);


	return float4(0,0.2,0.7,1) * saturate(dot(normal, gLightDirection));
}

technique11 Default
{
  pass P0
  {
		SetRasterizerState(RS);
		SetDepthStencilState(DSS, 0);

    SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
    SetPixelShader( CompileShader( ps_4_0, PS() ) );
  }
}