// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Actor/ActorRegistry.h"
#include "Carla/Game/CarlaStatics.h"
#include "TrafficLightFallbackComponent.h"

UTrafficLightFallbackComponent::UTrafficLightFallbackComponent()
{

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}


void UTrafficLightFallbackComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UTrafficLightFallbackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UCarlaEpisode* Episode = UCarlaStatics::GetCurrentEpisode(GetWorld());

	auto It = Episode->GetActorRegistry().begin();
    for (; It != Episode->GetActorRegistry().end(); ++It)
	{
		const FCarlaActor& View = *(It.Value());
		FString debug_actor_name = View.GetActor()->GetName();
		if(debug_actor_name.Contains(TEXT("TrafficLight"), ESearchCase::IgnoreCase))
		{
			//UE_LOG(LogCarla, Log, TEXT("TrafficLight Found: %s"), *debug_actor_name);
		}
	}

	// struct DebugROSMessage {
	// 	int id;
	// 	carla::rpc::TrafficLightState trafficlight_state;
	// 	int confidence;
	// 	double tracking_time;
	// 	bool blink;
	// 	bool contains_lights;
	// };
}

