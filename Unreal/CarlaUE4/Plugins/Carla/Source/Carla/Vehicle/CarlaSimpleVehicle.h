// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
PRAGMA_DISABLE_DEPRECATION_WARNINGS

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MovementComponents/BaseCarlaMovementComponent.h"
#include "DisplayDebugHelpers.h"

#include "CarlaSimpleVehicle.generated.h"


UCLASS()
class CARLA_API ACarlaSimpleVehicle : public APawn
{
  GENERATED_BODY()

public:
  ACarlaSimpleVehicle(const FObjectInitializer& ObjectInitializer);
  ~ACarlaSimpleVehicle();

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

public:
  // Called every frame
  virtual void Tick(float DeltaTime) override;

  /**  The main skeletal mesh associated with this Vehicle */
  UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* Mesh;

  /** vehicle simulation component */
  UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class UWheeledVehicleMovementComponent* VehicleMovement;


public:

  /** Name of the MeshComponent. Use this name if you want to prevent creation of the component (with ObjectInitializer.DoNotCreateDefaultSubobject). */
  static FName VehicleMeshComponentName;

  /** Name of the VehicleMovement. Use this name if you want to use a different class (with ObjectInitializer.SetDefaultSubobjectClass). */
  static FName VehicleMovementComponentName;

  /** Util to get the wheeled vehicle movement component */
  class UWheeledVehicleMovementComponent* GetVehicleMovementComponent() const;

  //~ Begin AActor Interface
  virtual void DisplayDebug(class UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
  //~ End Actor Interface

  /** Returns Mesh subobject **/
  class USkeletalMeshComponent* GetMesh() const { return Mesh; }
  /** Returns VehicleMovement subobject **/
  class UWheeledVehicleMovementComponent* GetVehicleMovement() const { return VehicleMovement; }


private:

  UPROPERTY(Category = "CARLA Simple Wheeled Vehicle", VisibleAnywhere)
    bool bPhysicsEnabled = true;

  // Small workarround to allow optional CarSim plugin usage
  UPROPERTY(Category = "CARLA Simple Wheeled Vehicle", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UBaseCarlaMovementComponent* BaseMovementComponent = nullptr;
};


PRAGMA_ENABLE_DEPRECATION_WARNINGS
