#define Sample2D(textureName, texCoord) t##textureName##Texture.Sample(s##textureName##Sampler, texCoord)
#define Sample3D(textureName, texCoord) t##textureName##Texture.Sample(s##textureName##Sampler, texCoord)
#define SampleCube(textureName, texCoord) t##textureName##Texture.Sample(s##textureName##Sampler, texCoord)

Texture2D tDiffuseTexture : register(t0);
SamplerState sDiffuseSampler : register(s0);

Texture2D tNormalTexture : register(t1);
SamplerState sNormalSampler : register(s1);

Texture2D tSpecularTexture : register(t2);
SamplerState sSpecularSampler : register(s2);

Texture3D tVolumeTexture : register(t3);
SamplerState sVolumeSampler : register(s3);

TextureCube tCubeTexture : register(t3);
SamplerState sCubeSampler : register(s3);

Texture3D tShadowTexture : register(t5);
SamplerState sShadowSampler : register(s5);

Texture2D tPositionMorphingTexture : register(t6);
SamplerState sPositionMorphingSampler : register(s6);

Texture2D tNormalMorphingTexture : register(t7);
SamplerState sNormalMorphingSampler : register(s7);