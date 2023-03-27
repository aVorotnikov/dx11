#pragma once

#include <d3d11.h>
#include <string>
#include <vector>

class TextureArray
{
public:
     TextureArray(
          ID3D11Device *device,
          ID3D11DeviceContext *context,
          const std::vector<std::wstring>& fileNames,
          const D3D11_SAMPLER_DESC &samplerDesc = defaultSamplerDescription_);
     ~TextureArray();
     ID3D11ShaderResourceView *GetTextures();
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
