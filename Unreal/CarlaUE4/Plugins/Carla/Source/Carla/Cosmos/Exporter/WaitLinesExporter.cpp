// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// WaitLinesExporter.cpp
#include "Carla.h"
#include "WaitLinesExporter.h"
#include "Carla/Traffic/TrafficLightBase.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"

// --------------------- helpers ---------------------

bool UWaitLinesExporter::ParseSessionIdParts(const FString& In, FString& OutUuid, FString& OutStart, FString& OutEnd)
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

void UWaitLinesExporter::AppendWaitLineLabel(
    TArray<TSharedPtr<FJsonValue>>& LabelsArr,
    const FString& StartTs,
    const FVector& StartPoint,
    const FVector& EndPoint)
{
  TSharedRef<FJsonObject> Label = MakeShared<FJsonObject>();

  // labelData.shape3d.polyline3d
  TSharedRef<FJsonObject> LabelData = MakeShared<FJsonObject>();
  {
    TSharedRef<FJsonObject> Shape3D = MakeShared<FJsonObject>();
    Shape3D->SetStringField(TEXT("unit"), TEXT("METRIC"));

    TSharedRef<FJsonObject> Polyline3D = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> VertArray;

    // Add start point
    TArray<TSharedPtr<FJsonValue>> StartTriple;
    StartTriple.Add(MakeShared<FJsonValueNumber>(StartPoint.X));
    StartTriple.Add(MakeShared<FJsonValueNumber>(StartPoint.Y));
    StartTriple.Add(MakeShared<FJsonValueNumber>(StartPoint.Z));
    VertArray.Add(MakeShared<FJsonValueArray>(StartTriple));

    // Add end point
    TArray<TSharedPtr<FJsonValue>> EndTriple;
    EndTriple.Add(MakeShared<FJsonValueNumber>(EndPoint.X));
    EndTriple.Add(MakeShared<FJsonValueNumber>(EndPoint.Y));
    EndTriple.Add(MakeShared<FJsonValueNumber>(EndPoint.Z));
    VertArray.Add(MakeShared<FJsonValueArray>(EndTriple));

    Polyline3D->SetArrayField(TEXT("vertices"), VertArray);
    Shape3D->SetObjectField(TEXT("polyline3d"), Polyline3D);
    LabelData->SetObjectField(TEXT("shape3d"), Shape3D);
  }
  Label->SetObjectField(TEXT("labelData"), LabelData);
  Label->SetStringField(TEXT("timestampMicroseconds"), StartTs);

  LabelsArr.Add(MakeShared<FJsonValueObject>(Label));
}

// --------------------- main export ---------------------

bool UWaitLinesExporter::ExportCosmosWaitLines(UWorld* World, const FString& SessionId, const FString& OutFilePath, FString& OutError)
{
  FString Uuid, StartTs, EndTs;
  if (!ParseSessionIdParts(SessionId, Uuid, StartTs, EndTs))
  {
    OutError = FString::Printf(TEXT("SessionId '%s' is not 'uuid_start_end'"), *SessionId);
    return false;
  }

  // Root JSON
  TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  TArray<TSharedPtr<FJsonValue>> LabelsArr;

  // Find all traffic lights and create wait lines based on their trigger volumes
  // This follows the same logic as CosmosControlSensor
  TArray<AActor*> TrafficLights;
  UGameplayStatics::GetAllActorsOfClass(World, ATrafficLightBase::StaticClass(), TrafficLights);

  for (AActor* TrafficLight : TrafficLights)
  {
    UBoxComponent* StopBoxCollider = Cast<UBoxComponent>(TrafficLight->GetComponentByClass(UBoxComponent::StaticClass()));
    if (!StopBoxCollider) continue;

    // Calculate the wait line position based on the traffic light's box component
    // This replicates the logic from CosmosControlSensor::PostPhysTick
    FVector GroundPos = FVector(StopBoxCollider->GetComponentLocation().X,
                               StopBoxCollider->GetComponentLocation().Y,
                               0.0f);

    // Calculate the start and end points of the wait line
    FVector ForwardVector = StopBoxCollider->GetForwardVector();
    FVector RightVector = StopBoxCollider->GetRightVector();
    FVector BoxExtent = StopBoxCollider->GetScaledBoxExtent();

    // The line extends across the width of the box, positioned back from the traffic light
    FVector StartPoint = GroundPos + (-BoxExtent.X * ForwardVector) - (710.0f * RightVector);
    FVector EndPoint = GroundPos + (BoxExtent.X * ForwardVector) - (710.0f * RightVector);

    // Convert to meters
    StartPoint /= 100.0f;
    EndPoint /= 100.0f;

    AppendWaitLineLabel(LabelsArr, StartTs, StartPoint, EndPoint);
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
  FString FullPath = OutFilePath + TEXT("3d_wait_lines/") + SessionId + TEXT(".wait_lines.json");
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

