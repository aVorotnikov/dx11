#include "defines.hlsli"

cbuffer LightBuffer : register (b2)
{
     float4 cameraPos;
     int4 lightCount;  // x - count, y - use normals, z - show normals
     float4 lightPos[MAX_LIGHTS];
     float4 lightColor[MAX_LIGHTS];
     float4 ambientColor;
};
