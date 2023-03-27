#include "calculate_light.hlsli"
#include "geom_buffer.hlsli"

Texture2DArray cubeTexture : register (t0);
Texture2D cubeNormalTexture : register (t1);
SamplerState cubeSampler : register(s0);
SamplerState cubeNormalSampler : register (s1);

struct VSOutput
{
     float4 position : SV_Position;
     float4 worldPos : POSITION;
     float2 texCoord : TEXCOORD;
     float3 normal : NORMAL;
     float3 tangent : TANGENT;
     nointerpolation uint instanceId : INST_ID;
};

float4 main(VSOutput input) : SV_Target0
{
     float3 color = cubeTexture.Sample(cubeSampler, float3(input.texCoord, geomBuffer[input.instanceId].shineSpeedTexIdNmp.z)).xyz;
     float3 finalColor = ambientColor.xyz * color;

     float3 norm = float3(0, 0, 0);
     if (lightCount.y > 0 && geomBuffer[input.instanceId].shineSpeedTexIdNmp.w > 0.0f)
     {
          float3 binorm = normalize(cross(input.normal, input.tangent));
          float3 localNorm = cubeNormalTexture.Sample(cubeNormalSampler, input.texCoord).xyz * 2.0 - 1.0;
          norm = localNorm.x * normalize(input.tangent) + localNorm.y * binorm + localNorm.z * normalize(input.normal);
     }
     else
     {
          norm = input.normal;
     }

     return float4(CalculateColor(color, norm, input.worldPos.xyz, geomBuffer[input.instanceId].shineSpeedTexIdNmp.x, false), 1.0);
}
