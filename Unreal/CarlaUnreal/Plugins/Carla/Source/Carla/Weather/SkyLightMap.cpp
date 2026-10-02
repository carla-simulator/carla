// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Weather/SkyLightMap.h"
#include "Carla.h"

#include <util/ue-header-guard-begin.h>
#include "Engine/TextureCube.h"
#include "Math/Float16Color.h"
#include "PixelFormat.h"
#include <util/ue-header-guard-end.h>

#include <cmath>

namespace
{
  // Direction of a cube face texel, Unreal/D3D convention. su, sv in [-1, 1]
  // with sv growing downwards (texture rows). Mirrors GetCubemapVector in
  // Engine/Shaders/Private/ReflectionEnvironmentShaders.usf.
  FVector FaceDirection(int32 Face, float su, float sv)
  {
    switch (Face)
    {
      case 0:  return FVector( 1.0f, -sv, -su);
      case 1:  return FVector(-1.0f, -sv,  su);
      case 2:  return FVector( su,  1.0f,  sv);
      case 3:  return FVector( su, -1.0f, -sv);
      case 4:  return FVector( su, -sv,  1.0f);
      default: return FVector(-su, -sv, -1.0f);
    }
  }

  // Bilinear fetch, wrapping in azimuth, clamping in elevation.
  void SampleEquirect(const float* RGBA, int32 W, int32 H, float u, float v, float Out[4])
  {
    u -= 0.5f; v -= 0.5f;
    const float fu = std::floor(u), fv = std::floor(v);
    const float tu = u - fu, tv = v - fv;
    int32 u0 = int32(fu), v0 = int32(fv);
    int32 u1 = u0 + 1, v1 = v0 + 1;
    u0 = ((u0 % W) + W) % W; u1 = ((u1 % W) + W) % W;
    v0 = FMath::Clamp(v0, 0, H - 1); v1 = FMath::Clamp(v1, 0, H - 1);
    const float* p00 = RGBA + 4 * (v0 * W + u0);
    const float* p10 = RGBA + 4 * (v0 * W + u1);
    const float* p01 = RGBA + 4 * (v1 * W + u0);
    const float* p11 = RGBA + 4 * (v1 * W + u1);
    for (int32 c = 0; c < 4; ++c)
    {
      Out[c] = (p00[c] * (1 - tu) + p10[c] * tu) * (1 - tv) + (p01[c] * (1 - tu) + p11[c] * tu) * tv;
    }
  }
}

UTextureCube* CarlaSkyLightMap::CreateCubemapFromEquirect(
    const float* RGBA, int32 Width, int32 Height, int32 FaceSize)
{
  if (RGBA == nullptr || Width < 2 || Height < 2 || FaceSize < 4)
  {
    UE_LOG(LogCarla, Error, TEXT("SkyLightMap: invalid panorama %dx%d face %d"), Width, Height, FaceSize);
    return nullptr;
  }

  // Unique name on purpose: a fixed name makes NewObject overwrite the previous
  // (still rooted, still assigned) cubemap in place, so the sky light sees the
  // same pointer and never re-captures.
  UTextureCube* Cube = UTextureCube::CreateTransient(FaceSize, FaceSize, PF_FloatRGBA,
      MakeUniqueObjectName(GetTransientPackage(), UTextureCube::StaticClass(), TEXT("CarlaSkyLightMap")));
  if (Cube == nullptr)
  {
    return nullptr;
  }
  Cube->SRGB = false;
  Cube->CompressionSettings = TC_HDR;
  Cube->NeverStream = true;
  Cube->Filter = TF_Bilinear;

  FTexture2DMipMap& Mip = Cube->GetPlatformData()->Mips[0];
  FFloat16Color* Dst = static_cast<FFloat16Color*>(Mip.BulkData.Lock(LOCK_READ_WRITE));
  const float TwoPi = 2.0f * PI;
  for (int32 Face = 0; Face < 6; ++Face)
  {
    FFloat16Color* FaceDst = Dst + int64(Face) * FaceSize * FaceSize;
    for (int32 j = 0; j < FaceSize; ++j)
    {
      const float sv = (j + 0.5f) / FaceSize * 2.0f - 1.0f;
      for (int32 i = 0; i < FaceSize; ++i)
      {
        const float su = (i + 0.5f) / FaceSize * 2.0f - 1.0f;
        const FVector d = FaceDirection(Face, su, sv).GetSafeNormal();
        float phi = std::atan2(d.Y, d.X);            // from +X toward +Y
        if (phi < 0.0f) phi += TwoPi;
        const float theta = std::asin(FMath::Clamp(float(d.Z), -1.0f, 1.0f));
        const float u = phi / TwoPi * Width;
        const float v = (0.5f - theta / PI) * Height;
        float Px[4];
        SampleEquirect(RGBA, Width, Height, u, v, Px);
        FaceDst[j * FaceSize + i] = FFloat16Color(FLinearColor(Px[0], Px[1], Px[2], 1.0f));
      }
    }
  }
  Mip.BulkData.Unlock();
  Cube->UpdateResource();
  UE_LOG(LogCarla, Log, TEXT("SkyLightMap: built %dx%d cubemap from %dx%d panorama"), FaceSize, FaceSize, Width, Height);
  return Cube;
}
