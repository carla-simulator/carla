// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// CrosswalksExporter.cpp
#include "Carla.h"
#include "CrosswalksExporter.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Game/CarlaStatics.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

// libcarla
#include <compiler/disable-ue4-macros.h>
#include <carla/road/Map.h>
#include <carla/geom/Location.h>
#include <carla/road/element/RoadInfoCrosswalk.h>
#include <compiler/enable-ue4-macros.h>
// --------------------- helpers ---------------------

bool UCrosswalksExporter::ParseSessionIdParts(const FString& In, FString& OutUuid, FString& OutStart, FString& OutEnd)
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

FString UCrosswalksExporter::MakeStableLabelId(const FString& Uuid, const TArray<FVector>& Vertices)
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

void UCrosswalksExporter::AppendCrosswalkLabel(
    TArray<TSharedPtr<FJsonValue>>& LabelsArr,
    const FString& Uuid,
    const FString& StartTs,
    const TArray<FVector>& PolyMeters,
    const FString& SensorName,
    const FString& LabelClassNamespace,
    const FString& LabelClassIdentifier,
    const FString& LabelClassVersion,
    const FString& CoordinateFrame,
    const FString& LabelNameEnum,
    const FString& TypeText,
    bool bIsImplicit,
    const FString& OptionalAssetRefUri,
    const FString& OptionalFeatureId,
    const FString& OptionalFeatureVer,
    const FString& OptionalClipVerId)
{

  TSharedRef<FJsonObject> Label = MakeShared<FJsonObject>();
  Label->SetStringField(TEXT("labelFamily"), TEXT("SHAPE3D"));

  // assetRef
  {
    TSharedRef<FJsonObject> AssetRef = MakeShared<FJsonObject>();
    AssetRef->SetStringField(TEXT("sessionId"), Uuid);
    AssetRef->SetStringField(TEXT("sensorName"), SensorName);
    TSharedRef<FJsonObject> FramesObj = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> FramesVals; FramesVals.Add(MakeShared<FJsonValueNumber>(0));
    FramesObj->SetArrayField(TEXT("frames"), FramesVals);
    AssetRef->SetObjectField(TEXT("frames"), FramesObj);
    Label->SetObjectField(TEXT("assetRef"), AssetRef);
  }

  // labelClassKey
  {
    TSharedRef<FJsonObject> ClassKey = MakeShared<FJsonObject>();
    ClassKey->SetStringField(TEXT("labelClassNamespace"), LabelClassNamespace);
    ClassKey->SetStringField(TEXT("labelClassIdentifier"), LabelClassIdentifier);
    ClassKey->SetStringField(TEXT("labelClassVersion"),  LabelClassVersion);
    Label->SetObjectField(TEXT("labelClassKey"), ClassKey);
  }

  Label->SetStringField(TEXT("labelId"), MakeStableLabelId(Uuid, PolyMeters));

  // labelData.shape3d
  TSharedRef<FJsonObject> LabelData = MakeShared<FJsonObject>();
  {
    TSharedRef<FJsonObject> Shape3D = MakeShared<FJsonObject>();

    // attributes
    {
      TArray<TSharedPtr<FJsonValue>> Attrs;

      auto AddAttrText = [&](const TCHAR* Name, const FString& Text)
      {
        if (!Text.IsEmpty())
        {
          TSharedRef<FJsonObject> A = MakeShared<FJsonObject>();
          A->SetStringField(TEXT("name"), Name);
          A->SetStringField(TEXT("text"), Text);
          Attrs.Add(MakeShared<FJsonValueObject>(A));
        }
      };

      AddAttrText(TEXT("coordinate_frame"), CoordinateFrame);
      AddAttrText(TEXT("timestamp"),        StartTs);
      AddAttrText(TEXT("asset_ref"),        OptionalAssetRefUri);
      AddAttrText(TEXT("feature_id"),       OptionalFeatureId);
      AddAttrText(TEXT("feature_version"),  OptionalFeatureVer);
      AddAttrText(TEXT("clip_version_id"),  OptionalClipVerId);

      // label_name (enum)
      {
        TSharedRef<FJsonObject> A = MakeShared<FJsonObject>();
        A->SetStringField(TEXT("name"), TEXT("label_name"));
        A->SetStringField(TEXT("enum"), LabelNameEnum);
        Attrs.Add(MakeShared<FJsonValueObject>(A));
      }
      // type (text)
      AddAttrText(TEXT("type"), TypeText);

      // is_implicit (flag)
      {
        TSharedRef<FJsonObject> A = MakeShared<FJsonObject>();
        A->SetStringField(TEXT("name"), TEXT("is_implicit"));
        A->SetBoolField   (TEXT("flag"), bIsImplicit);
        Attrs.Add(MakeShared<FJsonValueObject>(A));
      }

      Shape3D->SetArrayField(TEXT("attributes"), Attrs);
    }

    Shape3D->SetStringField(TEXT("unit"), TEXT("METRIC"));

    // surface.vertices
    {
      TSharedRef<FJsonObject> Surface = MakeShared<FJsonObject>();
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
      Surface->SetArrayField(TEXT("vertices"), VertArray);
      Shape3D->SetObjectField(TEXT("surface"), Surface);
    }

    LabelData->SetObjectField(TEXT("shape3d"), Shape3D);
  }
  Label->SetObjectField(TEXT("labelData"), LabelData);

  Label->SetStringField(TEXT("timestampMicroseconds"), StartTs);

  LabelsArr.Add(MakeShared<FJsonValueObject>(Label));
}

// --------------------- main export ---------------------

bool UCrosswalksExporter::ExportCosmosCrosswalk(UWorld* World, const FString& SessionId, const FString& OutFilePath, FString& OutError)
{
  FString SensorName           = TEXT("lidar_gt_top_p128");
  FString LabelClassNamespace  = TEXT("minimap");
  FString LabelClassIdentifier = TEXT("crosswalks:autolabels");
  FString LabelClassVersion    = TEXT("v0");
  FString CoordinateFrame      = TEXT("rig");
  FString LabelNameEnum        = TEXT("crosswalk");
  FString TypeText             = TEXT("PEDESTRIAN");
  bool    bIsImplicit          = false;

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

  // Get all crosswalks (no processing; world meters)
  const auto Crosswalks = CarlaMap->GetAllCrosswalksInfo();
  // Root JSON
  TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  TArray<TSharedPtr<FJsonValue>> LabelsArr;
  LabelsArr.Reserve(static_cast<int32>(Crosswalks.size()));

  for (const auto& kv : Crosswalks)
  {
    const std::vector<carla::geom::Location>& Locs = kv.second;
    if (Locs.empty())
      continue;

    TArray<FVector> PolyMeters;
    PolyMeters.Reserve(static_cast<int32>(Locs.size()));
    for (const carla::geom::Location& L : Locs)
    {
      PolyMeters.Emplace(static_cast<float>(L.x),
                         static_cast<float>(L.y),
                         static_cast<float>(L.z));
    }

    AppendCrosswalkLabel(
      LabelsArr, Uuid, StartTs, PolyMeters,
      /* SensorName           */ SensorName,
      /* LabelClassNamespace  */ LabelClassNamespace,
      /* LabelClassIdentifier */ LabelClassIdentifier,
      /* LabelClassVersion    */ LabelClassVersion,
      /* CoordinateFrame      */ CoordinateFrame,
      /* LabelNameEnum        */ LabelNameEnum,
      /* TypeText             */ TypeText,
      /* bIsImplicit          */ bIsImplicit,
      /* OptionalAssetRefUri  */ AssetRefUri,
      /* OptionalFeatureId    */ FeatureId,
      /* OptionalFeatureVer   */ FeatureVersion,
      /* OptionalClipVerId    */ ClipVersionId
    );
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
  FString FullPath = OutFilePath + TEXT("3d_crosswalks/") + SessionId + TEXT(".crosswalks.json");
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
