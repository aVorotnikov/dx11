#pragma once

#include <directxmath.h>

class Frustum
{
public:
     Frustum(const float screenDepth);
     void Construct(DirectX::XMMATRIX view, DirectX::XMMATRIX projection);
     bool CheckRectangle(float maxWidth, float maxHeight, float maxDepth, float minWidth, float minHeight, float minDepth);

private:
     const float screenDepth_;
     float planes_[6][4];
};