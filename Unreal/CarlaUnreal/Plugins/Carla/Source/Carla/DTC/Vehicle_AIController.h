#pragma once
#include "AIController.h"
#include "GeographicCoordinates.h"
#include "GeoReferencingSystem.h"
#include "Vehicle.h"

#include "Vehicle_AIController.generated.h"
/** Please add a class description */
UCLASS(Blueprintable, BlueprintType)
class AVehicle_AIController : public AAIController
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaSeconds) override;

	virtual void OnPossess(APawn* InPawn) override;
	
	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void MoveToTargetLocation(FVector Target);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SetNewLocation(FVector InLocation);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SetNewLocationCoord(FGeographicCoordinates GeographicCoords);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Blueprintable)
	void LookAtTarget(AActor* target);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Blueprintable)
	void LookAtComponent(USkeletalMeshComponent* component);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Blueprintable)
	bool LookForward();
public:
	
	

	UPROPERTY(BlueprintReadOnly, Category="Default")
	bool FacingCenter;

	UPROPERTY(BlueprintReadOnly, Category="Default")
	bool CameraBusy;

	float MaxSpeedBase;
};
