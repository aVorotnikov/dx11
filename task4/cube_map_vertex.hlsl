cbuffer WorldBuffer : register (b0)
{
     float4x4 world;
     float4 size; // x - size of sphere
};

cbuffer SceneBuffer : register (b1)
{
     float4x4 viewProj;
     float4 pov;
};

struct VSInput
{
     float3 position : POSITION;
};

struct VSOutput
{
     float4 position : SV_POSITION;
     float3 localPos : POSITION1;
};

VSOutput main(VSInput input)
{
     VSOutput output;

     float3 pos = pov.xyz + input.position * size.x;
     output.position = mul(viewProj, mul(world, float4(pos, 1.0f)));
     output.localPos = input.position;

     return output;
}
