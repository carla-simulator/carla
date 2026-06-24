// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// RoadMarkingExporter.cpp
//
// JSON writer field sequence (byte-for-byte contract, from the ue4-dev
// RoadMarkingExporter::AppendRoadMarkingLabel / ExportCosmosRoadMarkings):
//
//   {
//     "labels": [
//       {
//         "labelData": {
//           "shape3d": {
//             "unit": "METRIC",
//             "surface": {
//               "vertices": [ [x,y,z], [x,y,z], [x,y,z], [x,y,z] ]
//             }
//           }
//         },
//         "timestampMicroseconds": "<StartTs>"
//       },
//       ...
//     ]
//   }
//
// Per-label order: labelData -> shape3d -> unit ("METRIC") -> surface ->
// vertices (4 corner triples, meters), then timestampMicroseconds (string).
// Output path: <OutFilePath>3d_road_markings/<SessionId>.road_markings.json.
// Quad corner order matches ue4 (CCW from -L/-W): (-L/2,-W/2), (+L/2,-W/2),
// (+L/2,+W/2), (-L/2,+W/2) about the stencil center.

#include "Carla.h"
#include "RoadMarkingExporter.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Game/CarlaStatics.h"

// libcarla
#include <util/disable-ue4-macros.h>
#include <carla/road/Map.h>
#include <carla/road/Stencil.h>
#include <carla/geom/Transform.h>
#include <carla/geom/Vector3D.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include <util/ue-header-guard-end.h>

// --------------------- helpers ---------------------

bool URoadMarkingExporter::ParseSessionIdParts(const FString& In, FString& OutUuid, FString& OutStart, FString& OutEnd)
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

void URoadMarkingExporter::AppendRoadMarkingLabel(
    TArray<TSharedPtr<FJsonValue>>& LabelsArr,
    const FString& StartTs,
    const TArray<FVector>& SurfaceVertices)
{
  TSharedRef<FJsonObject> Label = MakeShared<FJsonObject>();

  // labelData.shape3d.surface
  TSharedRef<FJsonObject> LabelData = MakeShared<FJsonObject>();
  {
    TSharedRef<FJsonObject> Shape3D = MakeShared<FJsonObject>();
    Shape3D->SetStringField(TEXT("unit"), TEXT("METRIC"));

    TSharedRef<FJsonObject> Surface = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> VertArray;
    VertArray.Reserve(4);

    for (const FVector& V : SurfaceVertices)
    {
      TArray<TSharedPtr<FJsonValue>> Triple;
      Triple.Add(MakeShared<FJsonValueNumber>(V.X));
      Triple.Add(MakeShared<FJsonValueNumber>(V.Y));
      Triple.Add(MakeShared<FJsonValueNumber>(V.Z));
      VertArray.Add(MakeShared<FJsonValueArray>(Triple));
    }

    Surface->SetArrayField(TEXT("vertices"), VertArray);
    Shape3D->SetObjectField(TEXT("surface"), Surface);
    LabelData->SetObjectField(TEXT("shape3d"), Shape3D);
  }
  Label->SetObjectField(TEXT("labelData"), LabelData);
  Label->SetStringField(TEXT("timestampMicroseconds"), StartTs);

  LabelsArr.Add(MakeShared<FJsonValueObject>(Label));
}

// --------------------- main export ---------------------

bool URoadMarkingExporter::ExportCosmosRoadMarkings(UWorld* World, const FString& SessionId, const FString& OutFilePath, FString& OutError)
{
  FString Uuid, StartTs, EndTs;
  if (!ParseSessionIdParts(SessionId, Uuid, StartTs, EndTs))
  {
    OutError = FString::Printf(TEXT("SessionId '%s' is not 'uuid_start_end'"), *SessionId);
    return false;
  }

  // Access the road map from the CARLA game mode (same as the sibling
  // exporters). ue4 used ACarlaGameModeBase::GetMap() (returning an
  // FRoadDataNetwork-like object); on ue5 GetMap() returns the LibCarla
  // std::optional<carla::road::Map>.
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

  // Get road stencils from the CARLA map (same logic as the ue4 exporter,
  // which read CarlaGameMode->GetMap()->GetStencils()).
  const auto& RoadStencils = CarlaMap->GetStencils();

  for (const auto& StencilPair : RoadStencils)
  {
    const auto& Stencil = StencilPair.second;
    if (!Stencil)
    {
      continue;
    }

    // Calculate stencil vertices (same logic as the ue4 exporter). The
    // road::Map already works in meters, so no cm conversion is needed: the
    // ue4 exporter multiplied width/length by 100 (to UE cm) and divided the
    // final vertices by 100, yielding the same meters emitted here directly.
    const carla::geom::Transform& Transform = Stencil->GetTransform();
    const float StencilWidth = static_cast<float>(Stencil->GetWidth());
    const float StencilLength = static_cast<float>(Stencil->GetLength());

    // Create the 4 corner vertices of the rectangular stencil. The ue4
    // exporter rotated each local corner by the stencil orientation and added
    // the stencil location; Transform::TransformPoint does the same (rotation
    // then translation) using the stencil transform.
    auto Corner = [&](float LX, float LY) -> FVector
    {
      carla::geom::Vector3D Local(LX, LY, 0.0f);
      Transform.TransformPoint(Local);
      return FVector(Local.x, Local.y, Local.z);
    };

    TArray<FVector> SurfaceVerticesMeters;
    SurfaceVerticesMeters.Reserve(4);
    SurfaceVerticesMeters.Add(Corner(-StencilLength/2, -StencilWidth/2));
    SurfaceVerticesMeters.Add(Corner( StencilLength/2, -StencilWidth/2));
    SurfaceVerticesMeters.Add(Corner( StencilLength/2,  StencilWidth/2));
    SurfaceVerticesMeters.Add(Corner(-StencilLength/2,  StencilWidth/2));

    AppendRoadMarkingLabel(LabelsArr, StartTs, SurfaceVerticesMeters);
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
  FString FullPath = OutFilePath + TEXT("3d_road_markings/") + SessionId + TEXT(".road_markings.json");
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
