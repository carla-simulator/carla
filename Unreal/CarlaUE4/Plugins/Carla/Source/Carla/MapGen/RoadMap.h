// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "UObject/NoExportTypes.h"
#include "MapGen/CityMapMeshTag.h"
#include "RoadMap.generated.h"

/// Road map intersection result. See URoadMap.
USTRUCT(BlueprintType)
struct CARLA_API FRoadMapIntersectionResult
{
  GENERATED_BODY()

  /// Percentage of the box lying off-road.
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
  float OffRoad;

  /// Percentage of the box invading opposite lane (wrong direction).
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
  float OppositeLane;
};

/// Data stored in a road map pixel. See URoadMap.
struct FRoadMapPixelData
{
  friend class URoadMap;

  constexpr static int IsRoadRow = 15;

  constexpr static int HasDirectionRow = 14;

  constexpr static uint16 MaximumEncodedAngle = (1 << 14) - 1;

  constexpr static uint16 AngleMask = (0xFFFF >> 2);

public:

  explicit FRoadMapPixelData(uint16 inValue) : Value(inValue) {}

  /// Whether this pixel lies in-road.
  bool IsRoad() const
  {
    return (Value & (1 << IsRoadRow)) != 0;
  }

  /// Whether this pixel has a direction defined (e.g. road intersections are
  /// not off-road but neither have defined direction).
  bool HasDirection() const
  {
    return (Value & (1 << HasDirectionRow)) != 0;
  }

  /// Get the azimuth angle [-PI, PI] of the road direction (in spherical
  /// coordinates) at this pixel.
  ///
  /// Undefined if !HasDirection().
  float GetDirectionAzimuthalAngle() const
  {
    const float Angle = AngleMask & Value;
    // Internally the angle is rotated by PI.
    return (Angle * 2.0f * PI / MaximumEncodedAngle) - PI;
  }

  /// Get the road direction at this pixel.
  ///
  /// Undefined if !HasDirection().
  FVector GetDirection() const
  {
    const FVector2D SphericalCoords(HALF_PI, GetDirectionAzimuthalAngle());
    return SphericalCoords.SphericalToUnitCartesian();
  }

  FColor EncodeAsColor() const;

private:

  static uint16 Encode(bool IsRoad, bool HasDirection, const FVector &Direction);

  uint16 Value;
};

/// Road map of the level. Contains information in 2D of which areas are road
/// and lane directions.
UCLASS()
class CARLA_API URoadMap : public UObject
{
  GENERATED_BODY()

public:

  /// Creates a valid empty map (every point is off-road).
  URoadMap(const FObjectInitializer& ObjectInitializer);

  /// Resets current map an initializes an empty map of the given size.
  void Reset(
      uint32 Width,
      uint32 Height,
      float PixelsPerCentimeter,
      const FTransform &WorldToMap,
      const FVector &MapOffset);

  void SetPixelAt(
      uint32 PixelX,
      uint32 PixelY,
      ECityMapMeshTag Tag,
      const FTransform &Transform,
      bool bInvertDirection = false);

  uint32 GetWidth() const
  {
    return Width;
  }

  uint32 GetHeight() const
  {
    return Height;
  }

  /// Return the world location of a given pixel.
  FVector GetWorldLocation(uint32 PixelX, uint32 PixelY) const;

  /// Retrieve the data stored at a given pixel.
  FRoadMapPixelData GetDataAt(uint32 PixelX, uint32 PixelY) const
  {
    check(IsValid());
    return FRoadMapPixelData(RoadMapData[GetIndex(PixelX, PixelY)]);
  }

  /// Clamps value if lies outside map limits.
  FRoadMapPixelData GetDataAt(const FVector &WorldLocation) const;

  /// Intersect actor bounds with map.
  ///
  /// Bounds box is projected to the map and checked against it for possible
  /// intersections with off-road areas and opposite lanes.
  FRoadMapIntersectionResult Intersect(
      const FTransform &BoxTransform,
      const FVector &BoxExtent,
      float ChecksPerCentimeter) const;

  /// Save the current map as PNG with the pixel data encoded as color.
  bool SaveAsPNG(const FString &Folder, const FString &MapName) const;

#if WITH_EDITOR

  /// Log status of the map to the console.
  void Log() const;

  /// Draw every pixel of the image as debug point.
  void DrawDebugPixelsToLevel(UWorld *World, bool bJustFlushDoNotDraw = false) const;

#endif // WITH_EDITOR

private:

  int32 GetIndex(uint32 PixelX, uint32 PixelY) const
  {
    return PixelX + Width * PixelY;
  }

  bool IsValid() const
  {
    return ((RoadMapData.Num() > 0) && (RoadMapData.Num() == Height * Width));
  }

  /// World-to-map transform.
  UPROPERTY(VisibleAnywhere)
  FTransform WorldToMap;

  /// Offset of the map in map coordinates.
  UPROPERTY(VisibleAnywhere)
  FVector MapOffset;

  /// Number of pixels per centimeter.
  UPROPERTY(VisibleAnywhere)
  float PixelsPerCentimeter;

  /// Width of the map in pixels.
  UPROPERTY(VisibleAnywhere)
  uint32 Width;

  /// Height of the map in pixels.
  UPROPERTY(VisibleAnywhere)
  uint32 Height;

  UPROPERTY()
  TArray<uint16> RoadMapData;
};
