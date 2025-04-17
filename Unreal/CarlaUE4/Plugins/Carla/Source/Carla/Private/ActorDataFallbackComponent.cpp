// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "ActorDataFallbackComponent.h"
#include "Carla/Actor/ActorRegistry.h"
#include "Carla/Game/CarlaStatics.h"
#include "Math/DVector.h"

UActorDataFallbackComponent::UActorDataFallbackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}


// Called when the game starts
void UActorDataFallbackComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UActorDataFallbackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Get All Actors
	// TArray<AActor*> actors;
	// UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), actors);

	UCarlaEpisode* Episode = UCarlaStatics::GetCurrentEpisode(GetWorld());
	DebugROSMessage debug_msg;

	ROSObstacleData.Empty();

	auto It = Episode->GetActorRegistry().begin();
    for (; It != Episode->GetActorRegistry().end(); ++It){
		const FCarlaActor& View = *(It.Value());
		debug_msg.id = It.Key();

		
		debug_msg.pos_x = View.GetActorGlobalLocation().X;
		debug_msg.pos_y = View.GetActorGlobalLocation().Y;
		debug_msg.pos_z = View.GetActorGlobalLocation().Z;
		
		debug_msg.yaw = View.GetActorGlobalTransform().Rotator().Yaw;
		
		debug_msg.speed_x = View.GetActorVelocity().X;
		debug_msg.speed_y = View.GetActorVelocity().Y;
		debug_msg.speed_z = View.GetActorVelocity().Z;
		
		FVector origin, extent;
		View.GetActor()->GetActorBounds(true, origin, extent);
		debug_msg.length 	= extent.X;
		debug_msg.width 	= extent.Y;
		debug_msg.height 	= extent.Z;
		
		FVector temp = FVector(origin.X + extent.X * 0.5, origin.Y, origin.Z + extent.Z * 0.5);
		debug_msg.corner_points[0].x = temp.X; debug_msg.corner_points[0].y = temp.Y; debug_msg.corner_points[0].z = temp.Z;

		temp = FVector(origin.X - extent.X * 0.5, origin.Y, origin.Z + extent.Z * 0.5);
		debug_msg.corner_points[1].x = temp.X; debug_msg.corner_points[1].y = temp.Y; debug_msg.corner_points[1].z = temp.Z;

		temp = FVector(origin.X - extent.X * 0.5, origin.Y, origin.Z - extent.Z * 0.5);
		debug_msg.corner_points[2].x = temp.X; debug_msg.corner_points[2].y = temp.Y; debug_msg.corner_points[2].z = temp.Z;

		temp = FVector(origin.X - extent.X * 0.5, origin.Y, origin.Z - extent.Z * 0.5);
		debug_msg.corner_points[3].x = temp.X; debug_msg.corner_points[3].y = temp.Y; debug_msg.corner_points[3].z = temp.Z;
		
		debug_msg.tracking_time = 0.0f;
		debug_msg.timestamp = 0.0f;	

		debug_msg.acc_x = 0.0f;
		debug_msg.acc_x = 0.0f;
		debug_msg.acc_x = 0.0f;

		debug_msg.anchor_x = origin.X;
		debug_msg.anchor_y = origin.Y;
		debug_msg.anchor_z = origin.Z;

		debug_msg.type = "UNKNOWN_MOVABLE";
		
		// FString debug_actor_name = View.GetActor()->GetName();
		// UE_LOG(LogCarla, Log, TEXT("Actor Name: %s"), *debug_actor_name);
		ROSObstacleData.Add(debug_msg);
	}

}
