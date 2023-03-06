#include "cube_map.h"
#include "utils.h"
#include <d3dcompiler.h>
#include <vector>
#include <stdexcept>

namespace
{

     struct Vertex
     {
          float x, y, z;
     };

     struct WorldBuffer
     {
          DirectX::XMMATRIX world;
          DirectX::XMFLOAT4 size;
     };

     struct SceneBuffer
     {
          DirectX::XMMATRIX viewProj;
          DirectX::XMFLOAT4 pov;
     };

}

CubeMap::CubeMap(
     ID3D11Device *device,
     ID3D11DeviceContext *context,
     const unsigned width,
     const unsigned height,
     const float fov,
     const float frustumNear) :
     context_(context),
     pVertexBuffer_(NULL),
     pIndexBuffer_(NULL),
     pWorldBuffer_(NULL),
     pSceneBuffer_(NULL),
     pRasterizerState_(NULL),
     pInputLayout_(NULL),
     pVertexShader_(NULL),
     pPixelShader_(NULL),
     pTexture_(nullptr),
     fov_(fov),
     near_(frustumNear),
     radius_(0.0f)
{
     Resize(width, height);

     float sphereYaw = 0.0f;
     float spherePitch = 0.0f;

     std::vector<Vertex> vertices(sphereVertices_);
     DirectX::XMVECTOR curVertexPos = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
     vertices[0].x = 0.0f;
     vertices[0].y = 0.0f;
     vertices[0].z = 1.0f;
     for (unsigned i = 0; i < latitudeLines_ - 2; i++) {
          spherePitch = (i + 1) * (DirectX::XM_PI / (latitudeLines_ - 1));
          DirectX::XMMATRIX rotationX = DirectX::XMMatrixRotationX(spherePitch);
          for (unsigned j = 0; j < longtitudeLines_; j++) {
               sphereYaw = j * (DirectX::XM_2PI / longtitudeLines_);
               DirectX::XMMATRIX rotationY = DirectX::XMMatrixRotationZ(sphereYaw);
               curVertexPos = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (rotationX * rotationY));
               curVertexPos = DirectX::XMVector3Normalize(curVertexPos);
               vertices[i * longtitudeLines_ + j + 1].x = DirectX::XMVectorGetX(curVertexPos);
               vertices[i * longtitudeLines_ + j + 1].y = DirectX::XMVectorGetY(curVertexPos);
               vertices[i * longtitudeLines_ + j + 1].z = DirectX::XMVectorGetZ(curVertexPos);
          }
     }
     vertices[sphereVertices_ - 1].x = 0.0f;
     vertices[sphereVertices_ - 1].y = 0.0f;
     vertices[sphereVertices_ - 1].z = -1.0f;

     std::vector<UINT> indices(sphereFaces_ * 3);
     int k = 0;
     for (unsigned i = 0; i < longtitudeLines_ - 1; i++) {
          indices[k] = 0;
          indices[k + 1] = i + 1;
          indices[k + 2] = i + 2;
          k += 3;
     }
     indices[k] = 0;
     indices[k + 1] = longtitudeLines_;
     indices[k + 2] = 1;
     k += 3;
     for (unsigned i = 0; i < latitudeLines_ - 3; i++) {
          for (unsigned j = 0; j < longtitudeLines_ - 1; j++) {
               indices[k] = i * longtitudeLines_ + j + 1;
               indices[k + 1] = i * longtitudeLines_ + j + 2;
               indices[k + 2] = (i + 1) * longtitudeLines_ + j + 1;
               indices[k + 3] = (i + 1) * longtitudeLines_ + j + 1;
               indices[k + 4] = i * longtitudeLines_ + j + 2;
               indices[k + 5] = (i + 1) * longtitudeLines_ + j + 2;
               k += 6;
          }
          indices[k] = (i * longtitudeLines_) + longtitudeLines_;
          indices[k + 1] = (i * longtitudeLines_) + 1;
          indices[k + 2] = ((i + 1) * longtitudeLines_) + longtitudeLines_;
          indices[k + 3] = ((i + 1) * longtitudeLines_) + longtitudeLines_;
          indices[k + 4] = (i * longtitudeLines_) + 1;
          indices[k + 5] = ((i + 1) * longtitudeLines_) + 1;
          k += 6;
     }
     for (unsigned i = 0; i < longtitudeLines_ - 1; i++) {
          indices[k] = sphereVertices_ - 1;
          indices[k + 1] = (sphereVertices_ - 1) - (i + 1);
          indices[k + 2] = (sphereVertices_ - 1) - (i + 2);
          k += 3;
     }
     indices[k] = sphereVertices_ - 1;
     indices[k + 1] = (sphereVertices_ - 1) - longtitudeLines_;
     indices[k + 2] = sphereVertices_ - 2;

     D3D11_BUFFER_DESC desc = {};
     ZeroMemory(&desc, sizeof(desc));
     desc.ByteWidth = sizeof(Vertex) * sphereVertices_;
     desc.Usage = D3D11_USAGE_IMMUTABLE;
     desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;
     D3D11_SUBRESOURCE_DATA data;
     ZeroMemory(&data, sizeof(data));
     data.pSysMem = &vertices[0];
     auto result = device->CreateBuffer(&desc, &data, &pVertexBuffer_);
     if (FAILED(result))
          throw std::runtime_error("Failed to create vertex buffer");

     ZeroMemory(&desc, sizeof(desc));
     desc.ByteWidth = sizeof(UINT) * sphereFaces_ * 3;
     desc.Usage = D3D11_USAGE_IMMUTABLE;
     desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;
     ZeroMemory(&data, sizeof(data));
     data.pSysMem = &indices[0];
     result = device->CreateBuffer(&desc, &data, &pIndexBuffer_);
     if (FAILED(result))
          throw std::runtime_error("Failed to create index buffer");

     ID3DBlob *pVertexShaderBlob = NULL;
     result = CompileShaderFromFile(L"cube_map_vertex.hlsl", "main", "vs_5_0", &pVertexShaderBlob);
     if (FAILED(result))
          throw std::runtime_error("Failed to compile vertex shader");

     result = device->CreateVertexShader(
          pVertexShaderBlob->GetBufferPointer(),
          pVertexShaderBlob->GetBufferSize(),
          NULL,
          &pVertexShader_);
     if (FAILED(result))
     {
          pVertexShaderBlob->Release();
          throw std::runtime_error("Failed to create vertex shader");
     }

     static const D3D11_INPUT_ELEMENT_DESC layout[] =
     {
          {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
     };
     UINT numElements = ARRAYSIZE(layout);

     result = device->CreateInputLayout(
          layout,
          numElements,
          pVertexShaderBlob->GetBufferPointer(),
          pVertexShaderBlob->GetBufferSize(),
          &pInputLayout_);
     pVertexShaderBlob->Release();
     if (FAILED(result))
          throw std::runtime_error("Failed to create input layout");

     ID3DBlob *pPixelShaderBlob = NULL;
     result = CompileShaderFromFile(L"cube_map_pixel.hlsl", "main", "ps_5_0", &pPixelShaderBlob);
     if (FAILED(result))
          throw std::runtime_error("Failed to compile pixel shader");

     result = device->CreatePixelShader(
          pPixelShaderBlob->GetBufferPointer(),
          pPixelShaderBlob->GetBufferSize(),
          nullptr,
          &pPixelShader_);
     pPixelShaderBlob->Release();
     if (FAILED(result))
          throw std::runtime_error("Failed to create vertex shader");

     ZeroMemory(&desc, sizeof(desc));
     desc.ByteWidth = sizeof(WorldBuffer);
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;
     WorldBuffer worldBuffer;
     worldBuffer.world = DirectX::XMMatrixIdentity();
     ZeroMemory(&data, sizeof(data));
     data.pSysMem = &worldBuffer;
     data.SysMemPitch = sizeof(worldBuffer);
     data.SysMemSlicePitch = 0;
     result = device->CreateBuffer(&desc, &data, &pWorldBuffer_);
     if (FAILED(result))
          throw std::runtime_error("Failed to create world buffer");

     ZeroMemory(&desc, sizeof(desc));
     desc.ByteWidth = sizeof(SceneBuffer);
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;
     result = device->CreateBuffer(&desc, nullptr, &pSceneBuffer_);
     if (FAILED(result))
          throw std::runtime_error("Failed to create scene buffer");

     D3D11_RASTERIZER_DESC rasterizerDesc = {};
     rasterizerDesc.AntialiasedLineEnable = false;
     rasterizerDesc.FillMode = D3D11_FILL_SOLID;
     rasterizerDesc.CullMode = D3D11_CULL_NONE;
     rasterizerDesc.DepthBias = 0;
     rasterizerDesc.DepthBiasClamp = 0.0f;
     rasterizerDesc.FrontCounterClockwise = false;
     rasterizerDesc.DepthClipEnable = true;
     rasterizerDesc.ScissorEnable = false;
     rasterizerDesc.MultisampleEnable = false;
     rasterizerDesc.SlopeScaledDepthBias = 0.0f;
     result = device->CreateRasterizerState(&rasterizerDesc, &pRasterizerState_);
     if (FAILED(result))
          throw std::runtime_error("Failed to create rasterizer state");

     D3D11_SAMPLER_DESC samplerDesc = {};
     samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
     samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
     samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
     samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
     samplerDesc.MinLOD = 0;
     samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
     samplerDesc.MipLODBias = 0.0f;
     samplerDesc.MaxAnisotropy = 16;
     samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
     samplerDesc.BorderColor[0] = samplerDesc.BorderColor[1] = samplerDesc.BorderColor[2] = samplerDesc.BorderColor[3] = 0.0f;
     pTexture_ = std::make_shared<Texture>(device, context, textureFileName_, samplerDesc);
}

CubeMap::~CubeMap()
{
     SafeRelease(pPixelShader_);
     SafeRelease(pVertexShader_);
     SafeRelease(pInputLayout_);
     SafeRelease(pRasterizerState_);
     SafeRelease(pSceneBuffer_);
     SafeRelease(pWorldBuffer_);
     SafeRelease(pIndexBuffer_);
     SafeRelease(pVertexBuffer_);
}

void CubeMap::Resize(const unsigned width, const unsigned height)
{
     float halfW = tanf(fov_ / 2) * near_;
     float halfH = static_cast<float>(height / width) * halfW;
     radius_ = sqrtf(near_ * near_ + halfH * halfH + halfW * halfW) * 10.0f;
}

void CubeMap::Render()
{
     context_->RSSetState(pRasterizerState_);

     context_->IASetIndexBuffer(pIndexBuffer_, DXGI_FORMAT_R32_UINT, 0);
     ID3D11SamplerState *samplers[] = {pTexture_->GetSampler()};
     context_->PSSetSamplers(0, 1, samplers);
     ID3D11ShaderResourceView *resources[] = {pTexture_->GetTexture()};
     context_->PSSetShaderResources(0, 1, resources);
     ID3D11Buffer *vertexBuffers[] = {pVertexBuffer_};
     UINT strides[] = {12};
     UINT offsets[] = {0};
     context_->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
     context_->IASetInputLayout(pInputLayout_);
     context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
     context_->VSSetShader(pVertexShader_, nullptr, 0);
     context_->VSSetConstantBuffers(0, 1, &pWorldBuffer_);
     context_->VSSetConstantBuffers(1, 1, &pSceneBuffer_);
     context_->PSSetShader(pPixelShader_, nullptr, 0);

     context_->DrawIndexed(sphereFaces_ * 3, 0, 0);
}

void CubeMap::Update(const DirectX::XMMATRIX &view, const DirectX::XMMATRIX &proj, const DirectX::XMFLOAT3 &pov)
{
     WorldBuffer worldBuffer;
     worldBuffer.world = DirectX::XMMatrixIdentity();
     worldBuffer.size = DirectX::XMFLOAT4(radius_, 0.0f, 0.0f, 0.0f);
     context_->UpdateSubresource(pWorldBuffer_, 0, nullptr, &worldBuffer, 0, 0);

     SceneBuffer sceneBuffer;
     sceneBuffer.viewProj = XMMatrixMultiply(view, proj);
     sceneBuffer.pov = DirectX::XMFLOAT4(pov.x, pov.y, pov.z, 1.0f);
     context_->UpdateSubresource(pSceneBuffer_, 0, nullptr, &sceneBuffer, 0, 0);
}
