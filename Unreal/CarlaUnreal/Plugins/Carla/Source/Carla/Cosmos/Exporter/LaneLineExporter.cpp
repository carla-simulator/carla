// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// LaneLineExporter.cpp
//
// JSON writer field sequence (byte-for-byte contract, from the ue4-dev
// LaneLineExporter::AppendLaneLineLabel / ExportCosmosLaneLines):
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
// <OutFilePath>3d_lanelines/<SessionId>.lanelines.json.
//
// Re-implementation note: the ue4 exporter walked ARoadSpline boundary points.
// ARoadSpline is intentionally not ported; here the painted lane-line polyline
// is reconstructed from carla::road::Map by sampling driving-lane waypoints
// along s and offsetting the lane center outward by half the lane width along
// the lane right vector (the lane's painted outer border). The emitted JSON
// fields/units are unchanged.

#include "LaneLineExporter.h"
#include "Carla.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Game/CarlaStatics.h"

// libcarla
#include <util/disable-ue4-macros.h>
#include <carla/road/Map.h>
#include <carla/road/Lane.h>
#include <carla/road/element/Waypoint.h>
#include <carla/road/element/RoadInfoMarkRecord.h>
#include <carla/geom/Transform.h>
#include <carla/geom/Vector3D.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include <util/ue-header-guard-end.h>

#include <set>

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

  // Access the road map from the CARLA game mode (same as the sibling
  // exporters). ue4 obtained the road network from the world via ARoadSpline
  // actors; here the road::Map is the source of truth.
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

  // Sampling step along s, in meters (road::Map is metric).
  const double SampleStep = 1.0;
  const double HalfLaneWidthEpsilon = 1e-3;

  // Collect the set of road IDs that carry driving lanes. The const Map does
  // not expose its road map publicly, so derive the road set from the
  // per-road-entry driving waypoints.
  std::set<carla::road::RoadId> RoadIds;
  for (const auto& Waypoint :
       CarlaMap->GenerateWaypointsOnRoadEntries(carla::road::Lane::LaneType::Driving))
  {
    RoadIds.insert(Waypoint.road_id);
  }

  for (const carla::road::RoadId RoadId : RoadIds)
  {
    // One waypoint per driving lane at the lane entry of this road.
    const std::vector<carla::road::element::Waypoint> LaneEntries =
        CarlaMap->GenerateWaypointsInRoad(RoadId, carla::road::Lane::LaneType::Driving);

    for (const carla::road::element::Waypoint& Entry : LaneEntries)
    {
      const carla::road::Lane& Lane = CarlaMap->GetLane(Entry);
      const double LaneStart = Lane.GetDistance();
      const double LaneEnd = LaneStart + Lane.GetLength();

      // Walk s and offset the lane center outward by half the lane width along
      // the lane right vector to reconstruct the painted outer-border polyline.
      // Reference geometry walk: MeshFactory.cpp (s_start..s_end, sample at a
      // fixed resolution, query the lane mark record at each s).
      TArray<FVector> PolyMeters;
      double S = LaneStart;
      bool bMore = true;
      while (bMore)
      {
        if (S >= LaneEnd)
        {
          S = LaneEnd;
          bMore = false;
        }

        carla::road::element::Waypoint Waypoint = Entry;
        Waypoint.s = S;

        // Inner/outer painted-mark records at this s. A mark record is required
        // for the border to be a painted lane line (ue4 only rendered painted
        // driving-lane boundaries).
        const auto MarkRecords = CarlaMap->GetMarkRecord(Waypoint);
        const carla::road::element::RoadInfoMarkRecord* OuterMark = MarkRecords.second;
        if (OuterMark != nullptr)
        {
          const carla::geom::Transform Transform = CarlaMap->ComputeTransform(Waypoint);
          const double HalfWidth = CarlaMap->GetLaneWidth(Waypoint) * 0.5;
          if (HalfWidth > HalfLaneWidthEpsilon)
          {
            const carla::geom::Vector3D Right = Transform.GetRightVector();
            const carla::geom::Location& Center = Transform.location;
            // Right lanes (positive direction) have their outer border on the
            // +right side; left lanes on the -right side.
            const double Sign = Lane.IsPositiveDirection() ? 1.0 : -1.0;
            const FVector Point(
                static_cast<float>(Center.x + Sign * HalfWidth * Right.x),
                static_cast<float>(Center.y + Sign * HalfWidth * Right.y),
                static_cast<float>(Center.z + Sign * HalfWidth * Right.z));
            PolyMeters.Add(Point);
          }
        }

        S += SampleStep;
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
