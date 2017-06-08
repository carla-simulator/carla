// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GameFramework/PlayerController.h"
#include "AICarlaVehicleController.generated.h"
//#include "TrafficLight.h"

class UBoxComponent; 
class USphereComponent; 
class URoadMap;
class UWheeledVehicleMovementComponent;

/**
 * 
 */
UCLASS()
class CARLA_API AAICarlaVehicleController : public APlayerController
{
	GENERATED_BODY()

	// ===========================================================================
  	/// @name Constructor and destructor
  	// ===========================================================================
  	/// @{
public:

  	AAICarlaVehicleController();

  	~AAICarlaVehicleController();

	/// @}
  	// ===========================================================================
  	/// @name APlayerController overrides
  	// ===========================================================================
  	/// @{
public:

  	virtual void SetupInputComponent() override;

  	virtual void Possess(APawn *aPawn) override;

  	virtual void BeginPlay() override;

  	virtual void Tick(float DeltaTime) override;

  	/// @}
  	// ===========================================================================
  	/// @name Vehicle pawn info
  	// ===========================================================================
  	/// @{
public:

  	bool IsPossessingAVehicle() const
  	{
  	  return MovementComponent != nullptr;
  	}

  	/// World location of the vehicle.
  	FVector GetVehicleLocation() const;

  	/// Speed forward in km/h. Might be negative if goes backwards.
  	float GetVehicleForwardSpeed() const;

  	/// Orientation vector of the vehicle, pointing forward.
  	FVector GetVehicleOrientation() const;

  	int32 GetVehicleCurrentGear() const;

private:
    void InitVehilceValues();

  	/// @}
  	// ===========================================================================
  	/// @name Vehicle movement
  	// ===========================================================================
  	/// @{
public:

  	void SetThrottleInput(float Value);

  	void SetSteeringInput(float Value);

  	void SetHandbrakeInput(bool Value);

    /// @}
    // ===========================================================================
    /// @name Blueprint functions
    // ===========================================================================
    /// @{
public:

    UFUNCTION(BlueprintCallable, Category="Trigger")
    void RedTrafficLight(bool state);

    UFUNCTION(BlueprintCallable, Category="Trigger")
    void NewSpeedLimit(float speed);

    UFUNCTION(BlueprintCallable, Category="Trigger")
    void NewRoute(TArray<FVector> positions);


private:
  float CalcStreeringValue(FVector &direction);
  float GoTo(FVector objective, FVector &direction);
  float Stop(float &speed);
  float Move(float &speed);

  bool DoTrace();

private:

	UPROPERTY()
  	UBoxComponent *VehicleBounds;

  UPROPERTY()
    USphereComponent* VehicleRightControl;

  UPROPERTY()
    USphereComponent* VehicleLeftControl;

  UPROPERTY()
    AActor *forwardTrigger;

	UPROPERTY()
  URoadMap *RoadMap;

  UPROPERTY()
  	UWheeledVehicleMovementComponent *MovementComponent;

  UPROPERTY(EditAnywhere)
  float MAX_SPEED = 30.0f;

  bool TrafficLightStop;

  int route_it = 0;
  TArray<FVector> route;
	
  float steerAngle; 
};
