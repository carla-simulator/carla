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
struct FSphereInMesh
{
  GENERATED_BODY()
  AActor* SpawnedActor {nullptr};
  int ComponentID {0};
  int InstanceIndex {0};
  FTransform OriginalTransform {FVector(0.0f, 0.0f, -1000000.0f)};
};

UENUM()
enum class EFoliageType : uint8 {

  None = 0,
  Tree = 1,
  Grass = 2,
  Rock = 3,
  Bush = 4
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

public:

  UFUNCTION(Category = "CARLA Wheeled Vehicle", BlueprintCallable)
  void UpdateSphereOverlap();

  UPROPERTY(Category = "CARLA Wheeled Vehicle", EditAnywhere, BlueprintReadWrite)
  float SphereRadius = 1000.0f;

  UPROPERTY(Category = "CARLA Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly)
  TArray<AActor*> SphereOverlappedActors;

  TArray<FSphereInMesh> MeshesInSphere;

  UFUNCTION()
  const float GetDistanceToInstancedMesh(const FTransform& transform) const
  {
    const FVector OtherLocation = transform.GetLocation();
    const FVector OwnLocation = GetActorLocation();

    const float Result = FVector::Distance(OwnLocation, OtherLocation);
    return Result;
  }

  UFUNCTION()
  bool IsMeshInList(const FSphereInMesh& sim)
  {
    for (const auto& element : MeshesInSphere)
    {
      if (element.ComponentID == sim.ComponentID && element.InstanceIndex == sim.InstanceIndex && element.SpawnedActor != nullptr)
        return true;
    }
    return false;
  }

  UFUNCTION()
  bool AddElementToList(const FSphereInMesh& sim)
  {
    for (auto& element : MeshesInSphere)
    {
      if (element.SpawnedActor == nullptr)
      {
        element = sim;
        return true;
      }
    }
    MeshesInSphere.Add(sim);
    return true;
  }

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

//Lista de objetos que he escondido, struct con componente, instancia y transformada.
//Para reaparecer cosas, comprobar si el componnent is valid
  UFUNCTION(BlueprintCallable)
  void HideOverlappedMeshes(AActor* actor)
  {
    const TSet<UActorComponent*>& Components = actor->GetComponents();
    for (UActorComponent* Component : Components)
    {
      if (UInstancedStaticMeshComponent* Mesh = Cast<UInstancedStaticMeshComponent>(Component))
      {
        UObject* Object = Mesh->GetStaticMesh();
        const FString Path = Object->GetPathName();
        TArray<int32> InstancesInOverlappingSphere = Mesh->GetInstancesOverlappingSphere(GetActorLocation(), SphereRadius, false);
        //for (int i = Mesh->GetInstanceCount() - 1; i >= 0; i--)
        for (int32 i : InstancesInOverlappingSphere)
        {     
          //TODO: detectar malla especifica del tree.
          //TODO: Crear BP base que tenga springbase, y que tenga una skeletal-mesh
          /*
          En cada carpeta (rock, tree...) que haya un BP que herede del base
          Nombre del bp igual al de la malla igual. BP_malla

          Coger el nombre de la mesh, y buscar ene l content el bp asocidado y spawnear si existe.

          Hacer la esfera lo más pqeuaña posible -> Done.
          */
          EFoliageType type = EFoliageType::None;     
          if (Path.Contains("Rock"))
          {
            type = EFoliageType::Rock;
            continue;
          } else  if (Path.Contains("Tree"))
          {          
            type = EFoliageType::Tree;

          } else if (Path.Contains("BushSalt"))
          {
            type = EFoliageType::Bush;
          } 
          else if(Path.Contains("Cactus"))
          {
            type = EFoliageType::Grass;
          } else {
            //GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, Path);
            continue;
          }
          FTransform transform;
          Mesh->GetInstanceTransform(i, transform, false);
          //const float d = GetDistanceToInstancedMesh(transform);
          //if (d < SphereRadius)
          {
            FSphereInMesh sim;
            sim.ComponentID = Mesh->GetUniqueID();
            sim.InstanceIndex = i;
            sim.OriginalTransform = transform;
            if (!IsMeshInList(sim))
            {
              sim.SpawnedActor = SpawnFoliage(transform, type, Path);
              //sim.SpawnedActor = SpawnFoliageBP(transform, Path);
              if (sim.SpawnedActor)
              {
                bool added = AddElementToList(sim);
                FTransform aux {transform};
                aux.SetLocation({0.0f, 0.0f, -1000000.0f});
                Mesh->UpdateInstanceTransform(i, aux);
                if (added)
                  GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, FString::Printf(TEXT("Added element to Array")));
              }
            }
          }
        }
      }
    }
  }

  UFUNCTION()
  AActor* SpawnFoliage(const FTransform& transform, EFoliageType type, FString Path)
  {
    ///Game/Racer-Sim/Static/Vegetation/Tree/TreeJoshua_01/SM_D_TJoshua_01_v02.SM_D_TJoshua_01_v02
    /*
    1 - Coger path hasta el folder: /Game/Racer-Sim/Static/Vegetation/Tree/TreeJoshua_01/
    2 - Coger nombre de la carpeta: TreeJoshua_01
    2 - Sacar la version del asset: ..._vxx
    3 - Formar string con el nuevo path:
      - Path hasta folder: /Game/Racer-Sim/Static/Vegetation/Tree/TreeJoshua_01/
      - Blueprint Identifier: BP_
      - Nombre de la carpeta: TreeJoshua_01
      - Nombre de la version: _v02
      - Nombre final: /Game/Racer-Sim/Static/Vegetation/Tree/TreeJoshua_01/BP_TreeJoshua_01_v02
    */
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
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("FAILED TO GET THE VERSION")));
      return nullptr;
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
    //GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, Path);
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, FullClassPath);

    
    TSubclassOf<AActor> SpawnedClass;
    UObject* LoadedObject = StaticLoadObject(UObject::StaticClass(), nullptr, *FullClassPath);    
    UBlueprint* CastedBlueprint = Cast<UBlueprint>(LoadedObject);            
    if (CastedBlueprint && CastedBlueprint->GeneratedClass->IsChildOf(AActor::StaticClass()))
    {
      SpawnedClass = *CastedBlueprint->GeneratedClass;
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString::Printf(TEXT("CLASS FOUND FROM PATH")));

      AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(SpawnedClass, transform.GetLocation(), transform.Rotator());
      SpawnedActor->SetActorScale3D({3.0f, 3.0f, 3.0f});
      return SpawnedActor;
    }
    else
    {
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("CLASS NOT FOUND FROM PATH")));
    }
    return nullptr;
  }

  UFUNCTION()
  void HideFoliage(AActor* ActorToHide)
  {/*
    const FVector InactiveLocation {0.0f, 0.0f, -1000000.0f};
    for (AActor* actor : BushPool)
    {
      if (actor == ActorToHide)
      {
        actor->SetActorTransform(FTransform());
        actor->SetActorLocation(InactiveLocation);
        return;
      }
    }
    for (AActor* actor : TreePool)
    {
      if (actor == ActorToHide)
      {
        actor->SetActorTransform(FTransform());
        actor->SetActorLocation(InactiveLocation);
        return;
      }
    }*/
  }

  UFUNCTION(BlueprintCallable)
  void ShowNonOverlappedMeshes(AActor* actor)
  {/*
    const TSet<UActorComponent*>& components = actor->GetComponents();
    for (UActorComponent* component : components)
    {
      if (UInstancedStaticMeshComponent* Mesh = Cast<UInstancedStaticMeshComponent>(component))
      {
        for (FSphereInMesh& element : MeshesInSphere)
        {
          if (element.ComponentID == Mesh->GetUniqueID())
          {
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Found instance in Array")));
            const float DistanceToOriginalMesh = GetDistanceToInstancedMesh(element.OriginalTransform);
            if (DistanceToOriginalMesh > SphereRadius)
            {
              Mesh->UpdateInstanceTransform(element.InstanceIndex, element.OriginalTransform);
              element.SpawnedActor->Destroy();
              //HideFoliage(element.SpawnedActor);
              element = FSphereInMesh();
              GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Deleted element from Array")));
            }
          }
        }
      }
    }*/
  }

  UFUNCTION(BlueprintCallable)
  FORCEINLINE bool IsAnyActorInSphere()
  {
    return SphereOverlappedActors.Num() > 0;
  }

  UFUNCTION(BlueprintCallable)
  FORCEINLINE int GetSphereOverlappedActorsNum()
  {
    return static_cast<int>(SphereOverlappedActors.Num() - 1);
  }

  UFUNCTION(BlueprintCallable)
  FORCEINLINE TArray<int32> GetSphereOverlappedActorsIDs()
  {
    TArray<int32> results;
    for (const AActor* const actor : SphereOverlappedActors)
    {
      results.Add(actor->GetUniqueID());
    }
    return results;
  }

  UFUNCTION(BlueprintCallable)
  FORCEINLINE TArray<FString> GetSphereOverlappedActorsNames()
  {
    TArray<FString> results;
    for (const AActor* const actor : SphereOverlappedActors)
    {
      results.Add(actor->GetName());
    }
    return results;
  }

  UFUNCTION(BlueprintCallable)
  FORCEINLINE TArray<UClass*> GetSphereOverlappedActorsClasses()
  {
    TArray<UClass*> results;
    for (const AActor* const actor : SphereOverlappedActors)
    {
      results.Add(actor->GetClass());
    }
    return results;
  }

  UFUNCTION(BlueprintCallable)
  FORCEINLINE TArray<FString> GetActorComponents(const AActor* actor)
  {
    TArray<FString> results;
    const TSet<UActorComponent*>& components = actor->GetComponents(); 

    for (const auto* component : components)
    {
      results.Add(component->GetReadableName());
    }
    return results;
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

};
