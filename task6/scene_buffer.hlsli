cbuffer SceneBuffer : register (b1)
{
     float4x4 viewProj;
     float4 cameraPos;
     int4 lightCount;  // x - count, y - use normals, z - show normals
     float4 lightPos[10];
     float4 lightColor[10];
     float4 ambientColor;
};
