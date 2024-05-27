#pragma once


#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "GeographicCoordinates.h"
#include "GeoReferencingSystem.h"
#include "CollisionShape.h"


#include "Vehicle.generated.h"

class USphereComponent;

/** Please add a class description */
UCLASS(Blueprintable, BlueprintType)
class AVehicle : public APawn
{
	GENERATED_BODY()
public:
	AVehicle();
	
	/** Moves the vehicle relative to the terrain */
	UFUNCTION(BlueprintCallable)
	void BasicMovementWithRotation(FVector2D Move);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void AdjustToTerrain();

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void LookRotation(FVector2D Delta);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void UnboundMovement(FVector2D Move);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void BasicMovement(FVector2D Move);

	UFUNCTION(BlueprintCallable)
	void RotateVehicleForLook(FVector target);

	UFUNCTION(BlueprintCallable)
	bool GetCoordinates(FGeographicCoordinates &Coords);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Blueprintable, Category="Vehicle")
	void ExecuteNextWaypoint();

	UFUNCTION(BlueprintCallable, Blueprintable, Category="Vehicle")
	void PauseVehicle();

	UFUNCTION(BlueprintCallable, Blueprintable, Category="Vehicle")
	void ResumeVehicle();
public:

	virtual void Tick(float DeltaSeconds) override;
	
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Vehicle Characteristics")
	TObjectPtr<UCameraComponent> Camera;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Vehicle Characteristics")
	TObjectPtr<USpringArmComponent> SpringArm;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Vehicle Characteristics")
	TObjectPtr<UFloatingPawnMovement> FloatingPawnMovement;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Vehicle Characteristics")
	TObjectPtr<USphereComponent> RootSphere;

	UPROPERTY(BlueprintReadWrite, Blueprintable, EditAnywhere, Category="Vehicle Characteristics")
	FName VehicleID;

	UPROPERTY(BlueprintReadWrite, Blueprintable, EditAnywhere, Category="Vehicle Characteristics")
	TArray<FVector> MoveToLocations;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Vehicle Characteristics")
	int32 WaypointIndex;
	
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Vehicle Characteristics")
	double Altitude;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Vehicle Characteristics")
	bool AtAltitude;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Vehicle Characteristics")
	float AltitudeAcceptableRange;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Vehicle Characteristics")
	bool UseNavMesh;

	UPROPERTY(BlueprintReadWrite, Blueprintable, EditAnywhere, Category="Vehicle Characteristics")
	float AtLocationWaitDuration;

	UPROPERTY(BlueprintReadWrite, Blueprintable, EditAnywhere, Category="Vehicle Characteristics")
	float CameraLookAtSpeed;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Vehicle Pathfinding")
	FVector FindNavmeshExtentRange;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Vehicle Pathfinding")
	float TraceStartOffset;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Vehicle Pathfinding")
	float TraceEndOffset;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Vehicle Pathfinding")
	float TraceCapsuleRadius;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Vehicle Pathfinding")
	float TraceCapsuleHalfHeight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Vehicle Pathfinding")
	float YawDiffAcceptableRange;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Vehicle Pathfinding")
	float YawDiffDivisor;
};
