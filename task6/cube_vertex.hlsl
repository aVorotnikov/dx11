#include "scene_buffer.hlsli"

cbuffer WorldBuffer : register (b0)
{
     float4x4 world;
     float4 shine;  // x - specular power
};

struct VSInput
{
     float3 position : POSITION;
     float2 texCoord : TEXCOORD;
     float3 normal : NORMAL;
     float3 tangent : TANGENT;
};

struct VSOutput
{
     float4 position : SV_Position;
     float4 worldPos : POSITION;
     float2 texCoord : TEXCOORD;
     float3 normal : NORMAL;
     float3 tangent : TANGENT;
};

VSOutput main(VSInput input)
{
     VSOutput output;

     output.worldPos = mul(world, float4(input.position, 1.0f));
     output.position = mul(viewProj, output.worldPos);
     output.texCoord = input.texCoord;
     output.normal = mul(world, float4(input.normal, 1.0f)).xyz;
     output.tangent = mul(world, float4(input.tangent, 1.0f)).xyz;

     return output;
}
