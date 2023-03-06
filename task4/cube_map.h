#pragma once

#include "texture.h"
#include <d3d11.h>
#include <directxmath.h>
#include <memory>

class CubeMap
{
public:
     CubeMap(
          ID3D11Device *device,
          ID3D11DeviceContext *context,
          const unsigned width,
          const unsigned height,
          const float fov,
          const float frustumNear);
     ~CubeMap();
     void Resize(const unsigned width, const unsigned height);
     void Render();
     void Update(const DirectX::XMMATRIX &view, const DirectX::XMMATRIX &proj, const DirectX::XMFLOAT3 &pov);

private:
     static constexpr const WCHAR textureFileName_[] = L"images/sb.dds";

     static constexpr const unsigned latitudeLines_ = 10;
     static constexpr const unsigned longtitudeLines_ = 10;
     static constexpr const unsigned sphereVertices_ = (latitudeLines_ - 2) * longtitudeLines_ + 2;
     static constexpr const unsigned sphereFaces_ = (latitudeLines_ - 3) * longtitudeLines_ * 2 + longtitudeLines_ * 2;

     ID3D11DeviceContext *context_;

     ID3D11Buffer *pVertexBuffer_;
     ID3D11Buffer *pIndexBuffer_;
     ID3D11Buffer *pWorldBuffer_;
     ID3D11Buffer *pSceneBuffer_;
     ID3D11RasterizerState *pRasterizerState_;

     ID3D11InputLayout *pInputLayout_;
     ID3D11VertexShader *pVertexShader_;
     ID3D11PixelShader *pPixelShader_;

     std::shared_ptr<Texture> pTexture_;

     const float fov_;
     const float near_;
     float radius_;
};
