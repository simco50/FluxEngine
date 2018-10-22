#include "Constants.hlsl"
#include "Math.hlsl"

float4x4 BlendBoneTransformsToMatrix(float4 boneIndices, float4 boneWeights)
{
    float4x4 finalMatrix;
    for(int i = 0; i < MAX_BONES_PER_VERTEX; ++i)
    {
        if(boneIndices[i] == -1)
        {
            break;
        }
        finalMatrix += boneWeights[i] * cSkinMatrices[boneIndices[i]];
    }
    return finalMatrix;
}

float2x4 NormalizeDualQuaternion(float2x4 dualQ)
{
    return dualQ / length(dualQ[0]);
}

float2x4 BlendBoneTransformsToDualQuaternion(float4 boneIndices, float4 boneWeights)
{
    float2x4 dual = (float2x4)0;
    float4 dq0 = cSkinDualQuaternions[boneIndices.x][0];
    for(int i = 0; i < MAX_BONES_PER_VERTEX; ++i)
    {
        if(boneIndices[i] == -1)
        {
            break;
        }
        if(dot(dq0, cSkinDualQuaternions[boneIndices[i]][0]) < 0)
        {
            boneWeights[i] *= -1;
        }
        dual += boneWeights[i] * cSkinDualQuaternions[boneIndices[i]];
    }
    return NormalizeDualQuaternion(dual);
}

float3 QuaternionRotateVector(float3 v, float4 quatReal)
{
    return v + 2.0f * cross(quatReal.xyz, quatReal.w * v + cross(quatReal.xyz, v));
}

float3 DualQuatTransformPoint(float3 p, float4 quatReal, float4 quatDual)
{
    //Reconstruct translation from the dual quaternion
    float3 t = 2 * (quatReal.w * quatDual.xyz - quatDual.w * quatReal.xyz + cross(quatReal.xyz, quatDual.xyz));
   
    //Combine with rotation of the input point
    return QuaternionRotateVector(p, quatReal) + t;
}