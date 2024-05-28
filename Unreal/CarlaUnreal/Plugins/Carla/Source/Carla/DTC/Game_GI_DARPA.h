#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Game/CarlaGameInstance.h"
#include "Game_GI_DARPA.generated.h"

// USTRUCT(BlueprintType)
// struct FWaypoint_List {
// 	GENERATED_BODY()

// 	UPROPERTY(BlueprintReadWrite)
// 	FString waypoint;

// 	UPROPERTY(BlueprintReadWrite)
// 	float time;

// 	UPROPERTY(BlueprintReadWrite)
// 	FString classification;
// };

UCLASS()
class UGame_GI_DARPA : public UCarlaGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UGame_GI_DARPA();

	// Carla variables
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool IsSimulationReady;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MaxSimulationTime;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FTimerHandle SimulationTimeHandle;

	UPROPERTY(EditDefaultsOnly)
	bool bQuitOnVehiclePathEnd;
	
	// Carla triggerable functions
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="CARLA")
	void CarlaPrepareSimulation(int32 PathIndex);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="CARLA")
	bool CarlaStartSimulation();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="CARLA")
	bool CarlaResumeVehicle(FName VehicleID);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="CARLA")
	bool CarlaPauseVehicle(FName VehicleID);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="CARLA")
	bool CarlaNextWaypoint(FName VehicleID);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="CARLA")
	bool CarlaIsReady();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="CARLA")
	float CarlaGetSimulationTime();

	// Stops Unreal process utility
	UFUNCTION(BlueprintCallable, Category="CARLA")
	void CarlaStopUnreal();

	// //YamlNode Variables
	// UPROPERTY(BlueprintReadWrite, EditAnywhere)
	// TArray<FWaypoint_List> Waypoints;
	
	// YamlNode Functions
	UFUNCTION(BlueprintCallable, Category="YamlNode")
	void ParseYamlStruct();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Darpa")
	void SetIREnabled(const bool newEnabled);

	UFUNCTION(BlueprintCallable, Category="CARLA")
	void StartSimulationTimer();

	UFUNCTION()
	void HandleSimulationTimerComplete();

	UFUNCTION(BlueprintCallable, Category="CARLA")
	void HandleVehiclePathComplete(const FName& VehicleID);

	UFUNCTION(BlueprintCallable, Category="DARPA", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Darpa Game Instance"))
	static UGame_GI_DARPA* Get(const UObject* WorldContextObject);
};
