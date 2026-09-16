// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Commandlet/CarlaCrosswalkNavBuilder.h"

#include "Carla/Navigation/CarlaNavAreas.h"
#include "Carla/Navigation/CarlaNavModifierBox.h"
#include "Carla/OpenDrive/OpenDrive.h"

#include <util/ue-header-guard-begin.h>
#include "Engine/World.h"
#include "PackageSourceControlHelper.h"
#include "SourceControlHelpers.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionActorDescInstance.h"
#include "WorldPartition/WorldPartitionHelpers.h"
#include <util/ue-header-guard-end.h>

#include <util/disable-ue4-macros.h>
#include <carla/geom/Location.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/road/Map.h>
#include <util/enable-ue4-macros.h>

#include <string>
#include <vector>

DEFINE_LOG_CATEGORY_STATIC(LogCarlaCrosswalkNavBuilder, Log, All);

namespace {

/// Split the flattened GetAllCrosswalkZones() output into polygons. The
/// nominal CARLA convention (see carla.Map.get_crosswalks) closes each
/// outline by repeating its first corner, but some OpenDRIVE exports close
/// onto an interior corner instead (all 34 Town15 outlines do), so a
/// polygon closes when the incoming point matches ANY earlier corner of
/// the current run; corners before the matched one are a leading fragment
/// of the previous outline and are dropped. A safety cap flushes unclosed
/// runs so one malformed outline cannot swallow the rest of the map.
TArray<TArray<FVector>> SplitCrosswalkPolygons(
    const std::vector<carla::geom::Location> &Points)
{
  constexpr float CloseEpsilon = 1.0f;   // cm
  constexpr int32 MaxCorners = 16;
  TArray<TArray<FVector>> Polygons;
  TArray<FVector> Current;
  for (const auto &Point : Points)
  {
    const FVector UEPoint(Point);   // meters -> centimeters
    int32 MatchIndex = INDEX_NONE;
    for (int32 i = 0; i < Current.Num(); ++i)
    {
      if (FVector::Dist2D(UEPoint, Current[i]) < CloseEpsilon)
      {
        MatchIndex = i;
        break;
      }
    }
    if (MatchIndex != INDEX_NONE)
    {
      TArray<FVector> Polygon(Current.GetData() + MatchIndex,
                              Current.Num() - MatchIndex);
      if (Polygon.Num() >= 3)
      {
        Polygons.Add(MoveTemp(Polygon));
      }
      Current.Reset();
      continue;
    }
    Current.Add(UEPoint);
    if (Current.Num() > MaxCorners)
    {
      UE_LOG(LogCarlaCrosswalkNavBuilder, Warning, TEXT(
          "Crosswalk outline with more than %d corners never closed; flushing."),
          MaxCorners);
      Polygons.Add(MoveTemp(Current));
      Current.Reset();
    }
  }
  if (Current.Num() >= 3)
  {
    Polygons.Add(MoveTemp(Current));
  }
  return Polygons;
}

/// Fit an oriented box to a crosswalk polygon: X axis along the longest
/// edge, extents from the corner projections.
void FitOrientedBox(
    const TArray<FVector> &Corners,
    FVector &OutCenter,
    FRotator &OutRotation,
    FVector &OutExtent)
{
  FVector Centroid = FVector::ZeroVector;
  for (const FVector &Corner : Corners)
  {
    Centroid += Corner;
  }
  Centroid /= Corners.Num();

  FVector LongestEdge = FVector::ForwardVector;
  float LongestLength = 0.0f;
  for (int32 i = 0; i < Corners.Num(); ++i)
  {
    const FVector Edge = Corners[(i + 1) % Corners.Num()] - Corners[i];
    const float Length = Edge.Size2D();
    if (Length > LongestLength)
    {
      LongestLength = Length;
      LongestEdge = Edge;
    }
  }
  const FVector AxisX = LongestEdge.GetSafeNormal2D();
  const FVector AxisY = FVector::CrossProduct(FVector::UpVector, AxisX);

  float MaxX = 0.0f, MaxY = 0.0f, MaxZ = 0.0f;
  for (const FVector &Corner : Corners)
  {
    const FVector Rel = Corner - Centroid;
    MaxX = FMath::Max(MaxX, FMath::Abs(static_cast<float>(Rel | AxisX)));
    MaxY = FMath::Max(MaxY, FMath::Abs(static_cast<float>(Rel | AxisY)));
    MaxZ = FMath::Max(MaxZ, FMath::Abs(static_cast<float>(Rel.Z)));
  }

  OutCenter = Centroid;
  OutRotation = FRotationMatrix::MakeFromXZ(AxisX, FVector::UpVector).Rotator();
  // Generous Z so the stamp reaches the navmesh through the road's actual
  // elevation; the modifier only relabels overlapped polys, so oversizing Z
  // is harmless.
  OutExtent = FVector(MaxX, MaxY, FMath::Max(MaxZ + 100.0f, 200.0f));
}

} // namespace

bool UCarlaCrosswalkNavBuilder::RunInternal(
    UWorld *World,
    const FCellInfo &InCellInfo,
    FPackageSourceControlHelper &PackageHelper)
{
  UWorldPartition *WorldPartition = World->GetWorldPartition();
  if (WorldPartition == nullptr)
  {
    UE_LOG(LogCarlaCrosswalkNavBuilder, Error, TEXT(
        "%s is not a World Partition map."), *World->GetName());
    return false;
  }

  // 1. Delete boxes from previous runs (rerunnable builder).
  TArray<FString> StalePackages;
  FWorldPartitionHelpers::ForEachActorDescInstance<ACarlaNavModifierBox>(
      WorldPartition,
      [&StalePackages](const FWorldPartitionActorDescInstance *Desc)
      {
        StalePackages.Add(Desc->GetActorPackage().ToString());
        return true;
      });
  if (StalePackages.Num() > 0)
  {
    UE_LOG(LogCarlaCrosswalkNavBuilder, Display, TEXT(
        "Deleting %d nav modifier boxes from a previous run."), StalePackages.Num());
    if (!PackageHelper.Delete(StalePackages))
    {
      UE_LOG(LogCarlaCrosswalkNavBuilder, Error, TEXT("Failed deleting stale boxes."));
      return false;
    }
  }

  // 2. Parse the map's OpenDRIVE and collect crosswalk polygons.
  const FString OpenDriveContent = UOpenDrive::LoadXODR(World->GetName());
  if (OpenDriveContent.IsEmpty())
  {
    UE_LOG(LogCarlaCrosswalkNavBuilder, Error, TEXT(
        "No OpenDRIVE file found for map %s."), *World->GetName());
    return false;
  }
  const auto CarlaMap = carla::opendrive::OpenDriveParser::Load(
      std::string(TCHAR_TO_UTF8(*OpenDriveContent)));
  if (!CarlaMap.has_value())
  {
    UE_LOG(LogCarlaCrosswalkNavBuilder, Error, TEXT(
        "Failed parsing the OpenDRIVE of %s."), *World->GetName());
    return false;
  }
  const auto Zones = CarlaMap->GetAllCrosswalkZones();
  const TArray<TArray<FVector>> Polygons = SplitCrosswalkPolygons(Zones);
  UE_LOG(LogCarlaCrosswalkNavBuilder, Display, TEXT(
      "%s: %d crosswalk corner points -> %d polygons."),
      *World->GetName(), static_cast<int32>(Zones.size()), Polygons.Num());

  // 3. Spawn one modifier box per polygon, each in its own external-actor
  // package (same pattern as the engine navigation data chunk builder).
  TArray<UPackage *> PackagesToSave;
  for (const TArray<FVector> &Polygon : Polygons)
  {
    if (Polygon.Num() < 3)
    {
      continue;
    }
    FVector Center;
    FRotator Rotation;
    FVector Extent;
    FitOrientedBox(Polygon, Center, Rotation, Extent);

    FActorSpawnParameters SpawnParams;
    SpawnParams.bCreateActorPackage = true;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ACarlaNavModifierBox *Box = World->SpawnActor<ACarlaNavModifierBox>(
        Center, Rotation, SpawnParams);
    if (Box == nullptr)
    {
      UE_LOG(LogCarlaCrosswalkNavBuilder, Error, TEXT("SpawnActor failed."));
      return false;
    }
    Box->Configure(UNavAreaCarlaCrosswalk::StaticClass(), Extent);
#if WITH_EDITOR
    Box->SetActorLabel(FString::Printf(TEXT("CarlaCrosswalkNav_%d"),
        PackagesToSave.Num()));
#endif
    if (UPackage *ActorPackage = Box->GetExternalPackage())
    {
      PackagesToSave.Add(ActorPackage);
    }
  }

  // 4. Save the new actor packages.
  UE_LOG(LogCarlaCrosswalkNavBuilder, Display, TEXT(
      "Saving %d crosswalk nav modifier boxes."), PackagesToSave.Num());
  for (UPackage *Package : PackagesToSave)
  {
    const FString PackageFileName = SourceControlHelpers::PackageFilename(Package);
    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Standalone;
    SaveArgs.SaveFlags = SAVE_Async;
    if (!UPackage::SavePackage(Package, nullptr, *PackageFileName, SaveArgs))
    {
      UE_LOG(LogCarlaCrosswalkNavBuilder, Error, TEXT(
          "Error saving package %s."), *Package->GetName());
      return false;
    }
  }
  UPackage::WaitForAsyncFileWrites();

  UE_LOG(LogCarlaCrosswalkNavBuilder, Display, TEXT(
      "Done: %d crosswalk boxes generated for %s."),
      PackagesToSave.Num(), *World->GetName());
  return true;
}
