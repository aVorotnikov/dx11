cbuffer WorldBuffer : register (b0)
{
     float4x4 world;
     float4 color;
};

struct VSOutput
{
     float4 position : SV_Position;
};

float4 main(VSOutput input) : SV_Target0
{
     return color;
}
