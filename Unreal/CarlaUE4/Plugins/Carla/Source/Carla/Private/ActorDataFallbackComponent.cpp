// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "ActorDataFallbackComponent.h"
#include "Carla/Actor/ActorRegistry.h"
#include "Carla/Game/CarlaStatics.h"
#include "Components/MeshComponent.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Walker/WalkerBase.h"
#include "Carla/Util/BoundingBox.h"
#include "Carla/Util/BoundingBoxCalculator.h"
#include "GameFramework/CharacterMovementComponent.h"
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

	float timestamp = GetWorld()->GetTimeSeconds();

	auto It = Episode->GetActorRegistry().begin();
	for (; It != Episode->GetActorRegistry().end(); ++It){

		const FCarlaActor& View = *(It.Value());

		// Filter off ego vehicle
		if(View.GetActor() == GetOwner())
		{
			continue;
		}

		UActorComponent *actor_comp = View.GetActor()->GetComponentByClass(UMeshComponent::StaticClass());
		if(actor_comp == nullptr)
		{
			continue;
		}

		const ACarlaWheeledVehicle* vehicle = Cast<ACarlaWheeledVehicle>(View.GetActor());
		const AWalkerBase* walker = Cast<AWalkerBase>(View.GetActor());

		// Only Wheeled Vehicles & Pedestrians
		if(vehicle == nullptr && walker == nullptr)
		{
			continue;
		}

		UMeshComponent* mesh_comp = Cast<UMeshComponent>(actor_comp);
		if(mesh_comp->bHiddenInGame)
		{
			continue;
		}

		debug_msg.id = It.Key();
		
		debug_msg.pos_x = View.GetActorGlobalLocation().X;
		debug_msg.pos_y = View.GetActorGlobalLocation().Y;
		debug_msg.pos_z = View.GetActorGlobalLocation().Z;
		
		debug_msg.yaw = View.GetActorGlobalTransform().Rotator().Yaw;
		
		if(vehicle)
		{
			debug_msg.speed_x = vehicle->GetVelocity().X;
			debug_msg.speed_y = vehicle->GetVelocity().Y;
			debug_msg.speed_z = vehicle->GetVelocity().Z;
		}
		else
		{
			debug_msg.speed_x = View.GetActorVelocity().X;
			debug_msg.speed_y = View.GetActorVelocity().Y;
			debug_msg.speed_z = View.GetActorVelocity().Z;
		}

		// if(walker != nullptr)
		// {
		// 	FString speed_x_str = FString::SanitizeFloat(debug_msg.speed_x);
		// 	FString speed_y_str = FString::SanitizeFloat(debug_msg.speed_y);
		// 	FString speed_z_str = FString::SanitizeFloat(debug_msg.speed_z);
		// 	UE_LOG(LogCarla, Log, TEXT("Actor Speed: X:%s Y:%s Z:%s"), *speed_x_str, *speed_y_str, *speed_z_str);
		// }
		
		FVector origin, extent;
		FBoundingBox bounding_box = UBoundingBoxCalculator::GetActorBoundingBox(View.GetActor(), 0);
		debug_msg.length 	= bounding_box.Extent.X * 2.0;
		debug_msg.width 	= bounding_box.Extent.Y;
		debug_msg.height 	= bounding_box.Extent.Z * 2.0;
		
		FVector temp 	= FVector(bounding_box.Origin.X + bounding_box.Extent.X * 0.5, bounding_box.Origin.Y  + bounding_box.Extent.Y * 0.5, bounding_box.Origin.Z);
		debug_msg.corner_points[0].x = temp.X; debug_msg.corner_points[0].y = temp.Y; debug_msg.corner_points[0].z = temp.Z;

		temp 			= FVector(bounding_box.Origin.X - bounding_box.Extent.X * 0.5, bounding_box.Origin.Y  + bounding_box.Extent.Y * 0.5, bounding_box.Origin.Z);
		debug_msg.corner_points[1].x = temp.X; debug_msg.corner_points[1].y = temp.Y; debug_msg.corner_points[1].z = temp.Z;

		temp 			= FVector(bounding_box.Origin.X - bounding_box.Extent.X * 0.5, bounding_box.Origin.Y  - bounding_box.Extent.Y * 0.5, bounding_box.Origin.Z);
		debug_msg.corner_points[2].x = temp.X; debug_msg.corner_points[2].y = temp.Y; debug_msg.corner_points[2].z = temp.Z;

		temp 			= FVector(bounding_box.Origin.X - bounding_box.Extent.X * 0.5, bounding_box.Origin.Y  - bounding_box.Extent.Y * 0.5, bounding_box.Origin.Z);
		debug_msg.corner_points[3].x = temp.X; debug_msg.corner_points[3].y = temp.Y; debug_msg.corner_points[3].z = temp.Z;
		
		
		debug_msg.tracking_time = 0.0f; //moment the actor is "detected" aka spawned in our case
		debug_msg.timestamp = timestamp;

		if(walker)
		{
			debug_msg.acc_x = walker->GetCharacterMovement()->GetCurrentAcceleration().X;
			debug_msg.acc_y = walker->GetCharacterMovement()->GetCurrentAcceleration().Y;
			debug_msg.acc_z = walker->GetCharacterMovement()->GetCurrentAcceleration().Z;
		}
		else
		{
			debug_msg.acc_x = 0.0f;
			debug_msg.acc_y = 0.0f;
			debug_msg.acc_z = 0.0f;
		}
		//const FVector Acceleration = (Velocity - PreviousVelocity) / DeltaSeconds;

		debug_msg.anchor_x = bounding_box.Origin.X;
		debug_msg.anchor_y = bounding_box.Origin.Y;
		debug_msg.anchor_z = bounding_box.Origin.Z;

		FActorDescription description = View.GetActorInfo()->Description;
		for (auto& element : description.Variations)
		{
			if(element.Value.Value.Contains("walker"))
			{
				debug_msg.type = "PEDESTRIAN";
				break;
			}
			if(element.Value.Value.Contains("vehicle"))
			{
				debug_msg.type = "VEHICLE";
				break;
			}	
		}

		if(debug_msg.type.IsEmpty())
			debug_msg.type = "UNKNOWN_MOVABLE";

		UE_LOG(LogCarla, Log, TEXT("Actor Type: %s"), *(debug_msg.type));
		
		//FString debug_actor_name = View.GetActor()->GetName();
		//UE_LOG(LogCarla, Log, TEXT("Actor Name: %s"), *debug_actor_name);
		ROSObstacleData.Add(debug_msg);
	}

}
