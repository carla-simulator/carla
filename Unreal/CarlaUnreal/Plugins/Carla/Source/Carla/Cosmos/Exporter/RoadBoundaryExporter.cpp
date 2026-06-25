// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// RoadBoundaryExporter.cpp
//
// JSON writer field sequence (byte-for-byte contract, from the ue4-dev
// RoadBoundaryExporter::AppendRoadBoundaryLabel / ExportCosmosRoadBoundaries):
//
//   {
//     "labels": [
//       {
//         "labelData": {
//           "shape3d": {
//             "unit": "METRIC",
//             "polyline3d": {
//               "vertices": [ [x,y,z], [x,y,z], ... ]
//             }
//           }
//         },
//         "timestampMicroseconds": "<StartTs>"
//       },
//       ...
//     ]
//   }
//
// Per-label order: labelData -> shape3d -> unit ("METRIC") -> polyline3d ->
// vertices (one triple per sampled point, meters), then timestampMicroseconds
// (string). Output path:
// <OutFilePath>3d_road_boundaries/<SessionId>.road_boundaries.json.
//
// Re-implementation note: the ue4 exporter walked ARoadSpline boundary points
// for Shoulder/Sidewalk/Median splines. ARoadSpline is intentionally not
// ported; here the boundary polyline is reconstructed from carla::road::Map by
// sampling Shoulder/Sidewalk/Median lane waypoints along s and offsetting the
// lane center outward by half the lane width along the lane right vector (the
// lane's outer edge). The emitted JSON fields/units are unchanged.

#include "RoadBoundaryExporter.h"
#include "Carla.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Cosmos/CosmosRoadGeometry.h"

// libcarla
#include <util/disable-ue4-macros.h>
#include <carla/road/Map.h>
#include <carla/road/Lane.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include <util/ue-header-guard-end.h>

// --------------------- helpers ---------------------

bool URoadBoundaryExporter::ParseSessionIdParts(const FString& In, FString& OutUuid, FString& OutStart, FString& OutEnd)
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

void URoadBoundaryExporter::AppendRoadBoundaryLabel(
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

bool URoadBoundaryExporter::ExportCosmosRoadBoundaries(UWorld* World, const FString& SessionId, const FString& OutFilePath, FString& OutError)
{
  FString Uuid, StartTs, EndTs;
  if (!ParseSessionIdParts(SessionId, Uuid, StartTs, EndTs))
  {
    OutError = FString::Printf(TEXT("SessionId '%s' is not 'uuid_start_end'"), *SessionId);
    return false;
  }

  // Access the road map from the CARLA game mode (same as the sibling
  // exporters).
  ACarlaGameModeBase *GameMode = UCarlaStatics::GetGameMode(World);
  if (!GameMode)
  {
    OutError = TEXT("No GameMode found");
    return false;
  }

  const std::optional<carla::road::Map>& CarlaMap = GameMode->GetMap();
  if (!CarlaMap)
  {
    OutError = TEXT("No CarlaMap found");
    return false;
  }

  // Root JSON
  TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  TArray<TSharedPtr<FJsonValue>> LabelsArr;

  // Reconstruct the boundary outer-edge polylines from road::Map (Shoulder /
  // Sidewalk / Median lanes; no painted-mark requirement). Shared with
  // ACosmosControlSensor so the exported geometry and the in-engine overlay
  // stay identical. Vertices are in meters.
  TArray<TArray<FVector>> Polylines;
  CosmosRoadGeometry::BuildOuterBorderPolylines(
      *CarlaMap,
      CosmosRoadGeometry::BoundaryLaneTypes(),
      /*bRequirePaintedMark=*/false,
      Polylines);

  for (const TArray<FVector>& PolyMeters : Polylines)
  {
    AppendRoadBoundaryLabel(LabelsArr, StartTs, PolyMeters);
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
  FString FullPath = OutFilePath + TEXT("3d_road_boundaries/") + SessionId + TEXT(".road_boundaries.json");
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
