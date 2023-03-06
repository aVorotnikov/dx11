Texture2D cubeTexture : register (t0);

SamplerState cubeSampler : register(s0);

struct VSOutput
{
     float4 position : SV_Position;
     float2 texCoord : TEXCOORD;
};

float4 main(VSOutput input) : SV_Target0
{
     return float4(cubeTexture.Sample(cubeSampler, input.texCoord).xyz, 1.0);
}
