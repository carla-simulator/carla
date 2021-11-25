// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <iostream>
#include "Carla.h"
#include "Carla/Sensor/RayCastLidar.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Math/UnrealMathUtility.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/geom/Math.h"
#include <compiler/enable-ue4-macros.h>

#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"


FActorDefinition ARayCastLidar::GetSensorDefinition()
{
	auto Definition = UActorBlueprintFunctionLibrary::MakeLidarDefinition(TEXT("ray_cast"));
	FActorVariation LidarMode;
	LidarMode.Id = TEXT("speed_mode");
	LidarMode.Type = EActorAttributeType::Bool;
	LidarMode.bRestrictToRecommended = false;
	LidarMode.RecommendedValues.Emplace(TEXT("true"));

	FActorVariation GpuMode;
	GpuMode.Id = TEXT("gpu_mode");
	GpuMode.Type = EActorAttributeType::Bool;
	GpuMode.bRestrictToRecommended = false;
	GpuMode.RecommendedValues.Emplace(TEXT("true"));

	FActorVariation OldMode;
	OldMode.Id = TEXT("old_mode");
	OldMode.Type = EActorAttributeType::Bool;
	OldMode.bRestrictToRecommended = false;
	OldMode.RecommendedValues.Emplace(TEXT("false"));

	FActorVariation LidarWidth;
	LidarWidth.Id = TEXT("camera_width");
	LidarWidth.Type = EActorAttributeType::Int;
	LidarWidth.RecommendedValues = { TEXT("1500") };

	FActorVariation LidarHeight;
	LidarHeight.Id = TEXT("camera_height");
	LidarHeight.Type = EActorAttributeType::Int;
	LidarHeight.RecommendedValues = { TEXT("1500") };

	FActorVariation LidarVisible;
	LidarVisible.Id = TEXT("lidar_visible");
	LidarVisible.Type = EActorAttributeType::Int;
	LidarVisible.RecommendedValues.Emplace(TEXT("1"));
	
	FActorVariation CalRatio;
	CalRatio.Id = TEXT("cal_ratio");
	CalRatio.Type = EActorAttributeType::Bool;
	CalRatio.bRestrictToRecommended = false;
	CalRatio.RecommendedValues.Emplace(TEXT("true"));

	FActorVariation CalError;
	CalError.Id = TEXT("cal_error");
	CalError.Type = EActorAttributeType::Bool;
	CalError.bRestrictToRecommended = false;
	CalError.RecommendedValues.Emplace(TEXT("false"));

	FActorVariation LidarShift;
	LidarShift.Id = TEXT("lidar_shift");
	LidarShift.Type = EActorAttributeType::Float;
	LidarShift.RecommendedValues = { TEXT("1.0") };

	FActorVariation PointCloudAll;
	PointCloudAll.Id = TEXT("pc_all");
	PointCloudAll.Type = EActorAttributeType::Bool;
	PointCloudAll.bRestrictToRecommended = false;
	PointCloudAll.RecommendedValues.Emplace(TEXT("true"));

	FActorVariation PointCloudCycle;
	PointCloudCycle.Id = TEXT("pc_cycle");
	PointCloudCycle.Type = EActorAttributeType::Bool;
	PointCloudCycle.bRestrictToRecommended = false;
	PointCloudCycle.RecommendedValues.Emplace(TEXT("false"));


	Definition.Variations.Append({ LidarMode, GpuMode,OldMode, LidarWidth, LidarHeight, LidarVisible, CalRatio, CalError, LidarShift, PointCloudAll, PointCloudCycle });
	return Definition;
}

ARayCastLidar::ARayCastLidar(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	LoadPostProcessingMaterial(
#if PLATFORM_LINUX
		TEXT("Material'/Carla/PostProcessingMaterials/DepthEffectMaterial_GLSL.DepthEffectMaterial_GLSL'")
#else
		TEXT("Material'/Carla/PostProcessingMaterials/DepthEffectMaterial.DepthEffectMaterial'")
#endif
	);

	for (int i = 0; i < MaxCaptureCnt; ++i) {
		FlagLidarBuffer[i] = false;
	}
	//FlagRotator = false;
	NowLidarCamera = 2;
	LidarNumber = 3;
	for (int i = 0; i < LidarNumber; ++i) {
		LidarLaserPointId[i] = 0;
	}
	
}

void ARayCastLidar::Set(const FActorDescription &ActorDescription)
{
	Super::Set(ActorDescription);
	FLidarDescription LidarDescription;
	UActorBlueprintFunctionLibrary::SetLidar(ActorDescription, LidarDescription);
	int lidar_visible_flag =
		UActorBlueprintFunctionLibrary::RetrieveActorAttributeToInt("lidar_visible", ActorDescription.Variations, 1);
	lidar_visible[0] = lidar_visible_flag & 1;
	lidar_visible[1] = lidar_visible_flag & 2;
	lidar_visible[2] = lidar_visible_flag & 4;
	lidar_visible_cnt = int(lidar_visible[0]) + int(lidar_visible[1]) + int(lidar_visible[2]);
	gpu_mode = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToBool("gpu_mode", ActorDescription.Variations, true);
	old_mode = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToBool("old_mode", ActorDescription.Variations, false);
	cal_ratio = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToBool("cal_ratio", ActorDescription.Variations, true);
	cal_error = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToBool("cal_error", ActorDescription.Variations, false);
	lidar_shift = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat("lidar_shift", ActorDescription.Variations, 1.0);
	pc_all = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToBool("pc_all", ActorDescription.Variations, true);
	pc_cycle = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToBool("pc_cycle", ActorDescription.Variations, false);
	UE_LOG(LogCarla, Warning, TEXT("INFO SC %d %d"), ImageWidth, ImageHeight);
	Set(LidarDescription);
}

void ARayCastLidar::Set(const FLidarDescription &LidarDescription)
{
	Description = LidarDescription;
	LidarMeasurement = FLidarMeasurement(Description.Channels);
	CreateLasers();

	pointsOneChannel = Description.PointsPerSecond / Description.RotationFrequency / Description.Channels  + 0.1;

	for (int i = 0; i < LidarNumber; ++i) 
	{
		LidarLaserPointDepth[i] = new float*[Description.Channels];
		for (uint32 j = 0; j < Description.Channels; ++j)
		{
			LidarLaserPointDepth[i][j] = new float[pointsOneChannel];
			for (int k = 0; k < pointsOneChannel; ++k) {
				LidarLaserPointDepth[i][j][k] = -1.0;
			}
		}
	}
	
	each_count[0] = each_count[1] = each_count[2] = each_count[3] = 0;
}


void ARayCastLidar::CreateLasers()
{
	const auto NumberOfLasers = Description.Channels;
	check(NumberOfLasers > 0u);
	const float DeltaAngle = NumberOfLasers == 1u ? 0.f :
		(Description.UpperFovLimit - Description.LowerFovLimit) /
		static_cast<float>(NumberOfLasers - 1);
	LaserAngles.Empty(NumberOfLasers);
	for (auto i = 0u; i < NumberOfLasers; ++i)
	{
		const float VerticalAngle =
			Description.UpperFovLimit - static_cast<float>(i) * DeltaAngle;
		LaserAngles.Emplace(VerticalAngle);
	}
}

void ARayCastLidar::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (LidarSpeedMode) {
		FPixelReader::SendPixelsInRenderThreadToBuffer(*this, LidarBuffer[NowLidarCamera], FlagLidarBuffer[NowLidarCamera], BufferMutex[NowLidarCamera], NowLidarCamera);
	}
	else {
		for (int i = 0; i < MaxCaptureCnt; ++i) {
			FPixelReader::SendPixelsInRenderThreadToBuffer(*this, LidarBuffer[i], FlagLidarBuffer[i], BufferMutex[i], i);
		}
	}
	//FPixelReader::SendPixelsInRenderThreadToBuffer(*this, LidarBuffer[NowLidarCamera], FlagLidarBuffer[NowLidarCamera], BufferMutex[NowLidarCamera], NowLidarCamera);
	ReadPoints(DeltaTime);

	NowLidarCamera = (NowLidarCamera + 1) % MaxCaptureCnt;
	//UE_LOG(LogCarla, Warning, TEXT("INFO %d"), NowLidarCamera);
	auto DataStream = GetDataStream(*this);

	DataStream.Send(*this, LidarMeasurement, DataStream.PopBufferFromPool());
}

void ARayCastLidar::ReadPoints(const float DeltaTime)
{
	const uint32 ChannelCount = Description.Channels;
	uint32 PointsToScanWithOneLaser =
		FMath::RoundHalfFromZero(
			Description.PointsPerSecond * DeltaTime / float(ChannelCount));
	if (pc_cycle) {
		PointsToScanWithOneLaser = pointsOneChannel;
	}
	if (PointsToScanWithOneLaser <= 0)
	{
		UE_LOG(
			LogCarla,
			Warning,
			TEXT("%s: no points requested this frame, try increasing the number of points per second."),
			*GetName());
		return;
	}

	check(ChannelCount == LaserAngles.Num());

	const float CurrentHorizontalAngle = carla::geom::Math::ToDegrees(
		LidarMeasurement.GetHorizontalAngle());
	// points in one cycle: Description.PointsPerSecond / float(ChannelCount) / Description.RotationFrequency
	
	const float AngleDistanceOfLaserMeasure = 360.0f / pointsOneChannel;
	const float AngleDistanceOfTick = AngleDistanceOfLaserMeasure * PointsToScanWithOneLaser;

	LidarMeasurement.Reset(lidar_visible_cnt * ChannelCount * PointsToScanWithOneLaser);
	FVector LidarPosition[3] = {
		FVector(100, 0, 0) * lidar_shift, //前lidar
		FVector(-50, -86.6, 0) * lidar_shift, //后左lidar
		FVector(-50, 86, 0) * lidar_shift,  //后右lidar
		//FVector(0, 0, 0),
		//FVector(0, 0, 0),
		//FVector(0, 0, 0),
	};
	FRotator LidarRotation[3] = {
		FRotator(0, 0, 0), //前左lidar
		FRotator(0, 0, 0), //前右lidar
		FRotator(0, 0, 0), //后侧lidar
	};

	each_count[0] = each_count[1] = each_count[2] = each_count[3] = 0;

	float errorSum = 0;
	int errorCnt = 0;
	int error1Cnt = 0, error5Cnt = 0, error10Cnt = 0, error20Cnt = 0, error40Cnt = 0, error80Cnt = 0;
	for (auto lidar_index = 0; lidar_index < LidarNumber; ++lidar_index) 
	{
		if (lidar_visible[lidar_index]) {
			for (auto Channel = 0u; Channel < ChannelCount; ++Channel)
			{
				for (auto i = 0u; i < PointsToScanWithOneLaser; ++i)
				{
					FVector Point;
					const float Angle = CurrentHorizontalAngle + AngleDistanceOfLaserMeasure * i;
					int pointIndex = (LidarLaserPointId[lidar_index] + i) % pointsOneChannel;
					float PointLen0, PointLen1;
					if (old_mode == false) {
						if (ShootLaser(lidar_index, LidarPosition[lidar_index], LidarRotation[lidar_index], pointIndex, Channel, Angle, gpu_mode, Point, PointLen0))
						{
							LidarMeasurement.WritePoint(Channel, Point);
						}
					}
					else {
						if (ShootLaser2(Channel, Angle, Point, PointLen0)) {
							LidarMeasurement.WritePoint(Channel, Point);
						}
					}
					if (cal_error) {
						ShootLaser(lidar_index, LidarPosition[lidar_index], LidarRotation[lidar_index], pointIndex, Channel, Angle, false, Point, PointLen1);
						
						int DeltaLen = FMath::Abs(PointLen1 - PointLen0);
						if (PointLen0 < Description.Range && PointLen1 < Description.Range) {
							errorSum += DeltaLen;
							errorCnt++;
							if (DeltaLen < 1) {
								error1Cnt++;
							}
							else if (DeltaLen < 5) {
								error5Cnt++;
							}
							else if (DeltaLen < 10) {
								error10Cnt++;
							}
							else if (DeltaLen < 20) {
								error20Cnt++;
							}
							else if (DeltaLen < 40) {
								error40Cnt++;
							}
							else if (DeltaLen < 80) {
								error80Cnt++;
							}
						}
					}
				
				}
			}
		}
	}
	if (cal_ratio ) {
		UE_LOG(LogCarla, Warning, TEXT("INFO each_count1  %d %d %d %d %f"), each_count[0], each_count[1], each_count[2], each_count[3], float(each_count[2])/ float(each_count[0] + each_count[1]+each_count[2] + each_count[3]));
	}
	if (cal_error) {
		if (!errorCnt) {
			UE_LOG(LogCarla, Warning, TEXT("INFO each_error no value"));
		}
		else {
			error5Cnt += error1Cnt;
			error10Cnt += error5Cnt;
			error20Cnt += error10Cnt;
			error40Cnt += error20Cnt;
			error80Cnt += error40Cnt;
			UE_LOG(LogCarla, Warning, TEXT("INFO each_error2 mean: %f, e1: %f e5: %f, e10: %f, e20: %f, e40: %f, e80: %f"), errorSum / errorCnt, float(error1Cnt) / errorCnt, float(error5Cnt) / errorCnt, float(error10Cnt) / errorCnt, float(error20Cnt) / errorCnt, float(error40Cnt) / errorCnt, float(error80Cnt) / errorCnt);
		}
	}
	const float HorizontalAngle = carla::geom::Math::ToRadians(
		std::fmod(CurrentHorizontalAngle + AngleDistanceOfTick, 360.0f));
	LidarMeasurement.SetHorizontalAngle(HorizontalAngle);

	for (auto lidar_index = 0; lidar_index < LidarNumber; ++lidar_index) {
		LidarLaserPointId[lidar_index] = (LidarLaserPointId[lidar_index] + PointsToScanWithOneLaser)% pointsOneChannel;
	}
	
}

bool ARayCastLidar::CalImpactPoint(const FRotator LookRot, const FVector LookLoc, FVector &XYZ) {

	FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Laser_Trace")), true, this);
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = false;

	FHitResult HitInfo(ForceInit);

	FVector EndTrace = 100000.0 * UKismetMathLibrary::GetForwardVector(LookRot) + LookLoc;

	GetWorld()->LineTraceSingleByChannel(
		HitInfo,
		LookLoc,
		EndTrace,
		ECC_MAX,
		TraceParams,
		FCollisionResponseParams::DefaultResponseParam
	);

	if (HitInfo.bBlockingHit){
		XYZ = HitInfo.ImpactPoint;
		return true;
	}
	else {
		XYZ = EndTrace;
		return false;
	}

}
void ARayCastLidar::CalConfficient(uint32 mapId, const FVector pointLoc, const FVector pointVec, float mapDep, float &kx_1, float &bx_1, float &ky_1, float &by_1, float &k_2, float &b_2)
{
	if (mapId == 0) {
		kx_1 = pointVec.Y*mapDep;
		bx_1 = pointLoc.Y*mapDep;
		ky_1 = pointVec.Z*mapDep;
		by_1 = pointLoc.Z*mapDep;
		k_2 = pointVec.X;
		b_2 = pointLoc.X;
	}
	else if (mapId == 1) {
		kx_1 = (-FMath::Sqrt(3)*pointVec.X - pointVec.Y)*mapDep;
		bx_1 = (-FMath::Sqrt(3)*pointLoc.X - pointLoc.Y)*mapDep;
		ky_1 = 2 * pointVec.Z*mapDep;
		by_1 = 2 * pointLoc.Z*mapDep;
		k_2 = (-pointVec.X + FMath::Sqrt(3)*pointVec.Y);
		b_2 = (-pointLoc.X + FMath::Sqrt(3)*pointLoc.Y);
	}
	else {
		kx_1 = (FMath::Sqrt(3)*pointVec.X - pointVec.Y)*mapDep;
		bx_1 = (FMath::Sqrt(3)*pointLoc.X - pointLoc.Y)*mapDep;
		ky_1 = 2 * pointVec.Z*mapDep;
		by_1 = 2 * pointLoc.Z*mapDep;
		k_2 = (-pointVec.X - FMath::Sqrt(3)*pointVec.Y);
		b_2 = (-pointLoc.X - FMath::Sqrt(3)*pointLoc.Y);
	}
	return;
}
bool ARayCastLidar::CalDepmapCoordinate(float kx_1, float bx_1, float ky_1, float by_1, float k_2, float b_2, float tt, float &xx, float &yy)
{
	float base_factor = k_2 * tt + b_2;
	if (base_factor != 0) {
		xx = (kx_1*tt + bx_1) / base_factor;
		yy = (ky_1*tt + by_1) / base_factor;
		return true;
	}
	else {
		return false;
	}
	/*
	if (mapId == 0) {
		xx = pointLoc.Y / pointLoc.X * mapDep;
		yy = pointLoc.Z / pointLoc.X * mapDep;
		return true;
	}
	else if (mapId == 1) {
		xx = (-FMath::Sqrt(3)*pointLoc.X - pointLoc.Y) / (-pointLoc.X + FMath::Sqrt(3)*pointLoc.Y) * mapDep;
		yy = 2 * pointLoc.Z / (-pointLoc.X + FMath::Sqrt(3)*pointLoc.Y) * mapDep;
		return true;
	}
	else if (mapId == 2) {
		xx = (FMath::Sqrt(3)*pointLoc.X - pointLoc.Y) / (-pointLoc.X - FMath::Sqrt(3)*pointLoc.Y) * mapDep;
		yy = 2 * pointLoc.Z / (-pointLoc.X - FMath::Sqrt(3)*pointLoc.Y) * mapDep;
		return true;
	}
	else {
		return false;
	}
	*/
}



bool ARayCastLidar::CalDepmapTRange(float kx_1, float bx_1, float ky_1, float by_1, float k_2, float b_2, float &left_b, float &right_b)
{
	float col0 = (ImageWidth - 1) / 2.0 + bx_1 / b_2;
	float row0 = (ImageHeight - 1) / 2.0 - by_1 / b_2;
	float low_xx = -((ImageWidth - 1) / 2.0)  + FMath::Min(FMath::Max(FMath::FloorToInt(col0) - 1, 0), int(ImageWidth - 1)); // TO FIX
	float up_xx = -((ImageWidth - 1) / 2.0)  + FMath::Min(FMath::Max(FMath::FloorToInt(col0) + 2, 0), int(ImageWidth - 1));
	float low_yy = (ImageHeight - 1) / 2.0 - FMath::Min(FMath::Max(FMath::FloorToInt(row0) + 2, 0), int(ImageHeight - 1)); // TO FIX
	float up_yy = (ImageHeight - 1) / 2.0 - FMath::Min(FMath::Max(FMath::FloorToInt(row0) - 1, 0), int(ImageHeight - 1));

	float t_x_1, t_x_2, t_y_1, t_y_2;
	if (kx_1 != k_2 * low_xx && ky_1 != k_2 * low_yy && kx_1 != k_2 * up_xx && ky_1 != k_2 * up_yy) {
		t_x_1 = -(bx_1 - b_2 * low_xx) / (kx_1 - k_2 * low_xx);
		t_y_1 = -(by_1 - b_2 * low_yy) / (ky_1 - k_2 * low_yy);
		t_x_2 = -(bx_1 - b_2 * up_xx) / (kx_1 - k_2 * up_xx);
		t_y_2 = -(by_1 - b_2 * up_yy) / (ky_1 - k_2 * up_yy);
		
		//if (left_b > right_b) {
		//	return false;
		//}
		if (t_x_1*t_x_2 < 0 && t_y_1*t_y_2 < 0) {
			left_b = FMath::Max(FMath::Min(t_x_1, t_x_2), FMath::Min(t_y_1, t_y_2));
			right_b = FMath::Min(FMath::Max(t_x_1, t_x_2), FMath::Max(t_y_1, t_y_2));
		}
		else if (t_x_1*t_x_2 < 0) {
			left_b = FMath::Min(t_x_1, t_x_2);
			right_b = FMath::Max(t_x_1, t_x_2);
		}
		else if (t_y_1*t_y_2 < 0) {
			left_b = FMath::Min(t_y_1, t_y_2);
			right_b = FMath::Max(t_y_1, t_y_2);
		}
		else {
			return false;
		}
		//UE_LOG(LogCarla, Warning, TEXT("INFO low up tt1 %lf %lf %lf %lf"), t_x_1, t_x_2, t_y_1, t_y_2);
		//UE_LOG(LogCarla, Warning, TEXT("INFO low up tt2 %lf %lf %lf %lf"), col0, row0, left_b, right_b);
		return true;
	}
	else {
		return false;
	}
	/*
	if (mapId == 0) {
		// 这里的合法条件还需要进一步考虑
		//  pointVec.X*(pointLoc.Y - (pointLoc.X*low_xx) / mapDep) != pointLoc.X*(pointVec.Y - (low_xx*pointVec.X) / mapDep) && pointVec.X*(pointLoc.Y - (pointLoc.X*low_yy) / mapDep) != pointLoc.X * (pointVec.Y - (low_yy*pointVec.X) / mapDep) 
		if (mapDep*pointVec.Y != low_xx * pointVec.X && mapDep*pointVec.Z != low_yy * pointVec.X) {
			t_x_1 = -(pointLoc.Y - (pointLoc.X*low_xx) / mapDep) / (pointVec.Y - (low_xx*pointVec.X) / mapDep);
			t_y_1 = -(pointLoc.Z - (pointLoc.X*low_yy) / mapDep) / (pointVec.Z - (low_yy*pointVec.X) / mapDep);
		}
		else {
			return false;
		}
		if (mapDep*pointVec.Y != up_xx * pointVec.X && mapDep*pointVec.Z != up_yy * pointVec.X) {
			t_x_2 = -(pointLoc.Y - (pointLoc.X*up_xx) / mapDep) / (pointVec.Y - (up_xx*pointVec.X) / mapDep);
			t_y_2 = -(pointLoc.Z - (pointLoc.X*up_yy) / mapDep) / (pointVec.Z - (up_yy*pointVec.X) / mapDep);
		}
		else {
			return false;
		}
	}
	else if (mapId == 1) {
		if (mapDep*pointVec.Y + FMath::Sqrt(3)*mapDep*pointVec.X != low_xx * (pointVec.X - FMath::Sqrt(3)*pointVec.Y) && 2 * mapDep *pointVec.Z + low_yy * (pointVec.X - FMath::Sqrt(3)*pointVec.Y) != 0) {
			t_x_1 = -(pointLoc.Y + FMath::Sqrt(3)*pointLoc.X - (low_xx*(pointLoc.X - FMath::Sqrt(3)*pointLoc.Y)) / mapDep) / (pointVec.Y + FMath::Sqrt(3)*pointVec.X - (low_xx*(pointVec.Y - FMath::Sqrt(3)*pointVec.Y)) / mapDep);
			t_y_1 = -(2 * pointLoc.Z + (low_yy*(pointLoc.X - FMath::Sqrt(3)*pointLoc.Y)) / mapDep) / (2 * pointVec.Z + (low_yy*(pointVec.X - FMath::Sqrt(3)*pointVec.Y)) / mapDep);
		}
		else {
			return false;
		}
		if (mapDep*pointVec.Y + FMath::Sqrt(3)*mapDep*pointVec.X != up_xx * (pointVec.X - FMath::Sqrt(3)*pointVec.Y) && 2 * mapDep *pointVec.Z + up_yy * (pointVec.X - FMath::Sqrt(3)*pointVec.Y) != 0) {
			t_x_2 = -(pointLoc.Y + FMath::Sqrt(3)*pointLoc.X - (up_xx*(pointLoc.X - FMath::Sqrt(3)*pointLoc.Y)) / mapDep) / (pointVec.Y + FMath::Sqrt(3)*pointVec.X - (up_xx*(pointVec.Y - FMath::Sqrt(3)*pointVec.Y)) / mapDep);
			t_y_2 = -(2 * pointLoc.Z + (up_yy*(pointLoc.X - FMath::Sqrt(3)*pointLoc.Y)) / mapDep) / (2 * pointVec.Z + (up_yy*(pointVec.X - FMath::Sqrt(3)*pointVec.Y)) / mapDep);
		}
		else {
			return false;
		}
	}
	else if (mapId == 2) {
		if (mapDep*pointVec.Y != low_xx * (pointVec.X + FMath::Sqrt(3)*pointVec.Y) + FMath::Sqrt(3)*mapDep*pointVec.X && 2 * mapDep *pointVec.Z + low_yy * (pointVec.X + FMath::Sqrt(3)*pointVec.Y) != 0) {
			t_x_1 = -(FMath::Sqrt(3)*pointLoc.X - pointLoc.Y + (low_xx*(pointLoc.X + FMath::Sqrt(3)*pointLoc.Y)) / mapDep) / (FMath::Sqrt(3)*pointVec.X - pointVec.Y + (low_xx*(pointVec.X + FMath::Sqrt(3)*pointVec.Y)) / mapDep);
			t_y_1 = -(2 * pointLoc.Z + (low_yy*(pointLoc.X + FMath::Sqrt(3)*pointLoc.Y)) / mapDep) / (2 * pointVec.Z + (low_yy*(pointVec.X + FMath::Sqrt(3)*pointVec.Y)) / mapDep);
		}
		else {
			return false;
		}
		if (mapDep*pointVec.Y != up_xx * (pointVec.X + FMath::Sqrt(3)*pointVec.Y) + FMath::Sqrt(3)*mapDep*pointVec.X && 2 * mapDep *pointVec.Z + up_yy * (pointVec.X + FMath::Sqrt(3)*pointVec.Y) != 0) {
			t_x_2 = -(FMath::Sqrt(3)*pointLoc.X - pointLoc.Y + (up_xx*(pointLoc.X + FMath::Sqrt(3)*pointLoc.Y)) / mapDep) / (FMath::Sqrt(3)*pointVec.X - pointVec.Y + (up_xx*(pointVec.X + FMath::Sqrt(3)*pointVec.Y)) / mapDep);
			t_y_2 = -(2 * pointLoc.Z + (up_yy*(pointLoc.X + FMath::Sqrt(3)*pointLoc.Y)) / mapDep) / (2 * pointVec.Z + (up_yy*(pointVec.X + FMath::Sqrt(3)*pointVec.Y)) / mapDep);
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
	left_b = FMath::Max(FMath::Min(t_x_1, t_x_2), FMath::Min(t_y_1, t_y_1));
	right_b = FMath::Min(FMath::Max(t_x_1, t_x_1), FMath::Max(t_y_1, t_y_1));
	return true;
	*/
}

bool ARayCastLidar::BilinearInterpolation(float mapDep, float xxt, float yyt, uint32 map_id, float &laser_dep)
{
	float _col = (ImageWidth - 1) / 2.0 + xxt;
	float _row = (ImageHeight - 1) / 2.0 - yyt;
	if (_col < 0 || _col > ImageWidth - 1 || _row < 0 || _row > ImageHeight - 1) {
		return false;
	}
	int l_col = FMath::FloorToInt(_col);
	int u_row = FMath::FloorToInt(_row);
	int r_col = l_col + 1;
	int d_row = u_row + 1;
	float LaserDep = 0.0;
	//UE_LOG(LogCarla, Warning, TEXT("INFO row col  %lf %lf"), _row, _col);
	if (u_row >= 0 && d_row < (int(ImageHeight)) && l_col >= 0 && r_col < (int(ImageWidth))) {
		
		//return false;
		int index_ld = 4 * (d_row * ImageWidth + l_col);
		int index_lu = 4 * (u_row * ImageWidth + l_col);
		int index_rd = 4 * (d_row * ImageWidth + r_col);
		int index_ru = 4 * (u_row * ImageWidth + r_col);
		//return false;
		double Dep_ld = 0.0, Dep_lu = 0.0, Dep_rd = 0.0, Dep_ru = 0.0;
		{
			FScopeLock ScopeLook(&BufferMutex[map_id]);
			//unsigned char *LidarBufferPointer = LidarBuffer[map_id].data();
			carla::Buffer &lbuffer = LidarBuffer[map_id];
			//UE_LOG(LogCarla, Warning, TEXT("INFO row col  %d %d %d %d %d"), index_ld, index_lu, index_rd, index_ru, lbuffer.size());
			
			Dep_ld = ((int(lbuffer[index_ld])) * 256.0 * 256.0 + (int(lbuffer[index_ld + 1])) * 256.0 + (int)(lbuffer[index_ld + 2])) / (256.0 * 256.0 * 256.0 - 1.0) * 100000.0;
			Dep_lu = ((int(lbuffer[index_lu])) * 256.0 * 256.0 + (int(lbuffer[index_lu + 1])) * 256.0 + (int)(lbuffer[index_lu + 2])) / (256.0 * 256.0 * 256.0 - 1.0) * 100000.0;
			Dep_rd = ((int(lbuffer[index_rd])) * 256.0 * 256.0 + (int(lbuffer[index_rd + 1])) * 256.0 + (int)(lbuffer[index_rd + 2])) / (256.0 * 256.0 * 256.0 - 1.0) * 100000.0;
			Dep_ru = ((int(lbuffer[index_ru])) * 256.0 * 256.0 + (int(lbuffer[index_ru + 1])) * 256.0 + (int)(lbuffer[index_ru + 2])) / (256.0 * 256.0 * 256.0 - 1.0) * 100000.0;
		}
		LaserDep = Dep_ld * (r_col - _col)*(1.0 - d_row + _row)
			+ Dep_rd * (_col - l_col)*(1.0 - d_row + _row)
			+ Dep_lu * (r_col - _col)*(1.0 - _row + u_row)
			+ Dep_ru * (_col - l_col)*(1.0 - _row + u_row);

		LaserDep = LaserDep / mapDep * FMath::Sqrt(mapDep*mapDep + xxt * xxt + yyt * yyt);
		if (LaserDep > 100000.0) {
			LaserDep = 100000.0;
		}
		laser_dep = LaserDep;
		return true;
	}
	else{
		return false;
	}
}

bool ARayCastLidar::CalMinLoss(const uint32 mapId, const FVector pointLoc, const FVector pointVec, float mapDep, float &min_loss, float &min_t)
{
	float kx_1, bx_1, ky_1, by_1, k_2, b_2;
	CalConfficient(mapId, pointLoc, pointVec, mapDep, kx_1, bx_1, ky_1, by_1, k_2, b_2);

	float now_dep = 0.0, now_xx = 0.0, now_yy = 0.0;
	if (!CalDepmapCoordinate(kx_1, bx_1, ky_1, by_1, k_2, b_2, 0.0, now_xx, now_yy)) {
		//UE_LOG(LogCarla, Warning, TEXT("INFO bxdd4"));
		return false;
	}
	if (!BilinearInterpolation(mapDep, now_xx, now_yy, mapId, now_dep)) {
		//UE_LOG(LogCarla, Warning, TEXT("INFO bxdd7"));
		return false;
	}

	min_loss = FMath::Abs(pointLoc.Size() - now_dep);
	//UE_LOG(LogCarla, Warning, TEXT("INFO loss %lf"), min_loss);
	min_t = 0;
	//return true;
	//UE_LOG(LogCarla, Warning, TEXT("INFO t0 %lf %lf"), min_loss, min_t);
	float low_tt, up_tt;
	if (!CalDepmapTRange(kx_1, bx_1, ky_1, by_1, k_2, b_2, low_tt, up_tt)) {
		//UE_LOG(LogCarla, Warning, TEXT("INFO bxdd1"));
		//return true;
		low_tt = -pointLoc.Size() / 2.0;
		up_tt = pointLoc.Size() / 2.0;
		
	}
	else {
		//if (low_tt < -pointLoc.Size() || up_tt > pointLoc.Size()) {
		//	return true;
		//}
		low_tt = FMath::Max(low_tt, float(-pointLoc.Size()/2.0f));
		up_tt = FMath::Min(up_tt, float(pointLoc.Size() / 2.0f));
	}

	float low_xx = 0.0, low_yy = 0.0, up_xx = 0.0, up_yy = 0.0;
	
	if (!CalDepmapCoordinate(kx_1, bx_1, ky_1, by_1, k_2, b_2, low_tt, low_xx, low_yy)) {
		//UE_LOG(LogCarla, Warning, TEXT("INFO bxdd2"));
		return true;
	}
	if (!CalDepmapCoordinate(kx_1, bx_1, ky_1, by_1, k_2, b_2, up_tt, up_xx, up_yy)) {
		//UE_LOG(LogCarla, Warning, TEXT("INFO bxdd3"));
		return true;
	}
	
	
	//UE_LOG(LogCarla, Warning, TEXT("INFO bxdd1  %lf %lf %lf %lf %lf %lf"), kx_1, bx_1, ky_1, by_1, k_2, b_2);
	//UE_LOG(LogCarla, Warning, TEXT("INFO bxdd2  %d %lf %lf %lf %lf %lf"), mapId, pointLoc.X, pointLoc.Y, pointLoc.Z, now_xx, now_yy);
	float low_dep = 0.0, up_dep = 0.0;
	if (!BilinearInterpolation(mapDep, low_xx, low_yy, mapId, low_dep)) {
		//UE_LOG(LogCarla, Warning, TEXT("INFO bxdd5"));
		return true;
	}
	if (!BilinearInterpolation(mapDep, up_xx, up_yy, mapId, up_dep)) {
		//UE_LOG(LogCarla, Warning, TEXT("INFO bxdd6"));
		return true;
	}

	float check_parrel = (pointLoc.GetSafeNormal() ^ pointVec).Size();
	if (check_parrel < 1e-3) {
		float rdep = (low_dep + up_dep) / 2.0;
		min_loss = 0;
		//UE_LOG(LogCarla, Warning, TEXT("INFO loss %lf"), min_loss);
		min_t = rdep - pointLoc.Size();
		return true;
	}
	
	//UE_LOG(LogCarla, Warning, TEXT("INFO bxdd8"));
	
	float cs1 = (pointLoc | pointVec) / pointLoc.Size();
	float eq_c, eq_v, k_1, b_1 ,temp_loss;
	bool XYFlag;
	if (FMath::Abs(low_xx - up_xx) >= FMath::Abs(low_yy - up_yy)) {
		XYFlag = true;
		eq_c = pointLoc.Size() + low_xx / (up_xx - low_xx) * (up_dep - low_dep) - low_dep;
		eq_v = (up_dep - low_dep) / (up_xx - low_xx);
		k_1 = kx_1;
		b_1 = bx_1;
		
		temp_loss = FMath::Abs(eq_c + cs1*low_tt - eq_v * low_xx);
		if (temp_loss < min_loss) {
			min_loss = temp_loss;
			min_t = low_tt;
		}
		temp_loss = FMath::Abs(eq_c + cs1*up_tt - eq_v * up_xx);
		if (temp_loss < min_loss) {
			min_loss = temp_loss;
			min_t = up_tt;
		}
		
	}
	else {
		XYFlag = false;
		eq_c = pointLoc.Size() + low_yy / (up_yy - low_yy) * (up_dep - low_dep) - low_dep;
		eq_v = (up_dep - low_dep) / (up_yy - low_yy);
		k_1 = ky_1;
		b_1 = by_1;
		
		temp_loss = FMath::Abs(eq_c + cs1*low_tt - eq_v * low_yy);
		if (temp_loss < min_loss) {
			min_loss = temp_loss;
			min_t = low_tt;
		}
		temp_loss = FMath::Abs(eq_c + cs1*up_tt - eq_v * up_yy);
		if (temp_loss < min_loss) {
			min_loss = temp_loss;
			min_t = up_tt;
		}
		
	}
	
	if (cs1 * k_2 != 0) {
		float tt_0, tt_1, tt_2, tt_3;
		float check_tt01 = b_2 * b_2 * cs1 * cs1 - 2 * b_2*eq_c*cs1*k_2 - 2 * b_2*cs1*k_1*eq_v + eq_c * eq_c * k_2*k_2 - 2 * eq_c*k_2*k_1*eq_v + 4 * b_1*cs1*k_2*eq_v + k_1 * k_1*eq_v*eq_v;
		float check_tt23 = cs1 * eq_v * (b_2*k_1 - b_1 * k_2);
		if (check_tt01 >= 0) {
			float base_tt01 = FMath::Sqrt(check_tt01);
			tt_0 = -(b_2*cs1 + eq_c * k_2 - k_1 * eq_v - base_tt01) / (2 * cs1 * k_2);
			tt_1 = -(b_2*cs1 + eq_c * k_2 - k_1 * eq_v + base_tt01) / (2 * cs1 * k_2);
			if (tt_0 > low_tt && tt_0 < up_tt) {
				if (XYFlag) {
					temp_loss = FMath::Abs(eq_c + cs1*tt_0 - eq_v * (bx_1 + tt_0 * kx_1) / (b_2 + tt_0 * k_2));
				}
				else {
					temp_loss = FMath::Abs(eq_c + cs1*tt_0 - eq_v * (by_1 + tt_0 * ky_1) / (b_2 + tt_0 * k_2));
				}
				if (temp_loss < min_loss) {
					min_loss = temp_loss;
					min_t = tt_0;
					//UE_LOG(LogCarla, Warning, TEXT("INFO t1 %lf %lf"), temp_loss, tt_0);
				}
			}
			if (tt_1 > low_tt && tt_1 < up_tt) {
				if (XYFlag) {
					temp_loss = FMath::Abs(eq_c + cs1*tt_1 - eq_v * (bx_1 + tt_1 * kx_1) / (b_2 + tt_1 * k_2));
				}
				else {
					temp_loss = FMath::Abs(eq_c + cs1*tt_1 - eq_v * (by_1 + tt_1 * ky_1) / (b_2 + tt_1 * k_2));
				}
				if (temp_loss < min_loss) {
					min_loss = temp_loss;
					min_t = tt_1;
					//UE_LOG(LogCarla, Warning, TEXT("INFO t2 %lf %lf"), temp_loss, tt_1);
				}
			}
		}
		if (check_tt23 >= 0) {
			float base_tt23 = FMath::Sqrt(check_tt23);
			tt_2 = -(b_2*cs1 - base_tt23) / (cs1* k_2);
			tt_3 = -(b_2*cs1 + base_tt23) / (cs1 *k_2);
			if (tt_2 > low_tt && tt_2 < up_tt) {
				if (XYFlag) {
					temp_loss = FMath::Abs(eq_c + cs1 * tt_2 - eq_v * (bx_1 + tt_2 * kx_1) / (b_2 + tt_2 * k_2));
				}
				else {
					temp_loss = FMath::Abs(eq_c + cs1 * tt_2 - eq_v * (by_1 + tt_2 * ky_1) / (b_2 + tt_2 * k_2));
				}
				if (temp_loss < min_loss) {
					min_loss = temp_loss;
					min_t = tt_2;
					//UE_LOG(LogCarla, Warning, TEXT("INFO t3 %lf %lf"), temp_loss, tt_2);
				}
			}
			if (tt_3 > low_tt && tt_3 < up_tt) {
				if (XYFlag) {
					temp_loss = FMath::Abs(eq_c + cs1 * tt_3 - eq_v * (bx_1 + tt_3 * kx_1) / (b_2 + tt_3 * k_2));
				}
				else {
					temp_loss = FMath::Abs(eq_c + cs1 * tt_3 - eq_v * (by_1 + tt_3 * ky_1) / (b_2 + tt_3 * k_2));
				}
				if (temp_loss < min_loss) {
					min_loss = temp_loss;
					min_t = tt_3;
					//UE_LOG(LogCarla, Warning, TEXT("INFO t4 %lf %lf"), temp_loss, tt_3);
				}
			}
		}
	}
	return true;
}

bool ARayCastLidar::ShootLaser(const uint32 lidarId, const FVector LidarRelativeLoc, const FRotator LidarRelativeRot, const uint32 PointIndex, const uint32 Channel, const float HorizontalAngle, const bool gpuMode, FVector &XYZ, float &XYZLen)
{
	
	const float VerticalAngle = LaserAngles[Channel];
	const float DeltaLidarAngle = 360.0 / MaxCaptureCnt;

	FRotator OriRot = GetActorRotation();
	FVector OriLoc = GetActorLocation();

	FRotator LidarRot = UKismetMathLibrary::ComposeRotators(
		LidarRelativeRot,
		OriRot
	);
	
	FTransform LidarTrans = FTransform(LidarRelativeLoc) * FTransform(OriRot);

	FVector LidarLoc = OriLoc + LidarTrans.GetLocation();

	FRotator LaserSelfRot = FRotator(VerticalAngle, HorizontalAngle, 0);

	FRotator LaserRelativeRot= UKismetMathLibrary::ComposeRotators(
		LaserSelfRot,
		LidarRelativeRot
	);

	FRotator LaserRot = UKismetMathLibrary::ComposeRotators(
		LaserRelativeRot,
		OriRot
	);

	FVector ImpactPoint;
	if ((!gpuMode) || LidarLaserPointDepth[lidarId][Channel][PointIndex] < 0) {
		if (gpuMode) {
			each_count[0] ++;
		}
		if (CalImpactPoint(LaserRot, LidarLoc, ImpactPoint)) {
			float VecLength = (ImpactPoint - LidarLoc).Size();
			if (gpuMode) {
				LidarLaserPointDepth[lidarId][Channel][PointIndex] = VecLength;
			}

			//FVector vtest = VecLength * UKismetMathLibrary::GetForwardVector(LaserRelativeRot) + LidarRelativeLoc;
			//UE_LOG(LogCarla, Warning, TEXT("len1 %lf, len2 %lf"), (ImpactPoint - OriLoc).Size(), vtest.Size());
			//FVector ImpactPoint_2;
			//CalImpactPoint((ImpactPoint - OriLoc).Rotation(), OriLoc, ImpactPoint_2);

			//float len_ori = (ImpactPoint_2 - OriLoc).Size();
			//float len_lidar = (ImpactPoint - OriLoc).Size();
			//UE_LOG(LogCarla, Warning, TEXT("len ori %lf, lidar %lf, delta %lf"), len_ori, len_lidar, len_ori - len_lidar);

			if (VecLength <= Description.Range) {
				XYZ = -VecLength * UKismetMathLibrary::GetForwardVector(LaserSelfRot);
				XYZ = UKismetMathLibrary::RotateAngleAxis(
					XYZ,
					90,
					FVector(0, 0, 1)
				);
				XYZLen = VecLength;
				return true;
			}
			else {
				XYZLen = VecLength;
				return false;
			}
		}
		else if(gpuMode){
			LidarLaserPointDepth[lidarId][Channel][PointIndex] = 100000.0;
			XYZLen = 100000.0;
			return false;
		}
	}
	
	FVector RelativeImpactPoint = LidarLaserPointDepth[lidarId][Channel][PointIndex] * UKismetMathLibrary::GetForwardVector(LaserRelativeRot) + LidarRelativeLoc;
	


	//FVector LidarLocation = OriLocation
	//FRotator LidarBodyRot = GetActorRotation();
	//FRotator LidarBodyRot = GetActorRotation();
	//UE_LOG(LogCarla, Warning, TEXT("Rot %lf, %lf, %lf"), LidarBodyRot.Pitch, LidarBodyRot.Yaw, LidarBodyRot.Roll);
	FRotator LaserPointRelativeRot = RelativeImpactPoint.Rotation();// float InPitch, float InYaw, float InRoll
	//FRotator ResultRot = UKismetMathLibrary::ComposeRotators(
	//	LaserPointRelativeRot,
	//	LidarBodyRot
	//);
	float LaserPointPitch = LaserPointRelativeRot.Pitch;
	float LaserPointYaw = LaserPointRelativeRot.Yaw - floor(LaserPointRelativeRot.Yaw / 360.0) * 360.0;

	float LaserPointYawBorder = LaserPointYaw + DeltaLidarAngle / 2;
	if (LaserPointYawBorder >= 360.0) {
		LaserPointYawBorder -= 360.0;
	}

	int LidarCameraId = LaserPointYawBorder / DeltaLidarAngle;
	if (LidarCameraId < 0) { 
		LidarCameraId = 0;
	}
	if (LidarCameraId >= MaxCaptureCnt) {
		LidarCameraId = MaxCaptureCnt - 1;
	}
	//LidarCameraId = 0;
	/*
	if(FlagMemLidar != LidarCameraId)
	{
		{
			FScopeLock ScopeLook(&BufferMutex);
			if (!FlagLidarBuffer[LidarCameraId]) {
				return false;
			}
			NowLidarBufferSize = LidarBuffer[LidarCameraId].size();
			//UE_LOG(LogCarla, Warning, TEXT("Lidar Buffer Size %d"), NowLidarBufferSize);
			memcpy(NowLidarBuffer, LidarBuffer[LidarCameraId].data(), (size_t)(sizeof(unsigned char)*NowLidarBufferSize));
		}
		FlagMemLidar = LidarCameraId;
	}
	*/

	if (!FlagLidarBuffer[LidarCameraId]) {
		return false;
	}


	float ImageDep = (ImageWidth/ 2.0) / FMath::Tan(DeltaLidarAngle / 2.0 / 180.0 * PI);
	FVector unit_forward_vector = (RelativeImpactPoint - LidarRelativeLoc).GetSafeNormal();

	//FVector ImpactPoint_1;
	//if (CalImpactPoint(LaserRot, LidarLoc, ImpactPoint) && CalImpactPoint((ImpactPoint - OriLoc).Rotation(), OriLoc, ImpactPoint_1)) {
	//	float len_1 = (ImpactPoint - OriLoc).Size();
	//	float len_2 = (ImpactPoint_1 - OriLoc).Size();
	//	float len_3 = RelativeImpactPoint.Size();
	//	UE_LOG(LogCarla, Warning, TEXT("len1 %lf, len2 %lf, len3 %lf"), len_1, len_2, len_3);
	//}
	

	//float len_ori = (ImpactPoint_2 - OriLoc).Size();
	//float len_lidar = (ImpactPoint - OriLoc).Size();
	//UE_LOG(LogCarla, Warning, TEXT("len ori %lf, lidar %lf, delta %lf"), len_ori, len_lidar, len_ori - len_lidar);

	float min_loss, min_tt;
	if (CalMinLoss(LidarCameraId, RelativeImpactPoint, unit_forward_vector, ImageDep, min_loss, min_tt)) {
		//UE_LOG(LogCarla, Warning, TEXT("INFO loss  %lf %lf"), min_loss, min_tt);
		//UE_LOG(LogCarla, Warning, TEXT("INFO tt %lf %lf"), min_loss, min_tt);
		//if (FMath::Abs(min_tt) > 1e-5) {
		//	UE_LOG(LogCarla, Warning, TEXT("INFO loss  %lf %lf"), min_loss, min_tt);
		//}
		if (min_loss < 30) {
			if (gpuMode) {
				each_count[1] ++;
			}
			//min_tt = 0;
			float VecLength = (RelativeImpactPoint - LidarRelativeLoc).Size() + min_tt;
			
			LidarLaserPointDepth[lidarId][Channel][PointIndex] = VecLength;
			if (pc_all && VecLength <= Description.Range) {
				XYZ = -VecLength * UKismetMathLibrary::GetForwardVector(LaserSelfRot);
				XYZ = UKismetMathLibrary::RotateAngleAxis(
					XYZ,
					90,
					FVector(0, 0, 1)
				);
				XYZLen = VecLength;
				return true;
			}
			else {
				XYZLen = VecLength;
				return false;
			}
		}
		else {
			if (gpuMode) {
				each_count[2] ++;
			}
			if (CalImpactPoint(LaserRot, LidarLoc, ImpactPoint)) {
				float VecLength = (ImpactPoint - LidarLoc).Size();
				LidarLaserPointDepth[lidarId][Channel][PointIndex] = VecLength;
				if (VecLength <= Description.Range) {
					XYZ = -VecLength * UKismetMathLibrary::GetForwardVector(LaserSelfRot);
					XYZ = UKismetMathLibrary::RotateAngleAxis(
						XYZ,
						90,
						FVector(0, 0, 1)
					);
					XYZLen = VecLength;
					return true;
				}
				else {
					XYZLen = VecLength;
					return false;
				}
			}
			else {
				LidarLaserPointDepth[lidarId][Channel][PointIndex] = 100000.0;
				XYZLen = 100000.0;
				return false;
			}
		}
	}
	else {
		if (gpuMode) {
			each_count[3] ++;
		}
		if (CalImpactPoint(LaserRot, LidarLoc, ImpactPoint)) {
			float VecLength = (ImpactPoint - LidarLoc).Size();
			LidarLaserPointDepth[lidarId][Channel][PointIndex] = VecLength;
			if (VecLength <= Description.Range) {
				XYZ = -VecLength * UKismetMathLibrary::GetForwardVector(LaserSelfRot);
				XYZ = UKismetMathLibrary::RotateAngleAxis(
					XYZ,
					90,
					FVector(0, 0, 1)
				);
				XYZLen = VecLength;
				return true;
			}
			else {
				XYZLen = VecLength;
				return false;
			}
		}
		else {
			LidarLaserPointDepth[lidarId][Channel][PointIndex] = 100000.0;
			XYZLen = 100000.0;
			return false;
		}
	}
}



bool ARayCastLidar::ShootLaser2(const uint32 Channel, const float HorizontalAngle, FVector &XYZ, float &VecLen)
{

	const float VerticalAngle = LaserAngles[Channel];
	const float DeltaLidarAngle = 360.0 / MaxCaptureCnt;

	//FRotator LidarBodyRot = GetActorRotation();
	FRotator LidarBodyRot = GetActorRotation();
	//UE_LOG(LogCarla, Warning, TEXT("Rot %lf, %lf, %lf"), LidarBodyRot.Pitch, LidarBodyRot.Yaw, LidarBodyRot.Roll);
	FRotator LaserRelativeRot(VerticalAngle, HorizontalAngle, 0);// float InPitch, float InYaw, float InRoll
	//FRotator ResultRot = UKismetMathLibrary::ComposeRotators(
	//	LaserRelativeRot,
	//	LidarBodyRot
	//);
	float LaserPitch = LaserRelativeRot.Pitch;
	float LaserYaw = LaserRelativeRot.Yaw - floor(LaserRelativeRot.Yaw / 360.0) * 360.0;

	float LaserYawBorder = LaserYaw + DeltaLidarAngle / 2;
	if (LaserYawBorder >= 360.0) {
		LaserYawBorder -= 360.0;
	}

	int LidarCameraId = LaserYawBorder / DeltaLidarAngle;
	if (LidarCameraId < 0) {
		LidarCameraId = 0;
	}
	if (LidarCameraId >= MaxCaptureCnt) {
		LidarCameraId = MaxCaptureCnt - 1;
	}
	//LidarCameraId = 0;
	/*
	if(FlagMemLidar != LidarCameraId)
	{
		{
			FScopeLock ScopeLook(&BufferMutex);
			if (!FlagLidarBuffer[LidarCameraId]) {
				return false;
			}
			NowLidarBufferSize = LidarBuffer[LidarCameraId].size();
			//UE_LOG(LogCarla, Warning, TEXT("Lidar Buffer Size %d"), NowLidarBufferSize);
			memcpy(NowLidarBuffer, LidarBuffer[LidarCameraId].data(), (size_t)(sizeof(unsigned char)*NowLidarBufferSize));
		}
		FlagMemLidar = LidarCameraId;
	}
	*/

	if (!FlagLidarBuffer[LidarCameraId]) {
		return false;
	}


	float LaserSubRelativePitch = LaserPitch;
	float LaserSubRelativeYaw = LaserYawBorder - LidarCameraId * DeltaLidarAngle - DeltaLidarAngle / 2.0;
	float ImageDep = (ImageWidth / 2.0) / FMath::Tan(DeltaLidarAngle / 2.0 / 180.0 * PI);
	FVector LaserImageVector = UKismetMathLibrary::GetForwardVector(FRotator(LaserSubRelativePitch, LaserSubRelativeYaw, 0));
	double LaserImageCol_f = (ImageWidth - 1) / 2.0 + ImageDep / LaserImageVector.X * LaserImageVector.Y;
	double LaserImageRow_f = (ImageHeight - 1) / 2.0 - ImageDep / LaserImageVector.X * LaserImageVector.Z;
	uint32 LaserImageCol_l = FMath::FloorToInt(float(LaserImageCol_f));
	uint32 LaserImageRow_u = FMath::FloorToInt(float(LaserImageRow_f));
	uint32 LaserImageCol_r = LaserImageCol_l + 1;
	uint32 LaserImageRow_d = LaserImageRow_u + 1;
	double LaserDep = 0.0;
	if (LaserImageRow_u >= 0 && LaserImageRow_d < ImageHeight && LaserImageCol_l >= 0 && LaserImageCol_r < ImageWidth) {
		uint32 IndexLeftDown = 4 * (LaserImageRow_d * ImageWidth + LaserImageCol_l);
		uint32 IndexLeftUp = 4 * (LaserImageRow_u * ImageWidth + LaserImageCol_l);
		uint32 IndexRightDown = 4 * (LaserImageRow_d * ImageWidth + LaserImageCol_r);
		uint32 IndexRightUp = 4 * (LaserImageRow_u * ImageWidth + LaserImageCol_r);
		double Dep_ld = 0.0, Dep_lu = 0.0, Dep_rd = 0.0, Dep_ru = 0.0;

		{
			FScopeLock ScopeLook(&BufferMutex[LidarCameraId]);
			unsigned char *LidarBufferPointer = LidarBuffer[LidarCameraId].data();
			Dep_ld = ((int(LidarBufferPointer[IndexLeftDown])) * 256.0 * 256.0 + (int(LidarBufferPointer[IndexLeftDown + 1])) * 256.0 + (int)(LidarBufferPointer[IndexLeftDown + 2])) / (256.0 * 256.0 * 256.0 - 1.0) * 100000.0;
			Dep_lu = ((int(LidarBufferPointer[IndexLeftUp])) * 256.0 * 256.0 + (int(LidarBufferPointer[IndexLeftUp + 1])) * 256.0 + (int)(LidarBufferPointer[IndexLeftUp + 2])) / (256.0 * 256.0 * 256.0 - 1.0) * 100000.0;
			Dep_rd = ((int(LidarBufferPointer[IndexRightDown])) * 256.0 * 256.0 + (int(LidarBufferPointer[IndexRightDown + 1])) * 256.0 + (int)(LidarBufferPointer[IndexRightDown + 2])) / (256.0 * 256.0 * 256.0 - 1.0) * 100000.0;
			Dep_ru = ((int(LidarBufferPointer[IndexRightUp])) * 256.0 * 256.0 + (int(LidarBufferPointer[IndexRightUp + 1])) * 256.0 + (int)(LidarBufferPointer[IndexRightUp + 2])) / (256.0 * 256.0 * 256.0 - 1.0) * 100000.0;
		}
		LaserDep = Dep_ld * (LaserImageCol_r - LaserImageCol_f)*(1.0 - LaserImageRow_d + LaserImageRow_f)
			+ Dep_rd * (LaserImageCol_f - LaserImageCol_l)*(1.0 - LaserImageRow_d + LaserImageRow_f)
			+ Dep_lu * (LaserImageCol_r - LaserImageCol_f)*(1.0 - LaserImageRow_f + LaserImageRow_u)
			+ Dep_ru * (LaserImageCol_f - LaserImageCol_l)*(1.0 - LaserImageRow_f + LaserImageRow_u);
	}
	else {
		uint32 LaserImageRow = FMath::RoundToInt(LaserImageRow_f);
		uint32 LaserImageCol = FMath::RoundToInt(LaserImageCol_f);
		if (LaserImageCol < 0) {
			LaserImageCol = 0;
		}
		if (LaserImageCol >= ImageWidth) {
			LaserImageCol = ImageWidth - 1;
		}
		if (LaserImageRow < 0) {
			LaserImageRow = 0;
		}
		if (LaserImageRow >= ImageHeight) {
			LaserImageRow = ImageHeight - 1;
		}
		uint32 LaserImageIndex = 4 * (LaserImageRow * ImageWidth + LaserImageCol);
		{
			FScopeLock ScopeLook(&BufferMutex[LidarCameraId]);
			unsigned char *LidarBufferPointer = LidarBuffer[LidarCameraId].data();
			LaserDep = ((int(LidarBufferPointer[LaserImageIndex])) * 256.0 * 256.0 + (int(LidarBufferPointer[LaserImageIndex + 1])) * 256.0 + (int)(LidarBufferPointer[LaserImageIndex + 2])) / (256.0 * 256.0 * 256.0 - 1.0) * 100000.0;
		}
	}
	LaserDep /= FMath::Cos(FMath::DegreesToRadians(LaserSubRelativePitch)) * FMath::Cos(FMath::DegreesToRadians(LaserSubRelativeYaw));

	//UE_LOG(LogCarla, Warning, TEXT("Dep :%d, (%lf, %lf) Row %d, Col %d, Index %d, Cid %d, Hor %lf"), LaserDep, LaserSubRelativePitch, LaserSubRelativeYaw, LaserImageRow, LaserImageCol, LaserImageIndex/4, LidarCameraId, HorizontalAngle);
/*
XYZ = UKismetMathLibrary::RotateAngleAxis(
	FVector(5000, 0, 0),
	HorizontalAngle,
	FVector(0, 0, 1)
);
*/
//XYZ = FVector(0, 2000 ,0);
//return true;
	if (LaserDep <= Description.Range) {
		FVector LaserVector = -LaserDep * UKismetMathLibrary::GetForwardVector(LaserRelativeRot);
		//float OriLaserVectorZ = LaserDep * FMath::Sin(LaserSubRelativePitch / 180.0 * PI);
		//float OriLaserVectorX = LaserDep * FMath::Cos(LaserSubRelativePitch / 180.0 * PI) * FMath::Cos((LaserSubRelativeYaw + 90.0 - (DeltaLidarAngle / 2)) / 180.0 * PI);
		//float OriLaserVectorY = LaserDep * FMath::Cos(LaserSubRelativePitch / 180.0 * PI) * FMath::Sin((LaserSubRelativeYaw + 90.0 - (DeltaLidarAngle / 2)) / 180.0 * PI);
		//float newLaserVectorX = FMath::Cos(LidarCameraId * DeltaLidarAngle / 180.0 * PI) * OriLaserVectorX - FMath::Sin(LidarCameraId * DeltaLidarAngle / 180.0 * PI) * OriLaserVectorY;
		//float newLaserVectorY = FMath::Sin(LidarCameraId * DeltaLidarAngle / 180.0 * PI) * OriLaserVectorX + FMath::Cos(LidarCameraId * DeltaLidarAngle / 180.0 * PI) * OriLaserVectorY;
		//XYZ = FVector(newLaserVectorX, newLaserVectorY, OriLaserVectorZ);

		XYZ = UKismetMathLibrary::RotateAngleAxis(
			LaserVector,
			90,
			FVector(0, 0, 1)
		);
		VecLen = LaserDep;
		return true;
	}
	else {
		VecLen = LaserDep;
		return false;
	}
}

