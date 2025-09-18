// RoadMarkingExporter.cpp
#include "RoadMarkingExporter.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Game/CarlaStatics.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/Crc.h"

// libcarla
#include <carla/geom/Location.h>
#include <carla/road/Stencil.h>

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

FString URoadMarkingExporter::MakeStableLabelId(const FString& Uuid, const TArray<FVector>& Vertices)
{
  uint32 Crc = 0;
  if (Vertices.Num() > 0)
  {
    Crc = FCrc::MemCrc32(Vertices.GetData(), Vertices.Num() * sizeof(FVector));
  }
  FString Hex = FString::Printf(TEXT("%08x%08x%08x%08x"),
                                Crc, Crc ^ 0xA5A5A5A5u, Crc * 2654435761u, ~Crc);
  return FString::Printf(TEXT("mads:000:%s:%06u"), *Hex.Mid(0, 32), 0u);
}

void URoadMarkingExporter::AppendRoadMarkingLabel(
    TArray<TSharedPtr<FJsonValue>>& LabelsArr,
    const FString& Uuid,
    const FString& StartTs,
    const TArray<FVector>& PolygonMeters,
    const FString& SensorName,
    const FString& LabelClassNamespace,
    const FString& LabelClassIdentifier,
    const FString& LabelClassVersion,
    const FString& CoordinateFrame,
    const FString& LabelNameEnum,
    const FString& OptionalAssetRefUri,
    const FString& OptionalFeatureId,
    const FString& OptionalFeatureVer,
    const FString& OptionalClipVerId)
{
  TSharedRef<FJsonObject> Label = MakeShared<FJsonObject>();

  // labelData.shape3d (minimal structure for RDS-HQ)
  TSharedRef<FJsonObject> LabelData = MakeShared<FJsonObject>();
  {
    TSharedRef<FJsonObject> Shape3D = MakeShared<FJsonObject>();

    Shape3D->SetStringField(TEXT("unit"), TEXT("METRIC"));

    // surface.vertices (road markings use surface/polygon format)
    {
      TSharedRef<FJsonObject> Surface = MakeShared<FJsonObject>();
      TArray<TSharedPtr<FJsonValue>> VertArray;
      VertArray.Reserve(PolygonMeters.Num());

      for (const FVector& V : PolygonMeters)
      {
        TArray<TSharedPtr<FJsonValue>> Triple;
        // Output in meters (libcarla stencils are already in meters)
        Triple.Add(MakeShared<FJsonValueNumber>(V.X));
        Triple.Add(MakeShared<FJsonValueNumber>(V.Y));
        Triple.Add(MakeShared<FJsonValueNumber>(V.Z));
        VertArray.Add(MakeShared<FJsonValueArray>(Triple));
      }
      Surface->SetArrayField(TEXT("vertices"), VertArray);
      Shape3D->SetObjectField(TEXT("surface"), Surface);
    }

    LabelData->SetObjectField(TEXT("shape3d"), Shape3D);
  }
  Label->SetObjectField(TEXT("labelData"), LabelData);

  // Add minimal timestamp for compatibility
  Label->SetStringField(TEXT("timestampMicroseconds"), StartTs);

  LabelsArr.Add(MakeShared<FJsonValueObject>(Label));
}

// --------------------- main export ---------------------

bool URoadMarkingExporter::ExportCosmosRoadMarkings(UWorld* World, const FString& SessionId, const FString& OutFilePath, FString& OutError)
{
  FString SensorName           = TEXT("lidar_gt_top_p128");
  FString LabelClassNamespace  = TEXT("minimap");
  FString LabelClassIdentifier = TEXT("road_markings:autolabels");
  FString LabelClassVersion    = TEXT("v0");
  FString CoordinateFrame      = TEXT("rig");
  FString LabelNameEnum        = TEXT("road_marking");

  FString AssetRefUri;
  FString FeatureId;
  FString FeatureVersion;
  FString ClipVersionId;
  FString Uuid, StartTs, EndTs;

  if (!ParseSessionIdParts(SessionId, Uuid, StartTs, EndTs))
  {
    OutError = FString::Printf(TEXT("SessionId '%s' is not 'uuid_start_end'"), *SessionId);
    return false;
  }

  ACarlaGameModeBase *GameMode = UCarlaStatics::GetGameMode(World);
  if (!GameMode)
  {
    OutError = TEXT("No GameMode found");
    return false;
  }

  const boost::optional<carla::road::Map>& CarlaMap = GameMode->GetMap();
  if (!CarlaMap)
  {
    OutError = TEXT("No CarlaMap found");
    return false;
  }

  // Get all stencils (road markings)
  const auto& Stencils = CarlaMap->GetStencils();

  if (Stencils.empty())
  {
    OutError = TEXT("No road markings (stencils) found in the map");
    return false;
  }

  // Root JSON
  TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  TArray<TSharedPtr<FJsonValue>> LabelsArr;
  LabelsArr.Reserve(static_cast<int32>(Stencils.size()));

  for (const auto& StencilPair : Stencils)
  {
    const auto& Stencil = StencilPair.second;
    if (!Stencil)
      continue;

    // Extract points exactly like CosmosControlSensor does
    const FTransform Transform = Stencil->GetTransform();
    const float StencilWidth = Stencil->GetWidth() * 100.0;   // Convert to cm like CosmosControlSensor
    const float StencilLength = Stencil->GetLength() * 100.0; // Convert to cm like CosmosControlSensor
    FQuat StencilOrientation = Transform.GetRotation();

    // Create 4 corners exactly like CosmosControlSensor
    TArray<FVector> PolygonCentimeters;
    PolygonCentimeters.Reserve(4);

    PolygonCentimeters.Add(Transform.GetLocation() + StencilOrientation.RotateVector(FVector(-StencilLength/2, -StencilWidth/2, 0)));
    PolygonCentimeters.Add(Transform.GetLocation() + StencilOrientation.RotateVector(FVector(StencilLength/2, -StencilWidth/2, 0)));
    PolygonCentimeters.Add(Transform.GetLocation() + StencilOrientation.RotateVector(FVector(StencilLength/2, StencilWidth/2, 0)));
    PolygonCentimeters.Add(Transform.GetLocation() + StencilOrientation.RotateVector(FVector(-StencilLength/2, StencilWidth/2, 0)));

    // Convert from cm to meters for JSON export
    TArray<FVector> PolygonMeters;
    PolygonMeters.Reserve(4);
    for (const FVector& Point : PolygonCentimeters)
    {
      PolygonMeters.Add(Point / 100.0);
    }

    // Append this road marking as a label
    AppendRoadMarkingLabel(
      LabelsArr, Uuid, StartTs, PolygonMeters,
      /* SensorName           */ SensorName,
      /* LabelClassNamespace  */ LabelClassNamespace,
      /* LabelClassIdentifier */ LabelClassIdentifier,
      /* LabelClassVersion    */ LabelClassVersion,
      /* CoordinateFrame      */ CoordinateFrame,
      /* LabelNameEnum        */ LabelNameEnum,
      /* OptionalAssetRefUri  */ AssetRefUri,
      /* OptionalFeatureId    */ FeatureId,
      /* OptionalFeatureVer   */ FeatureVersion,
      /* OptionalClipVerId    */ ClipVersionId
    );
  }

  Root->SetArrayField(TEXT("labels"), LabelsArr);

  // Write to file
  FString OutText;
  TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutText);

  if (!FJsonSerializer::Serialize(Root, Writer))
  {
    OutError = TEXT("JSON serialization failed");
    return false;
  }

  const FString Dir = FPaths::GetPath(OutFilePath);
  IFileManager::Get().MakeDirectory(*Dir, /*Tree=*/true);

  // UE4 encoding option
  if (!FFileHelper::SaveStringToFile(OutText, *OutFilePath, FFileHelper::EEncodingOptions::ForceUTF8))
  {
    OutError = FString::Printf(TEXT("Failed to write file: %s"), *OutFilePath);
    return false;
  }

  UE_LOG(LogCarla, Log, TEXT("Exported %d road markings to %s"), LabelsArr.Num(), *OutFilePath);
  return true;
}