// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <PxScene.h>
#include <cmath>
#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Sensor/RayCastSemanticLidar.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/geom/Math.h"
#include <compiler/enable-ue4-macros.h>

#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"

#include "LivoxHorizon.h"
#include <iostream>
#include <sstream>
#include <string>

namespace crp = carla::rpc;

FActorDefinition ARayCastSemanticLidar::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeLidarDefinition(TEXT("ray_cast_semantic"));
}

ARayCastSemanticLidar::ARayCastSemanticLidar(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
}

void ARayCastSemanticLidar::Set(const FActorDescription &ActorDescription)
{
  Super::Set(ActorDescription);
  FLidarDescription LidarDescription;
  UActorBlueprintFunctionLibrary::SetLidar(ActorDescription, LidarDescription);
  Set(LidarDescription);
}

void ARayCastSemanticLidar::Set(const FLidarDescription &LidarDescription)
{
  Description = LidarDescription;
  SemanticLidarData = FSemanticLidarData(Description.Channels);
  CreateLasers();
  PointsPerChannel.resize(Description.Channels);
}

void ARayCastSemanticLidar::CreateLasers()
{
  const auto NumberOfLasers = Description.Channels;
  check(NumberOfLasers > 0u);
  const float DeltaAngle = NumberOfLasers == 1u ? 0.f : (Description.UpperFovLimit - Description.LowerFovLimit) / static_cast<float>(NumberOfLasers - 1);
  LaserAngles.Empty(NumberOfLasers);
  for (auto i = 0u; i < NumberOfLasers; ++i)
  {
    const float VerticalAngle =
        Description.UpperFovLimit - static_cast<float>(i) * DeltaAngle;
    LaserAngles.Emplace(VerticalAngle);
  }
}

// livox
void ARayCastSemanticLidar::CreateLasers_livox() 
{
  const auto NumberOfLasers = Description.Channels;
  check(NumberOfLasers > 0u);
  const float DeltaAngle = NumberOfLasers == 1u ? 0.f : (Description.UpperFovLimit - Description.LowerFovLimit) / static_cast<float>(NumberOfLasers - 1);
  LaserAngles.Empty(NumberOfLasers);
  for (auto i = 0u; i < NumberOfLasers; ++i)
  {
    const float VerticalAngle =
        Description.UpperFovLimit - static_cast<float>(i) * DeltaAngle;
    LaserAngles.Emplace(VerticalAngle);
  }

  // livox csv file reading
  clock_t start = clock();
  LivoxCsvInfo = LivoxCsvReader();
  clock_t finish = clock();
  double TIME = finish - start;
  LivoxSize = LivoxCsvInfo.size();
      UE_LOG(
        LogCarla,
        Display,
        TEXT("Livox lasers created")
        );
}

void ARayCastSemanticLidar::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ARayCastSemanticLidar::PostPhysTick);
  SimulateLidar(DeltaTime);

  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("Send Stream");
    auto DataStream = GetDataStream(*this);
    DataStream.Send(*this, SemanticLidarData, DataStream.PopBufferFromPool());
  }
}

void ARayCastSemanticLidar::SimulateLidar(const float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ARayCastSemanticLidar::SimulateLidar);
  const uint32 ChannelCount = Description.Channels;
  const uint32 PointsToScanWithOneLaser =
      FMath::RoundHalfFromZero(
          Description.PointsPerSecond * DeltaTime / float(ChannelCount));

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
      SemanticLidarData.GetHorizontalAngle());
  const float AngleDistanceOfTick = Description.RotationFrequency * Description.HorizontalFov * DeltaTime;
  const float AngleDistanceOfLaserMeasure = AngleDistanceOfTick / PointsToScanWithOneLaser;

  ResetRecordedHits(ChannelCount, PointsToScanWithOneLaser);
  PreprocessRays(ChannelCount, PointsToScanWithOneLaser);

  GetWorld()->GetPhysicsScene()->GetPxScene()->lockRead();
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(ParallelFor);
    ParallelFor(ChannelCount, [&](int32 idxChannel)
                {
      TRACE_CPUPROFILER_EVENT_SCOPE(ParallelForTask);

      FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Laser_Trace")), true, this);
      TraceParams.bTraceComplex = true;
      TraceParams.bReturnPhysicalMaterial = false;

      for (auto idxPtsOneLaser = 0u; idxPtsOneLaser < PointsToScanWithOneLaser; idxPtsOneLaser++) {
        FHitResult HitResult;
        const float VertAngle = LaserAngles[idxChannel];
        const float HorizAngle = std::fmod(CurrentHorizontalAngle + AngleDistanceOfLaserMeasure
            * idxPtsOneLaser, Description.HorizontalFov) - Description.HorizontalFov / 2;
        const bool PreprocessResult = RayPreprocessCondition[idxChannel][idxPtsOneLaser];
        if (PreprocessResult && ShootLaser(VertAngle, HorizAngle, HitResult, TraceParams)) {
          WritePointAsync(idxChannel, HitResult);
        }
      }; });
  }
  GetWorld()->GetPhysicsScene()->GetPxScene()->unlockRead();

  FTransform ActorTransf = GetTransform();
  ComputeAndSaveDetections(ActorTransf);

  const float HorizontalAngle = carla::geom::Math::ToRadians(
      std::fmod(CurrentHorizontalAngle + AngleDistanceOfTick, Description.HorizontalFov));
  SemanticLidarData.SetHorizontalAngle(HorizontalAngle);
}

// livox simulation
void ARayCastSemanticLidar::SimulateLidar_livox(const float DeltaTime) 
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ARayCastSemanticLidar::SimulateLidar);
  const uint32 ChannelCount = Description.Channels;

  float decayTime = Description.Decay; // Point cloud density control parameter
  const uint32 PointsToScanWithOneLaser = FMath::RoundHalfFromZero(float(LivoxSize) * DeltaTime * decayTime);

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
      SemanticLidarData.GetHorizontalAngle());
  const float AngleDistanceOfTick = Description.RotationFrequency * Description.HorizontalFov * DeltaTime;
  const float AngleDistanceOfLaserMeasure = AngleDistanceOfTick / PointsToScanWithOneLaser;

  ResetRecordedHits(ChannelCount, PointsToScanWithOneLaser);
  PreprocessRays(ChannelCount, PointsToScanWithOneLaser); // MaxPointsPerChannel

  GetWorld()->GetPhysicsScene()->GetPxScene()->lockRead(); 
  {
    TRACE_CPUPROFILER_EVENT_SCOPE(ParallelFor);
    ParallelFor(ChannelCount, [&](int32 idxChannel)
                {
                    TRACE_CPUPROFILER_EVENT_SCOPE(ParallelForTask);

                    FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Laser_Trace")), true, this);
                    TraceParams.bTraceComplex = true;
                    TraceParams.bReturnPhysicalMaterial = false;

                    int i_count = LivoxCount;

                    
                    int i_limit = i_count + FMath::RoundHalfFromZero(float(LivoxSize) * DeltaTime * decayTime); 


                    int RayCheck = 0;
                    for (int i = i_count; i < i_limit; i++)
                    {
                        
                        if (i >= LivoxSize)
                        {
                        i_count = 0;
                        i = 0;
                        i_limit = i_limit - LivoxSize; 
                        
                        LivoxCount = i;
                        continue;
                        }

                        FHitResult HitResult;
                        float simTime = LivoxCsvInfo[i][0]; // simulation timestamp
                        float livoxTimeStamp = (simTime - std::floor(simTime)) * 0.1 ;// livox-type timestamp
                        float Azimuth = LivoxCsvInfo[i][1]; // Azimuth angle
                        float Height = LivoxCsvInfo[i][2] - 90; // Height angle
                        int LineIndex = LivoxCsvInfo[i][3]; // Line number
                        

                        const float VertAngle = Height;
                        const float HorizAngle = Azimuth;

                        const bool PreprocessResult = RayPreprocessCondition[idxChannel][RayCheck];
                        ++RayCheck;
                        

                        if (PreprocessResult && ShootLaser_livox(VertAngle, HorizAngle, HitResult, TraceParams, LineIndex, livoxTimeStamp)) 
                        { 
                        WritePointAsync(idxChannel, HitResult);
                        }
                        LivoxCount = i;
                    } });
  }
  GetWorld()->GetPhysicsScene()->GetPxScene()->unlockRead();

  FTransform ActorTransf = GetTransform();
  ComputeAndSaveDetections(ActorTransf);

  const float HorizontalAngle = carla::geom::Math::ToRadians(
      std::fmod(CurrentHorizontalAngle + AngleDistanceOfTick, Description.HorizontalFov));
  SemanticLidarData.SetHorizontalAngle(HorizontalAngle);
}

void ARayCastSemanticLidar::ResetRecordedHits(uint32_t Channels, uint32_t MaxPointsPerChannel)
{
  RecordedHits.resize(Channels);

  for (auto &hits : RecordedHits)
  {
    hits.clear();
    hits.reserve(MaxPointsPerChannel);
  }
}

void ARayCastSemanticLidar::PreprocessRays(uint32_t Channels, uint32_t MaxPointsPerChannel)
{
  RayPreprocessCondition.resize(Channels);

  for (auto &conds : RayPreprocessCondition)
  {
    conds.clear();
    conds.resize(MaxPointsPerChannel);
    std::fill(conds.begin(), conds.end(), true);
  }
}

void ARayCastSemanticLidar::WritePointAsync(uint32_t channel, FHitResult &detection)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  DEBUG_ASSERT(GetChannelCount() > channel);
  RecordedHits[channel].emplace_back(detection);
}

void ARayCastSemanticLidar::ComputeAndSaveDetections(const FTransform &SensorTransform)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
  for (auto idxChannel = 0u; idxChannel < Description.Channels; ++idxChannel)
    PointsPerChannel[idxChannel] = RecordedHits[idxChannel].size();
  SemanticLidarData.ResetMemory(PointsPerChannel);

  for (auto idxChannel = 0u; idxChannel < Description.Channels; ++idxChannel)
  {
    for (auto &hit : RecordedHits[idxChannel])
    {
      FSemanticDetection detection;
      ComputeRawDetection(hit, SensorTransform, detection);
      SemanticLidarData.WritePointSync(detection);
    }
  }

  SemanticLidarData.WriteChannelCount(PointsPerChannel);
}

void ARayCastSemanticLidar::ComputeRawDetection(const FHitResult &HitInfo, const FTransform &SensorTransf, FSemanticDetection &Detection) const
{
  const FVector HitPoint = HitInfo.ImpactPoint;
  Detection.point = SensorTransf.Inverse().TransformPosition(HitPoint);

  const FVector VecInc = -(HitPoint - SensorTransf.GetLocation()).GetSafeNormal();
  Detection.cos_inc_angle = FVector::DotProduct(VecInc, HitInfo.ImpactNormal);

  const FActorRegistry &Registry = GetEpisode().GetActorRegistry();

  const AActor *actor = HitInfo.Actor.Get();
  Detection.object_idx = 0;
  Detection.object_tag = static_cast<uint32_t>(HitInfo.Component->CustomDepthStencilValue);

  if (actor != nullptr)
  {

    const FCarlaActor *view = Registry.FindCarlaActor(actor);
    if (view)
      Detection.object_idx = view->GetActorId();
  }
  else
  {
    UE_LOG(LogCarla, Warning, TEXT("Actor not valid %p!!!!"), actor);
  }
}

bool ARayCastSemanticLidar::ShootLaser(const float VerticalAngle, const float HorizontalAngle, FHitResult &HitResult, FCollisionQueryParams &TraceParams) const
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

  FHitResult HitInfo(ForceInit);

  FTransform ActorTransf = GetTransform();
  FVector LidarBodyLoc = ActorTransf.GetLocation();
  FRotator LidarBodyRot = ActorTransf.Rotator();

  FRotator LaserRot(VerticalAngle, HorizontalAngle, 0); // float InPitch, float InYaw, float InRoll
  FRotator ResultRot = UKismetMathLibrary::ComposeRotators(
      LaserRot,
      LidarBodyRot);

  const auto Range = Description.Range;
  FVector EndTrace = Range * UKismetMathLibrary::GetForwardVector(ResultRot) + LidarBodyLoc;

  GetWorld()->ParallelLineTraceSingleByChannel(
      HitInfo,
      LidarBodyLoc,
      EndTrace,
      ECC_GameTraceChannel2,
      TraceParams,
      FCollisionResponseParams::DefaultResponseParam);
  if (HitInfo.bBlockingHit)
  {
    HitResult = HitInfo;
    return true;
  }
  else
  {
    return false;
  }
}

bool ARayCastSemanticLidar::ShootLaser_livox(const float VerticalAngle, const float HorizontalAngle, FHitResult &HitResult, FCollisionQueryParams &TraceParams, int LineIndex, float livoxTimeStamp) const
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

  FHitResult HitInfo(ForceInit);

  FTransform ActorTransf = GetTransform();
  FVector LidarBodyLoc = ActorTransf.GetLocation();
  FRotator LidarBodyRot = ActorTransf.Rotator();

  FRotator LaserRot(VerticalAngle, HorizontalAngle, 0); // float InPitch, float InYaw, float InRoll
  FRotator ResultRot = UKismetMathLibrary::ComposeRotators(
      LaserRot,
      LidarBodyRot);

  const auto Range = Description.Range;
  FVector EndTrace = Range * UKismetMathLibrary::GetForwardVector(ResultRot) + LidarBodyLoc;

  GetWorld()->ParallelLineTraceSingleByChannel(
      HitInfo,
      LidarBodyLoc,
      EndTrace,
      ECC_GameTraceChannel2,
      TraceParams,
      FCollisionResponseParams::DefaultResponseParam);

  // Modify the information in HitInfo to line number and livox-type timestamp
  HitInfo.ElementIndex = LineIndex; 
  HitInfo.Time = livoxTimeStamp;

  if (HitInfo.bBlockingHit)
  {
    HitResult = HitInfo;
    return true;
  }
  else
  {
    return false;
  }
}

// func of reading csv file
std::vector<std::vector<float>> ARayCastSemanticLidar::LivoxCsvReader() 
{
  /*  CSV file content
      Layer 1: contains all data segments in 4 seconds
      Layer 2: contains single scan data. 
      Position 0 is the scanning time stamp; 
      Position 1 is azimuth; 
      Position 2 is zenith angle (height angle = zenith angle - 90°); 
      Position 3 is the line number.
  */
  /*  lidar type
      0--Horizon
      1--Mid40 - Not working
      2--Avia - Not working
      3--Tele - Not working
  */

  float check_csv = Description.LidarType;
  std::string filename;
  std::vector<std::vector<float>> LivoxPointInfo;

  bool csv_checked = false;

  if (check_csv == 0.0)
  {
    UE_LOG(LogCarla, Display, TEXT("Using Livox Horizon. Configuration is part of the program instead of csv file."));
    csv_checked = true;
    std::istringstream  LivoxCsv(horizon_csv);

    std::string lineStr;
    while (getline(LivoxCsv, lineStr)) 
    {
      std::stringstream ss(lineStr);
      std::string str;
      std::vector<float> lineArray;
      
      while (getline(ss, str, ',')){
        lineArray.push_back(std::stof(str));
      }
      LivoxPointInfo.push_back(lineArray);
    }    
  }
  if (check_csv == 1.0)
  {
    filename = "/home/LivoxCsv/mid40.csv";
    UE_LOG(LogCarla, Warning, TEXT("Type Mid 40 is not intended for use with this package. Only supported type is horizon (lidar_type 0)"));
    csv_checked = true;
  }
  if (check_csv == 2.0)
  {
    filename = "/home/LivoxCsv/avia.csv";
    UE_LOG(LogCarla, Warning, TEXT("Type Avia is not intended for use with this package. Only supported type is horizon (lidar_type 0)"));
    csv_checked = true;
  }
  if (check_csv == 3.0)
  {
    filename = "/home/LivoxCsv/tele.csv";
    UE_LOG(LogCarla, Warning, TEXT("Type Tele is not intended for use with this package. Only supported type is horizon (lidar_type 0)"));
    csv_checked = true;
  }





  if (check_csv != 0.0){    

      std::ifstream LivoxCsv(filename, std::ios::in);
      std::string lineStr;
    while (getline(LivoxCsv, lineStr)) 
    {
      std::stringstream ss(lineStr);
      std::string str;
      std::vector<float> lineArray;
      
      while (getline(ss, str, ',')){
        lineArray.push_back(std::stof(str));
      }
      LivoxPointInfo.push_back(lineArray);
    }
  }

  if (!csv_checked){
    UE_LOG(LogCarla, Error, TEXT("No csv was read! Incorect lidar type. Supported type is Horizon (lidar_type 0)."));
  }
  return LivoxPointInfo; 
}