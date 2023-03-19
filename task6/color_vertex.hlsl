cbuffer WorldBuffer : register (b0)
{
     float4x4 world;
     float4 color;
};

cbuffer SceneBuffer : register (b1)
{
     float4x4 viewProj;
};

struct VSInput
{
     float3 position : POSITION;
};

struct VSOutput
{
     float4 position : SV_Position;
};

VSOutput main(VSInput input)
{
     VSOutput output;
     output.position = mul(viewProj, mul(world, float4(input.position, 1.0f)));

     return output;
}
