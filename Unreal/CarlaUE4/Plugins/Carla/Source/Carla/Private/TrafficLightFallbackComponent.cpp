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

	TrafficLights.Empty();
	UCarlaEpisode* Episode = UCarlaStatics::GetCurrentEpisode(GetWorld());

	auto It = Episode->GetActorRegistry().begin();
	for (; It != Episode->GetActorRegistry().end(); ++It)
	{
		const FCarlaActor& View = *(It.Value());
		const ATrafficLightBase* TrafficLight = Cast<ATrafficLightBase>(View.GetActor());
		if (!IsValid(TrafficLight)) continue;

		const UTrafficLightComponent* TrafficLightComponent = TrafficLight->GetTrafficLightComponent();
		if (!IsValid(TrafficLightComponent)) continue;

		const UActorComponent* actor_comp = View.GetActor()->GetComponentByClass(UTrafficLightComponent::StaticClass());
		if(!IsValid(actor_comp)) continue;

		const UTrafficLightComponent* light_comp = Cast<UTrafficLightComponent>(actor_comp);
		if(!IsValid(light_comp)) continue;

		const std::string SignId = carla::rpc::FromFString(TrafficLightComponent->GetSignId());
		DebugROSMessage debug_msg;
		debug_msg.id = FString(SignId.c_str());
		debug_msg.light_state = light_comp->GetLightState();
		debug_msg.confidence = 3; //always most confident, since it's ground truth
		debug_msg.blink = false; //not supported
		debug_msg.contains_lights = true;
		TrafficLights.Add(debug_msg);
	}
}
