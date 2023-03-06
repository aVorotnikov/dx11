#include "texture.h"
#include "utils.h"
#include <stdexcept>

Texture::Texture(
     ID3D11Device *device,
     std::wstring fileName,
     const D3D11_SAMPLER_DESC &samplerDesc) : pTextureView_(NULL), pSampler_(NULL)
{
     if (FAILED(DirectX::CreateDDSTextureFromFile(device, fileName.c_str(), nullptr, &pTextureView_)))
          throw std::runtime_error("Failed to create texture");
     if (FAILED(device->CreateSamplerState(&samplerDesc, &pSampler_)))
     {
          SafeRelease(pTextureView_);
          throw std::runtime_error("Failed to create texture sample");
     }
}

Texture::Texture(
     ID3D11Device *device,
     ID3D11DeviceContext *context,
     std::wstring fileName,
     const D3D11_SAMPLER_DESC &samplerDesc) : pTextureView_(NULL), pSampler_(NULL)
{
     if (FAILED(DirectX::CreateDDSTextureFromFileEx(
          device,
          context,
          fileName.c_str(),
          0,
          D3D11_USAGE_DEFAULT,
          D3D11_BIND_SHADER_RESOURCE,
          0,
          D3D11_RESOURCE_MISC_TEXTURECUBE,
          false,
          nullptr,
          &pTextureView_)))
          throw std::runtime_error("Failed to create texture");
     if (FAILED(device->CreateSamplerState(&samplerDesc, &pSampler_)))
     {
          SafeRelease(pTextureView_);
          throw std::runtime_error("Failed to create texture sample");
     }
}

Texture::~Texture()
{
     SafeRelease(pSampler_);
     SafeRelease(pTextureView_);
}

ID3D11ShaderResourceView *Texture::GetTexture()
{
     return pTextureView_;
}

ID3D11SamplerState *Texture::GetSampler()
{
     return pSampler_;
}
