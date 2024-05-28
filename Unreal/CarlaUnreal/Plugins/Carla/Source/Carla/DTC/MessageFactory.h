#pragma once

#include "CoreMinimal.h"
#include "Carla/Actor/CarlaActorFactoryBlueprint.h"
#include "MessageFactory.generated.h"

UCLASS()
class CARLA_API AMessageFactory : public ACarlaActorFactoryBlueprint
{
	GENERATED_BODY()

public:
	AMessageFactory();

	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void UPy_SendCasualties(const TMap<FString, int32>& CasualtyDictionary);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void UPy_SendWaypoints(const TArray<int32>& WaypointList);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void UPy_StartSimulation();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void UPy_StopSimulation();

	

	// Migrated from Game_GI_DARPA

	static UGameInstance* GetGameInstance(const UObject* WorldContextObject);
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool IsSimulationReady;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MaxSimulationTime;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FTimerHandle SimulationTimeHandle;

	UPROPERTY(EditDefaultsOnly)
	bool bQuitOnVehiclePathEnd;
	
	UFUNCTION(BlueprintCallable)
	bool GetIsSimulationReady();

	UFUNCTION(BlueprintCallable, BlueprintCallable)
	float CarlaGetSimulationTime();
	
	UFUNCTION(BlueprintCallable)
	void CarlaStopUnreal();
	
	UFUNCTION(BlueprintCallable)
	void StartSimulationTimer();

	UFUNCTION(BlueprintCallable)
	void UPy_SetTimerLength(float TimerLength);
	
	UFUNCTION()
	void HandleSimulationTimerComplete();

	UFUNCTION(BlueprintCallable)
	void HandleVehiclePathComplete(const FName& VehicleID);

};