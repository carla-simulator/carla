// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GameFramework/Actor.h"
#include "TrafficLight.generated.h"


UENUM(BlueprintType)
	enum class ETrafficLightState : uint8{
		RED 		UMETA(DisplayName = "Red"),
		YELLOW		UMETA(DisplayName = "Yellow"),
		GEEN		UMETA(DisplayName = "Green")
	};


UCLASS()
class CARLA_API ATrafficLight : public AActor
{

	GENERATED_BODY()
	
public:	

  	ATrafficLight();

  	~ATrafficLight(){};

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintReadWrite, Category=TrafficLightValues, EditAnywhere)
 	ETrafficLightState state;
	
};
