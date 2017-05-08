// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "RoadMap.h"

#include "DrawDebugHelpers.h"
#include "HighResScreenshot.h"

#include <type_traits>

/// ============================================================================
/// -- Static local methods ----------------------------------------------------
/// ============================================================================

static uint32 ClampFloatToUInt(const float Value, int32 Min, int32 Max)
{
  return FMath::Clamp(FMath::FloorToInt(Value), Min, Max);
}

// Return the azimuth angle (in spherical coordinates) rotated by PI so it lies
// in the range [0, 2*PI].
static float GetRotatedAzimuthAngle(const FVector &Direction)
{
  const FVector2D SphericalCoords = Direction.UnitCartesianToSpherical();
  return SphericalCoords.Y + PI;
}

/// ============================================================================
/// -- FRoadMapPixelData -------------------------------------------------------
/// ============================================================================

uint16 FRoadMapPixelData::Encode(bool IsRoad, bool HasDirection, const FVector &Direction)
{
  const uint16 AngleAsUInt = MaximumEncodedAngle * GetRotatedAzimuthAngle(Direction) / (2.0f * PI);
  check(!(AngleAsUInt & (1 << IsRoadRow)));
  check(!(AngleAsUInt & (1 << HasDirectionRow)));
  return (IsRoad << IsRoadRow) | (HasDirection << HasDirectionRow) | (AngleAsUInt);
}

FColor FRoadMapPixelData::EncodeAsColor() const
{
  if (!IsRoad()) {
    return FColor(0u, 0u, 0u, 255u);
  } else if (!HasDirection()) {
    return FColor(255u, 255u, 255u, 255u);
  } else {
    auto ToColor = [](float X){
      return FMath::FloorToInt(256.0 * (X + PI) / (2.0f * PI)) % 256;
    };
    const float Azimuth = GetDirectionAzimuthalAngle();
    return FColor(0u, 255u, ToColor(Azimuth), 255u);
  }
}

/// ============================================================================
/// -- URoadMap ----------------------------------------------------------------
/// ============================================================================

URoadMap::URoadMap(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer),
  PixelsPerCentimeter(1.0f),
  Width(1u),
  Height(1u)
{
  RoadMapData.Add(0u);
  static_assert(
      std::is_same<decltype(FRoadMapPixelData::Value), typename decltype(RoadMapData)::ElementType>::value,
      "Declaration map of FRoadMapPixelData's value does not match current serialization type");
}

void URoadMap::Reset(
    const uint32 inWidth,
    const uint32 inHeight,
    const float inPixelsPerCentimeter,
    const FTransform &inWorldToMap,
    const FVector &inMapOffset)
{
  RoadMapData.Init(0u, inWidth * inHeight);
  Width = inWidth;
  Height = inHeight;
  PixelsPerCentimeter = inPixelsPerCentimeter;
  WorldToMap = inWorldToMap;
  MapOffset = inMapOffset;
}

void URoadMap::SetPixelAt(
    const uint32 PixelX,
    const uint32 PixelY,
    const ECityMapMeshTag Tag,
    const FTransform &Transform,
    const bool bInvertDirection)
{
  bool bIsRoad = false;
  bool bHasDirection = false;
  FVector Direction(0.0f, 0.0f, 0.0f);

  auto Rotator = Transform.GetRotation().Rotator();

  switch (Tag) {
    default:
      // Is not road.
      break;
    case ECityMapMeshTag::RoadTwoLanes_LaneRight:
    case ECityMapMeshTag::Road90DegTurn_Lane0:
      bIsRoad = true;
      bHasDirection = true;
      break;
    case ECityMapMeshTag::RoadTwoLanes_LaneLeft:
    case ECityMapMeshTag::Road90DegTurn_Lane1:
      bIsRoad = true;
      bHasDirection = true;
      Rotator.Yaw += 180.0f;
      break;
    case ECityMapMeshTag::Road90DegTurn_Lane2:
      bIsRoad = true;
      bHasDirection = true;
      Rotator.Yaw += 90.0f;
      break;
    case ECityMapMeshTag::Road90DegTurn_Lane3:
      bIsRoad = true;
      bHasDirection = true;
      Rotator.Yaw += 270.0f;
      break;
    case ECityMapMeshTag::RoadTIntersection_Lane0:
    case ECityMapMeshTag::RoadTIntersection_Lane1:
    case ECityMapMeshTag::RoadTIntersection_Lane2:
    case ECityMapMeshTag::RoadTIntersection_Lane3:
    case ECityMapMeshTag::RoadXIntersection_Lane0:
    case ECityMapMeshTag::RoadXIntersection_Lane1:
    case ECityMapMeshTag::RoadXIntersection_Lane2:
    case ECityMapMeshTag::RoadXIntersection_Lane3:
      bIsRoad = true;
      bHasDirection = false;
      break;
  }
  if (bHasDirection) {
    FQuat Rotation(Rotator);
    Direction = Rotation.GetForwardVector();
    if (bInvertDirection) {
      Direction *= -1.0f;
    }
  }
  const auto Value = FRoadMapPixelData::Encode(bIsRoad, bHasDirection, Direction);
  RoadMapData[GetIndex(PixelX, PixelY)] = Value;
}

FVector URoadMap::GetWorldLocation(uint32 PixelX, uint32 PixelY) const
{
  const FVector RelativePosition(
      static_cast<float>(PixelX) / PixelsPerCentimeter,
      static_cast<float>(PixelY) / PixelsPerCentimeter,
      0.0f);
  return WorldToMap.InverseTransformPosition(RelativePosition + MapOffset);
}

FRoadMapPixelData URoadMap::GetDataAt(const FVector &WorldLocation) const
{
  check(IsValid());
  const FVector Location = WorldToMap.TransformPosition(WorldLocation) - MapOffset;
  uint32 X = ClampFloatToUInt(PixelsPerCentimeter * Location.X, 0, Width - 1);
  uint32 Y = ClampFloatToUInt(PixelsPerCentimeter * Location.Y, 0, Height - 1);
  return GetDataAt(X, Y);
}

FRoadMapIntersectionResult URoadMap::Intersect(
    const FTransform &BoxTransform,
    const FVector &BoxExtent,
    float ChecksPerCentimeter) const
{
  auto DirectionOfMovement = BoxTransform.GetRotation().GetForwardVector();
  DirectionOfMovement.Z = 0.0f; // Project to XY plane (won't be normalized anymore).
  uint32 CheckCount = 0u;
  FRoadMapIntersectionResult Result = {0.0f, 0.0f};
  const float Step = 1.0f / ChecksPerCentimeter;
  for (float X = -BoxExtent.X; X < BoxExtent.X; X += Step) {
    for (float Y = -BoxExtent.Y; Y < BoxExtent.Y; Y += Step) {
      ++CheckCount;
      auto Location = BoxTransform.TransformPosition(FVector(X, Y, 0.0f));
      const auto &Data = GetDataAt(Location);
      if (!Data.IsRoad()) {
        Result.OffRoad += 1.0f;
      } else if (Data.HasDirection() &&
                 0.0f < FVector::DotProduct(Data.GetDirection(), DirectionOfMovement)) {
        Result.OppositeLane += 1.0f;
      }
    }
  }
  if (CheckCount > 0u) {
    Result.OffRoad /= static_cast<float>(CheckCount);
    Result.OppositeLane /= static_cast<float>(CheckCount);
  } else {
    UE_LOG(LogCarla, Warning, TEXT("URoadMap::Intersect did zero checks"));
  }
  return Result;
}

bool URoadMap::SaveAsPNG(const FString &Path) const
{
  if (!IsValid()) {
    UE_LOG(LogCarla, Error, TEXT("Cannot save invalid road map to disk"));
    return false;
  }

  TArray<FColor> BitMap;
  for (auto Value : RoadMapData) {
    BitMap.Emplace(FRoadMapPixelData(Value).EncodeAsColor());
  }

  FIntPoint DestSize(Width, Height);
  FString ResultPath;
  FHighResScreenshotConfig &HighResScreenshotConfig = GetHighResScreenshotConfig();
  HighResScreenshotConfig.SetHDRCapture(false);
  HighResScreenshotConfig.SaveImage(Path, BitMap, DestSize, &ResultPath);

  UE_LOG(LogCarla, Log, TEXT("Saved road map to \"%s\""), *ResultPath);
  return true;
}

#ifdef WITH_EDITOR

void URoadMap::Log() const
{
  const float MapSizeInMB = // Only map data, not the class itself.
      static_cast<float>(sizeof(decltype(RoadMapData)::ElementType) * RoadMapData.Num()) /
      (1024.0f * 1024.0f);
  UE_LOG(
      LogCarla,
      Log,
      TEXT("Generated road map %dx%d (%.2fMB) with %.2f cm/pixel"),
      GetWidth(),
      GetHeight(),
      MapSizeInMB,
      1.0f / PixelsPerCentimeter);

  if (!IsValid()) {
    UE_LOG(LogCarla, Error, TEXT("Error generating road map"));
    return;
  }
}

void URoadMap::DrawDebugPixelsToLevel(UWorld *World, const bool bJustFlushDoNotDraw) const
{
  FlushPersistentDebugLines(World);
  if (!bJustFlushDoNotDraw) {
    for (auto X = 0u; X < Width; ++X) {
      for (auto Y = 0u; Y < Height; ++Y) {
        auto Location = GetWorldLocation(X, Y);
        auto Color = GetDataAt(X, Y).EncodeAsColor();
        DrawDebugPoint(World, Location, 20.0f, Color, true);
      }
    }
  }
}

#endif // WITH_EDITOR
