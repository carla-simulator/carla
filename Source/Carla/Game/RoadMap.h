// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "Math/Vector.h"

struct FCapturedImage;

/// Road map of the level. Contains information in 2D of which areas are road
/// and lane directions.
class CARLA_API RoadMap
{
public:

  void SetCapture(
      const FCapturedImage &MapCapture,
      const FVector2D &Origin,
      const FVector2D &MapExtent);

  bool IsValid() const;

  FVector2D GetDirectionAt(const FVector &Location) const;

private:

  uint32 GetPixelIndexAt(const FVector2D &Location) const;

  TArray<FVector2D> DirectionMap;
};
