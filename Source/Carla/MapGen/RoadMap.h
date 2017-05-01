// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "UObject/NoExportTypes.h"
#include "RoadMap.generated.h"

USTRUCT()
struct FRoadMapIntersectionResult
{
  GENERATED_BODY()

  /** Percentage of the box lying off-road */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
  float OffRoad;

  /** Percentage of the box invading opposite lane (wrong direction) */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
  float OppositeLane;
};

USTRUCT()
struct FRoadMapPixelData
{
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
  bool bIsOffRoad = true;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
  bool bHasDirection = false;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
  FVector Direction;
};

/// Road map of the level. Contains information in 2D of which areas are road
/// and lane directions.
UCLASS()
class CARLA_API URoadMap : public UObject
{
  GENERATED_BODY()

public:

  UFUNCTION(BlueprintCallable)
  bool IsValid() const
  {
    return ((RoadMap.Num() > 0) && (RoadMap.Num() == Height * Width));
  }

  uint32 GetWidth() const
  {
    return Width;
  }

  uint32 GetHeight() const
  {
    return Height;
  }

  const FRoadMapPixelData &GetDataAt(uint32 PixelX, uint32 PixelY) const
  {
    check(IsValid());
    return RoadMap[PixelX + Height * PixelY];
  }

  /** Clamps value if lies outside map limits */
  UFUNCTION(BlueprintCallable)
  const FRoadMapPixelData &GetDataAt(const FVector &WorldLocation) const;

  /** Intersect actor bounds with map.
   *
   *  Bounds box is projected to the map and checked against it for possible
   *  intersections with off-road areas and opposite lanes.
   */
  UFUNCTION(BlueprintCallable)
  FRoadMapIntersectionResult Intersect(
      const FTransform &BoxTransform,
      const FVector &BoxExtent,
      float ChecksPerCentimeter,
      bool LeftHandDriving) const;

  UFUNCTION(BlueprintCallable)
  bool SaveAsPNG(const FString &Path) const;

private:

  friend class ACityMapGenerator;

  void AppendEmptyPixel()
  {
    RoadMap.AddDefaulted(1);
  }

  void AppendPixel(ECityMapMeshTag Tag, const FTransform &Transform);

  void Set(uint32 Width, uint32 Height, float PixelsPerCentimeter, const FTransform &WorldToMap);

  UPROPERTY(VisibleAnywhere)
  FTransform WorldToMap;

  UPROPERTY(VisibleAnywhere)
  float PixelsPerCentimeter = 0.0f;

  /** Width of the map in pixels */
  UPROPERTY(VisibleAnywhere)
  uint32 Width = 0u;

  /** Height of the map in pixels */
  UPROPERTY(VisibleAnywhere)
  uint32 Height = 0u;

  UPROPERTY()
  TArray<FRoadMapPixelData> RoadMap;
};
