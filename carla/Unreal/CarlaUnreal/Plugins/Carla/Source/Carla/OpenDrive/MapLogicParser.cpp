// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "MapLogicParser.h"
#include "Traffic/TrafficLightController.h"
#include "Traffic/TrafficLightComponent.h"
#include "Traffic/DigitalTwinsTrafficLight.h"
#include "Traffic/TrafficLightBase.h"

#include <util/ue-header-guard-begin.h>
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "HAL/PlatformFileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include <util/ue-header-guard-end.h>

#include <util/disable-ue4-macros.h>
#include <carla/road/element/RoadInfoSignal.h>
#include <util/enable-ue4-macros.h>

bool UMapLogicParser::ParseAndApplyMapLogic(const FString& XODRFilePath, ATrafficLightManager* TrafficLightManager)
{
  if (!TrafficLightManager)
  {
    UE_LOG(LogCarla, Warning, TEXT("MapLogicParser: TrafficLightManager is null"));
    return false;
  }

  if (XODRFilePath.IsEmpty())
  {
    UE_LOG(LogCarla, Warning, TEXT("MapLogicParser: XODR file path is empty"));
    return false;
  }

  TArray<FTrafficLightLogicData> LogicData = LoadMapLogicFromJSON(XODRFilePath);

  if (LogicData.Num() == 0)
  {
    UE_LOG(LogCarla, Log, TEXT("MapLogicParser: No map_logic.json found for %s, using default timing"), *XODRFilePath);
    return false;
  }

  ApplyTrafficLightLogic(LogicData, TrafficLightManager);

  UE_LOG(LogCarla, Log, TEXT("MapLogicParser: Applied custom timing to %d traffic light controllers"), LogicData.Num());
  return true;
}

TArray<FTrafficLightLogicData> UMapLogicParser::LoadMapLogicFromJSON(const FString& XODRFilePath)
{
  TArray<FTrafficLightLogicData> Result;

  FString DirectoryPath = GetDirectoryPath(XODRFilePath);
  FString JsonFilePath = FPaths::Combine(DirectoryPath, TEXT("map_logic.json"));

  if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*JsonFilePath))
  {
    UE_LOG(LogCarla, Log, TEXT("MapLogicParser: map_logic.json not found at %s"), *JsonFilePath);
    return Result;
  }

  FString JsonString;
  if (!FFileHelper::LoadFileToString(JsonString, *JsonFilePath))
  {
    UE_LOG(LogCarla, Error, TEXT("MapLogicParser: Failed to load map_logic.json from %s"), *JsonFilePath);
    return Result;
  }

  UE_LOG(LogCarla, Log, TEXT("MapLogicParser: Successfully loaded map_logic.json from %s"), *JsonFilePath);
  TSharedPtr<FJsonObject> JsonObject;
  TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

  if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
  {
    UE_LOG(LogCarla, Error, TEXT("MapLogicParser: Failed to parse JSON from %s"), *JsonFilePath);
    return Result;
  }

  const TArray<TSharedPtr<FJsonValue>>* TrafficLightsArray;
  if (!JsonObject->TryGetArrayField(TEXT("TrafficLights"), TrafficLightsArray))
  {
    UE_LOG(LogCarla, Error, TEXT("MapLogicParser: No 'TrafficLights' array found in %s"), *JsonFilePath);
    return Result;
  }

  for (const auto& JsonValue : *TrafficLightsArray)
  {
    TSharedPtr<FJsonObject> TrafficLightJson = JsonValue->AsObject();
    if (TrafficLightJson.IsValid())
    {
      FTrafficLightLogicData LogicData = ParseTrafficLightFromJSON(TrafficLightJson);
      Result.Add(LogicData);
    }
  }

  UE_LOG(LogCarla, Log, TEXT("MapLogicParser: Loaded %d traffic light configurations from %s"), Result.Num(), *JsonFilePath);

  return Result;
}

void UMapLogicParser::ApplyTrafficLightLogic(const TArray<FTrafficLightLogicData>& LogicData, ATrafficLightManager* TrafficLightManager)
{
  int32 SuccessCount = 0;
  int32 FailedCount = 0;

  for (const auto& Data : LogicData)
  {
    UTrafficLightController* Controller = TrafficLightManager->GetController(Data.TrafficLightGroupID);

    if (Controller)
    {
      Controller->SetRedTime(Data.Timing.RedDuration);
      Controller->SetGreenTime(Data.Timing.GreenDuration);
      Controller->SetYellowTime(Data.Timing.AmberDuration);
      SuccessCount++;
    }
    else
    {
      UE_LOG(LogCarla, Error, TEXT("MapLogicParser: Controller '%s' not found for traffic light '%s' (JunctionID=%d)"),
             *Data.TrafficLightGroupID, *Data.ActorName, Data.JunctionID);
      FailedCount++;
    }
  }

  UE_LOG(LogCarla, Log, TEXT("MapLogicParser: Applied timing to %d/%d traffic lights (%d failed)"),
         SuccessCount, LogicData.Num(), FailedCount);
}

FTrafficLightLogicData UMapLogicParser::ParseTrafficLightFromJSON(TSharedPtr<FJsonObject> TrafficLightJson)
{
  FTrafficLightLogicData Result;

  if (!TrafficLightJson.IsValid())
  {
    return Result;
  }

  TrafficLightJson->TryGetStringField(TEXT("ActorName"), Result.ActorName);
  TrafficLightJson->TryGetStringField(TEXT("SignalID"), Result.SignalID);
  TrafficLightJson->TryGetNumberField(TEXT("JunctionID"), Result.JunctionID);
  TrafficLightJson->TryGetStringField(TEXT("TrafficLightGroupID"), Result.TrafficLightGroupID);
  const TSharedPtr<FJsonObject>* TimingObject;
  if (TrafficLightJson->TryGetObjectField(TEXT("Timing"), TimingObject))
  {
    Result.Timing = ParseTimingFromJSON(*TimingObject);
  }

  const TArray<TSharedPtr<FJsonValue>>* ModulesArray;
  if (TrafficLightJson->TryGetArrayField(TEXT("Modules"), ModulesArray))
  {
    for (const auto& ModuleValue : *ModulesArray)
    {
      TSharedPtr<FJsonObject> ModuleObject = ModuleValue->AsObject();
      if (ModuleObject.IsValid())
      {
        FTrafficLightModule Module;
        const TArray<TSharedPtr<FJsonValue>>* LaneIdsArray;
        if (ModuleObject->TryGetArrayField(TEXT("LaneIds"), LaneIdsArray))
        {
          for (const auto& LaneIdValue : *LaneIdsArray)
          {
            int32 LaneId;
            if (LaneIdValue->TryGetNumber(LaneId))
            {
              Module.LaneIds.Add(LaneId);
            }
          }
        }
        Result.Modules.Add(Module);
      }
    }
  }

  return Result;
}

FTrafficLightTiming UMapLogicParser::ParseTimingFromJSON(TSharedPtr<FJsonObject> TimingJson)
{
  FTrafficLightTiming Result;

  if (!TimingJson.IsValid())
  {
    return Result;
  }

  double TempValue;

  if (TimingJson->TryGetNumberField(TEXT("RedDuration"), TempValue))
  {
    Result.RedDuration = static_cast<float>(TempValue);
  }

  if (TimingJson->TryGetNumberField(TEXT("GreenDuration"), TempValue))
  {
    Result.GreenDuration = static_cast<float>(TempValue);
  }

  if (TimingJson->TryGetNumberField(TEXT("AmberDuration"), TempValue))
  {
    Result.AmberDuration = static_cast<float>(TempValue);
  }

  if (TimingJson->TryGetNumberField(TEXT("AmberBlinkInterval"), TempValue))
  {
    Result.AmberBlinkInterval = static_cast<float>(TempValue);
  }

  return Result;
}

FString UMapLogicParser::GetDirectoryPath(const FString& FilePath)
{
  FString Directory, Filename, Extension;
  FPaths::Split(FilePath, Directory, Filename, Extension);
  return Directory;
}

void UMapLogicParser::ApplyLaneIdsFromMapLogic(const FString& XODRFilePath, ATrafficLightManager* TrafficLightManager)
{
  if (!TrafficLightManager)
  {
    UE_LOG(LogCarla, Warning, TEXT("MapLogicParser::ApplyLaneIdsFromMapLogic: TrafficLightManager is null"));
    return;
  }

  const std::optional<carla::road::Map>& Map = TrafficLightManager->GetMap();
  if (!Map.has_value())
  {
    UE_LOG(LogCarla, Warning, TEXT("MapLogicParser::ApplyLaneIdsFromMapLogic: Map is not available"));
    return;
  }

  TArray<FTrafficLightLogicData> LogicData = LoadMapLogicFromJSON(XODRFilePath);
  if (LogicData.Num() == 0)
  {
    UE_LOG(LogCarla, Log, TEXT("MapLogicParser::ApplyLaneIdsFromMapLogic: No map_logic.json found or no data"));
    return;
  }

  const auto& Signals = Map->GetSignals();

  int32 SuccessCount = 0;

  for (const FTrafficLightLogicData& Data : LogicData)
  {
    if (Data.SignalID.IsEmpty())
    {
      UE_LOG(LogCarla, Warning, TEXT("Skipping traffic light with empty SignalID (ActorName='%s')"), *Data.ActorName);
      continue;
    }

    std::string SignalIdStr(TCHAR_TO_UTF8(*Data.SignalID));

    if (Signals.find(SignalIdStr) == Signals.end())
    {
      UE_LOG(LogCarla, Warning, TEXT("Signal '%s' not found in OpenDRIVE"), *Data.SignalID);
      continue;
    }

    const auto& Signal = Signals.at(SignalIdStr);
    auto CarlaTransform = Signal->GetTransform();
    FTransform UETransform(CarlaTransform);
    FVector SignalLocation = UETransform.GetLocation();

    constexpr float MaxDistanceMatchSqr = 2500.0f;
    AActor* ClosestActor = nullptr;
    float MinDistance = MaxDistanceMatchSqr;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(TrafficLightManager->GetWorld(), AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
      if (!Actor) continue;

      float Dist = FVector::DistSquared(Actor->GetActorLocation(), SignalLocation);
      if (Dist < MinDistance)
      {
        MinDistance = Dist;
        ClosestActor = Actor;
      }
    }

    if (!ClosestActor)
    {
      UE_LOG(LogCarla, Error, TEXT("No actor found within 50cm of signal '%s'"), *Data.SignalID);
      continue;
    }

    ATrafficLightBase* TrafficLightActor = Cast<ATrafficLightBase>(ClosestActor);
    UTrafficLightComponent* TrafficLightComp = nullptr;

    if (TrafficLightActor)
    {
      TrafficLightComp = TrafficLightActor->GetTrafficLightComponent();

      if (TrafficLightComp->GetSignId() != Data.SignalID)
      {
        TrafficLightComp->SetSignId(Data.SignalID);
      }
    }
    else
    {
      TArray<UStaticMeshComponent*> MeshComponents;
      ClosestActor->GetComponents<UStaticMeshComponent>(MeshComponents);

      if (MeshComponents.Num() == 0)
      {
        UE_LOG(LogCarla, Error, TEXT("DigitalTwins actor has no StaticMeshComponents"));
        continue;
      }
      FActorSpawnParameters SpawnParams;
      SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

      ADigitalTwinsTrafficLight* NewTrafficLight = TrafficLightManager->GetWorld()->SpawnActor<ADigitalTwinsTrafficLight>(
          ADigitalTwinsTrafficLight::StaticClass(),
          ClosestActor->GetActorLocation(),
          ClosestActor->GetActorRotation(),
          SpawnParams);

      if (!NewTrafficLight)
      {
        UE_LOG(LogCarla, Error, TEXT("Failed to spawn ADigitalTwinsTrafficLight"));
        continue;
      }

      for (UStaticMeshComponent* SourceMesh : MeshComponents)
      {
        if (!SourceMesh) continue;

        UStaticMeshComponent* NewMesh = NewObject<UStaticMeshComponent>(NewTrafficLight);
        NewMesh->SetStaticMesh(SourceMesh->GetStaticMesh());
        NewMesh->SetRelativeTransform(SourceMesh->GetRelativeTransform());

        for (int32 i = 0; i < SourceMesh->GetNumMaterials(); i++)
        {
          NewMesh->SetMaterial(i, SourceMesh->GetMaterial(i));
        }

        NewMesh->RegisterComponent();
        NewMesh->AttachToComponent(NewTrafficLight->GetRootComponent(),
                                   FAttachmentTransformRules::KeepRelativeTransform);
      }

      TrafficLightComp = NewTrafficLight->GetTrafficLightComponent();
      if (!TrafficLightComp)
      {
        UE_LOG(LogCarla, Error, TEXT("ADigitalTwinsTrafficLight has no TrafficLightComponent"));
        NewTrafficLight->Destroy();
        continue;
      }

      TrafficLightComp->SetSignId(Data.SignalID);
      TrafficLightManager->RegisterLightComponentFromOpenDRIVE(TrafficLightComp);
      ClosestActor->Destroy();
    }

    UTrafficLightController* Controller = TrafficLightManager->GetController(Data.TrafficLightGroupID);
    if (Controller)
    {
      Controller->SetRedTime(Data.Timing.RedDuration);
      Controller->SetGreenTime(Data.Timing.GreenDuration);
      Controller->SetYellowTime(Data.Timing.AmberDuration);
    }
    else
    {
      UE_LOG(LogCarla, Error, TEXT("Failed to get controller '%s'"), *Data.TrafficLightGroupID);
    }

    TrafficLightComp->InitializeSign(Map.value());
    SuccessCount++;
  }

  UE_LOG(LogCarla, Log, TEXT("Applied lane IDs to %d traffic lights"), SuccessCount);
}
