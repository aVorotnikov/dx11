#include "defines.hlsli"

struct GeomBuffer
{
     float4x4 world;
     float4x4 norm;
     float4 shineSpeedTexIdNmp; // x - specular power, y - rotation speed, z - texture id, w - normal map presence
};

cbuffer GeomBufferInstancing : register (b0)
{
     GeomBuffer geomBuffer[MAX_CUBES];
};
