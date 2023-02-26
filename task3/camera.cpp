#include "camera.h"

Camera::Camera(const DirectX::XMFLOAT3 pov, const DirectX::XMFLOAT3 poi, const DirectX::XMFLOAT3 nearUp) :
     pov_(pov), poi_(poi), nearUp_(nearUp)
{
}

DirectX::XMMATRIX Camera::GetView()
{
     return DirectX::XMMatrixLookAtLH(
          DirectX::XMVectorSet(pov_.x, pov_.y, pov_.z, 0.0f),
          DirectX::XMVectorSet(poi_.x, poi_.y, poi_.z, 0.0f),
          DirectX::XMVectorSet(nearUp_.x, nearUp_.y, nearUp_.z, 0.0f)
     );
}
