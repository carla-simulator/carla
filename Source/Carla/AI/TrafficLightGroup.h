// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GameFramework/Actor.h"
#include "TrafficLightGroup.generated.h"

UCLASS()
class CARLA_API ATrafficLightGroup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATrafficLightGroup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:

	UPROPERTY (BlueprintReadOnly, Category=TrafficLights, EditAnywhere)
	TArray<AActor*> LightComponents;
	
};
