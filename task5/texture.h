#pragma once

#include <d3d11.h>
#include <string>
#include "DDSTextureLoader.h"

class Texture
{
public:
     Texture(
          ID3D11Device *device,
          std::wstring fileName,
          const D3D11_SAMPLER_DESC &samplerDesc = defaultSamplerDescription_);
     Texture(
          ID3D11Device *device,
          ID3D11DeviceContext *context,
          std::wstring fileName,
          const D3D11_SAMPLER_DESC &samplerDesc = defaultSamplerDescription_);
     ~Texture();
     ID3D11ShaderResourceView *GetTexture();
     ID3D11SamplerState *GetSampler();

     inline static const D3D11_SAMPLER_DESC defaultSamplerDescription_ =
     {
          D3D11_FILTER_ANISOTROPIC,
          D3D11_TEXTURE_ADDRESS_CLAMP,
          D3D11_TEXTURE_ADDRESS_CLAMP,
          D3D11_TEXTURE_ADDRESS_CLAMP,
          0.0f,
          16,
          D3D11_COMPARISON_NEVER,
          {1.0f, 1.0f, 1.0f, 1.0f},
          -D3D11_FLOAT32_MAX,
          D3D11_FLOAT32_MAX
     };

private:
     ID3D11ShaderResourceView *pTextureView_;
     ID3D11SamplerState *pSampler_;
};
