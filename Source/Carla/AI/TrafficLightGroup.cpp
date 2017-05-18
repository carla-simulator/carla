// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "TrafficLightGroup.h"


// Sets default values
ATrafficLightGroup::ATrafficLightGroup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATrafficLightGroup::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATrafficLightGroup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

