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

  const TArray<TSharedPtr<FJsonValue>>* HeadsArray;
  if (TrafficLightJson->TryGetArrayField(TEXT("Heads"), HeadsArray))
  {
    for (const auto& HeadValue : *HeadsArray)
    {
      TSharedPtr<FJsonObject> HeadObject = HeadValue->AsObject();
      if (!HeadObject.IsValid())
      {
        continue;
      }
      FTrafficLightHead Head;
      HeadObject->TryGetStringField(TEXT("ComponentPrefix"), Head.ComponentPrefix);
      HeadObject->TryGetStringField(TEXT("SignalID"), Head.SignalID);
      const TArray<TSharedPtr<FJsonValue>>* LaneIdsArray;
      if (HeadObject->TryGetArrayField(TEXT("LaneIds"), LaneIdsArray))
      {
        for (const auto& LaneIdValue : *LaneIdsArray)
        {
          int32 LaneId;
          if (LaneIdValue->TryGetNumber(LaneId))
          {
            Head.LaneIds.Add(LaneId);
          }
        }
      }
      if (!Head.ComponentPrefix.IsEmpty())
      {
        Result.Heads.Add(Head);
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

namespace
{
  /// The baked rig of a map_logic entry.
  ///
  /// By name first: the placer stamps the entry's ActorName on the baked actor as a tag (and
  /// as its editor label), so the binding is explicit. The nearest-actor search is only a
  /// fallback for a level baked before that, and it is a guess: two rigs of one junction sit
  /// within the 50 cm radius by construction, and a gantry that serves two approaches has two
  /// signals at one location.
  AActor* FindBakedActor(UWorld* World, const FString& ActorName, const FVector& SignalLocation,
                         const FString& SignalId, bool& bOutFoundByName)
  {
    bOutFoundByName = false;
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    if (!ActorName.IsEmpty())
    {
      const FName Wanted(*ActorName);
      for (AActor* Actor : AllActors)
      {
        if (Actor && Actor->ActorHasTag(Wanted))
        {
          bOutFoundByName = true;
          return Actor;
        }
      }
#if WITH_EDITOR
      for (AActor* Actor : AllActors)
      {
        if (Actor && Actor->GetActorLabel() == ActorName)
        {
          bOutFoundByName = true;
          return Actor;
        }
      }
#endif
    }

    constexpr float MaxDistanceMatchSqr = 2500.0f;
    AActor* ClosestActor = nullptr;
    float MinDistance = MaxDistanceMatchSqr;
    for (AActor* Actor : AllActors)
    {
      if (!Actor) continue;

      // Never match a light that another signal already claimed. Two rigs on one junction sit
      // within the 50 cm match radius, and adopting a claimed one would overwrite its SignId
      // -- the second signal would silently steal the first one's identity.
      if (ATrafficLightBase* Existing = Cast<ATrafficLightBase>(Actor))
      {
        UTrafficLightComponent* ExistingComp = Existing->GetTrafficLightComponent();
        if (ExistingComp && !ExistingComp->GetSignId().IsEmpty()
            && ExistingComp->GetSignId() != SignalId)
        {
          continue;
        }
      }

      const float Dist = FVector::DistSquared(Actor->GetActorLocation(), SignalLocation);
      if (Dist < MinDistance)
      {
        MinDistance = Dist;
        ClosestActor = Actor;
      }
    }
    return ClosestActor;
  }

  /// One ADigitalTwinsTrafficLight carrying copies of ``Meshes``, spawned deferred so that
  /// BeginPlay -- and with it the lamp scan -- runs after the meshes are in place.
  ADigitalTwinsTrafficLight* SpawnDigitalTwinsLight(UWorld* World, const FTransform& SpawnTransform,
                                                    const TArray<UStaticMeshComponent*>& Meshes,
                                                    const FString& SignalId)
  {
    ADigitalTwinsTrafficLight* NewTrafficLight = World->SpawnActorDeferred<ADigitalTwinsTrafficLight>(
        ADigitalTwinsTrafficLight::StaticClass(), SpawnTransform, nullptr, nullptr,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (!NewTrafficLight)
    {
      UE_LOG(LogCarla, Error, TEXT("Failed to spawn ADigitalTwinsTrafficLight for '%s'"), *SignalId);
      return nullptr;
    }

    for (UStaticMeshComponent* SourceMesh : Meshes)
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

    UTrafficLightComponent* Comp = NewTrafficLight->GetTrafficLightComponent();
    if (!Comp)
    {
      UE_LOG(LogCarla, Error, TEXT("ADigitalTwinsTrafficLight has no TrafficLightComponent"));
      NewTrafficLight->Destroy();
      return nullptr;
    }
    // SignId before FinishSpawning so BeginPlay's lamp census can name the signal.
    Comp->SetSignId(SignalId);
    NewTrafficLight->FinishSpawning(SpawnTransform);
    return NewTrafficLight;
  }

  /// The mesh components of one baked rig, grouped by the OpenDRIVE signal their head names.
  /// Anything that matches no head (Pole_%02d_Base / _Extensible / _Cap, and every backplate
  /// or mesh a future rig adds) is structure and goes with the anchor signal.
  void GroupMeshesBySignal(const TArray<UStaticMeshComponent*>& MeshComponents,
                           const FTrafficLightLogicData& Data,
                           TMap<FString, TArray<UStaticMeshComponent*>>& OutBySignal,
                           TArray<FString>& OutOrder)
  {
    TArray<UStaticMeshComponent*> Structural;
    for (UStaticMeshComponent* Mesh : MeshComponents)
    {
      if (!Mesh) continue;
      const FString Name = Mesh->GetName();
      const FTrafficLightHead* Best = nullptr;
      for (const FTrafficLightHead& Head : Data.Heads)
      {
        if (Name.StartsWith(Head.ComponentPrefix)
            && (!Best || Head.ComponentPrefix.Len() > Best->ComponentPrefix.Len()))
        {
          Best = &Head;
        }
      }
      if (!Best)
      {
        Structural.Add(Mesh);
        continue;
      }
      const FString SignalId = Best->SignalID.IsEmpty() ? Data.SignalID : Best->SignalID;
      if (!OutBySignal.Contains(SignalId))
      {
        OutOrder.Add(SignalId);
      }
      OutBySignal.FindOrAdd(SignalId).Add(Mesh);
    }
    if (Structural.Num() == 0)
    {
      return;
    }
    const FString Anchor = OutBySignal.Contains(Data.SignalID) ? Data.SignalID
                         : (OutOrder.Num() > 0 ? OutOrder[0] : Data.SignalID);
    if (!OutBySignal.Contains(Anchor))
    {
      OutOrder.Add(Anchor);
    }
    OutBySignal.FindOrAdd(Anchor).Append(Structural);
  }
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
  int32 SplitRigs = 0;
  int32 ByProximity = 0;

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

    bool bFoundByName = false;
    AActor* ClosestActor = FindBakedActor(TrafficLightManager->GetWorld(), Data.ActorName,
                                          SignalLocation, Data.SignalID, bFoundByName);
    if (!ClosestActor)
    {
      UE_LOG(LogCarla, Error, TEXT("No baked actor named '%s' and none within 50cm of signal '%s'"),
             *Data.ActorName, *Data.SignalID);
      continue;
    }
    if (!bFoundByName)
    {
      ++ByProximity;
      UE_LOG(LogCarla, Warning,
             TEXT("Signal '%s': no actor tagged/labelled '%s'; fell back to the nearest actor. "
                  "Re-place the lights so the binding is by name, not by position."),
             *Data.SignalID, *Data.ActorName);
    }

    // Timing of a light's own stage: prefer the controller the component actually joined
    // (RegisterLightComponentFromOpenDRIVE derives it from the signal), so a rig split across
    // two signals times both of their stages, not just the one TrafficLightGroupID names.
    auto ApplyTiming = [&](UTrafficLightComponent* Comp)
    {
      UTrafficLightController* Controller = Comp ? Comp->GetController() : nullptr;
      if (!Controller)
      {
        Controller = TrafficLightManager->GetController(Data.TrafficLightGroupID);
      }
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
    };

    ATrafficLightBase* TrafficLightActor = Cast<ATrafficLightBase>(ClosestActor);

    if (TrafficLightActor)
    {
      UTrafficLightComponent* TrafficLightComp = TrafficLightActor->GetTrafficLightComponent();

      if (TrafficLightComp->GetSignId() != Data.SignalID)
      {
        TrafficLightComp->SetSignId(Data.SignalID);
      }

      // An adopted light used to be given a SignId and nothing else, so it joined no
      // ATrafficLightGroup and no controller ever ticked it. Register it like a spawned one.
      // GetController() is set by UTrafficLightController::AddTrafficLight, so it doubles as
      // the "already registered" guard -- registering twice would put the component into the
      // controller's list twice.
      if (!TrafficLightComp->GetController())
      {
        TrafficLightManager->RegisterLightComponentFromOpenDRIVE(TrafficLightComp);
      }
      ApplyTiming(TrafficLightComp);
      TrafficLightComp->InitializeSign(Map.value());
      ++SuccessCount;
      continue;
    }

    TArray<UStaticMeshComponent*> MeshComponents;
    ClosestActor->GetComponents<UStaticMeshComponent>(MeshComponents);

    if (MeshComponents.Num() == 0)
    {
      UE_LOG(LogCarla, Error, TEXT("DigitalTwins actor has no StaticMeshComponents"));
      continue;
    }

    // One ADigitalTwinsTrafficLight per distinct signal the rig's heads name. With no "Heads"
    // array (or with every head on the same signal) that is exactly one, i.e. the old
    // behaviour; with an arrow head beside a through head it is two, each with its own
    // ETrafficLightState, its own trigger boxes and its own stage.
    TMap<FString, TArray<UStaticMeshComponent*>> BySignal;
    TArray<FString> Order;
    if (Data.Heads.Num() > 0)
    {
      GroupMeshesBySignal(MeshComponents, Data, BySignal, Order);
    }
    if (BySignal.Num() == 0)
    {
      Order.Add(Data.SignalID);
      BySignal.Add(Data.SignalID, MeshComponents);
    }
    if (BySignal.Num() > 1)
    {
      ++SplitRigs;
      UE_LOG(LogCarla, Log, TEXT("Rig '%s' carries %d signals: %s"), *Data.ActorName,
             BySignal.Num(), *FString::Join(Order, TEXT(", ")));
    }

    const FTransform SpawnTransform(ClosestActor->GetActorRotation(), ClosestActor->GetActorLocation());
    int32 SpawnedHere = 0;
    for (const FString& HeadSignalId : Order)
    {
      const std::string HeadSignalIdStr(TCHAR_TO_UTF8(*HeadSignalId));
      if (Signals.find(HeadSignalIdStr) == Signals.end())
      {
        UE_LOG(LogCarla, Error, TEXT("Head signal '%s' of rig '%s' is not in the OpenDRIVE"),
               *HeadSignalId, *Data.ActorName);
        continue;
      }
      ADigitalTwinsTrafficLight* NewTrafficLight = SpawnDigitalTwinsLight(
          TrafficLightManager->GetWorld(), SpawnTransform, BySignal[HeadSignalId], HeadSignalId);
      if (!NewTrafficLight)
      {
        continue;
      }
      UTrafficLightComponent* TrafficLightComp = NewTrafficLight->GetTrafficLightComponent();
      TrafficLightManager->RegisterLightComponentFromOpenDRIVE(TrafficLightComp);
      ApplyTiming(TrafficLightComp);
      TrafficLightComp->InitializeSign(Map.value());
      ++SpawnedHere;
      ++SuccessCount;
    }
    if (SpawnedHere > 0)
    {
      ClosestActor->Destroy();
    }
  }

  if (SplitRigs > 0 || ByProximity > 0)
  {
    UE_LOG(LogCarla, Log,
           TEXT("Map logic: %d rig(s) split across several signals, %d matched by position "
                "instead of by name"), SplitRigs, ByProximity);
  }
  UE_LOG(LogCarla, Log, TEXT("Applied lane IDs to %d traffic lights"), SuccessCount);
}
