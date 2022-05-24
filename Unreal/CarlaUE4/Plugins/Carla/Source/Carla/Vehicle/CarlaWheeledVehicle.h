// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "WheeledVehicle.h"

#include "Vehicle/AckermannController.h"
#include "Vehicle/AckermannControllerSettings.h"
#include "Vehicle/CarlaWheeledVehicleState.h"
#include "Vehicle/VehicleAckermannControl.h"
#include "Vehicle/VehicleControl.h"
#include "Vehicle/VehicleLightState.h"
#include "Vehicle/VehicleInputPriority.h"
#include "Vehicle/VehiclePhysicsControl.h"
#include "VehicleVelocityControl.h"
#include "WheeledVehicleMovementComponent4W.h"
#include "WheeledVehicleMovementComponentNW.h"
#include "VehicleAnimInstance.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "MovementComponents/BaseCarlaMovementComponent.h"
 	

#include "FoliageInstancedStaticMeshComponent.h"
 	

#include "UObject/ConstructorHelpers.h"

#include "Containers/Array.h"
#include "Containers/Map.h"

#include "CoreMinimal.h"

//-----CARSIM--------------------------------
#ifdef WITH_CARSIM
#include "CarSimMovementComponent.h"
#endif
//-------------------------------------------

#include <utility>

#include "carla/rpc/VehicleFailureState.h"
#include "CarlaWheeledVehicle.generated.h"

class UBoxComponent;

UENUM()
enum class EVehicleWheelLocation : uint8 {

  FL_Wheel = 0,
  FR_Wheel = 1,
  BL_Wheel = 2,
  BR_Wheel = 3,
  //Use for bikes and bicycles
  Front_Wheel = 0,
  Back_Wheel = 1,
};

/// Type of door to open/close
// When adding new door types, make sure that All is the last one.
UENUM(BlueprintType)
enum class EVehicleDoor : uint8 {
  FL = 0,
  FR = 1,
  RL = 2,
  RR = 3,
  Hood = 4,
  Trunk = 5,
  All = 6
};

USTRUCT()
struct FSpawnedFoliage
{
  GENERATED_BODY()
  AActor* Actor {nullptr};
  UInstancedStaticMeshComponent* Owner {nullptr};
  int InstanceIndex {-1};
  FString FoliageType {FString()};
  FTransform Transform {FVector(0.0f, 0.0f, -1000000.0f)};
};

USTRUCT()
struct FCacheBPClasses
{
  GENERATED_BODY()
  FString Path;
  TSubclassOf<AActor> SpawnedClass = nullptr;
};

/// Base class for CARLA wheeled vehicles.
UCLASS()
class CARLA_API ACarlaWheeledVehicle : public AWheeledVehicle
{
  GENERATED_BODY()

  // ===========================================================================
  /// @name Constructor and destructor
  // ===========================================================================
  /// @{
public:

  ACarlaWheeledVehicle(const FObjectInitializer &ObjectInitializer);

  ~ACarlaWheeledVehicle();

  /// @}
  // ===========================================================================
  /// @name Get functions
  // ===========================================================================
  /// @{
public:

  /// Vehicle control currently applied to this vehicle.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  const FVehicleControl &GetVehicleControl() const
  {
    return LastAppliedControl;
  }

  /// Vehicle Ackermann control currently applied to this vehicle.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  const FVehicleAckermannControl &GetVehicleAckermannControl() const
  {
    return LastAppliedAckermannControl;
  }

  /// Transform of the vehicle. Location is shifted so it matches center of the
  /// vehicle bounds rather than the actor's location.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FTransform GetVehicleTransform() const
  {
    return GetActorTransform();
  }

  /// Forward speed in cm/s. Might be negative if goes backwards.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  float GetVehicleForwardSpeed() const;

  /// Orientation vector of the vehicle, pointing forward.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FVector GetVehicleOrientation() const;

  /// Active gear of the vehicle.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  int32 GetVehicleCurrentGear() const;

  /// Transform of the vehicle's bounding box relative to the vehicle.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FTransform GetVehicleBoundingBoxTransform() const;

  /// Extent of the vehicle's bounding box.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FVector GetVehicleBoundingBoxExtent() const;

  /// Get vehicle's bounding box component.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  UBoxComponent *GetVehicleBoundingBox() const
  {
    return VehicleBounds;
  }

  /// Get the maximum angle at which the front wheel can steer.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  float GetMaximumSteerAngle() const;

  /// @}
  // ===========================================================================
  /// @name AI debug state
  // ===========================================================================
  /// @{
public:

  /// @todo This function should be private to AWheeledVehicleAIController.
  void SetAIVehicleState(ECarlaWheeledVehicleState InState)
  {
    State = InState;
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  ECarlaWheeledVehicleState GetAIVehicleState() const
  {
    return State;
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FVehiclePhysicsControl GetVehiclePhysicsControl() const;

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FAckermannControllerSettings GetAckermannControllerSettings() const {
    return AckermannController.GetSettings();
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void RestoreVehiclePhysicsControl();

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  FVehicleLightState GetVehicleLightState() const;

  void ApplyVehiclePhysicsControl(const FVehiclePhysicsControl &PhysicsControl);

  void ApplyAckermannControllerSettings(const FAckermannControllerSettings &AckermannControllerSettings) {
    return AckermannController.ApplySettings(AckermannControllerSettings);
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetSimulatePhysics(bool enabled);

  void SetWheelCollision(UWheeledVehicleMovementComponent4W *Vehicle4W, const FVehiclePhysicsControl &PhysicsControl);

  void SetWheelCollisionNW(UWheeledVehicleMovementComponentNW *VehicleNW, const FVehiclePhysicsControl &PhysicsControl);

  void SetVehicleLightState(const FVehicleLightState &LightState);

  void SetFailureState(const carla::rpc::VehicleFailureState &FailureState);

  UFUNCTION(BlueprintNativeEvent)
  bool IsTwoWheeledVehicle();
  virtual bool IsTwoWheeledVehicle_Implementation() {
    return false;
  }

  /// @}
  // ===========================================================================
  /// @name Vehicle input control
  // ===========================================================================
  /// @{
public:

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ApplyVehicleControl(const FVehicleControl &Control, EVehicleInputPriority Priority)
  {
    if (bAckermannControlActive) {
      AckermannController.Reset();
    }
    bAckermannControlActive = false;

    if (InputControl.Priority <= Priority)
    {
      InputControl.Control = Control;
      InputControl.Priority = Priority;
    }
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ApplyVehicleAckermannControl(const FVehicleAckermannControl &AckermannControl, EVehicleInputPriority Priority)
  {
    bAckermannControlActive = true;
    LastAppliedAckermannControl = AckermannControl;
    AckermannController.SetTargetPoint(AckermannControl);
  }

  bool IsAckermannControlActive() const
  {
    return bAckermannControlActive;
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ActivateVelocityControl(const FVector &Velocity);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void DeactivateVelocityControl();

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ShowDebugTelemetry(bool Enabled);

  /// @todo This function should be private to AWheeledVehicleAIController.
  void FlushVehicleControl();

  /// @}
  // ===========================================================================
  /// @name DEPRECATED Set functions
  // ===========================================================================
  /// @{
public:

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetThrottleInput(float Value);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetSteeringInput(float Value);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetBrakeInput(float Value);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetReverse(bool Value);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ToggleReverse()
  {
    SetReverse(!LastAppliedControl.bReverse);
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetHandbrakeInput(bool Value);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void HoldHandbrake()
  {
    SetHandbrakeInput(true);
  }

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ReleaseHandbrake()
  {
    SetHandbrakeInput(false);
  }

  TArray<float> GetWheelsFrictionScale();

  void SetWheelsFrictionScale(TArray<float> &WheelsFrictionScale);

  void SetCarlaMovementComponent(UBaseCarlaMovementComponent* MoementComponent);

  template<typename T = UBaseCarlaMovementComponent>
  T* GetCarlaMovementComponent() const
  {
    return Cast<T>(BaseMovementComponent);
  }

  /// @}
  // ===========================================================================
  /// @name Overriden from AActor
  // ===========================================================================
  /// @{

protected:

  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

  UFUNCTION(BlueprintImplementableEvent)
  void RefreshLightState(const FVehicleLightState &VehicleLightState);

  UFUNCTION(BlueprintCallable, CallInEditor)
  void AdjustVehicleBounds();

  UPROPERTY(Category="Door Animation", EditAnywhere, BlueprintReadWrite)
  TArray<FName> ConstraintComponentNames;

  UPROPERTY(Category="Door Animation", EditAnywhere, BlueprintReadWrite)
  float DoorOpenStrength = 100.0f;

  UFUNCTION(BlueprintCallable, CallInEditor)
  void ResetConstraints();

private:

  /// Current state of the vehicle controller (for debugging purposes).
  UPROPERTY(Category = "AI Controller", VisibleAnywhere)
  ECarlaWheeledVehicleState State = ECarlaWheeledVehicleState::UNKNOWN;

  UPROPERTY(Category = "CARLA Wheeled Vehicle", EditAnywhere)
  UBoxComponent *VehicleBounds;  

  UPROPERTY(Category = "CARLA Wheeled Vehicle", EditAnywhere)
  UVehicleVelocityControl* VelocityControl;

  struct
  {
    EVehicleInputPriority Priority = EVehicleInputPriority::INVALID;
    FVehicleControl Control;
    FVehicleLightState LightState;
  }
  InputControl;

  FVehicleControl LastAppliedControl;
  FVehicleAckermannControl LastAppliedAckermannControl;
  FVehiclePhysicsControl LastPhysicsControl;

  bool bAckermannControlActive = false;
  FAckermannController AckermannController;

  float RolloverBehaviorForce = 0.35;
  int RolloverBehaviorTracker = 0;
  float RolloverFlagTime = 5.0f;

  carla::rpc::VehicleFailureState FailureState = carla::rpc::VehicleFailureState::None;

public:
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void UpdateProceduralFoliage();
    
  UPROPERTY(Category = "CARLA Wheeled Vehicle", EditAnywhere, BlueprintReadWrite)
  FVector DetectionSize { 3.0f, 5.0f, 1.0f};
  
  UPROPERTY(Category = "CARLA Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly)
  FBox FoliageBoundingBox;

  //Filters for debug, improving performance
  UPROPERTY(Category = "CARLA Wheeled Vehicle", EditAnywhere, BlueprintReadWrite)
  bool SpawnBushes = true;

  UPROPERTY(Category = "CARLA Wheeled Vehicle", EditAnywhere, BlueprintReadWrite)
  bool SpawnTrees = true;

  UPROPERTY(Category = "CARLA Wheeled Vehicle", EditAnywhere, BlueprintReadWrite)
  bool SpawnRocks = true;

  UPROPERTY(Category = "CARLA Wheeled Vehicle", EditAnywhere, BlueprintReadWrite)
  bool SpawnPlants = true;

  UPROPERTY(Category = "CARLA Wheeled Vehicle", EditAnywhere, BlueprintReadWrite)
  float SpawnScale = 0.0f;

  TArray<AActor*> OverlappedActors;
  TArray<FCacheBPClasses> FoliageCache;
  TArray<FSpawnedFoliage> MeshesCloseToVehicle;
  TMap<FString, TArray<AActor*>> MeshesPool;

  //Cache functions
  UFUNCTION()
  bool IsPathInCache(const FString& Path)
  {
    for (auto& element : FoliageCache)
    {
      if (element.Path == Path)
      {
        return true;
      }
    }
    return false;
  }

  UFUNCTION()
  TSubclassOf<AActor> GetFromCache(const FString& Path)
  {
    for (auto& element : FoliageCache)
    {
      if (element.Path == Path)
      {
        return element.SpawnedClass;
      }
    }
    return nullptr;
  }

  //List with items near vehicle functions.
  UFUNCTION()
  bool IsInMeshesCloseToVehicle(const FSpawnedFoliage& SpawnedFoliage)
  {
    for (const auto& element : MeshesCloseToVehicle)
    {
      if (element.Actor != nullptr && element.Actor->IsActorTickEnabled() && element.Owner == SpawnedFoliage.Owner && element.InstanceIndex ==  SpawnedFoliage.InstanceIndex)
        return true;
    }
    return false;
  }

  UFUNCTION()
  bool AddToMeshesCloseToVehicle(const FSpawnedFoliage& SpawnedFoliage)
  {
    for (auto& element : MeshesCloseToVehicle)
    {
      if (element.Actor == nullptr || !element.Actor->IsActorTickEnabled())
      {
        element = SpawnedFoliage;
        return true;
      }
    }
    MeshesCloseToVehicle.Add(SpawnedFoliage);
    return true;
  }

  //Spawner system functions.
  UFUNCTION(BlueprintCallable)
  void SpawnSkeletalMeshesInDetectionRange()
  {
    for (AActor* OverlappedActor : OverlappedActors)
    {
      const TSet<UActorComponent*>& Components = OverlappedActor->GetComponents();
      for (UActorComponent* Component : Components)
      {
        if (UInstancedStaticMeshComponent* Mesh = Cast<UInstancedStaticMeshComponent>(Component))
        {
          const FString Path = Mesh->GetStaticMesh()->GetPathName();
          const TArray<int32> InstancesInOverlapping = Mesh->GetInstancesOverlappingBox(FoliageBoundingBox);
          for (int32 i : InstancesInOverlapping)
          {
            if (!IsFoliageTypeEnabled(Path))
              continue;
            
            FSpawnedFoliage SpawnedFoliage;
            SpawnedFoliage.Owner = Mesh;
            SpawnedFoliage.InstanceIndex = i;

            if (!IsInMeshesCloseToVehicle(SpawnedFoliage))
            {
              FTransform Transform {};
              Mesh->GetInstanceTransform(i, Transform, false);
              SpawnedFoliage.Transform = Transform;
              SpawnedFoliage.Actor = GetFoliageFromPool(SpawnedFoliage, Path);
              if (SpawnedFoliage.Actor)
              {
                SpawnedFoliage.FoliageType = GetClassFromPath(Path);
                bool added = AddToMeshesCloseToVehicle(SpawnedFoliage);
                Transform.SetLocation({0.0f, 0.0f, -1000000.0f});
                Mesh->UpdateInstanceTransform(i, Transform);
              }
            }
          }
        }
      }
    }
  }

  UFUNCTION(BlueprintCallable)
  void HideSkeletalMeshesOutOfDetectionRange()
  {
    for (FSpawnedFoliage& element : MeshesCloseToVehicle)
    {
      if (element.Actor == nullptr || !element.Actor->IsActorTickEnabled())
        continue;
      if (!IsValid(element.Owner))
      {
        AddFoliageBackToPool(element.Actor);
        element = FSpawnedFoliage();
      }
      else
      {
        if(!FoliageBoundingBox.IsInside(element.Transform.GetLocation()))
        {
          element.Owner->UpdateInstanceTransform(element.InstanceIndex, element.Transform);
          AddFoliageBackToPool(element.Actor);
          element = FSpawnedFoliage();
        }
      }
    }
  }

  UFUNCTION()
  void AddFoliageBackToPool(AActor* Actor)
  {
    DisableActor(Actor);
  }

  UFUNCTION()
  AActor* GetFoliageFromPool(const FSpawnedFoliage& Foliage, const FString& Path)
  {
    TArray<AActor*>* Values = MeshesPool.Find(Path);    
    if (Values)
    {
      const FString BPClassName = GetClassFromPath(Path);
      for (AActor* Value : *Values)
      {
        if (Value->IsActorTickEnabled())
          continue;
        
        EnableActor(Value, Foliage.Transform);
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, "Get Element from Pool: " + BPClassName);
        return Value;
      }
      AActor* SpawnedActor = SpawnFoliage(Path, Foliage.Transform);
      if (SpawnedActor)
      {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, "Added Element to Pool " + BPClassName);
        Values->Add(SpawnedActor);

        for (int i = 0; i < 4; ++i)
        {
          AActor* Aux = SpawnFoliage(Path, Foliage.Transform);
          DisableActor(Aux);
          Values->Add(Aux);
        }
        return SpawnedActor;
      }
      else
      {
        return nullptr;
      } 
    }
    else
    {
      AActor* SpawnedActor = SpawnFoliage(Path, Foliage.Transform);
      if (SpawnedActor)
      {
        const FString BPClassName = GetClassFromPath(Path);
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, "Added New Element to Pool " + BPClassName);
        TArray<AActor*> NewArray { SpawnedActor};
        for (int i = 0; i < 4; ++i)
        {
          AActor* Aux = SpawnFoliage(Path, Foliage.Transform);
          DisableActor(Aux);
          NewArray.Add(Aux);
        }
        MeshesPool.Add(Path, NewArray);
        return SpawnedActor;
      }
      return nullptr;      
    }
  }

  UFUNCTION()
  AActor* SpawnFoliage(const FString& Path, const FTransform& Transform)
  {
    TSubclassOf<AActor> SpawnedClass = nullptr;
    if (IsPathInCache(Path))
    {
      SpawnedClass = GetFromCache(Path);
      if (SpawnedClass == nullptr)
        return nullptr;      
    }
    else
    {
      FString FullClassPath = GetClassFromPath(Path);
      if (FullClassPath.IsEmpty())
      {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("FAILED TO GET THE VERSION")));
        FoliageCache.Add({Path, nullptr});
        return nullptr;
      }
      
      UObject* LoadedObject = StaticLoadObject(UObject::StaticClass(), nullptr, *FullClassPath);    
      UBlueprint* CastedBlueprint = Cast<UBlueprint>(LoadedObject);            
      if (CastedBlueprint && CastedBlueprint->GeneratedClass->IsChildOf(AActor::StaticClass()))
      {
        SpawnedClass = *CastedBlueprint->GeneratedClass;
        FoliageCache.Add({Path, SpawnedClass});
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "BP Class found: " + FullClassPath);
      }
    }
    if (SpawnedClass == nullptr)
      return nullptr;
    AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(SpawnedClass, Transform.GetLocation(), Transform.Rotator());
    if (SpawnScale > 1.001f || SpawnScale < 0.999f)
      SpawnedActor->SetActorScale3D({SpawnScale, SpawnScale, SpawnScale});
    else
      SpawnedActor->SetActorScale3D(Transform.GetScale3D());
    return SpawnedActor;
  }

  UFUNCTION()
  void EnableActor(AActor* Actor, const FTransform& Transform)
  {
    Actor->SetActorRotation(Transform.Rotator());
    Actor->SetActorLocation(Transform.GetLocation());
    if (SpawnScale > 1.001f || SpawnScale < 0.999f)
      Actor->SetActorScale3D({SpawnScale, SpawnScale, SpawnScale});
    else
      Actor->SetActorScale3D(Transform.GetScale3D());
    Actor->SetTickableWhenPaused(true);
    Actor->SetActorTickEnabled(true);
    Actor->SetActorHiddenInGame(false);
    Actor->SetActorEnableCollision(true);
  }

  UFUNCTION()
  void DisableActor(AActor* Actor)
  {
    Actor->SetActorLocation({0.0f, 0.0f, -1000000.0f});
    Actor->SetActorEnableCollision(false);
    Actor->SetActorHiddenInGame(true);
    Actor->SetTickableWhenPaused(false);
    Actor->SetActorTickEnabled(false);
  }

  UFUNCTION()
  FString GetClassFromPath(const FString& Path)
  {
    TArray< FString > ParsedString;
    Path.ParseIntoArray(ParsedString, TEXT("/"), false);
    int Position = ParsedString.Num() - 1;
    const FString Version = ParsedString[Position];
    --Position;
    const FString Folder = ParsedString[Position];
    ++Position;
    const FString FullVersion = GetVersionFromFString(Version);
    if (FullVersion.IsEmpty())
    {
      return FString();
    }
    FString ClassPath = "BP_" + Folder + FullVersion;
    FString FullClassPath = "Blueprint'";
    for (int i = 0; i < Position; ++i)
    {
      FullClassPath += ParsedString[i];
      FullClassPath += '/';
    }
    FullClassPath += ClassPath;
    FullClassPath += ".";
    FullClassPath += ClassPath;
    FullClassPath += "'";

    return FullClassPath;
  }

  UFUNCTION()
  bool IsFoliageTypeEnabled(const FString& Path)
  {
    if (!SpawnRocks)
      if (Path.Contains("Rock"))
        return false;
    if (!SpawnTrees)
      if (Path.Contains("Tree"))
        return false;
    if (!SpawnBushes)
      if (Path.Contains("Bush"))
        return false;
    if (!SpawnPlants)
      if (Path.Contains("Plant"))
        return false;
    return true;
  }
  //Debug
  UFUNCTION()
  FString GetVersionFromFString(const FString& string)
  {
    auto IsDigit = [](TCHAR charToTest) {
        if (charToTest == TCHAR('0')) return true;
        if (charToTest == TCHAR('1')) return true;
        if (charToTest == TCHAR('2')) return true;
        if (charToTest == TCHAR('3')) return true;
        if (charToTest == TCHAR('4')) return true;
        if (charToTest == TCHAR('5')) return true;
        if (charToTest == TCHAR('6')) return true;
        if (charToTest == TCHAR('7')) return true;
        if (charToTest == TCHAR('8')) return true;
        if (charToTest == TCHAR('9')) return true;
        return false;
    };
    int index = string.Find(TEXT("_v"));
    if (index != -1)
    {
      index += 2;
      FString Version = "_v";
      while(IsDigit(string[index]))
      {
        Version += string[index];
        ++index;
        if (index == string.Len())
          return Version;
      }
      return Version;
    }
    return FString();
  }

  UFUNCTION(BlueprintCallable)
  TArray<FString> GetPoolsState()
  {
    TArray<FString> Results;
    for (const auto& Pool : MeshesPool)
    {
      const FString PoolName = GetClassFromPath(Pool.Key);
      int Elements = 0;
      int Active = 0;
      for (AActor* Actor : Pool.Value)
      {
        if (Actor->IsActorTickEnabled())
          ++Active;
        ++Elements;
      }
      const FString Result = PoolName + ": " + FString::FromInt(Elements) + " Elements. (" + FString::FromInt(Active) + " Active and " + FString::FromInt(Elements - Active) + " inActive)";
      Results.Add(Result);
    }
    return Results;
  }

  UFUNCTION(BlueprintCallable)
  TArray<FString> GetDetectionState()
  {
    TArray<FString> Results;
    int Count = 0;
    Results.Add(FString::FromInt(Count));
    for (const auto& Element : MeshesCloseToVehicle)
    {
      ++Count;
      Results.Add(Element.FoliageType);
    }
    Results[0] = "Total Meshes in the Detection Range: " + FString::FromInt(Count);
    return Results;
  }

  /// Set the rotation of the car wheels indicated by the user
  /// 0 = FL_VehicleWheel, 1 = FR_VehicleWheel, 2 = BL_VehicleWheel, 3 = BR_VehicleWheel
  /// NOTE : This is purely aesthetic. It will not modify the physics of the car in any way
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void SetWheelSteerDirection(EVehicleWheelLocation WheelLocation, float AngleInDeg);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  float GetWheelSteerAngle(EVehicleWheelLocation WheelLocation);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void OpenDoor(const EVehicleDoor DoorIdx);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void CloseDoor(const EVehicleDoor DoorIdx);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void OpenDoorPhys(const EVehicleDoor DoorIdx);

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void CloseDoorPhys(const EVehicleDoor DoorIdx);

  virtual FVector GetVelocity() const override;

//-----CARSIM--------------------------------
  UPROPERTY(Category="CARLA Wheeled Vehicle", EditAnywhere)
  float CarSimOriginOffset = 150.f;
//-------------------------------------------

  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere)
  bool bIsNWVehicle = false;  

  void SetRolloverFlag();

  carla::rpc::VehicleFailureState GetFailureState() const;

private:

  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere)
  bool bPhysicsEnabled = true;

  // Small workarround to allow optional CarSim plugin usage
  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  UBaseCarlaMovementComponent * BaseMovementComponent = nullptr;

  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  TArray<UPhysicsConstraintComponent*> ConstraintsComponents;

  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  TMap<UPhysicsConstraintComponent*, UPrimitiveComponent*> ConstraintDoor;

  // container of the initial transform of the door, used to reset its position
  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  TMap<UPrimitiveComponent*, FTransform> DoorComponentsTransform;

  UPROPERTY(Category="CARLA Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  TMap<UPrimitiveComponent*, UPhysicsConstraintComponent*> CollisionDisableConstraints;

  /// Rollovers tend to have too much angular velocity, resulting in the vehicle doing a full 360º flip.
  /// This function progressively reduces the vehicle's angular velocity so that it ends up upside down instead.
  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void ApplyRolloverBehavior();

  void CheckRollover(const float roll, const std::pair<float, float> threshold_roll);

  FTimerHandle TimerHandler;
};
