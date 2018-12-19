#include "Uniforms.hlsl"

//UTILITY FUNCTIONS

float DistanceBasedTessellationFactor(float4 vertex, float minDist, float maxDist, float tessellation)
{
    float3 wpos = mul(vertex, cWorld).xyz;
    float dist = distance(wpos, cViewInverse[3].xyz);
    float f = clamp(1.0 - (dist - minDist) / (maxDist - minDist), 0.01, 1.0) * tessellation;
    return f;
}

float4 TriangleEdgeTessellationFactors(float3 triVertexFactors)
{
    float4 tess;
    tess.x = 0.5 * (triVertexFactors.y + triVertexFactors.z);
    tess.y = 0.5 * (triVertexFactors.x + triVertexFactors.z);
    tess.z = 0.5 * (triVertexFactors.x + triVertexFactors.y);
    tess.w = (triVertexFactors.x + triVertexFactors.y + triVertexFactors.z) / 3.0f;
    return tess;
}

float EdgeTessellationFactor(float3 wpos0, float3 wpos1, float edgeLen)
{
    // distance to edge center
    float dist = distance (0.5 * (wpos0 + wpos1), cViewInverse[3].xyz);
    // length of the edge
    float len = distance(wpos0, wpos1);
    // edgeLen is approximate desired size in pixels
    float f = max(len * 720 / (edgeLen * dist), 1.0);
    return f;
}

float DistanceFromPlane(float3 pos, float4 plane)
{
    float d = dot (float4(pos, 1.0f), plane);
    return d;
}


// Returns true if triangle with given 3 world positions is outside of camera's view frustum.
// cullEps is distance outside of frustum that is still considered to be inside (i.e. max displacement)
/*bool WorldViewFrustumCull (float3 wpos0, float3 wpos1, float3 wpos2, float cullEps)
{
    float4 planeTest;

    // left
    planeTest.x = (( DistanceFromPlane(wpos0, unity_CameraWorldClipPlanes[0]) > -cullEps) ? 1.0f : 0.0f ) +
                  (( DistanceFromPlane(wpos1, unity_CameraWorldClipPlanes[0]) > -cullEps) ? 1.0f : 0.0f ) +
                  (( DistanceFromPlane(wpos2, unity_CameraWorldClipPlanes[0]) > -cullEps) ? 1.0f : 0.0f );
    // right
    planeTest.y = (( DistanceFromPlane(wpos0, unity_CameraWorldClipPlanes[1]) > -cullEps) ? 1.0f : 0.0f ) +
                  (( DistanceFromPlane(wpos1, unity_CameraWorldClipPlanes[1]) > -cullEps) ? 1.0f : 0.0f ) +
                  (( DistanceFromPlane(wpos2, unity_CameraWorldClipPlanes[1]) > -cullEps) ? 1.0f : 0.0f );
    // top
    planeTest.z = (( DistanceFromPlane(wpos0, unity_CameraWorldClipPlanes[2]) > -cullEps) ? 1.0f : 0.0f ) +
                  (( DistanceFromPlane(wpos1, unity_CameraWorldClipPlanes[2]) > -cullEps) ? 1.0f : 0.0f ) +
                  (( DistanceFromPlane(wpos2, unity_CameraWorldClipPlanes[2]) > -cullEps) ? 1.0f : 0.0f );
    // bottom
    planeTest.w = (( DistanceFromPlane(wpos0, unity_CameraWorldClipPlanes[3]) > -cullEps) ? 1.0f : 0.0f ) +
                  (( DistanceFromPlane(wpos1, unity_CameraWorldClipPlanes[3]) > -cullEps) ? 1.0f : 0.0f ) +
                  (( DistanceFromPlane(wpos2, unity_CameraWorldClipPlanes[3]) > -cullEps) ? 1.0f : 0.0f );

    // has to pass all 4 plane tests to be visible
    return !all (planeTest);
}*/

//TESSELLATION TECHNIQUES

// Distance based tessellation:
// Tessellation level is "tess" before "minDist" from camera, and linearly decreases to 1
// up to "maxDist" from camera.
float4 DistanceBasedTesellation(float4 v0, float4 v1, float4 v2, float minDist, float maxDist, float tess)
{
    float3 f;
    f.x = DistanceBasedTessellationFactor(v0, minDist, maxDist, tess);
    f.y = DistanceBasedTessellationFactor(v1, minDist, maxDist, tess);
    f.z = DistanceBasedTessellationFactor(v2, minDist, maxDist, tess);

    return TriangleEdgeTessellationFactors(f);
}

// Desired edge length based tessellation:
// Approximate resulting edge length in pixels is "edgeLength".
// Does not take viewing FOV into account, just flat out divides factor by distance.
float4 EdgeLengthBasedTessellation(float4 v0, float4 v1, float4 v2, float edgeLength)
{
    float3 pos0 = mul(cWorld, v0).xyz;
    float3 pos1 = mul(cWorld, v1).xyz;
    float3 pos2 = mul(cWorld, v2).xyz;
    float4 tess;
    tess.x = EdgeTessellationFactor(pos1, pos2, edgeLength);
    tess.y = EdgeTessellationFactor(pos2, pos0, edgeLength);
    tess.z = EdgeTessellationFactor(pos0, pos1, edgeLength);
    tess.w = (tess.x + tess.y + tess.z) / 3.0f;
    return tess;
}

// Same as EdgeLengthBasedTessellation, but also does patch frustum culling:
// patches outside of camera's view are culled before GPU tessellation. Saves some wasted work.
/*float4 EdgeLengthBasedTessellationCulled (float4 v0, float4 v1, float4 v2, float edgeLength, float maxDisplacement)
{
    float3 pos0 = mul(cWorld, v0).xyz;
    float3 pos1 = mul(cWorld, v1).xyz;
    float3 pos2 = mul(cWorld, v2).xyz;
    float4 tess;

    if (WorldViewFrustumCull(pos0, pos1, pos2, maxDisplacement))
    {
        tess = 0.0f;
    }
    else
    {
        tess.x = EdgeTessellationFactor(pos1, pos2, edgeLength);
        tess.y = EdgeTessellationFactor(pos2, pos0, edgeLength);
        tess.z = EdgeTessellationFactor(pos0, pos1, edgeLength);
        tess.w = (tess.x + tess.y + tess.z) / 3.0f;
    }
    return tess;
}*/

float3 FindNormal(Texture2D tex, SamplerState sam, float2 texCoord, float dx, float dy)
{
    float2 offsets[4];
    offsets[0] = texCoord + float2(dx, 0);
    offsets[1] = texCoord + float2(-dx, 0);
    offsets[2] = texCoord + float2(0, -dy);
    offsets[3] = texCoord + float2(0, dy);

    float hts[4];
    for (int i = 0; i < 4; i++)
    {
        hts[i] = tex.Sample(sam, float2(offsets[i].x, offsets[i].y)).x;
    }

    float2 step = float2(1.0, 0.0);
    float3 va = normalize(float3(step.xy, hts[1] - hts[0]));
    float3 vb = normalize(float3(step.yx, hts[3] - hts[2]));
    return cross(va, vb);
}