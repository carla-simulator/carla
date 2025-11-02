// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// TrafficSignsExporter.cpp
#include "Carla.h"
#include "TrafficSignsExporter.h"
#include "Carla/Game/Tagger.h"

#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/Crc.h"

// --------------------- helpers ---------------------

bool UTrafficSignsExporter::ParseSessionIdParts(const FString& In, FString& OutUuid, FString& OutStart, FString& OutEnd)
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

void UTrafficSignsExporter::BuildPolePolylineMeters(const UStaticMeshComponent* C, FVector& OutTopM, FVector& OutBaseM)
{
  // Get component bounds (same as CosmosControlSensor)
  FBoxSphereBounds Bounds = C->CalcBounds(C->GetComponentTransform());

  // Calculate pole height similar to CosmosControlSensor logic
  float HalfHeight = Bounds.BoxExtent.Z;
  float DistanceToRoad = C->GetComponentLocation().Z;
  FVector ComponentLocation = C->GetComponentLocation();

  // Top point: mesh center + half_height (same as CosmosControlSensor capsule center)
  FVector TopWS = ComponentLocation + FVector(0.0f, 0.0f, HalfHeight);

  // Base point: calculate based on CosmosControlSensor capsule logic
  // The capsule extends down by (half_height + distance_adjustment) from the top point
  float DistanceAdjustment = (DistanceToRoad > 250.0f ? 0.0f : DistanceToRoad);
  FVector BaseWS = TopWS - FVector(0.0f, 0.0f, HalfHeight + DistanceAdjustment);

  // Convert to meters
  OutTopM = TopWS / 100.0f;
  OutBaseM = BaseWS / 100.0f;
}

void UTrafficSignsExporter::BuildComponentCuboidMeters(const UStaticMeshComponent* C, TArray<FVector>& Out8VertsMeters)
{
  FVector Origin, Extent;
  C->GetLocalBounds(Origin, Extent);

  const FTransform& T = C->GetComponentTransform();

  // 8 corners of the bounding box in local space
  TArray<FVector> LocalCorners = {
    Origin + FVector(+Extent.X, +Extent.Y, +Extent.Z),
    Origin + FVector(-Extent.X, +Extent.Y, +Extent.Z),
    Origin + FVector(-Extent.X, -Extent.Y, +Extent.Z),
    Origin + FVector(+Extent.X, -Extent.Y, +Extent.Z),
    Origin + FVector(+Extent.X, +Extent.Y, -Extent.Z),
    Origin + FVector(-Extent.X, +Extent.Y, -Extent.Z),
    Origin + FVector(-Extent.X, -Extent.Y, -Extent.Z),
    Origin + FVector(+Extent.X, -Extent.Y, -Extent.Z)
  };

  Out8VertsMeters.Empty(8);
  for (const FVector& LocalCorner : LocalCorners)
  {
    FVector WorldCorner = T.TransformPosition(LocalCorner);
    Out8VertsMeters.Add(WorldCorner / 100.0f); // Convert cm to meters
  }
}

void UTrafficSignsExporter::AppendPoleLabel(
    TArray<TSharedPtr<FJsonValue>>& Labels,
    const FString& StartTs,
    const FVector& TopM,
    const FVector& BaseM)
{
  TSharedRef<FJsonObject> Label = MakeShared<FJsonObject>();

  // labelData.shape3d.polyline3d
  TSharedRef<FJsonObject> LabelData = MakeShared<FJsonObject>();
  {
    TSharedRef<FJsonObject> Shape3D = MakeShared<FJsonObject>();
    Shape3D->SetStringField(TEXT("unit"), TEXT("METRIC"));

    TSharedRef<FJsonObject> Polyline3D = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> VertArray;

    // Add top vertex
    TArray<TSharedPtr<FJsonValue>> TopTriple;
    TopTriple.Add(MakeShared<FJsonValueNumber>(TopM.X));
    TopTriple.Add(MakeShared<FJsonValueNumber>(TopM.Y));
    TopTriple.Add(MakeShared<FJsonValueNumber>(TopM.Z));
    VertArray.Add(MakeShared<FJsonValueArray>(TopTriple));

    // Add base vertex
    TArray<TSharedPtr<FJsonValue>> BaseTriple;
    BaseTriple.Add(MakeShared<FJsonValueNumber>(BaseM.X));
    BaseTriple.Add(MakeShared<FJsonValueNumber>(BaseM.Y));
    BaseTriple.Add(MakeShared<FJsonValueNumber>(BaseM.Z));
    VertArray.Add(MakeShared<FJsonValueArray>(BaseTriple));

    Polyline3D->SetArrayField(TEXT("vertices"), VertArray);
    Shape3D->SetObjectField(TEXT("polyline3d"), Polyline3D);
    LabelData->SetObjectField(TEXT("shape3d"), Shape3D);
  }
  Label->SetObjectField(TEXT("labelData"), LabelData);
  Label->SetStringField(TEXT("timestampMicroseconds"), StartTs);

  Labels.Add(MakeShared<FJsonValueObject>(Label));
}

void UTrafficSignsExporter::AppendTrafficSignLabel(
    TArray<TSharedPtr<FJsonValue>>& Labels,
    const FString& StartTs,
    const TArray<FVector>& V8Meters)
{
  TSharedRef<FJsonObject> Label = MakeShared<FJsonObject>();

  // labelData.shape3d.cuboid3d
  TSharedRef<FJsonObject> LabelData = MakeShared<FJsonObject>();
  {
    TSharedRef<FJsonObject> Shape3D = MakeShared<FJsonObject>();
    Shape3D->SetStringField(TEXT("unit"), TEXT("METRIC"));

    TSharedRef<FJsonObject> Cuboid3D = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> VertArray;
    VertArray.Reserve(8);

    for (const FVector& V : V8Meters)
    {
      TArray<TSharedPtr<FJsonValue>> Triple;
      Triple.Add(MakeShared<FJsonValueNumber>(V.X));
      Triple.Add(MakeShared<FJsonValueNumber>(V.Y));
      Triple.Add(MakeShared<FJsonValueNumber>(V.Z));
      VertArray.Add(MakeShared<FJsonValueArray>(Triple));
    }

    Cuboid3D->SetArrayField(TEXT("vertices"), VertArray);
    Shape3D->SetObjectField(TEXT("cuboid3d"), Cuboid3D);
    LabelData->SetObjectField(TEXT("shape3d"), Shape3D);
  }
  Label->SetObjectField(TEXT("labelData"), LabelData);
  Label->SetStringField(TEXT("timestampMicroseconds"), StartTs);

  Labels.Add(MakeShared<FJsonValueObject>(Label));
}

void UTrafficSignsExporter::AppendTrafficLightLabel(
    TArray<TSharedPtr<FJsonValue>>& Labels,
    const FString& StartTs,
    const TArray<FVector>& V8Meters)
{
  TSharedRef<FJsonObject> Label = MakeShared<FJsonObject>();

  // labelData.shape3d.cuboid3d (same as traffic signs)
  TSharedRef<FJsonObject> LabelData = MakeShared<FJsonObject>();
  {
    TSharedRef<FJsonObject> Shape3D = MakeShared<FJsonObject>();
    Shape3D->SetStringField(TEXT("unit"), TEXT("METRIC"));

    TSharedRef<FJsonObject> Cuboid3D = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> VertArray;
    VertArray.Reserve(8);

    for (const FVector& V : V8Meters)
    {
      TArray<TSharedPtr<FJsonValue>> Triple;
      Triple.Add(MakeShared<FJsonValueNumber>(V.X));
      Triple.Add(MakeShared<FJsonValueNumber>(V.Y));
      Triple.Add(MakeShared<FJsonValueNumber>(V.Z));
      VertArray.Add(MakeShared<FJsonValueArray>(Triple));
    }

    Cuboid3D->SetArrayField(TEXT("vertices"), VertArray);
    Shape3D->SetObjectField(TEXT("cuboid3d"), Cuboid3D);
    LabelData->SetObjectField(TEXT("shape3d"), Shape3D);
  }
  Label->SetObjectField(TEXT("labelData"), LabelData);
  Label->SetStringField(TEXT("timestampMicroseconds"), StartTs);

  Labels.Add(MakeShared<FJsonValueObject>(Label));
}

// --------------------- main export ---------------------

bool UTrafficSignsExporter::ExportCosmosTrafficSigns(UWorld* World, const FString& SessionId, const FString& OutFilePath, FString& OutError)
{
  FString Uuid, StartTs, EndTs;
  if (!ParseSessionIdParts(SessionId, Uuid, StartTs, EndTs))
  {
    OutError = FString::Printf(TEXT("SessionId '%s' is not 'uuid_start_end'"), *SessionId);
    return false;
  }

  // Three separate arrays for each type
  TArray<TSharedPtr<FJsonValue>> PolesLabels;
  TArray<TSharedPtr<FJsonValue>> TrafficSignsLabels;
  TArray<TSharedPtr<FJsonValue>> TrafficLightsLabels;

  // Scan all mesh components with tags (same approach as CosmosControlSensor)
  TArray<UObject*> AllMeshComponents;
  GetObjectsOfClass(UMeshComponent::StaticClass(), AllMeshComponents, true, EObjectFlags::RF_ClassDefaultObject, EInternalObjectFlags::AllFlags);

  UE_LOG(LogCarla, Warning, TEXT("TrafficSignsExporter: Found %d total mesh components"), AllMeshComponents.Num());

  int32 VisibleComponents = 0;
  int32 ComponentsWithOwner = 0;
  int32 FilteredComponents = 0;
  int32 StaticMeshComponents = 0;
  int32 PolesFound = 0;
  int32 TrafficSignsFound = 0;
  int32 TrafficLightsFound = 0;
  int32 TaggedComponents = 0;

  for (UObject* Obj : AllMeshComponents)
  {
    UMeshComponent* MeshComp = Cast<UMeshComponent>(Obj);
    if (!MeshComp) continue;

    if (!MeshComp->IsVisible()) continue;
    VisibleComponents++;

    if (!MeshComp->GetOwner()) continue;
    ComponentsWithOwner++;

    // Apply same filtering as CosmosControlSensor
    if (MeshComp->GetComponentLocation().Z > 10000.0f) continue;
    FilteredComponents++;

    // Cast to static mesh for our specific operations
    UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(MeshComp);
    if (!StaticMeshComp) continue;
    StaticMeshComponents++;

    const carla::rpc::CityObjectLabel Tag = ATagger::GetTagOfTaggedComponent(*MeshComp);

    if (Tag != carla::rpc::CityObjectLabel::None)
    {
      TaggedComponents++;
      // Log some tags we find to see what's available (only first few to avoid spam)
      if (TaggedComponents <= 5)
      {
        UE_LOG(LogCarla, Warning, TEXT("Tagged component %d: Tag=%d (%s), Owner=%s, Location=%s"),
               TaggedComponents, (int32)Tag,
               Tag == carla::rpc::CityObjectLabel::TrafficSigns ? TEXT("TrafficSigns") :
               Tag == carla::rpc::CityObjectLabel::TrafficLight ? TEXT("TrafficLight") :
               Tag == carla::rpc::CityObjectLabel::Poles ? TEXT("Poles") : TEXT("Other"),
               MeshComp->GetOwner() ? *MeshComp->GetOwner()->GetName() : TEXT("NoOwner"),
               *MeshComp->GetComponentLocation().ToString());
      }
    }

    if (Tag == carla::rpc::CityObjectLabel::TrafficSigns)
    {
      TrafficSignsFound++;
      UE_LOG(LogCarla, Warning, TEXT("Found traffic sign: %s at %s"),
             MeshComp->GetOwner() ? *MeshComp->GetOwner()->GetName() : TEXT("NoOwner"),
             *MeshComp->GetComponentLocation().ToString());

      // Build cuboid for traffic signs
      TArray<FVector> V8Meters;
      BuildComponentCuboidMeters(StaticMeshComp, V8Meters);
      if (V8Meters.Num() == 8)
      {
        AppendTrafficSignLabel(TrafficSignsLabels, StartTs, V8Meters);
      }
    }
    else if (Tag == carla::rpc::CityObjectLabel::TrafficLight)
    {
      TrafficLightsFound++;
      UE_LOG(LogCarla, Warning, TEXT("Found traffic light: %s at %s"),
             MeshComp->GetOwner() ? *MeshComp->GetOwner()->GetName() : TEXT("NoOwner"),
             *MeshComp->GetComponentLocation().ToString());

      // Build cuboid for traffic lights (same format as traffic signs)
      TArray<FVector> V8Meters;
      BuildComponentCuboidMeters(StaticMeshComp, V8Meters);
      if (V8Meters.Num() == 8)
      {
        AppendTrafficLightLabel(TrafficLightsLabels, StartTs, V8Meters);
      }
    }
    else if (Tag == carla::rpc::CityObjectLabel::Poles)
    {
      PolesFound++;
      UE_LOG(LogCarla, Warning, TEXT("Found pole: %s at %s"),
             MeshComp->GetOwner() ? *MeshComp->GetOwner()->GetName() : TEXT("NoOwner"),
             *MeshComp->GetComponentLocation().ToString());

      // Additional filtering like CosmosControlSensor (filter out horizontal poles)
      FBoxSphereBounds Bounds = StaticMeshComp->CalcBounds(StaticMeshComp->GetComponentTransform());

      // Filter out horizontal poles (poles should be taller than they are wide)
      if (FMath::Max(Bounds.BoxExtent.X, Bounds.BoxExtent.Y) > Bounds.BoxExtent.Z)
      {
        // Skip horizontal objects unless they explicitly contain "pole" in the name
        if (!StaticMeshComp->GetStaticMesh() || !StaticMeshComp->GetStaticMesh()->GetFName().ToString().Contains(TEXT("pole")))
        {
          continue;
        }
      }

      FVector TopM, BaseM;
      BuildPolePolylineMeters(StaticMeshComp, TopM, BaseM);
      AppendPoleLabel(PolesLabels, StartTs, TopM, BaseM);
    }
  }

  UE_LOG(LogCarla, Warning, TEXT("TrafficSignsExporter stats: Visible=%d, WithOwner=%d, PassedFilter=%d, StaticMesh=%d, Tagged=%d"),
         VisibleComponents, ComponentsWithOwner, FilteredComponents, StaticMeshComponents, TaggedComponents);
  UE_LOG(LogCarla, Warning, TEXT("TrafficSignsExporter found: Poles=%d, TrafficSigns=%d, TrafficLights=%d"),
         PolesFound, TrafficSignsFound, TrafficLightsFound);

  // Write three separate JSON files
  struct ExportInfo {
    FString SubDir;
    FString Filename;
    TArray<TSharedPtr<FJsonValue>>& Labels;
  };

  TArray<ExportInfo> Exports = {
    { TEXT("3d_poles/"), TEXT(".poles.json"), PolesLabels },
    { TEXT("3d_traffic_signs/"), TEXT(".traffic_signs.json"), TrafficSignsLabels },
    { TEXT("3d_traffic_lights/"), TEXT(".traffic_lights.json"), TrafficLightsLabels }
  };

  for (const ExportInfo& Export : Exports)
  {
    TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
    Root->SetArrayField(TEXT("labels"), Export.Labels);

    FString OutText;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutText);
    if (!FJsonSerializer::Serialize(Root, Writer))
    {
      OutError = FString::Printf(TEXT("JSON serialization failed for %s"), *Export.SubDir);
      return false;
    }

    FString FullPath = OutFilePath + Export.SubDir + SessionId + Export.Filename;
    const FString Dir = FPaths::GetPath(FullPath);
    IFileManager::Get().MakeDirectory(*Dir, /*Tree=*/true);

    if (!FFileHelper::SaveStringToFile(OutText, *FullPath, FFileHelper::EEncodingOptions::ForceUTF8))
    {
      OutError = FString::Printf(TEXT("Failed to write: %s"), *FullPath);
      return false;
    }
  }

  return true;
}

