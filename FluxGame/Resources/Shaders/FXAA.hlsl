#include "Uniforms.hlsl"
#include "Samplers.hlsl"

#define EDGE_THRESHOLD_MIN 0.0312f
#define EDGE_THRESHOLD_MAX 0.125f
#define SUBPIXEL_QUALITY 0.75f
#define ITERATIONS 12

cbuffer extraBuffer : register(b3)
{
	float cAberrationAmount;
}

struct VS_INPUT
{
	uint vertexId : SV_VertexID;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

float RgbToLuma(float3 rgb)
{
    return sqrt(dot(rgb, float3(0.299f, 0.587f, 0.114f)));    
}

float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
    int width, height;
    tDiffuseTexture.GetDimensions(width, height);
    float dx = 1.0f / width;
    float dy = 1.0f / height;

	float4 colorCenter = Sample2D(Diffuse, input.texCoord);
    float lumaCenter = RgbToLuma(colorCenter.rgb);

    float lumaDown = RgbToLuma(Sample2D(Diffuse, input.texCoord + float2(0, -dy)).rgb);
    float lumaUp = RgbToLuma(Sample2D(Diffuse, input.texCoord + float2(0, dy)).rgb);
    float lumaLeft = RgbToLuma(Sample2D(Diffuse, input.texCoord + float2(-dx, 0)).rgb);
    float lumaRight = RgbToLuma(Sample2D(Diffuse, input.texCoord + float2(dx, 0)).rgb);

    float lumaMin = min(lumaCenter, min(lumaDown, min(lumaUp, min(lumaRight, lumaLeft))));
    float lumaMax = max(lumaCenter, max(lumaDown, max(lumaUp, max(lumaRight, lumaLeft))));

    float lumaRange = lumaMax - lumaMin;

    if(lumaRange < max(EDGE_THRESHOLD_MIN, lumaMax * EDGE_THRESHOLD_MAX))
    {
        return colorCenter;
    }

    float lumaDownLeft = RgbToLuma(Sample2D(Diffuse, input.texCoord + float2(-dx, -dy)).rgb);
    float lumaUpRight = RgbToLuma(Sample2D(Diffuse, input.texCoord + float2(dx, dy)).rgb);
    float lumaUpLeft = RgbToLuma(Sample2D(Diffuse, input.texCoord + float2(-dx, dy)).rgb);
    float lumaDownRight = RgbToLuma(Sample2D(Diffuse, input.texCoord + float2(dx, -dy)).rgb);

    float lumaDownUp = lumaDown + lumaUp;
    float lumaLeftRight = lumaLeft + lumaRight;

    float lumaLeftCorners = lumaUpLeft + lumaDownLeft;
    float lumaDownCorners = lumaDownLeft + lumaDownRight;
    float lumaRightCorners = lumaUpRight + lumaDownRight;
    float lumaUpCorners = lumaUpLeft + lumaUpRight;

    float edgeHorizontal = abs(-2.0 * lumaLeft + lumaLeftCorners) + abs(-2.0 * lumaCenter + lumaDownUp ) * 2.0 + abs(-2.0 * lumaRight + lumaRightCorners);
    float edgeVertical = abs(-2.0 * lumaUp + lumaUpCorners) + abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0  + abs(-2.0 * lumaDown + lumaDownCorners);

    bool isHorizontal = edgeHorizontal >= edgeVertical;

    //Choosing edge orientation
    float luma1 = isHorizontal ? lumaDown : lumaLeft;
    float luma2 = isHorizontal ? lumaUp : lumaRight;

    float gradient1 = luma1 - lumaCenter;
    float gradient2 = luma2 - lumaCenter;
    
    bool is1Steepest = abs(gradient1) >= abs(gradient2);

    float gradientScaled = 0.25f * max(abs(gradient1), abs(gradient2));

    float stepLength = isHorizontal ? dy : dx;
    float lumaLocalAverage = 0.0f;

    if(is1Steepest)
    {
        stepLength = -stepLength;
        lumaLocalAverage = 0.5f * (luma1 + lumaCenter);
    }
    else
    {
        lumaLocalAverage = 0.5f * (luma2 + lumaCenter);

    }

    float2 currentUv = input.texCoord;
    if(isHorizontal)
    {
            currentUv.y += stepLength * 0.5f;
    }
    else
    {
        currentUv.x += stepLength * 0.5f;
    }

    //First exploration
    float2 offset = isHorizontal ? float2(dx, 0) : float2(dy, 0);
    float2 uv1 = currentUv - offset;
    float2 uv2 = currentUv + offset;

    float lumaEnd1 = RgbToLuma(Sample2D(Diffuse, uv1).rgb);
    float lumaEnd2 = RgbToLuma(Sample2D(Diffuse, uv2).rgb);
    lumaEnd1 -= lumaLocalAverage;
    lumaEnd2 -= lumaLocalAverage;

    bool reached1 = abs(lumaEnd1) >= gradientScaled;
    bool reached2 = abs(lumaEnd2) >= gradientScaled;
    bool reachedBoth = reached1 && reached2;

    if(!reached1)
    {
        uv1 -= offset;
    }
    if(!reached2)
    {
        uv2 += offset;        
    }

    if(!reachedBoth)
    {
        for(int i = 2; i < ITERATIONS; ++i)
        {
            if(!reached1)
            {
                lumaEnd1 = RgbToLuma(Sample2D(Diffuse, uv1).rgb);
                lumaEnd1 = lumaEnd1 - lumaLocalAverage;
            }
            if(!reached2)
            {
                lumaEnd2 = RgbToLuma(Sample2D(Diffuse, uv2).rgb);
                lumaEnd2 = lumaEnd1 - lumaLocalAverage;
            }

            reached1 = abs(lumaEnd1) >= gradientScaled;
            reached2 = abs(lumaEnd2) >= gradientScaled;
            reachedBoth = reached1 && reached2;

            if(!reached1)
            {
                uv1 -= offset;
            }
            if(!reached2)
            {
                uv2 += offset;        
            }

            if(reachedBoth)
            {
                break;
            }
        }
    }

    //Offsets

    float distance1 = isHorizontal ? (input.texCoord.x - uv1.x) : (input.texCoord.y - uv1.y);
    float distance2 = isHorizontal ? (uv2.x - input.texCoord.x) : (uv2.y - input.texCoord.y);

    bool isDirection1 = distance1 < distance2;
    float distanceFinal = min(distance1, distance2);

    float edgeThickness = distance1 + distance2;

    float pixelOffset = -distanceFinal / edgeThickness + 0.5f;

    bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;
    bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0f) != isLumaCenterSmaller;
    float finalOffset = correctVariation ? pixelOffset : 0.0f;

    //Subpixel
    float lumaAverage = (1.0f / 12.0f) * (2.0f * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
    float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter) / lumaRange, 0.0f, 1.0f);
    float subPixelOffset2 = (-2.0f * subPixelOffset1 + 3.0f) * subPixelOffset1 * subPixelOffset1;

    float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;
    finalOffset = max(finalOffset, subPixelOffsetFinal);

    float2 finalUv = input.texCoord;
    if(isHorizontal)
    {
        finalUv.y += finalOffset * stepLength;
    }
    else
    {
        finalUv.x += finalOffset * stepLength;
    }
    return Sample2D(Diffuse, finalUv);
}