// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// LaneLineExporter.cpp
#include "Carla.h"
#include "LaneLineExporter.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Traffic/RoadSpline.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Kismet/GameplayStatics.h"

// --------------------- helpers ---------------------

bool ULaneLineExporter::ParseSessionIdParts(const FString& In, FString& OutUuid, FString& OutStart, FString& OutEnd)
{
  int32 A = INDEX_NONE, B = INDEX_NONE;
  if (!In.FindChar(TEXT('_'), A)) return false;
  if (!In.FindLastChar(TEXT('_'), B)) return false;
  if (A <= 0 || B <= A+1 || B >= In.Len()-1) return false;
  OutUuid  = In.Left(A);
  OutStart = In.Mid(A+1, B-(A+1));
  OutEnd   = In.Mid(B+1);
  return true;
}

void ULaneLineExporter::AppendLaneLineLabel(
    TArray<TSharedPtr<FJsonValue>>& LabelsArr,
    const FString& StartTs,
    const TArray<FVector>& PolyMeters)
{
  TSharedRef<FJsonObject> Label = MakeShared<FJsonObject>();

  // labelData.shape3d.polyline3d
  TSharedRef<FJsonObject> LabelData = MakeShared<FJsonObject>();
  {
    TSharedRef<FJsonObject> Shape3D = MakeShared<FJsonObject>();
    Shape3D->SetStringField(TEXT("unit"), TEXT("METRIC"));

    // polyline3d.vertices
    {
      TSharedRef<FJsonObject> Polyline3D = MakeShared<FJsonObject>();
      TArray<TSharedPtr<FJsonValue>> VertArray;
      VertArray.Reserve(PolyMeters.Num());

      for (const FVector& V : PolyMeters)
      {
        TArray<TSharedPtr<FJsonValue>> Triple;
        Triple.Add(MakeShared<FJsonValueNumber>(V.X));
        Triple.Add(MakeShared<FJsonValueNumber>(V.Y));
        Triple.Add(MakeShared<FJsonValueNumber>(V.Z));
        VertArray.Add(MakeShared<FJsonValueArray>(Triple));
      }
      Polyline3D->SetArrayField(TEXT("vertices"), VertArray);
      Shape3D->SetObjectField(TEXT("polyline3d"), Polyline3D);
    }

    LabelData->SetObjectField(TEXT("shape3d"), Shape3D);
  }
  Label->SetObjectField(TEXT("labelData"), LabelData);

  Label->SetStringField(TEXT("timestampMicroseconds"), StartTs);

  LabelsArr.Add(MakeShared<FJsonValueObject>(Label));
}

// --------------------- main export ---------------------

bool ULaneLineExporter::ExportCosmosLaneLines(UWorld* World, const FString& SessionId, const FString& OutFilePath, FString& OutError)
{
  FString Uuid, StartTs, EndTs;
  if (!ParseSessionIdParts(SessionId, Uuid, StartTs, EndTs))
  {
    OutError = FString::Printf(TEXT("SessionId '%s' is not 'uuid_start_end'"), *SessionId);
    return false;
  }

  // Get all road splines from the world
  TArray<AActor*> RoadSplines;
  UGameplayStatics::GetAllActorsOfClass(World, ARoadSpline::StaticClass(), RoadSplines);

  // Group splines by road ID for filtering
  TMap<int32, TArray<ARoadSpline*>> SplinesByRoadId;
  for (AActor* RoadSplineActor : RoadSplines)
  {
    ARoadSpline* spline = Cast<ARoadSpline>(RoadSplineActor);
    if (!spline) continue;

    if (!SplinesByRoadId.Contains(spline->RoadID))
      SplinesByRoadId.Add(spline->RoadID);
    SplinesByRoadId[spline->RoadID].Add(spline);
  }

  // Root JSON
  TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  TArray<TSharedPtr<FJsonValue>> LabelsArr;

  // Process each road's splines
  for (TPair<int32, TArray<ARoadSpline*>> splines_pair : SplinesByRoadId)
  {
    TArray<ARoadSpline*> splines = splines_pair.Value;

    for (ARoadSpline* spline : splines)
    {
      if (!spline || !spline->SplineComponent) continue;

      // Only export driving lane lines
      if (spline->BoundaryType != ERoadSplineBoundaryType::Driving) continue;

      // Logic from CosmosControlSensor to determine if this lane line should be rendered
      bool should_render = false;

      // Find adjacent lane to determine if we should render this boundary
      TArray<ARoadSpline*> found_splines = splines_pair.Value.FilterByPredicate([spline](ARoadSpline* in_spline) {
        return in_spline->LaneID == spline->LaneID +
          (spline->OrientationType == ERoadSplineOrientationType::Left ?
            (spline->LaneID == 1 ? -2 : -1) : (spline->LaneID == -1 ? 2 : 1));
      });

      if (spline->bIsJunction)
      {
        // In junctions, don't render driving lane boundaries
        should_render = false;
      }
      else if (spline->OrientationType == ERoadSplineOrientationType::Left)
      {
        for (ARoadSpline* target_spline : found_splines)
        {
          if (target_spline->BoundaryType == ERoadSplineBoundaryType::Driving)
          {
            // Render lane lines between driving lanes on the same side of the road
            should_render = spline->LaneID > 0 && spline->LaneID * target_spline->LaneID > 0;
          }
        }
      }
      else if (spline->OrientationType == ERoadSplineOrientationType::Right)
      {
        for (ARoadSpline* target_spline : found_splines)
        {
          if (target_spline->BoundaryType == ERoadSplineBoundaryType::Driving)
          {
            // Only render for lanes on the negative side (right side in UE4 coords)
            should_render = spline->LaneID < 0;
          }
        }
      }

      if (!should_render) continue;

      // Extract spline points in meters (CARLA uses cm internally)
      TArray<FVector> PolyMeters;
      int32 NumPoints = spline->SplineComponent->GetNumberOfSplinePoints();

      for (int32 i = 0; i < NumPoints; ++i)
      {
        FVector WorldPos = spline->SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
        // Convert from UE4 cm to meters
        PolyMeters.Emplace(WorldPos.X / 100.0f, WorldPos.Y / 100.0f, WorldPos.Z / 100.0f);
      }

      if (PolyMeters.Num() > 0)
      {
        AppendLaneLineLabel(LabelsArr, StartTs, PolyMeters);
      }
    }
  }

  Root->SetArrayField(TEXT("labels"), LabelsArr);

  // Write single file
  FString OutText;
  TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutText);
  if (!FJsonSerializer::Serialize(Root, Writer))
  {
    OutError = TEXT("JSON serialization failed");
    return false;
  }

  // Construct full path with subdirectory and filename
  FString FullPath = OutFilePath + TEXT("3d_lanelines/") + SessionId + TEXT(".lanelines.json");
  const FString Dir = FPaths::GetPath(FullPath);
  IFileManager::Get().MakeDirectory(*Dir, /*Tree=*/true);

  // UE4 encoding option
  if (!FFileHelper::SaveStringToFile(OutText, *FullPath, FFileHelper::EEncodingOptions::ForceUTF8))
  {
    OutError = FString::Printf(TEXT("Failed to write: %s"), *FullPath);
    return false;
  }

  return true;
}

