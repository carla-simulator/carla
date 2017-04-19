// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "RoadMap.h"

#include "CapturedImage.h"

static FVector2D Decode(const FColor &Color);

// void RoadMap::SetCapture(FCapturedImage &inMapCapture, const FVector2D &inMapSizeInCm)
// {
//   MapCapture = &inMapCapture;
//   MapSizeInCm = inMapSizeInCm
// }

// bool RoadMap::IsValid() const
// {
//   return ((MapCapture != nullptr) &&
//           (MapCapture->BitMap.Num() > 0) &&
//           (MapSizeInCm.X > 0.0f) &&
//           (MapSizeInCm.Y > 0.0f));
// }

// FVector2D RoadMap::GetDirectionAt(const FVector &Location) const
// {
//   check(IsValid());
//   return FVector2D();
// }
