cbuffer WorldBuffer : register (b0)
{
     float4x4 world;
};

cbuffer SceneBuffer : register (b1)
{
     float4x4 viewProj;
};

struct VSInput
{
     float3 position : POSITION;
     float2 texCoord : TEXCOORD;
};

struct VSOutput
{
     float4 position : SV_Position;
     float2 texCoord : TEXCOORD;
};

VSOutput main(VSInput input)
{
     VSOutput output;
     output.position = mul(viewProj, mul(world, float4(input.position, 1.0f)));
     output.texCoord = input.texCoord;

     return output;
}
