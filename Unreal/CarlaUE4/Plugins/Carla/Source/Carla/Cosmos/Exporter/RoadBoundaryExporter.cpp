// RoadBoundaryExporter.cpp
#include "RoadBoundaryExporter.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Traffic/RoadSpline.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/Crc.h"
#include "EngineUtils.h"

// libcarla
#include <carla/geom/Location.h>

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

FString URoadBoundaryExporter::MakeStableLabelId(const FString& Uuid, const TArray<FVector>& Vertices)
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

TArray<ARoadSpline*> URoadBoundaryExporter::GetRoadBoundarySplines(UWorld* World)
{
  TArray<ARoadSpline*> RoadBoundarySplines;

  if (!World) return RoadBoundarySplines;

  // Iterate through all ARoadSpline actors in the world
  for (TActorIterator<ARoadSpline> It(World); It; ++It)
  {
    ARoadSpline* Spline = *It;
    if (!Spline || !Spline->SplineComponent) continue;

    // Filter for road boundary types (Shoulder, Sidewalk, Border, Median)
    // Based on CosmosControlSensor logic that uses RoadBoundaries color for non-Driving types
    if (Spline->BoundaryType == ERoadSplineBoundaryType::Shoulder ||
        Spline->BoundaryType == ERoadSplineBoundaryType::Sidewalk ||
        Spline->BoundaryType == ERoadSplineBoundaryType::Border ||
        Spline->BoundaryType == ERoadSplineBoundaryType::Median)
    {
      RoadBoundarySplines.Add(Spline);
    }
  }

  return RoadBoundarySplines;
}

void URoadBoundaryExporter::AppendRoadBoundaryLabel(
    TArray<TSharedPtr<FJsonValue>>& LabelsArr,
    const FString& Uuid,
    const FString& StartTs,
    const TArray<FVector>& PolylineMeters,
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

    // polyline3d.vertices (road boundaries use polyline3d, not surface)
    {
      TSharedRef<FJsonObject> Polyline3D = MakeShared<FJsonObject>();
      TArray<TSharedPtr<FJsonValue>> VertArray;
      VertArray.Reserve(PolylineMeters.Num());

      for (const FVector& V : PolylineMeters)
      {
        TArray<TSharedPtr<FJsonValue>> Triple;
        // Convert from Unreal units (cm) to meters
        Triple.Add(MakeShared<FJsonValueNumber>(V.X / 100.0));
        Triple.Add(MakeShared<FJsonValueNumber>(V.Y / 100.0));
        Triple.Add(MakeShared<FJsonValueNumber>(V.Z / 100.0));
        VertArray.Add(MakeShared<FJsonValueArray>(Triple));
      }
      Polyline3D->SetArrayField(TEXT("vertices"), VertArray);
      Shape3D->SetObjectField(TEXT("polyline3d"), Polyline3D);
    }

    LabelData->SetObjectField(TEXT("shape3d"), Shape3D);
  }
  Label->SetObjectField(TEXT("labelData"), LabelData);

  // Add minimal timestamp for compatibility
  Label->SetStringField(TEXT("timestampMicroseconds"), StartTs);

  LabelsArr.Add(MakeShared<FJsonValueObject>(Label));
}

// --------------------- main export ---------------------

bool URoadBoundaryExporter::ExportCosmosRoadBoundaries(UWorld* World, const FString& SessionId, const FString& OutFilePath, FString& OutError)
{
  FString SensorName           = TEXT("lidar_gt_top_p128");
  FString LabelClassNamespace  = TEXT("minimap");
  FString LabelClassIdentifier = TEXT("road_boundaries:autolabels");
  FString LabelClassVersion    = TEXT("v0");
  FString CoordinateFrame      = TEXT("rig");
  FString LabelNameEnum        = TEXT("road_boundary");

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

  // Get all road boundary splines
  TArray<ARoadSpline*> RoadBoundarySplines = GetRoadBoundarySplines(World);

  if (RoadBoundarySplines.Num() == 0)
  {
    OutError = TEXT("No road boundary splines found in the world");
    return false;
  }

  // Root JSON
  TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  TArray<TSharedPtr<FJsonValue>> LabelsArr;
  LabelsArr.Reserve(RoadBoundarySplines.Num());

  for (ARoadSpline* Spline : RoadBoundarySplines)
  {
    if (!Spline || !Spline->SplineComponent) continue;

    int32 NumPoints = Spline->SplineComponent->GetNumberOfSplinePoints();
    if (NumPoints < 2) continue; // Need at least 2 points for a line

    TArray<FVector> PolylineMeters;
    PolylineMeters.Reserve(NumPoints);

    // Extract spline points
    for (int32 i = 0; i < NumPoints; ++i)
    {
      FVector Point = Spline->SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
      PolylineMeters.Add(Point);
    }

    // Append this road boundary as a label
    AppendRoadBoundaryLabel(
      LabelsArr, Uuid, StartTs, PolylineMeters,
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

  UE_LOG(LogCarla, Log, TEXT("Exported %d road boundaries to %s"), LabelsArr.Num(), *OutFilePath);
  return true;
}