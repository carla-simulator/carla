// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "MapLogicParser.h"
#include "Traffic/TrafficLightController.h"
#include "TriggerBoxActor.h"

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

  // Load traffic light logic data from JSON
  TArray<FTrafficLightLogicData> LogicData = LoadMapLogicFromJSON(XODRFilePath);

  if (LogicData.Num() == 0)
  {
    // No JSON file found or no data - this is normal for native CARLA maps
    UE_LOG(LogCarla, Log, TEXT("MapLogicParser: No map_logic.json found for %s, using default timing"), *XODRFilePath);
    return false;
  }

  // Apply the configuration
  ApplyTrafficLightLogic(LogicData, TrafficLightManager);

  UE_LOG(LogCarla, Log, TEXT("MapLogicParser: Applied custom timing to %d traffic light controllers"), LogicData.Num());
  return true;
}

TArray<FTrafficLightLogicData> UMapLogicParser::LoadMapLogicFromJSON(const FString& XODRFilePath)
{
  TArray<FTrafficLightLogicData> Result;

  // Construct path to map_logic.json in the same directory as the XODR file
  FString DirectoryPath = GetDirectoryPath(XODRFilePath);
  FString JsonFilePath = FPaths::Combine(DirectoryPath, TEXT("map_logic.json"));

  UE_LOG(LogCarla, Log, TEXT("MapLogicParser: Looking for map_logic.json at: %s"), *JsonFilePath);

  // Check if file exists
  if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*JsonFilePath))
  {
    // This is normal - not all maps will have map_logic.json
    UE_LOG(LogCarla, Log, TEXT("MapLogicParser: map_logic.json not found at %s"), *JsonFilePath);
    return Result;
  }

  // Load file content
  FString JsonString;
  if (!FFileHelper::LoadFileToString(JsonString, *JsonFilePath))
  {
    UE_LOG(LogCarla, Error, TEXT("MapLogicParser: Failed to load map_logic.json from %s"), *JsonFilePath);
    return Result;
  }

  // DEBUG: Log the raw JSON content
  UE_LOG(LogCarla, Log, TEXT("MapLogicParser: Successfully loaded map_logic.json from %s"), *JsonFilePath);
  UE_LOG(LogCarla, Log, TEXT("MapLogicParser: Raw JSON content:\n%s"), *JsonString);

  // Parse JSON
  TSharedPtr<FJsonObject> JsonObject;
  TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

  if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
  {
    UE_LOG(LogCarla, Error, TEXT("MapLogicParser: Failed to parse JSON from %s"), *JsonFilePath);
    return Result;
  }

  // Get TrafficLights array
  const TArray<TSharedPtr<FJsonValue>>* TrafficLightsArray;
  if (!JsonObject->TryGetArrayField(TEXT("TrafficLights"), TrafficLightsArray))
  {
    UE_LOG(LogCarla, Error, TEXT("MapLogicParser: No 'TrafficLights' array found in %s"), *JsonFilePath);
    return Result;
  }

  // Parse each traffic light entry
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

  // DEBUG: Log each loaded traffic light configuration
  for (int32 i = 0; i < Result.Num(); i++)
  {
    const FTrafficLightLogicData& Data = Result[i];
    UE_LOG(LogCarla, Log, TEXT("MapLogicParser: [%d] ActorName='%s' GroupID='%s' JunctionID=%d Timing(R:%.1f G:%.1f Y:%.1f)"),
           i, *Data.ActorName, *Data.TrafficLightGroupID, Data.JunctionID,
           Data.Timing.RedDuration, Data.Timing.GreenDuration, Data.Timing.AmberDuration);

    // Log modules and their lane IDs
    for (int32 j = 0; j < Data.Modules.Num(); j++)
    {
      const FTrafficLightModule& Module = Data.Modules[j];
      FString LaneIdsStr = TEXT("[");
      for (int32 k = 0; k < Module.LaneIds.Num(); k++)
      {
        LaneIdsStr += FString::Printf(TEXT("%d"), Module.LaneIds[k]);
        if (k < Module.LaneIds.Num() - 1) LaneIdsStr += TEXT(", ");
      }
      LaneIdsStr += TEXT("]");
      UE_LOG(LogCarla, Log, TEXT("MapLogicParser: [%d] Module[%d] LaneIds=%s"), i, j, *LaneIdsStr);
    }
  }

  return Result;
}

void UMapLogicParser::ApplyTrafficLightLogic(const TArray<FTrafficLightLogicData>& LogicData, ATrafficLightManager* TrafficLightManager)
{
  int32 SuccessCount = 0;
  int32 FailedCount = 0;

  UE_LOG(LogCarla, Log, TEXT("MapLogicParser: Starting to apply timing configuration to %d traffic lights"), LogicData.Num());

  for (const auto& Data : LogicData)
  {
    UE_LOG(LogCarla, Log, TEXT("MapLogicParser: Processing traffic light '%s' (GroupID='%s', JunctionID=%d)"),
           *Data.ActorName, *Data.TrafficLightGroupID, Data.JunctionID);

    // STEP 1: Try to find the actor by name in the scene (search both AActor and AStaticMeshActor)
    UWorld* World = TrafficLightManager->GetWorld();
    AActor* FoundActor = nullptr;

    UE_LOG(LogCarla, Warning, TEXT("MapLogicParser: 🔍 DEBUG - Looking for actor: '%s'"), *Data.ActorName);

    // Search in all Actors first
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    UE_LOG(LogCarla, Warning, TEXT("MapLogicParser: 🔍 DEBUG - Found %d total actors in scene"), AllActors.Num());

    // DEBUG: Log first 10 actors to see naming patterns
    for (int32 i = 0; i < FMath::Min(10, AllActors.Num()); i++)
    {
      if (AllActors[i])
      {
        UE_LOG(LogCarla, Warning, TEXT("MapLogicParser: 🔍 DEBUG - Actor[%d]: '%s' (Class: %s)"),
               i, *AllActors[i]->GetName(), *AllActors[i]->GetClass()->GetName());
      }
    }

    // Look for actors that contain part of our target name
    UE_LOG(LogCarla, Warning, TEXT("MapLogicParser: 🔍 DEBUG - Searching for actors containing 'BP_TrafficLight'"));
    for (AActor* Actor : AllActors)
    {
      if (Actor && Actor->GetName().Contains(TEXT("BP_TrafficLight")))
      {
        UE_LOG(LogCarla, Warning, TEXT("MapLogicParser: 🔍 DEBUG - Found BP_TrafficLight actor: '%s'"), *Actor->GetName());
      }
    }

    // Exact match search
    for (AActor* Actor : AllActors)
    {
      if (Actor && Actor->GetName() == Data.ActorName)
      {
        FoundActor = Actor;
        UE_LOG(LogCarla, Warning, TEXT("MapLogicParser: ✅ FOUND ACTOR '%s' at location (%.1f, %.1f, %.1f)"),
               *Data.ActorName,
               FoundActor->GetActorLocation().X,
               FoundActor->GetActorLocation().Y,
               FoundActor->GetActorLocation().Z);
        break;
      }
    }

    // Search specifically in StaticMeshActors if not found
    if (!FoundActor)
    {
      TArray<AActor*> StaticMeshActors;
      UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), StaticMeshActors);

      UE_LOG(LogCarla, Warning, TEXT("MapLogicParser: 🔍 DEBUG - Found %d StaticMeshActors"), StaticMeshActors.Num());

      for (AActor* Actor : StaticMeshActors)
      {
        if (Actor && Actor->GetName() == Data.ActorName)
        {
          FoundActor = Actor;
          UE_LOG(LogCarla, Warning, TEXT("MapLogicParser: ✅ FOUND STATIC MESH ACTOR '%s' at location (%.1f, %.1f, %.1f)"),
                 *Data.ActorName,
                 FoundActor->GetActorLocation().X,
                 FoundActor->GetActorLocation().Y,
                 FoundActor->GetActorLocation().Z);
          break;
        }
      }
    }

    if (!FoundActor)
    {
      UE_LOG(LogCarla, Error, TEXT("MapLogicParser: ❌ NOT FOUND '%s' in scene"), *Data.ActorName);
      UE_LOG(LogCarla, Error, TEXT("MapLogicParser: 💡 Check if the actor name in the scene matches exactly (case-sensitive)"));
    }
    else
    {
      // STEP 1.5: Spawn a visible trigger box at the actor location for testing
      SpawnVisibleTriggerBox(FoundActor, Data.ActorName, World);
    }

    // STEP 2: Find the controller using the TrafficLightGroupID (which corresponds to ControllerID from OpenDRIVE)
    UTrafficLightController* Controller = TrafficLightManager->GetController(Data.TrafficLightGroupID);

    if (Controller)
    {
      // Apply custom timing
      Controller->SetRedTime(Data.Timing.RedDuration);
      Controller->SetGreenTime(Data.Timing.GreenDuration);
      Controller->SetYellowTime(Data.Timing.AmberDuration);

      UE_LOG(LogCarla, Log, TEXT("MapLogicParser: ✅ SUCCESS - Applied timing to controller '%s' - Red:%.1f Green:%.1f Yellow:%.1f"),
             *Data.TrafficLightGroupID,
             Data.Timing.RedDuration,
             Data.Timing.GreenDuration,
             Data.Timing.AmberDuration);
      SuccessCount++;
    }
    else
    {
      UE_LOG(LogCarla, Error, TEXT("MapLogicParser: ❌ FAILED - Controller '%s' not found for traffic light '%s' (JunctionID=%d)"),
             *Data.TrafficLightGroupID, *Data.ActorName, Data.JunctionID);
      FailedCount++;
    }
  }

  UE_LOG(LogCarla, Warning, TEXT("MapLogicParser: Summary - Successfully applied timing to %d/%d traffic lights. %d failed."),
         SuccessCount, LogicData.Num(), FailedCount);

  if (FailedCount > 0)
  {
    UE_LOG(LogCarla, Error, TEXT("MapLogicParser: %d traffic lights failed to get timing configuration! Check ControllerID mapping."), FailedCount);
  }
}

FTrafficLightLogicData UMapLogicParser::ParseTrafficLightFromJSON(TSharedPtr<FJsonObject> TrafficLightJson)
{
  FTrafficLightLogicData Result;

  if (!TrafficLightJson.IsValid())
  {
    return Result;
  }

  // Parse basic fields
  TrafficLightJson->TryGetStringField(TEXT("ActorName"), Result.ActorName);
  TrafficLightJson->TryGetNumberField(TEXT("JunctionID"), Result.JunctionID);
  TrafficLightJson->TryGetStringField(TEXT("TrafficLightGroupID"), Result.TrafficLightGroupID);

  // Parse timing object
  const TSharedPtr<FJsonObject>* TimingObject;
  if (TrafficLightJson->TryGetObjectField(TEXT("Timing"), TimingObject))
  {
    Result.Timing = ParseTimingFromJSON(*TimingObject);
  }

  // Parse modules array
  const TArray<TSharedPtr<FJsonValue>>* ModulesArray;
  if (TrafficLightJson->TryGetArrayField(TEXT("Modules"), ModulesArray))
  {
    for (const auto& ModuleValue : *ModulesArray)
    {
      TSharedPtr<FJsonObject> ModuleObject = ModuleValue->AsObject();
      if (ModuleObject.IsValid())
      {
        FTrafficLightModule Module;

        // Parse lane IDs array for this module
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

  // Parse timing fields with fallback to default values
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

void UMapLogicParser::SpawnVisibleTriggerBox(AActor* TargetActor, const FString& TrafficLightName, UWorld* World)
{
  if (!TargetActor || !World)
  {
    UE_LOG(LogCarla, Error, TEXT("MapLogicParser: Cannot spawn trigger box - invalid actor or world"));
    return;
  }

  // Get the target location (traffic light position)
  FVector SpawnLocation = TargetActor->GetActorLocation();

  // Offset the trigger box slightly above the traffic light for visibility
  SpawnLocation.Z += 50.0f;

  // Spawn custom trigger box actor that handles overlap events and shows wireframe
  FActorSpawnParameters SpawnParams;
  SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  SpawnParams.Name = FName(*FString::Printf(TEXT("TriggerBox_%s"), *TrafficLightName));

  ATriggerBoxActor* TriggerBoxActor = World->SpawnActor<ATriggerBoxActor>(ATriggerBoxActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);

  if (TriggerBoxActor)
  {
    // Set the traffic light name for logging
    TriggerBoxActor->TrafficLightName = TrafficLightName;

    // Get the box collision component
    UBoxComponent* BoxComponent = Cast<UBoxComponent>(TriggerBoxActor->GetCollisionComponent());
    if (BoxComponent)
    {
      // Set trigger box size (5m x 5m x 1m)
      BoxComponent->SetBoxExtent(FVector(250.0f, 250.0f, 50.0f)); // Half extents

      // Configure collision for overlap detection (redundant but ensuring)
      BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
      BoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
      BoxComponent->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Overlap);
      BoxComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
      BoxComponent->SetGenerateOverlapEvents(true);

      // Scale the wireframe mesh to match the collision box
      if (TriggerBoxActor->WireframeMesh)
      {
        TriggerBoxActor->WireframeMesh->SetWorldScale3D(FVector(5.0f, 5.0f, 1.0f)); // 5m x 5m x 1m
      }

      UE_LOG(LogCarla, Warning, TEXT("MapLogicParser: ✅ SPAWNED TRIGGER BOX ACTOR for '%s' at location (%.1f, %.1f, %.1f) - Wireframe visible in game"),
             *TrafficLightName,
             SpawnLocation.X,
             SpawnLocation.Y,
             SpawnLocation.Z);
    }
  }
  else
  {
    UE_LOG(LogCarla, Error, TEXT("MapLogicParser: Failed to spawn trigger box actor"));
  }
}
