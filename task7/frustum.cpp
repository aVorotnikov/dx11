#include "frustum.h"

#include <cmath>

namespace
{

     void Normalize(float &a1, float &a2, float &a3, float &a4)
     {
          float length = std::sqrt(a1 * a1 + a2 * a2 + a3 * a3);
          a1 /= length;
          a2 /= length;
          a3 /= length;
          a4 /= length;
     }

}

Frustum::Frustum(const float screenDepth) : screenDepth_(screenDepth)
{
}

void Frustum::Construct(DirectX::XMMATRIX view, DirectX::XMMATRIX projection)
{
     DirectX::XMFLOAT4X4 pMatrix;
     DirectX::XMStoreFloat4x4(&pMatrix, projection);

     float zMinimum = -pMatrix._43 / pMatrix._33;
     float r = screenDepth_ / (screenDepth_ - zMinimum);

     pMatrix._33 = r;
     pMatrix._43 = -r * zMinimum;
     projection = XMLoadFloat4x4(&pMatrix);

     DirectX::XMMATRIX final = DirectX::XMMatrixMultiply(view, projection);

     DirectX::XMFLOAT4X4 matrix;
     DirectX::XMStoreFloat4x4(&matrix, final);

     planes_[0][0] = matrix._14 + matrix._13;
     planes_[0][1] = matrix._24 + matrix._23;
     planes_[0][2] = matrix._34 + matrix._33;
     planes_[0][3] = matrix._44 + matrix._43;
     Normalize(planes_[0][0], planes_[0][1], planes_[0][2], planes_[0][3]);

     planes_[1][0] = matrix._14 - matrix._13;
     planes_[1][1] = matrix._24 - matrix._23;
     planes_[1][2] = matrix._34 - matrix._33;
     planes_[1][3] = matrix._44 - matrix._43;
     Normalize(planes_[1][0], planes_[1][1], planes_[1][2], planes_[1][3]);

     planes_[2][0] = matrix._14 + matrix._11;
     planes_[2][1] = matrix._24 + matrix._21;
     planes_[2][2] = matrix._34 + matrix._31;
     planes_[2][3] = matrix._44 + matrix._41;
     Normalize(planes_[2][0], planes_[2][1], planes_[2][2], planes_[2][3]);

     planes_[3][0] = matrix._14 - matrix._11;
     planes_[3][1] = matrix._24 - matrix._21;
     planes_[3][2] = matrix._34 - matrix._31;
     planes_[3][3] = matrix._44 - matrix._41;
     Normalize(planes_[3][0], planes_[3][1], planes_[3][2], planes_[3][3]);

     planes_[4][0] = matrix._14 - matrix._12;
     planes_[4][1] = matrix._24 - matrix._22;
     planes_[4][2] = matrix._34 - matrix._32;
     planes_[4][3] = matrix._44 - matrix._42;
     Normalize(planes_[4][0], planes_[4][1], planes_[4][2], planes_[4][3]);

     planes_[5][0] = matrix._14 + matrix._12;
     planes_[5][1] = matrix._24 + matrix._22;
     planes_[5][2] = matrix._34 + matrix._32;
     planes_[5][3] = matrix._44 + matrix._42;
     Normalize(planes_[5][0], planes_[5][1], planes_[5][2], planes_[5][3]);
}

bool Frustum::CheckRectangle(float maxWidth, float maxHeight, float maxDepth, float minWidth, float minHeight, float minDepth)
{
     for (int i = 0; i < 6; i++)
     {
          float dotProduct = ((planes_[i][0] * minWidth) + (planes_[i][1] * minHeight) + (planes_[i][2] * minDepth) + (planes_[i][3] * 1.0f));
          if (dotProduct >= 0.0f)
               continue;

          dotProduct = ((planes_[i][0] * maxWidth) + (planes_[i][1] * minHeight) + (planes_[i][2] * minDepth) + (planes_[i][3] * 1.0f));
          if (dotProduct >= 0.0f)
               continue;

          dotProduct = ((planes_[i][0] * minWidth) + (planes_[i][1] * maxHeight) + (planes_[i][2] * minDepth) + (planes_[i][3] * 1.0f));
          if (dotProduct >= 0.0f)
               continue;

          dotProduct = ((planes_[i][0] * maxWidth) + (planes_[i][1] * maxHeight) + (planes_[i][2] * minDepth) + (planes_[i][3] * 1.0f));
          if (dotProduct >= 0.0f)
               continue;

          dotProduct = ((planes_[i][0] * minWidth) + (planes_[i][1] * minHeight) + (planes_[i][2] * maxDepth) + (planes_[i][3] * 1.0f));
          if (dotProduct >= 0.0f)
               continue;

          dotProduct = ((planes_[i][0] * maxWidth) + (planes_[i][1] * minHeight) + (planes_[i][2] * maxDepth) + (planes_[i][3] * 1.0f));
          if (dotProduct >= 0.0f)
               continue;

          dotProduct = ((planes_[i][0] * minWidth) + (planes_[i][1] * maxHeight) + (planes_[i][2] * maxDepth) + (planes_[i][3] * 1.0f));
          if (dotProduct >= 0.0f)
               continue;

          dotProduct = ((planes_[i][0] * maxWidth) + (planes_[i][1] * maxHeight) + (planes_[i][2] * maxDepth) + (planes_[i][3] * 1.0f));
          if (dotProduct >= 0.0f)
               continue;

          return false;
     }

     return true;
}
