// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActorDataFallbackComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARLA_API UActorDataFallbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UActorDataFallbackComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	struct PolygonPoint {
		double x = 0.0;
		double y = 0.0;
		double z = 0.0;
	  };

	struct DebugROSMessage {
		int id;
		float pos_x, pos_y, pos_z;
		float yaw;
		float speed_x, speed_y, speed_z;
		float length, width, height;
		PolygonPoint corner_points[4];
		double tracking_time;
		double timestamp;
		float acc_x, acc_y, acc_z;
		float anchor_x, anchor_y, anchor_z;
		FString type;
	};
	
	TArray<DebugROSMessage> ROSObstacleData;
};
