// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/SpectatorPawn.h"
#include "CarlaSpectatorPawn.generated.h"


UENUM(BlueprintType)
enum class ESpectatorControlMode : uint8
{
  MANUAL_CONTROL  UMETA(DisplayName = "Controlled by player input"),
  FOLLOW_CONTROL  UMETA(DisplayName = "Following a target") 
};



UCLASS(config = Game, Blueprintable, BlueprintType)
class ACarlaSpectatorPawn : public ASpectatorPawn
{
	GENERATED_UCLASS_BODY()

    FORCEINLINE bool TraceSphere(const FVector& Start, const FVector& End,const float Radius,FHitResult& HitOut,ECollisionChannel TraceChannel=ECC_Pawn) const;
    void DetectForwardVehiclePawns();

protected:
    UPROPERTY(EditAnywhere)
    FVector PositionOffset = {0.0f,0.0f ,0.0f };

    UPROPERTY(EditAnywhere)
    FRotator RotationOffset = {0.0f,0.0f ,0.0f };

    USphereComponent* PawnDetectionSphere;

    USpringArmComponent* CameraSpringArm;

    UCameraComponent* Camera;

    FVector2D MovementInput;
    FVector2D CameraInput;
   
    virtual void BeginPlay() override;

    virtual void TickFollowingMode(float DeltaTime);

    virtual void TickManualMode(float DeltaTime);
    
    virtual void Tick(float DeltaSeconds) override;

    virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
public:
    virtual void MoveForward(float AxisValue) override;

    virtual void MoveRight(float AxisValue) override;

    virtual void YawCamera(float AxisValue);

    virtual void PitchCamera(float AxisValue);

    FORCEINLINE
    void SetCameraSpring(USpringArmComponent* cameraspring)
    {
      CameraSpringArm = cameraspring;
    }

    FORCEINLINE
    void SetCamera(UCameraComponent* camera)
    {
      Camera = camera;
    }

    UFUNCTION(BlueprintCallable)
    virtual void RestartLevel();

    UFUNCTION(BlueprintCallable)
    virtual void PossessLookedTarget();

    UFUNCTION(BlueprintCallable)
    virtual void FollowLookedTarget();

public:
    /**  */
    UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess="true"))
    ESpectatorControlMode ControlMode = ESpectatorControlMode::MANUAL_CONTROL;

    UFUNCTION(BlueprintCallable)
    void SetFollowTarget(APawn* Pawn);

    /** */
    UFUNCTION(BlueprintCallable)
    void SetManualControl();


    UFUNCTION(BlueprintPure)
    APawn* GetLookedTarget() const { return LookedTarget; }

    UFUNCTION(BlueprintPure)
    APawn* GetFollowedTarget() const { return FollowedTarget;  }

private:
    UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess="true") )
    float DistanceToDetectPawns = 4000.0f;

    UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess="true") )
    float RadiusToDetectPawns = 60.0f;

    UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess="true") )
    float TimeSinceLookForTargets = 0.0f;

    UPROPERTY(VisibleAnywhere)
    APawn* LookedTarget = nullptr;

    UPROPERTY(VisibleAnywhere)
    APawn* FollowedTarget = nullptr;
};
