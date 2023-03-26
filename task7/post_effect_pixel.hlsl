Texture2D sourceTexture : register(t0);
SamplerState Sampler : register(s0);

cbuffer PostEffectConstantBuffer : register(b0)
{
     int4 params; // x -  use Sobel filter, y - use gray color
     float4 size; // x - 1 / (texture width), y - 1 / (texture height)
}

struct PS_INPUT
{
     float4 pos : SV_POSITION;
     float2 tex : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET
{
     float3 color;
     if (params.x > 0)
     {
          float2 dx = float2(size.x, 0);
          float2 dy = float2(0, size.y);

          float3 z1 = sourceTexture.Sample(Sampler, input.tex - dx - dy).xyz;
          float3 z2 = sourceTexture.Sample(Sampler, input.tex - dy).xyz;
          float3 z3 = sourceTexture.Sample(Sampler, input.tex + dx - dy).xyz;

          float3 z4 = sourceTexture.Sample(Sampler, input.tex - dx).xyz;
          float3 z6 = sourceTexture.Sample(Sampler, input.tex + dx).xyz;

          float3 z7 = sourceTexture.Sample(Sampler, input.tex - dx + dy).xyz;
          float3 z8 = sourceTexture.Sample(Sampler, input.tex + dy).xyz;
          float3 z9 = sourceTexture.Sample(Sampler, input.tex + dx + dy).xyz;

          float3 g1 = z7 + 2 * z8 + z9 - (z1 + 2 * z2 + z3);
          float3 g2 = z3 + 2 * z6 + z9 - (z1 + 2 * z4 + z7);

          color = float3(
               sqrt(g1.x * g1.x + g2.x * g2.x),
               sqrt(g1.y * g1.y + g2.y * g2.y),
               sqrt(g1.z * g1.z + g2.z * g2.z));
     }
     else
          color = sourceTexture.Sample(Sampler, input.tex).xyz;
     if (params.y > 0)
     {
          float gray = 0.3 * color.x + 0.5 * color.y + 0.7 * color.z;
          color = float3(gray, gray, gray);
     }
     return float4(color, 1.0);
}