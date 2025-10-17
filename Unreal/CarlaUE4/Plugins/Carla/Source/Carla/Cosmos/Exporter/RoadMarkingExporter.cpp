// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// RoadMarkingExporter.cpp
#include "Carla.h"
#include "RoadMarkingExporter.h"
#include "Carla/Game/CarlaGameModeBase.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

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

  // Get the CARLA game mode to access road stencils
  ACarlaGameModeBase* CarlaGameMode = Cast<ACarlaGameModeBase>(World->GetAuthGameMode());
  if (!CarlaGameMode)
  {
    OutError = TEXT("Could not get CarlaGameModeBase");
    return false;
  }

  // Root JSON
  TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  TArray<TSharedPtr<FJsonValue>> LabelsArr;

  // Get road stencils from the CARLA map (same logic as CosmosControlSensor)
  const auto& RoadStencils = CarlaGameMode->GetMap()->GetStencils();

  for (const auto& StencilPair : RoadStencils)
  {
    const auto& Stencil = StencilPair.second;
    if (!Stencil)
    {
      continue;
    }

    // Calculate stencil vertices (same logic as CosmosControlSensor)
    const FTransform Transform = Stencil->GetTransform();
    const float StencilWidth = Stencil->GetWidth() * 100.0; // Convert to cm
    const float StencilLength = Stencil->GetLength() * 100.0; // Convert to cm
    FQuat StencilOrientation = Transform.GetRotation();

    // Create the 4 corner vertices of the rectangular stencil
    TArray<FVector> MeshVertices = {
      Transform.GetLocation() + StencilOrientation.RotateVector(FVector(-StencilLength/2, -StencilWidth/2, 0)),
      Transform.GetLocation() + StencilOrientation.RotateVector(FVector(StencilLength/2, -StencilWidth/2, 0)),
      Transform.GetLocation() + StencilOrientation.RotateVector(FVector(StencilLength/2, StencilWidth/2, 0)),
      Transform.GetLocation() + StencilOrientation.RotateVector(FVector(-StencilLength/2, StencilWidth/2, 0))
    };

    // Convert vertices from UE4 cm to meters
    TArray<FVector> SurfaceVerticesMeters;
    SurfaceVerticesMeters.Reserve(4);
    for (const FVector& Vertex : MeshVertices)
    {
      SurfaceVerticesMeters.Add(Vertex / 100.0f);
    }

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

