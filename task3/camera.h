#pragma once

#include <directxmath.h>

class Camera
{
public:
     Camera(
          const DirectX::XMFLOAT3 pov = DirectX::XMFLOAT3(2, 2, 0),
          const DirectX::XMFLOAT3 poi = DirectX::XMFLOAT3(0, 0, 0),
          const DirectX::XMFLOAT3 nearUp = DirectX::XMFLOAT3(0, 1, 0));

     DirectX::XMMATRIX GetView();

private:
     DirectX::XMFLOAT3 pov_;
     DirectX::XMFLOAT3 poi_;
     DirectX::XMFLOAT3 nearUp_;
};