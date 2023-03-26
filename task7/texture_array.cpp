#include "texture_array.h"
#include "utils.h"
#include "DDSTextureLoader.h"
#include <stdexcept>

TextureArray::TextureArray(
     ID3D11Device *device,
     ID3D11DeviceContext *context,
     const std::vector<std::wstring> &fileNames,
     const D3D11_SAMPLER_DESC &samplerDesc) : pTextureView_(NULL), pSampler_(NULL)
{
     std::vector<ID3D11Texture2D *> textures(fileNames.size());

     for (UINT i = 0; i < fileNames.size(); ++i)
     {
          auto result = DirectX::CreateDDSTextureFromFile(
               device,
               fileNames[i].c_str(),
               reinterpret_cast<ID3D11Resource **>(&textures[i]),
               nullptr);
          if (FAILED(result))
               throw std::exception("Failed to load texture from file");
     }

     D3D11_TEXTURE2D_DESC textureDesc;
     textures[0]->GetDesc(&textureDesc);
     D3D11_TEXTURE2D_DESC arrayDesc;
     arrayDesc.Width = textureDesc.Width;
     arrayDesc.Height = textureDesc.Height;
     arrayDesc.MipLevels = textureDesc.MipLevels;
     arrayDesc.ArraySize = static_cast<UINT>(fileNames.size());
     arrayDesc.Format = textureDesc.Format;
     arrayDesc.SampleDesc.Count = 1;
     arrayDesc.SampleDesc.Quality = 0;
     arrayDesc.Usage = D3D11_USAGE_DEFAULT;
     arrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
     arrayDesc.CPUAccessFlags = 0;
     arrayDesc.MiscFlags = 0;

     ID3D11Texture2D *textureArray = nullptr;
     auto result = device->CreateTexture2D(&arrayDesc, 0, &textureArray);
     if (FAILED(result))
          throw std::exception("Failed to create texture");

     for (std::size_t texElement = 0; texElement < fileNames.size(); ++texElement) {
          for (std::size_t mipLevel = 0; mipLevel < textureDesc.MipLevels; ++mipLevel) {
               const int sourceSubresource = D3D11CalcSubresource(static_cast<UINT32>(mipLevel), 0, textureDesc.MipLevels);
               const int destSubresource = D3D11CalcSubresource(
                    static_cast<UINT32>(mipLevel),
                    static_cast<UINT32>(texElement),
                    textureDesc.MipLevels);
               context->CopySubresourceRegion(
                    textureArray,
                    destSubresource,
                    0,
                    0,
                    0,
                    textures[texElement],
                    sourceSubresource,
                    nullptr);
          }
     }

     D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
     viewDesc.Format = arrayDesc.Format;
     viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
     viewDesc.Texture2DArray.MostDetailedMip = 0;
     viewDesc.Texture2DArray.MipLevels = arrayDesc.MipLevels;
     viewDesc.Texture2DArray.FirstArraySlice = 0;
     viewDesc.Texture2DArray.ArraySize = static_cast<UINT32>(fileNames.size());

     result = device->CreateShaderResourceView(textureArray, &viewDesc, &pTextureView_);
     if (FAILED(result))
          throw std::exception("Failed to create shader resource view");

     SafeRelease(textureArray);
     for (UINT i = 0; i < fileNames.size(); ++i)
          SafeRelease(textures[i]);

     if (FAILED(device->CreateSamplerState(&samplerDesc, &pSampler_)))
     {
          SafeRelease(pTextureView_);
          throw std::runtime_error("Failed to create texture sample");
     }
}

TextureArray::~TextureArray()
{
     SafeRelease(pSampler_);
     SafeRelease(pTextureView_);
}

ID3D11ShaderResourceView *TextureArray::GetTextures()
{
     return pTextureView_;
}

ID3D11SamplerState *TextureArray::GetSampler()
{
     return pSampler_;
}
