// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyCarlaWrapper.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"
#include "Kismet/GameplayStatics.h"
#include "Traffic/SpeedLimitComponent.h"
#include "Traffic/StopSignComponent.h"
#include "Traffic/TrafficLightBase.h"
#include "Traffic/YieldSignComponent.h"
#include "Lights/CarlaLight.h"

#include <disable-ue4-macros.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/road/Map.h>
#include <carla/road/Road.h>
#include <carla/road/SignalType.h>
#include <carla/rpc/String.h>
#include <enable-ue4-macros.h>


DECLARE_LOG_CATEGORY_EXTERN(LogSimReadyCarlaWrapper, Log, All);
DEFINE_LOG_CATEGORY(LogSimReadyCarlaWrapper);

// The module class
class FSimReadyCarlaWrapperModule :public IModuleInterface
{

};

IMPLEMENT_MODULE(FSimReadyCarlaWrapperModule, SimReadyCarlaWrapper);

// CARLA lib statically link this
namespace carla {

    void throw_exception(const std::exception& e) {
        UE_LOG(LogSimReadyCarlaWrapper, Fatal, TEXT("Exception occurred in LibCarla"));

        // It should never reach this part.
        std::terminate();
    }

} // namespace carla

namespace SimReadyCarlaWrapper
{
    // Cached XODR traffic signals
    static std::map<std::string, FVector> XodrSignals;

    bool LoadXodrInternal(const UWorld& World, boost::optional<carla::road::Map>& XodrMap)
    {
        auto Path = FPaths::GetPath(TSoftObjectPtr<UWorld>(&World).GetLongPackageName());
        Path.RemoveFromStart("/Game/");
        Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()) + Path;
        Path = FPaths::Combine(Path, TEXT("OpenDrive"), World.GetName() + ".xodr");

        FString Result;
        if (!FFileHelper::LoadFileToString(Result, *Path))
        {
            UE_LOG(LogSimReadyCarlaWrapper, Error, TEXT("Failed to load file %s."), *Path);
            return false;
        }

        XodrMap = carla::opendrive::OpenDriveParser::Load(TCHAR_TO_ANSI(*Result));
        if (!XodrMap)
        {
            UE_LOG(LogSimReadyCarlaWrapper, Error, TEXT("Failed to parse XODR map %s."), *Path);
            return false;
        }

        return true;
    }

    SIMREADYCARLAWRAPPER_API void SimReadyLoadXodr(const UWorld& World)
    {
        XodrSignals.clear();
  
        boost::optional<carla::road::Map> XodrMap;
        if (!LoadXodrInternal(World, XodrMap))
        {
            return;
        }

        for (auto& Entry : XodrMap->GetSignals())
        {
            if (!carla::road::SignalType::IsTrafficLight(Entry.second->GetType()))
                continue;

            auto Location = Entry.second->GetTransform().location;
            // OpenDrive uses meter units, convert to Unreal cm units
            XodrSignals[Entry.first] = FVector(Location.x * 100.0f, Location.y * 100.0f, Location.z * 100.0f);
        }
    }

    SIMREADYCARLAWRAPPER_API const char* SimReadyFindXodrTrafficSignalID(const FVector& Location)
    {
        float MinSqrDist = 250000.0;
        const char* Id = nullptr;
        for (auto& Entry : XodrSignals)
        {
            auto SqrDist = (Entry.second - Location).SizeSquared();
            if (SqrDist < MinSqrDist)
            {
                MinSqrDist = SqrDist;
                Id = Entry.first.c_str();
            }
        }

        return Id;
    }

    const UCarlaLight* GetCarlaLightForSimReadyTimeOfDay(const AActor& Actor)
    {
        // Find CarlaLight component
        TArray<UCarlaLight*> CarlaLightArray;
        Actor.GetComponents(CarlaLightArray);
        if (CarlaLightArray.Num() == 0)
        {
            return nullptr;
        }

        // Skip vehicle light
        auto& CarlaLight = *CarlaLightArray[0];
        if (CarlaLight.GetLightType() == ELightType::Vehicle)
        {
            return nullptr;
        }

        // return CARLA light
        return CarlaLightArray[0];
    }

    SIMREADYCARLAWRAPPER_API bool IsTrafficLight(const AActor& Actor)
    {
        return (Actor.IsA<ATrafficLightBase>());
    }

    TMap<const UActorComponent*, CarlaLightData> FindCarlaLights(const AActor& Actor)
    {
        // Find CarlaLight component
        TArray<UCarlaLight*> CarlaLightArray;
        Actor.GetComponents(CarlaLightArray);

        // Skip vehicle light
        TMap<const UActorComponent*, CarlaLightData> CarlaLightDataArray;
        for (auto CarlaLight : CarlaLightArray)
        {
            if (CarlaLight->GetLightType() != ELightType::Vehicle)
            {
                CarlaLightDataArray.Add(CarlaLight, { CarlaLight->GetLightIntensity(), CarlaLight->GetLightColor() });
            }
        }

        return CarlaLightDataArray;
    }

    bool MatchSignalAndActor(const carla::road::Signal& Signal, ATrafficSignBase* ClosestTrafficSign)
    {
        namespace cr = carla::road;
        if (ClosestTrafficSign)
        {
            if ((Signal.GetType() == cr::SignalType::StopSign()) &&
                ClosestTrafficSign->GetTrafficSignState() == ETrafficSignState::StopSign)
            {
                return true;
            }
            else if ((Signal.GetType() == cr::SignalType::YieldSign()) &&
                ClosestTrafficSign->GetTrafficSignState() == ETrafficSignState::YieldSign)
            {
                return true;
            }
            else if (cr::SignalType::IsTrafficLight(Signal.GetType()))
            {
                if (ClosestTrafficSign->GetTrafficSignState() == ETrafficSignState::TrafficLightRed ||
                    ClosestTrafficSign->GetTrafficSignState() == ETrafficSignState::TrafficLightYellow ||
                    ClosestTrafficSign->GetTrafficSignState() == ETrafficSignState::TrafficLightGreen)
                    return true;
            }
            else if (Signal.GetType() == cr::SignalType::MaximumSpeed())
            {
                if (Signal.GetSubtype() == "30" &&
                    ClosestTrafficSign->GetTrafficSignState() == ETrafficSignState::SpeedLimit_30)
                {
                    return true;
                }
                else if (Signal.GetSubtype() == "40" &&
                    ClosestTrafficSign->GetTrafficSignState() == ETrafficSignState::SpeedLimit_40)
                {
                    return true;
                }
                else if (Signal.GetSubtype() == "50" &&
                    ClosestTrafficSign->GetTrafficSignState() == ETrafficSignState::SpeedLimit_50)
                {
                    return true;
                }
                else if (Signal.GetSubtype() == "60" &&
                    ClosestTrafficSign->GetTrafficSignState() == ETrafficSignState::SpeedLimit_60)
                {
                    return true;
                }
                else if (Signal.GetSubtype() == "70" &&
                    ClosestTrafficSign->GetTrafficSignState() == ETrafficSignState::SpeedLimit_60)
                {
                    return true;
                }
                else if (Signal.GetSubtype() == "80" &&
                    ClosestTrafficSign->GetTrafficSignState() == ETrafficSignState::SpeedLimit_90)
                {
                    return true;
                }
                else if (Signal.GetSubtype() == "90" &&
                    ClosestTrafficSign->GetTrafficSignState() == ETrafficSignState::SpeedLimit_90)
                {
                    return true;
                }
                else if (Signal.GetSubtype() == "100" &&
                    ClosestTrafficSign->GetTrafficSignState() == ETrafficSignState::SpeedLimit_100)
                {
                    return true;
                }
                else if (Signal.GetSubtype() == "120" &&
                    ClosestTrafficSign->GetTrafficSignState() == ETrafficSignState::SpeedLimit_120)
                {
                    return true;
                }
                else if (Signal.GetSubtype() == "130" &&
                    ClosestTrafficSign->GetTrafficSignState() == ETrafficSignState::SpeedLimit_130)
                {
                    return true;
                }
            }
        }
        return false;
    }

    template<typename T = ATrafficSignBase>
    T* GetClosestTrafficSignActor(const carla::road::Signal& Signal, UWorld* World)
    {
        auto CarlaTransform = Signal.GetTransform();
        FTransform UETransform(CarlaTransform);
        FVector Location = UETransform.GetLocation();
        // max distance to match 500cm
        constexpr float MaxDistanceMatchSqr = 250000.0;
        T* ClosestTrafficSign = nullptr;
        TArray<AActor*> Actors;
        UGameplayStatics::GetAllActorsOfClass(World, T::StaticClass(), Actors);
        float MinDistance = MaxDistanceMatchSqr;
        for (AActor* Actor : Actors)
        {
            float Dist = FVector::DistSquared(Actor->GetActorLocation(), Location);
            T* TrafficSign = Cast<T>(Actor);
            if (Dist < MinDistance && MatchSignalAndActor(Signal, TrafficSign))
            {
                ClosestTrafficSign = TrafficSign;
                MinDistance = Dist;
            }
        }
        return ClosestTrafficSign;
    }

    // Cached Spawned Signs
    static TArray<AActor*> SpawnedSigns;
    // Backup umap dirty flag
    static bool bBackupDirtyFlag = false;
    void SpawnSignalsAndTrafficLights(UWorld& World)
    {
        SpawnedSigns.Reset();

        if (UPackage* Package = World.GetOutermost())
        {
            // Clear the dirty flag on the package, marking it as not needing save
            bBackupDirtyFlag = Package->IsDirty();
        }

        boost::optional<carla::road::Map> XodrMap;
        if (!LoadXodrInternal(World, XodrMap))
        {
            return;
        }

        TSubclassOf<AActor> TrafficLightModel;
        TMap<FString, TSubclassOf<USignComponent>> SignComponentModels;
        TMap<FString, TSubclassOf<AActor>> TrafficSignsModels;
        TMap<FString, TSubclassOf<AActor>> SpeedLimitModels;
        bool bIsRHT = true;
        const auto& roads = XodrMap->GetRoads();

        if (!roads.empty()) {
            const auto& firstPair = *roads.begin();   
            const carla::road::Road& firstRoad = firstPair.second; 
            bIsRHT = firstRoad.IsRHT();
        }

        if( bIsRHT )
        {
            UClass* TrafficLightClass = LoadClass<AActor>(nullptr, TEXT("/Game/Carla/Blueprints/TrafficLight/BP_TLOpenDrive_RHT.BP_TLOpenDrive_RHT_C"));
            if (TrafficLightClass)
            {
                TrafficLightModel = TrafficLightClass;
            }
            else
            {
                UE_LOG(LogSimReadyCarlaWrapper, Error, TEXT("Failed to load traffic light model at '/Game/Carla/Blueprints/TrafficLight/BP_TLOpenDrive_RHT.BP_TLOpenDrive_RHT_C'"));
            }
        }
        else
        {
            UClass* TrafficLightClass = LoadClass<AActor>(nullptr, TEXT("/Game/Carla/Blueprints/TrafficLight/BP_TLOpenDrive_LHT.BP_TLOpenDrive_LHT_C"));
            if (TrafficLightClass)
            {
                TrafficLightModel = TrafficLightClass;
            }
            else
            {
                UE_LOG(LogSimReadyCarlaWrapper, Error, TEXT("Failed to load traffic light model at '/Game/Carla/Blueprints/TrafficLight/BP_TLOpenDrive_LHT.BP_TLOpenDrive_LHT_C'"));
            }
        }

        // Default traffic signs models
        UClass* StopClass = LoadClass<AActor>(nullptr, TEXT("/Game/Carla/Static/TrafficSign/BP_Stop.BP_Stop_C"));
        if (StopClass)
        {
            TrafficSignsModels.Add(carla::road::SignalType::StopSign().c_str(), StopClass);
            SignComponentModels.Add(carla::road::SignalType::StopSign().c_str(), UStopSignComponent::StaticClass());
        }
        else
        {
            UE_LOG(LogSimReadyCarlaWrapper, Error, TEXT("Failed to load stop sign model at '/Game/Carla/Static/TrafficSign/BP_Stop.BP_Stop_C'"));
        }
        UClass* YieldClass = LoadClass<AActor>(nullptr, TEXT("/Game/Carla/Static/TrafficSign/BP_Yield.BP_Yield_C"));
        if (YieldClass)
        {
            TrafficSignsModels.Add(carla::road::SignalType::YieldSign().c_str(), YieldClass);
            SignComponentModels.Add(carla::road::SignalType::YieldSign().c_str(), UYieldSignComponent::StaticClass());
        }
        else
        {
            UE_LOG(LogSimReadyCarlaWrapper, Error, TEXT("Failed to load yield sign model at '/Game/Carla/Static/TrafficSign/BP_Yield.BP_Yield_C'"));
        }
        UClass* SpeedLimit30Class = LoadClass<AActor>(nullptr, TEXT("/Game/Carla/Static/TrafficSign/BP_SpeedLimit30.BP_SpeedLimit30_C"));
        if (SpeedLimit30Class)
        {
            SpeedLimitModels.Add("30", SpeedLimit30Class);
        }
        else
        {
            UE_LOG(LogSimReadyCarlaWrapper, Error, TEXT("Failed to load speed limit 30 sign model at '/Game/Carla/Static/TrafficSign/BP_SpeedLimit30.BP_SpeedLimit30_C'"));
        }
        UClass* SpeedLimit40Class = LoadClass<AActor>(nullptr, TEXT("/Game/Carla/Static/TrafficSign/BP_SpeedLimit40.BP_SpeedLimit40_C"));
        if (SpeedLimit40Class)
        {
            SpeedLimitModels.Add("40", SpeedLimit40Class);
        }
        else
        {
            UE_LOG(LogSimReadyCarlaWrapper, Error, TEXT("Failed to load speed limit 40 sign model at '/Game/Carla/Static/TrafficSign/BP_SpeedLimit40.BP_SpeedLimit40_C'"));
        }
        UClass* SpeedLimit50Class = LoadClass<AActor>(nullptr, TEXT("/Game/Carla/Static/TrafficSign/BP_SpeedLimit50.BP_SpeedLimit50_C"));
        if (SpeedLimit50Class)
        {
            SpeedLimitModels.Add("50", SpeedLimit50Class);
        }
        else
        {
            UE_LOG(LogSimReadyCarlaWrapper, Error, TEXT("Failed to load speed limit 50 sign model at '/Game/Carla/Static/TrafficSign/BP_SpeedLimit50.BP_SpeedLimit50_C'"));
        }
        UClass* SpeedLimit60Class = LoadClass<AActor>(nullptr, TEXT("/Game/Carla/Static/TrafficSign/BP_SpeedLimit60.BP_SpeedLimit60_C"));
        if (SpeedLimit60Class)
        {
            SpeedLimitModels.Add("60", SpeedLimit60Class);
        }
        else
        {
            UE_LOG(LogSimReadyCarlaWrapper, Error, TEXT("Failed to load speed limit 60 sign model at '/Game/Carla/Static/TrafficSign/BP_SpeedLimit60.BP_SpeedLimit60_C'"));
        }
        UClass* SpeedLimit70Class = LoadClass<AActor>(nullptr, TEXT("/Game/Carla/Static/TrafficSign/BP_SpeedLimit70.BP_SpeedLimit70_C"));
        if (SpeedLimit70Class)
        {
            SpeedLimitModels.Add("70", SpeedLimit70Class);
        }
        else
        {
            UE_LOG(LogSimReadyCarlaWrapper, Error, TEXT("Failed to load speed limit 70 sign model at '/Game/Carla/Static/TrafficSign/BP_SpeedLimit70.BP_SpeedLimit70_C'"));
        }
        UClass* SpeedLimit80Class = LoadClass<AActor>(nullptr, TEXT("/Game/Carla/Static/TrafficSign/BP_SpeedLimit80.BP_SpeedLimit80_C"));
        if (SpeedLimit80Class)
        {
            SpeedLimitModels.Add("80", SpeedLimit80Class);
        }
        else
        {
            UE_LOG(LogSimReadyCarlaWrapper, Error, TEXT("Failed to load speed limit 80 sign model at '/Game/Carla/Static/TrafficSign/BP_SpeedLimit80.BP_SpeedLimit80_C'"));
        }
        UClass* SpeedLimit90Class = LoadClass<AActor>(nullptr, TEXT("/Game/Carla/Static/TrafficSign/BP_SpeedLimit90.BP_SpeedLimit90_C"));
        if (SpeedLimit90Class)
        {
            SpeedLimitModels.Add("90", SpeedLimit90Class);
        }
        else
        {
            UE_LOG(LogSimReadyCarlaWrapper, Error, TEXT("Failed to load speed limit 90 sign model at '/Game/Carla/Static/TrafficSign/BP_SpeedLimit90.BP_SpeedLimit90_C'"));
        }
        UClass* SpeedLimit100Class = LoadClass<AActor>(nullptr, TEXT("/Game/Carla/Static/TrafficSign/BP_SpeedLimit100.BP_SpeedLimit100_C"));
        if (SpeedLimit100Class)
        {
            SpeedLimitModels.Add("100", SpeedLimit100Class);
        }
        else
        {
            UE_LOG(LogSimReadyCarlaWrapper, Error, TEXT("Failed to load speed limit 100 sign model at '/Game/Carla/Static/TrafficSign/BP_SpeedLimit100.BP_SpeedLimit100_C'"));
        }
        UClass* SpeedLimit110Class = LoadClass<AActor>(nullptr, TEXT("/Game/Carla/Static/TrafficSign/BP_SpeedLimit110.BP_SpeedLimit110_C"));
        if (SpeedLimit110Class)
        {
            SpeedLimitModels.Add("110", SpeedLimit110Class);
        }
        else
        {
            UE_LOG(LogSimReadyCarlaWrapper, Error, TEXT("Failed to load speed limit 110 sign model at '/Game/Carla/Static/TrafficSign/BP_SpeedLimit110.BP_SpeedLimit110_C'"));
        }
        UClass* SpeedLimit120Class = LoadClass<AActor>(nullptr, TEXT("/Game/Carla/Static/TrafficSign/BP_SpeedLimit120.BP_SpeedLimit120_C"));
        if (SpeedLimit120Class)
        {
            SpeedLimitModels.Add("120", SpeedLimit120Class);
        }
        else
        {
            UE_LOG(LogSimReadyCarlaWrapper, Error, TEXT("Failed to load speed limit 120 sign model at '/Game/Carla/Static/TrafficSign/BP_SpeedLimit120.BP_SpeedLimit120_C'"));
        }

        const auto& Signals = XodrMap->GetSignals();
        std::unordered_set<std::string> SignalsToSpawn;
        for (const auto& ControllerPair : XodrMap->GetControllers())
        {
            const auto& Controller = ControllerPair.second;
            for (const auto& SignalId : Controller->GetSignals())
            {
                auto& Signal = Signals.at(SignalId);
                if (!cr::SignalType::IsTrafficLight(Signal->GetType()))
                {
                    continue;
                }
                ATrafficLightBase* TrafficLight = GetClosestTrafficSignActor<ATrafficLightBase>(
                    *Signal.get(), &World);
                if (!TrafficLight)
                {
                    SignalsToSpawn.insert(SignalId);
                }
            }
        }

        for (const auto& SignalPair : Signals)
        {
            const auto& SignalId = SignalPair.first;
            const auto& Signal = SignalPair.second;
            FString SignalType = Signal->GetType().c_str();

            if (!Signal->GetControllers().size() &&
                !XodrMap->IsJunction(Signal->GetRoadId()) &&
                carla::road::SignalType::IsTrafficLight(Signal->GetType()) &&
                !SignalsToSpawn.count(SignalId))
            {
                ATrafficLightBase* TrafficLight = GetClosestTrafficSignActor<ATrafficLightBase>(
                    *Signal.get(), &World);
                if (!TrafficLight)
                {
                    SignalsToSpawn.insert(SignalId);
                }
            }
        
            ATrafficSignBase* ClosestTrafficSign = GetClosestTrafficSignActor(*Signal.get(), &World);
            if (!ClosestTrafficSign)
            {
                auto CarlaTransform = Signal->GetTransform();
                FTransform SpawnTransform(CarlaTransform);
                FVector SpawnLocation = SpawnTransform.GetLocation();
                FRotator SpawnRotation(SpawnTransform.GetRotation());
                SpawnRotation.Yaw += 90;
                // Remove road inclination
                SpawnRotation.Roll = 0;
                SpawnRotation.Pitch = 0;

                if (TrafficSignsModels.Contains(SignalType))
                {
                    // We do not spawn stops painted in the ground
                    if (Signal->GetName() == "Stencil_STOP")
                    {
                        continue;
                    }

                    // Spawn stop and yield signs
                    ATrafficSignBase* TrafficSign = World.SpawnActor<ATrafficSignBase>(
                        TrafficSignsModels[SignalType],
                        SpawnLocation,
                        SpawnRotation);

                    USignComponent* SignComponent =
                        NewObject<USignComponent>(TrafficSign, SignComponentModels[SignalType]);
                    SignComponent->SetSignId(Signal->GetSignalId().c_str());
                    SignComponent->RegisterComponent();
                    SignComponent->AttachToComponent(
                        TrafficSign->GetRootComponent(),
                        FAttachmentTransformRules::KeepRelativeTransform);
                    SignComponent->InitializeSign(XodrMap.get());
                    SpawnedSigns.Add(TrafficSign);
                }
                else if (Signal->GetType() == carla::road::SignalType::MaximumSpeed() &&
                    SpeedLimitModels.Contains(Signal->GetSubtype().c_str()))
                {
                    // Spawn speed limit signs
                    ATrafficSignBase* TrafficSign = World.SpawnActor<ATrafficSignBase>(
                        SpeedLimitModels[Signal->GetSubtype().c_str()],
                        SpawnLocation,
                        SpawnRotation);

                    USpeedLimitComponent* SignComponent =
                        NewObject<USpeedLimitComponent>(TrafficSign);
                    SignComponent->SetSignId(Signal->GetSignalId().c_str());
                    SignComponent->RegisterComponent();
                    SignComponent->AttachToComponent(
                        TrafficSign->GetRootComponent(),
                        FAttachmentTransformRules::KeepRelativeTransform);
                    SignComponent->InitializeSign(XodrMap.get());
                    SignComponent->SetSpeedLimit(Signal->GetValue());
                    SpawnedSigns.Add(TrafficSign);
                }
            }
        }

        for (auto& SignalId : SignalsToSpawn)
        {
            // Spawn traffic lights
            if (Signals.count(SignalId) == 0)
            {
                UE_LOG(LogSimReadyCarlaWrapper, Warning,
                    TEXT("Possible OpenDRIVE error, reference to nonexistent signal id: %s"),
                    *carla::rpc::ToFString(SignalId));
                continue;
            }
            const auto& Signal = Signals.at(SignalId);
            auto CarlaTransform = Signal->GetTransform();
            FTransform SpawnTransform(CarlaTransform);

            FVector SpawnLocation = SpawnTransform.GetLocation();
            FRotator SpawnRotation(SpawnTransform.GetRotation());
            // Blueprints are all rotated by 90 degrees
            SpawnRotation.Yaw += 90;
            // Remove road inclination
            SpawnRotation.Roll = 0;
            SpawnRotation.Pitch = 0;

            ATrafficLightBase* TrafficLight = World.SpawnActor<ATrafficLightBase>(
                TrafficLightModel,
                SpawnLocation,
                SpawnRotation);

            UTrafficLightComponent* TrafficLightComponent = TrafficLight->GetTrafficLightComponent();
            TrafficLightComponent->SetSignId(SignalId.c_str());
            TrafficLightComponent->InitializeSign(XodrMap.get());
            SpawnedSigns.Add(TrafficLight);
        }
    }

    void DestroySignalsAndTrafficLights(UWorld& World)
    {
        // Destroy actors
        for (auto Sign : SpawnedSigns)
        {
            World.DestroyActor(Sign);
        }

        // Restore dirty
        if (UPackage* Package = World.GetOutermost())
        {
            // Clear the dirty flag on the package, marking it as not needing save
            if (!bBackupDirtyFlag)
            {
                Package->ClearDirtyFlag();
            }
        }

        SpawnedSigns.Reset();
    }
}
