#include "defines.hlsli"

cbuffer SceneConstantBuffer : register (b1)
{
     float4x4 viewProj;
     int4 indexBuffer[MAX_CUBES]; // x - index
};
