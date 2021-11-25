// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "Misc/ScopeLock.h"
#include "Carla/Sensor/Sensor.h"
#include "Carla/Sensor/LidarShaderSensor.h"
#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Sensor/LidarDescription.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/sensor/s11n/LidarMeasurement.h>
#include <compiler/enable-ue4-macros.h>
#include "Components/BoxComponent.h"
#include "RayCastLidar.generated.h"

/// A ray-cast based Lidar sensor.
UCLASS()
class CARLA_API ARayCastLidar : public ALidarShaderSensor
{
	GENERATED_BODY()

		using FLidarMeasurement = carla::sensor::s11n::LidarMeasurement;

public:

	static const uint32 MaxLidarBufferSize = MaxLidarImgWidth * MaxLidarImgHeight;

	static FActorDefinition GetSensorDefinition();

	ARayCastLidar(const FObjectInitializer &ObjectInitializer);

	void Set(const FActorDescription &Description) override;

	void Set(const FLidarDescription &LidarDescription);

protected:

	void Tick(float DeltaTime) override;

private:

	/// Creates a Laser for each channel.
	void CreateLasers();

	/// Updates LidarMeasurement with the points read in DeltaTime.
	void ReadPoints(float DeltaTime);

	/// Shoot a laser ray-trace, return whether the laser hit something.
	bool ShootLaser(const uint32 lidarId, const FVector LidarRelativeLoc, const FRotator LidarRelativeRot, const uint32 PointIndex, const uint32 Channel, const float HorizontalAngle, const bool gpuMode, FVector &XYZ, float &XYZLen);

	bool CalImpactPoint(const FRotator LookRot, const FVector LookLoc, FVector &XYZ);

	void CalConfficient(uint32 mapId, const FVector pointLoc, const FVector pointVec, float mapDep, float &kx_1, float &bx_1, float &ky_1, float &by_1, float &k_2, float &b_2);

	bool CalDepmapCoordinate(float kx_1, float bx_1, float ky_1, float by_1, float k_2, float b_2, float tt, float &xx, float &yy);

	bool CalDepmapTRange(float kx_1, float bx_1, float ky_1, float by_1, float k_2, float b_2, float &left_b, float &right_b);

	bool BilinearInterpolation(float mapDep, float xxt, float yyt, uint32 map_id, float &laser_dep);

	bool CalMinLoss(const uint32 mapId, const FVector pointLoc, const FVector pointVec, float mapDep, float &min_loss, float &min_t);
	
	bool ShootLaser2(const uint32 Channel, const float HorizontalAngle, FVector &XYZ, float &VecLen);

	UPROPERTY(EditAnywhere)
		FLidarDescription Description;

	TArray<float> LaserAngles;

	FLidarMeasurement LidarMeasurement;

	FCriticalSection BufferMutex[MaxCaptureCnt];

	carla::Buffer LidarBuffer[MaxCaptureCnt];

	bool FlagLidarBuffer[MaxCaptureCnt];

	int NowLidarCamera;

	int LidarNumber;
	
	int pointsOneChannel; // point number for each channel

	int LidarLaserPointId[3]; // the last point id for each channel

	float** LidarLaserPointDepth[3];

	int each_count[4];

	bool lidar_visible[3];

	int lidar_visible_cnt;

	bool gpu_mode;

	bool cal_ratio;

	bool cal_error;

	float lidar_shift;

	bool pc_all;

	bool pc_cycle;

	bool old_mode;

	//FRotator LidarRelativeRotator;

	//bool FlagRotator;

	//AActor* LidarParentActor;

	//unsigned char NowLidarBuffer[MaxLidarBufferSize * 4 + 5];

	//unsigned int NowLidarBufferSize;

	//int FlagMemLidar;
};
