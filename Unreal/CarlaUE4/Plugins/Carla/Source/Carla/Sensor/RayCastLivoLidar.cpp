#include <PxScene.h>
#include <cmath>
#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Sensor/RayCastLivoxLidar.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/geom/Math.h"
#include <compiler/enable-ue4-macros.h>

#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"

#include <iostream>
#include <fstream>
#include <sstream>

namespace crp = carla::rpc;


FActorDefinition ARayCastLivoxLidar::GetSensorDefinition()
{
    return UActorBlueprintFunctionLibrary::MakeLidarDefinition(TEXT("ray_cast_livox"));
}

ARayCastLivoxLidar::ARayCastLivoxLidar(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    RandomEngine = CreateDefaultSubobject<URandomEngine>(TEXT("RandomEngine")); 
    SetSeed(Description.RandomSeed);
}


void ARayCastLivoxLidar::Set(const FActorDescription &ActorDescription)
{
    Super::Set(ActorDescription);
    FLidarDescription LidarDescription;
    UActorBlueprintFunctionLibrary::SetLidar(ActorDescription, LidarDescription);
    Set(LidarDescription);
}


void ARayCastLivoxLidar::Set(const FLidarDescription &LidarDescription)
{
    Description = LidarDescription;
    LivoxLidarData = FLivoxLidarData(Description.Channels); 
    CreateLasers_livox();

    PointsPerChannel.resize(Description.Channels);

    // Compute drop off model parameters
    DropOffBeta = 1.0f - Description.DropOffAtZeroIntensity;
    DropOffAlpha = Description.DropOffAtZeroIntensity / Description.DropOffIntensityLimit;
    DropOffGenActive = Description.DropOffGenRate > std::numeric_limits<float>::epsilon();
}

void ARayCastLivoxLidar::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ARayCastLivoxLidar::PostPhysTick);
    SimulateLidar_livox(DeltaTime); 

    {
        TRACE_CPUPROFILER_EVENT_SCOPE_STR("Send Stream");
        auto DataStream = GetDataStream(*this);
        auto data = DataStream.PopBufferFromPool();
        DataStream.Send(*this, LivoxLidarData, data);


    }
}

float ARayCastLivoxLidar::ComputeIntensity(const FSemanticDetection &RawDetection) const 
{
    const carla::geom::Location HitPoint = RawDetection.point; 
    const float Distance = HitPoint.Length();                  

    const float AttenAtm = Description.AtmospAttenRate; 
    const float AbsAtm = exp(-AttenAtm * Distance);    

    const float IntRec = AbsAtm; 
    return IntRec;
}

ARayCastLivoxLidar::FLivoxDetection ARayCastLivoxLidar::ComputeDetection(const FHitResult &HitInfo, const FTransform &SensorTransf) const 
{
    FLivoxDetection Detection;
    const FVector HitPoint = HitInfo.ImpactPoint;
    Detection.point = SensorTransf.Inverse().TransformPosition(HitPoint);

    const float Distance = Detection.point.Length();

    const float AttenAtm = Description.AtmospAttenRate;
    const float AbsAtm = exp(-AttenAtm * Distance);

    // const float IntRec = AbsAtm;

    // intensity of livox type，integer is the line number and decimal is the timestamp
    const float intensity_lineIndex = HitInfo.ElementIndex; 
    const float intensity_timestamp = HitInfo.Time;

    Detection.intensity = intensity_lineIndex + intensity_timestamp;

    return Detection;
}

void ARayCastLivoxLidar::PreprocessRays(uint32_t Channels, uint32_t MaxPointsPerChannel)
{
    Super::PreprocessRays(Channels, MaxPointsPerChannel);

    for (auto ch = 0u; ch < Channels; ch++)
    {
        for (auto p = 0u; p < MaxPointsPerChannel; p++)
        {
            RayPreprocessCondition[ch][p] = !(DropOffGenActive && RandomEngine->GetUniformFloat() < Description.DropOffGenRate);
        }
    }
}

bool ARayCastLivoxLidar::PostprocessDetection(FLivoxDetection &Detection) const
{
    if (Description.NoiseStdDev > std::numeric_limits<float>::epsilon())
    {
        const auto ForwardVector = Detection.point.MakeUnitVector();
        const auto Noise = ForwardVector * RandomEngine->GetNormalDistribution(0.0f, Description.NoiseStdDev);
        Detection.point += Noise;
    }

    const float Intensity = Detection.intensity;
    if (Intensity > Description.DropOffIntensityLimit)
        return true;
    else
        return RandomEngine->GetUniformFloat() < DropOffAlpha * Intensity + DropOffBeta; 
}


void ARayCastLivoxLidar::ComputeAndSaveDetections(const FTransform &SensorTransform)
{
    TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
    for (auto idxChannel = 0u; idxChannel < Description.Channels; ++idxChannel)
        PointsPerChannel[idxChannel] = RecordedHits[idxChannel].size();
    LivoxLidarData.ResetMemory(PointsPerChannel);

    for (auto idxChannel = 0u; idxChannel < Description.Channels; ++idxChannel)
    {
        for (auto &hit : RecordedHits[idxChannel])
        {
            FLivoxDetection Detection = ComputeDetection(hit, SensorTransform);
            if (PostprocessDetection(Detection)) 
                LivoxLidarData.WritePointSync(Detection);
            else
                PointsPerChannel[idxChannel]--;
        }
    }

    LivoxLidarData.WriteChannelCount(PointsPerChannel);
}